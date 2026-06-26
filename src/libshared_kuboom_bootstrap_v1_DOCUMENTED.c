/*
 * libshared_kuboom_bootstrap_v1.c
 * ================================================================
 * Objetivo
 * --------
 * Biblioteca nativa JNI de BOOTSTRAP para loader/casca Android no
 * laboratório CTF KUBOOM Unity/IL2CPP.
 *
 * Esta lib NÃO contém menu, hook, patch de memória, cheat, bypass de anti-cheat
 * ou lógica de gameplay. O papel dela é apenas preparar o ambiente Java do
 * processo do loader para que classes/resources/libs do APK alvo instalado sejam
 * resolvidos por um PathClassLoader apontando para o alvo.
 *
 * Alvo analisado:
 *   package:          com.Nobodyshot.kuboom
 *   application:      com.safedk.android.SafeDKApplication
 *   launcher activity:com.unity3d.player.UnityPlayerActivity
 *   ABI:              arm64-v8a
 *   engine:           Unity/IL2CPP
 *
 * Fluxo esperado:
 *   Application do loader sobe
 *   -> System.loadLibrary("shared")
 *   -> JNI_OnLoad registra nativeInit(...) se a classe declarar native
 *   -> attachBaseContext(context) chama nativeInit(context)
 *   -> bootstrap troca LoadedApk.mClassLoader e caminhos do target, preservando a identidade/UID do loader
 *   -> loadClass(com.unity3d.player.UnityPlayerActivity) deve validar
 *
 * Escopo:
 *   Uso apenas em ambiente próprio, laboratório ou CTF autorizado.
 */

#include <jni.h>
#include <android/log.h>
#include <stdlib.h>
#include <string.h>

#define TAG "KUBOOM_BOOT_V1_6"
#define TARGET_PACKAGE "com.Nobodyshot.kuboom"
#define TARGET_ACTIVITY "com.unity3d.player.UnityPlayerActivity"

/*
 * Classes onde a Application/entrada do shell pode declarar nativeInit(...).
 * PRIMARY é a Application real do APK KUBOOM analisado.
 * SHELL é opcional, caso você prefira criar uma Application própria no loader.
 */
#define REG_CLASS_PRIMARY "com/safedk/android/SafeDKApplication"
#define REG_CLASS_SHELL   "com/ctf/kuboom/loader/KuboomShellApplication"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

static JavaVM *g_vm = 0;

/*
 * clear_exc()
 * -----------
 * JNI deixa exceptions penduradas no JNIEnv. Se uma exception fica pendurada,
 * chamadas JNI seguintes podem falhar de modo confuso. Esta helper loga, descreve
 * no logcat e limpa a exception para permitir tentativa best-effort.
 */
static int clear_exc(JNIEnv *env, const char *where) {
    if ((*env)->ExceptionCheck(env)) {
        LOGE("EXCEPTION at %s", where);
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return 1;
    }
    return 0;
}

/* Converte jstring -> char* heap-owned. Quem chama deve free(). */
static char *jstrdup(JNIEnv *env, jstring s) {
    if (!s) return NULL;
    const char *u = (*env)->GetStringUTFChars(env, s, 0);
    if (!u) return NULL;
    char *out = strdup(u);
    (*env)->ReleaseStringUTFChars(env, s, u);
    return out;
}

static jstring new_string(JNIEnv *env, const char *s) {
    if (!s) return NULL;
    return (*env)->NewStringUTF(env, s);
}

static jobject call_obj0(JNIEnv *env, jobject obj, const char *name, const char *sig) {
    if (!obj) return NULL;
    jclass cls = (*env)->GetObjectClass(env, obj);
    if (!cls) return NULL;
    jmethodID mid = (*env)->GetMethodID(env, cls, name, sig);
    if (!mid) { clear_exc(env, name); return NULL; }
    jobject r = (*env)->CallObjectMethod(env, obj, mid);
    clear_exc(env, name);
    return r;
}

static jobject call_obj1_obj(JNIEnv *env, jobject obj, const char *name, const char *sig, jobject a) {
    if (!obj) return NULL;
    jclass cls = (*env)->GetObjectClass(env, obj);
    if (!cls) return NULL;
    jmethodID mid = (*env)->GetMethodID(env, cls, name, sig);
    if (!mid) { clear_exc(env, name); return NULL; }
    jobject r = (*env)->CallObjectMethod(env, obj, mid, a);
    clear_exc(env, name);
    return r;
}

static jobject call_obj2_obj_int(JNIEnv *env, jobject obj, const char *name, const char *sig, jobject a, jint b) {
    if (!obj) return NULL;
    jclass cls = (*env)->GetObjectClass(env, obj);
    if (!cls) return NULL;
    jmethodID mid = (*env)->GetMethodID(env, cls, name, sig);
    if (!mid) { clear_exc(env, name); return NULL; }
    jobject r = (*env)->CallObjectMethod(env, obj, mid, a, b);
    clear_exc(env, name);
    return r;
}

static jobject get_public_field_obj(JNIEnv *env, jobject obj, const char *name, const char *sig) {
    if (!obj) return NULL;
    jclass cls = (*env)->GetObjectClass(env, obj);
    if (!cls) return NULL;
    jfieldID fid = (*env)->GetFieldID(env, cls, name, sig);
    if (!fid) { clear_exc(env, name); return NULL; }
    jobject r = (*env)->GetObjectField(env, obj, fid);
    clear_exc(env, name);
    return r;
}

static int set_public_field_obj(JNIEnv *env, jobject obj, const char *name, const char *sig, jobject val) {
    if (!obj) return 0;
    jclass cls = (*env)->GetObjectClass(env, obj);
    if (!cls) return 0;
    jfieldID fid = (*env)->GetFieldID(env, cls, name, sig);
    if (!fid) { clear_exc(env, name); return 0; }
    (*env)->SetObjectField(env, obj, fid, val);
    return clear_exc(env, name) ? 0 : 1;
}

/*
 * reflect_get_field()/reflect_set_field()
 * ---------------------------------------
 * A parte crítica do bootstrap mexe em campos internos de ActivityThread/LoadedApk.
 * Esses campos são privados, então usamos reflexão Java por JNI.
 * Campos usados atualmente:
 *   ActivityThread.mPackages
 *   LoadedApk.mApplicationInfo
 *   LoadedApk.mClassLoader
 *   ContextImpl.mPackageInfo
 */
static jobject reflect_get_field(JNIEnv *env, jobject obj, const char *field_name) {
    if (!obj) return NULL;

    jclass objCls = (*env)->GetObjectClass(env, obj);
    if (!objCls) return NULL;

    jclass clsClass = (*env)->FindClass(env, "java/lang/Class");
    jclass fieldClass = (*env)->FindClass(env, "java/lang/reflect/Field");
    if (!clsClass || !fieldClass) return NULL;

    jmethodID getDeclaredField = (*env)->GetMethodID(env, clsClass, "getDeclaredField", "(Ljava/lang/String;)Ljava/lang/reflect/Field;");
    jmethodID setAccessible = (*env)->GetMethodID(env, fieldClass, "setAccessible", "(Z)V");
    jmethodID get = (*env)->GetMethodID(env, fieldClass, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");
    if (!getDeclaredField || !setAccessible || !get) return NULL;

    jstring fname = (*env)->NewStringUTF(env, field_name);
    jobject field = (*env)->CallObjectMethod(env, objCls, getDeclaredField, fname);
    if (clear_exc(env, "reflect_get_field:getDeclaredField")) return NULL;
    (*env)->CallVoidMethod(env, field, setAccessible, JNI_TRUE);
    if (clear_exc(env, "reflect_get_field:setAccessible")) return NULL;
    jobject val = (*env)->CallObjectMethod(env, field, get, obj);
    if (clear_exc(env, "reflect_get_field:get")) return NULL;
    return val;
}

static int reflect_set_field(JNIEnv *env, jobject obj, const char *field_name, jobject value) {
    if (!obj) return 0;

    jclass objCls = (*env)->GetObjectClass(env, obj);
    if (!objCls) return 0;

    jclass clsClass = (*env)->FindClass(env, "java/lang/Class");
    jclass fieldClass = (*env)->FindClass(env, "java/lang/reflect/Field");
    if (!clsClass || !fieldClass) return 0;

    jmethodID getDeclaredField = (*env)->GetMethodID(env, clsClass, "getDeclaredField", "(Ljava/lang/String;)Ljava/lang/reflect/Field;");
    jmethodID setAccessible = (*env)->GetMethodID(env, fieldClass, "setAccessible", "(Z)V");
    jmethodID set = (*env)->GetMethodID(env, fieldClass, "set", "(Ljava/lang/Object;Ljava/lang/Object;)V");
    if (!getDeclaredField || !setAccessible || !set) return 0;

    jstring fname = (*env)->NewStringUTF(env, field_name);
    jobject field = (*env)->CallObjectMethod(env, objCls, getDeclaredField, fname);
    if (clear_exc(env, "reflect_set_field:getDeclaredField")) return 0;
    (*env)->CallVoidMethod(env, field, setAccessible, JNI_TRUE);
    if (clear_exc(env, "reflect_set_field:setAccessible")) return 0;
    (*env)->CallVoidMethod(env, field, set, obj, value);
    if (clear_exc(env, "reflect_set_field:set")) return 0;
    return 1;
}

/* Retorna ActivityThread.currentApplication(), usado apenas como fallback. */
static jobject current_application(JNIEnv *env) {
    jclass at = (*env)->FindClass(env, "android/app/ActivityThread");
    if (!at) { clear_exc(env, "FindClass ActivityThread"); return NULL; }
    jmethodID curApp = (*env)->GetStaticMethodID(env, at, "currentApplication", "()Landroid/app/Application;");
    if (!curApp) { clear_exc(env, "currentApplication mid"); return NULL; }
    jobject app = (*env)->CallStaticObjectMethod(env, at, curApp);
    clear_exc(env, "currentApplication call");
    return app;
}

/* Retorna ActivityThread.currentActivityThread(), necessário para chegar no LoadedApk. */
static jobject current_activity_thread(JNIEnv *env) {
    jclass at = (*env)->FindClass(env, "android/app/ActivityThread");
    if (!at) { clear_exc(env, "FindClass ActivityThread"); return NULL; }
    jmethodID cur = (*env)->GetStaticMethodID(env, at, "currentActivityThread", "()Landroid/app/ActivityThread;");
    if (!cur) { clear_exc(env, "currentActivityThread mid"); return NULL; }
    jobject r = (*env)->CallStaticObjectMethod(env, at, cur);
    clear_exc(env, "currentActivityThread call");
    return r;
}

/*
 * build_dex_path()
 * ----------------
 * PathClassLoader espera uma string com todos os APKs separados por dois-pontos:
 *   /data/app/.../base.apk:/data/app/.../split_config.arm64_v8a.apk:...
 * sourceDir vira o primeiro item; splitSourceDirs entram na sequência.
 */
static char *build_dex_path(JNIEnv *env, jstring sourceDir, jobjectArray splits) {
    char *base = jstrdup(env, sourceDir);
    if (!base) return NULL;

    size_t total = strlen(base) + 1;
    jsize count = splits ? (*env)->GetArrayLength(env, splits) : 0;
    for (jsize i = 0; i < count; i++) {
        jstring s = (jstring)(*env)->GetObjectArrayElement(env, splits, i);
        char *cs = jstrdup(env, s);
        if (cs) { total += 1 + strlen(cs); free(cs); }
    }

    char *out = (char *)calloc(total + 1, 1);
    strcpy(out, base);
    free(base);

    for (jsize i = 0; i < count; i++) {
        jstring s = (jstring)(*env)->GetObjectArrayElement(env, splits, i);
        char *cs = jstrdup(env, s);
        if (cs) {
            strcat(out, ":");
            strcat(out, cs);
            free(cs);
        }
    }
    return out;
}

/*
 * get_loaded_apk()
 * ----------------
 * ActivityThread.mPackages é um mapa packageName -> WeakReference<LoadedApk>.
 * O LoadedApk do LOADER é o objeto que o Android usa para resolver classes,
 * resources e caminho nativo daquele pacote. Patchar mClassLoader aqui é o centro
 * do modelo DPFF.
 */
static jobject get_loaded_apk(JNIEnv *env, jstring loaderPkg) {
    jobject at = current_activity_thread(env);
    if (!at) return NULL;

    jobject mPackages = reflect_get_field(env, at, "mPackages");
    if (!mPackages) { LOGE("mPackages not found"); return NULL; }

    // android.util.ArrayMap has get(Object). java.util.Map also has get(Object).
    jobject weakRef = call_obj1_obj(env, mPackages, "get", "(Ljava/lang/Object;)Ljava/lang/Object;", loaderPkg);
    if (!weakRef) { LOGE("LoadedApk WeakReference not found for loader package"); return NULL; }

    jobject loadedApk = call_obj0(env, weakRef, "get", "()Ljava/lang/Object;");
    if (!loadedApk) { LOGE("LoadedApk weak ref is null"); return NULL; }

    return loadedApk;
}

/*
 * patch_application_info()
 * ------------------------
 * Ajusta ApplicationInfo para fingir que o pacote atual tem base/splits/libs do alvo.
 * Isso ajuda Resources, AssetManager e NativeLoader a enxergarem caminhos do target.
 * Não garante namespace nativo perfeito em todos Androids.
 */
static int patch_application_info(JNIEnv *env, jobject appInfo, jstring targetPkg, jstring src, jobjectArray splits, jstring nativeDir) {
    if (!appInfo) return 0;
    int ok = 1;
    /*
     * v1.6 SHELL-RESOURCES MODE:
     * - NÃO altera ApplicationInfo.packageName. O UID real continua sendo o loader.
     * - NÃO redireciona publicSourceDir/splitPublicSourceDirs para o alvo.
     *
     * Motivo: UnityPlayerActivity faz lookup de string usando o package atual
     * do Context. Em casca mínima esse package é com.Nobodyshot.loader. Se os
     * Resources forem redirecionados para o resources.arsc do alvo
     * com.Nobodyshot.kuboom, getIdentifier(..., com.Nobodyshot.loader) retorna
     * 0 e gera Resources$NotFoundException: String resource ID #0x0.
     *
     * Então nesta versão deixamos os Java resources no loader e redirecionamos
     * somente code/native paths para o alvo.
     */
    (void)targetPkg;
    ok &= set_public_field_obj(env, appInfo, "sourceDir", "Ljava/lang/String;", src);
    ok &= set_public_field_obj(env, appInfo, "splitSourceDirs", "[Ljava/lang/String;", splits);
    ok &= set_public_field_obj(env, appInfo, "nativeLibraryDir", "Ljava/lang/String;", nativeDir);
    LOGI("shell resources mode: keeping loader publicSourceDir/splitPublicSourceDirs");
    return ok;
}


/*
 * patch_loaded_apk_raw_paths()
 * ----------------------------
 * Unity não usa somente ApplicationInfo. Em várias versões, UnityPlayer consulta
 * Context.getPackageCodePath(), AssetManager/Resources ou caminhos internos do
 * LoadedApk. Se mAppDir/mResDir continuam apontando para o APK shell, a Activity
 * Unity sobe mas acusa falta de resources, por exemplo:
 *   "Not enough storage space to install required resources."
 *
 * Esta rotina replica os caminhos do target também nos campos privados do
 * LoadedApk do loader. É best-effort: campos ausentes em determinada versão do
 * Android são ignorados com exception limpa.
 */
static void patch_loaded_apk_raw_paths(JNIEnv *env, jobject loadedApk, jstring src, jobjectArray splits, jstring nativeDir) {
    if (!loadedApk) return;

    if (reflect_set_field(env, loadedApk, "mAppDir", src)) {
        LOGI("patched LoadedApk.mAppDir");
    }
    /* v1.6: do NOT patch mResDir/mSplitResDirs. Keep Java resources from loader. */
    LOGI("shell resources mode: keeping LoadedApk.mResDir/mSplitResDirs from loader");
    if (reflect_set_field(env, loadedApk, "mSplitAppDirs", splits)) {
        LOGI("patched LoadedApk.mSplitAppDirs");
    }
    if (reflect_set_field(env, loadedApk, "mLibDir", nativeDir)) {
        LOGI("patched LoadedApk.mLibDir");
    }
}

/*
 * patch_package_identity()
 * ------------------------
 * v1.6: desativado por segurança/compatibilidade.
 *
 * A v1.4 tentou trocar LoadedApk.mPackageName / ContextImpl.mBasePackageName /
 * ContextImpl.mOpPackageName para o pacote alvo. Em Android moderno isso dispara
 * validação de UID em chamadas do framework:
 *   SecurityException: Package com.Nobodyshot.kuboom does not belong to <uid>
 *
 * Para loader com package próprio, a identidade deve continuar sendo a do loader.
 * Em casca mínima, Java resources continuam no loader; code/native paths
 * apontam para o alvo. Assets Unity dependem de mAppDir/sourceDir.
 */
static void patch_package_identity(JNIEnv *env, jobject ctx, jobject loadedApk, jstring targetPkg) {
    (void)env;
    (void)ctx;
    (void)loadedApk;
    (void)targetPkg;
    LOGI("package identity patch disabled; preserving loader UID/package");
}

/*
 * create_path_classloader()
 * -------------------------
 * Cria PathClassLoader apontando para os APKs do alvo e lib dir do alvo.
 * Parent = parent do classloader antigo, para não prender resolução ao loader antigo.
 */
static jobject create_path_classloader(JNIEnv *env, const char *dexPath, jstring nativeDir, jobject oldLoader) {
    jobject parent = NULL;
    if (oldLoader) parent = call_obj0(env, oldLoader, "getParent", "()Ljava/lang/ClassLoader;");

    jclass pcl = (*env)->FindClass(env, "dalvik/system/PathClassLoader");
    if (!pcl) { clear_exc(env, "FindClass PathClassLoader"); return NULL; }

    jmethodID init = (*env)->GetMethodID(env, pcl, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V");
    if (!init) { clear_exc(env, "PathClassLoader.<init>"); return NULL; }

    jstring jdex = new_string(env, dexPath);
    jobject cl = (*env)->NewObject(env, pcl, init, jdex, nativeDir, parent);
    if (clear_exc(env, "New PathClassLoader")) return NULL;
    return cl;
}

/*
 * verify_activity()
 * -----------------
 * Validação diagnóstica. Se isto imprimir VERIFY OK, a parte Java/classloader
 * conseguiu resolver a Activity principal do target. Se depois cair, olhe libs,
 * Hilt, resources ou namespace nativo.
 */
static int verify_activity(JNIEnv *env, jobject cl) {
    if (!cl) return 0;
    jstring name = new_string(env, TARGET_ACTIVITY);
    jobject klass = call_obj1_obj(env, cl, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;", name);
    if (klass) {
        LOGI("VERIFY OK loadClass(%s)", TARGET_ACTIVITY);
        return 1;
    }
    LOGE("VERIFY FAIL loadClass(%s)", TARGET_ACTIVITY);
    return 0;
}

/*
 * bootstrap()
 * -----------
 * Função principal. Todo o fluxo descrito no cabeçalho acontece aqui.
 * Idealmente chamada cedo em Application.attachBaseContext(context), antes de
 * providers/activities do manifest serem instanciados.
 */
static int bootstrap(JNIEnv *env, jobject ctx) {
    LOGI("bootstrap ENTER");

    if (!ctx) {
        LOGI("context arg null, trying ActivityThread.currentApplication()");
        ctx = current_application(env);
    }
    if (!ctx) {
        LOGE("no Context available yet; nativeInit must be called with Context before Activity launch");
        return 0;
    }

    jstring loaderPkg = (jstring)call_obj0(env, ctx, "getPackageName", "()Ljava/lang/String;");
    char *loaderPkgC = jstrdup(env, loaderPkg);
    LOGI("loader package=%s target=%s", loaderPkgC ? loaderPkgC : "<null>", TARGET_PACKAGE);

    jobject pm = call_obj0(env, ctx, "getPackageManager", "()Landroid/content/pm/PackageManager;");
    if (!pm) { LOGE("getPackageManager failed"); if(loaderPkgC) free(loaderPkgC); return 0; }

    jstring targetPkg = new_string(env, TARGET_PACKAGE);
    jobject targetInfo = call_obj2_obj_int(env, pm, "getApplicationInfo", "(Ljava/lang/String;I)Landroid/content/pm/ApplicationInfo;", targetPkg, 0);
    if (!targetInfo) { LOGE("getApplicationInfo(%s) failed", TARGET_PACKAGE); if(loaderPkgC) free(loaderPkgC); return 0; }

    jstring src = (jstring)get_public_field_obj(env, targetInfo, "sourceDir", "Ljava/lang/String;");
    jobjectArray splits = (jobjectArray)get_public_field_obj(env, targetInfo, "splitSourceDirs", "[Ljava/lang/String;");
    jstring nativeDir = (jstring)get_public_field_obj(env, targetInfo, "nativeLibraryDir", "Ljava/lang/String;");

    char *srcC = jstrdup(env, src);
    char *nativeC = jstrdup(env, nativeDir);
    char *dexPath = build_dex_path(env, src, splits);

    LOGI("target sourceDir=%s", srcC ? srcC : "<null>");
    LOGI("target nativeLibraryDir=%s", nativeC ? nativeC : "<null>");
    LOGI("target dexPath=%s", dexPath ? dexPath : "<null>");

    if (!dexPath || !nativeDir) {
        LOGE("missing target paths");
        if(loaderPkgC) free(loaderPkgC); if(srcC) free(srcC); if(nativeC) free(nativeC); if(dexPath) free(dexPath);
        return 0;
    }

    jobject oldLoader = call_obj0(env, ctx, "getClassLoader", "()Ljava/lang/ClassLoader;");
    jobject newLoader = create_path_classloader(env, dexPath, nativeDir, oldLoader);
    if (!newLoader) {
        LOGE("create PathClassLoader failed");
        if(loaderPkgC) free(loaderPkgC); if(srcC) free(srcC); if(nativeC) free(nativeC); free(dexPath);
        return 0;
    }

    jobject loadedApk = get_loaded_apk(env, loaderPkg);
    if (!loadedApk) {
        LOGE("LoadedApk not found");
        if(loaderPkgC) free(loaderPkgC); if(srcC) free(srcC); if(nativeC) free(nativeC); free(dexPath);
        return 0;
    }

    jobject loaderAppInfo = reflect_get_field(env, loadedApk, "mApplicationInfo");
    if (loaderAppInfo) {
        patch_application_info(env, loaderAppInfo, targetPkg, src, splits, nativeDir);
        LOGI("patched LoadedApk.mApplicationInfo paths");
    } else {
        LOGE("LoadedApk.mApplicationInfo not available");
    }

    patch_loaded_apk_raw_paths(env, loadedApk, src, splits, nativeDir);
    patch_package_identity(env, ctx, loadedApk, targetPkg);

    if (reflect_set_field(env, loadedApk, "mClassLoader", newLoader)) {
        LOGI("patched LoadedApk.mClassLoader -> target PathClassLoader");
    } else {
        LOGE("failed to patch LoadedApk.mClassLoader");
    }

    // Also patch ContextImpl.mPackageInfo's app info if this context exposes it.
    jobject pkgInfo = reflect_get_field(env, ctx, "mPackageInfo");
    if (pkgInfo) {
        jobject ctxAppInfo = reflect_get_field(env, pkgInfo, "mApplicationInfo");
        if (ctxAppInfo) patch_application_info(env, ctxAppInfo, targetPkg, src, splits, nativeDir);
        patch_loaded_apk_raw_paths(env, pkgInfo, src, splits, nativeDir);
        /* v1.6: do not alter pkgInfo.mPackageName; keep loader package bound to its UID. */
        reflect_set_field(env, pkgInfo, "mClassLoader", newLoader);
        LOGI("patched ContextImpl.mPackageInfo best-effort");
    }

    verify_activity(env, newLoader);

    if(loaderPkgC) free(loaderPkgC);
    if(srcC) free(srcC);
    if(nativeC) free(nativeC);
    if(dexPath) free(dexPath);

    LOGI("bootstrap LEAVE");
    return 1;
}

static void nativeInit_ctx_int_obj(JNIEnv *env, jobject thiz, jobject ctx, jint mode, jobject obj) {
    (void)thiz; (void)mode; (void)obj;
    LOGI("nativeInit(Context,int,Object) ENTER");
    bootstrap(env, ctx);
    LOGI("nativeInit(Context,int,Object) LEAVE");
}

static void nativeInit_ctx_int(JNIEnv *env, jobject thiz, jobject ctx, jint mode) {
    (void)thiz; (void)mode;
    LOGI("nativeInit(Context,int) ENTER");
    bootstrap(env, ctx);
    LOGI("nativeInit(Context,int) LEAVE");
}

static void nativeInit_ctx(JNIEnv *env, jobject thiz, jobject ctx) {
    (void)thiz;
    LOGI("nativeInit(Context) ENTER");
    bootstrap(env, ctx);
    LOGI("nativeInit(Context) LEAVE");
}

static void nativeInit_void(JNIEnv *env, jobject thiz) {
    (void)thiz;
    LOGI("nativeInit() ENTER");
    bootstrap(env, NULL);
    LOGI("nativeInit() LEAVE");
}

/*
 * register_methods()
 * ------------------
 * Registra várias assinaturas possíveis de nativeInit.
 * Importante: registra UMA por vez. Se registrar todas juntas e alguma assinatura
 * não existir no smali, RegisterNatives lança NoSuchMethodError e System.loadLibrary
 * falha. Este foi exatamente o erro corrigido na v2 do DPFF.
 */
static int register_methods_for_class(JNIEnv *env, const char *slash_class_name) {
    jclass cls = (*env)->FindClass(env, slash_class_name);
    if (!cls) {
        clear_exc(env, "FindClass register candidate");
        LOGE("RegisterNatives class not found: %s", slash_class_name);
        return 0;
    }

    JNINativeMethod candidates[] = {
        {"nativeInit", "(Landroid/content/Context;ILjava/lang/Object;)V", (void*)nativeInit_ctx_int_obj},
        {"nativeInit", "(Landroid/content/Context;I)V", (void*)nativeInit_ctx_int},
        {"nativeInit", "(Landroid/content/Context;)V", (void*)nativeInit_ctx},
        {"nativeInit", "()V", (void*)nativeInit_void},
    };

    int ok = 0;
    int total = (int)(sizeof(candidates) / sizeof(candidates[0]));
    for (int i = 0; i < total; i++) {
        JNINativeMethod one = candidates[i];
        jint rc = (*env)->RegisterNatives(env, cls, &one, 1);
        if (rc == 0 && !(*env)->ExceptionCheck(env)) {
            LOGI("RegisterNatives OK class=%s nativeInit%s", slash_class_name, one.signature);
            ok++;
        } else {
            LOGE("RegisterNatives skip class=%s nativeInit%s rc=%d", slash_class_name, one.signature, rc);
            clear_exc(env, "RegisterNatives single candidate");
        }
    }

    if (ok == 0) {
        LOGE("RegisterNatives no nativeInit overload matched for class=%s", slash_class_name);
    }
    return ok;
}

/*
 * register_methods()
 * ------------------
 * Registra nativeInit(...) de forma flexível.
 *
 * Para KUBOOM há duas estratégias comuns de shell:
 *   A) patchar com/safedk/android/SafeDKApplication no APK-base do loader;
 *   B) declarar uma Application própria com/ctf/kuboom/loader/KuboomShellApplication.
 *
 * A função tenta ambas. Falhas são limpas e não derrubam System.loadLibrary.
 */
static int register_methods(JNIEnv *env) {
    int ok = 0;
    ok += register_methods_for_class(env, REG_CLASS_PRIMARY);
    ok += register_methods_for_class(env, REG_CLASS_SHELL);

    if (ok == 0) {
        LOGE("RegisterNatives no class/signature matched; exported JNI fallbacks may still resolve");
    } else {
        LOGI("RegisterNatives total matched count=%d", ok);
    }
    return ok;
}

/*
 * JNI_OnLoad()
 * ------------
 * Chamado automaticamente por System.loadLibrary("shared").
 * Deve retornar JNI_VERSION_1_6. Aqui apenas registra métodos e loga.
 */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    (void)reserved;
    g_vm = vm;
    JNIEnv *env = NULL;
    if ((*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_6) != JNI_OK || !env) {
        return JNI_ERR;
    }
    LOGI("JNI_OnLoad ENTER");
    register_methods(env);
    LOGI("JNI_OnLoad LEAVE");
    return JNI_VERSION_1_6;
}

/*
 * Exports JNI de fallback.
 * Mesmo se RegisterNatives não encontrar a assinatura, o linker JNI ainda pode
 * resolver chamadas pelo nome Java_com_... quando o smali declara nativeInit.
 */
/* Fallback exports para SafeDKApplication do APK KUBOOM. */
JNIEXPORT void JNICALL Java_com_safedk_android_SafeDKApplication_nativeInit(JNIEnv *env, jobject thiz) {
    nativeInit_void(env, thiz);
}

JNIEXPORT void JNICALL Java_com_safedk_android_SafeDKApplication_nativeInit__Landroid_content_Context_2(JNIEnv *env, jobject thiz, jobject ctx) {
    nativeInit_ctx(env, thiz, ctx);
}

JNIEXPORT void JNICALL Java_com_safedk_android_SafeDKApplication_nativeInit__Landroid_content_Context_2I(JNIEnv *env, jobject thiz, jobject ctx, jint mode) {
    nativeInit_ctx_int(env, thiz, ctx, mode);
}

JNIEXPORT void JNICALL Java_com_safedk_android_SafeDKApplication_nativeInit__Landroid_content_Context_2ILjava_lang_Object_2(JNIEnv *env, jobject thiz, jobject ctx, jint mode, jobject obj) {
    nativeInit_ctx_int_obj(env, thiz, ctx, mode, obj);
}

/* Fallback exports para Application shell opcional: com.ctf.kuboom.loader.KuboomShellApplication. */
JNIEXPORT void JNICALL Java_com_ctf_kuboom_loader_KuboomShellApplication_nativeInit(JNIEnv *env, jobject thiz) {
    nativeInit_void(env, thiz);
}

JNIEXPORT void JNICALL Java_com_ctf_kuboom_loader_KuboomShellApplication_nativeInit__Landroid_content_Context_2(JNIEnv *env, jobject thiz, jobject ctx) {
    nativeInit_ctx(env, thiz, ctx);
}

JNIEXPORT void JNICALL Java_com_ctf_kuboom_loader_KuboomShellApplication_nativeInit__Landroid_content_Context_2I(JNIEnv *env, jobject thiz, jobject ctx, jint mode) {
    nativeInit_ctx_int(env, thiz, ctx, mode);
}

JNIEXPORT void JNICALL Java_com_ctf_kuboom_loader_KuboomShellApplication_nativeInit__Landroid_content_Context_2ILjava_lang_Object_2(JNIEnv *env, jobject thiz, jobject ctx, jint mode, jobject obj) {
    nativeInit_ctx_int_obj(env, thiz, ctx, mode, obj);
}
