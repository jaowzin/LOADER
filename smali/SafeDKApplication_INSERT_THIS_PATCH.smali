# SafeDKApplication_INSERT_THIS_PATCH.smali
# ============================================================
# Arquivo alvo dentro do APK decompilado:
#   smali_classes*/com/safedk/android/SafeDKApplication.smali
#
# Use estes blocos dentro da classe SafeDKApplication.
# Não cole este arquivo inteiro como uma nova classe separada.
#
# Objetivo:
#   1) carregar lib/arm64-v8a/libshared.so cedo;
#   2) declarar nativeInit(Context);
#   3) chamar nativeInit(context) em attachBaseContext.


# ------------------------------------------------------------
# BLOCO 1 — declaração native
# Cole dentro da classe, fora de qualquer método.
# ------------------------------------------------------------
.method private static native nativeInit(Landroid/content/Context;)V
.end method


# ------------------------------------------------------------
# BLOCO 2A — se SafeDKApplication NÃO tiver <clinit>, adicione este método.
# ------------------------------------------------------------
.method static constructor <clinit>()V
    .locals 1

    const-string v0, "shared"
    invoke-static {v0}, Ljava/lang/System;->loadLibrary(Ljava/lang/String;)V

    return-void
.end method


# ------------------------------------------------------------
# BLOCO 2B — se SafeDKApplication JÁ tiver <clinit>, NÃO crie outro.
# Cole só estas duas linhas no começo do <clinit> existente.
# Se o método existente tiver .locals 0, mude para .locals 1.
# ------------------------------------------------------------
#    const-string v0, "shared"
#    invoke-static {v0}, Ljava/lang/System;->loadLibrary(Ljava/lang/String;)V


# ------------------------------------------------------------
# BLOCO 3A — se SafeDKApplication NÃO tiver attachBaseContext, adicione este método.
# ------------------------------------------------------------
.method protected attachBaseContext(Landroid/content/Context;)V
    .locals 0

    invoke-super {p0, p1}, Landroid/app/Application;->attachBaseContext(Landroid/content/Context;)V

    invoke-static {p1}, Lcom/safedk/android/SafeDKApplication;->nativeInit(Landroid/content/Context;)V

    return-void
.end method


# ------------------------------------------------------------
# BLOCO 3B — se SafeDKApplication JÁ tiver attachBaseContext, NÃO crie outro.
# Cole esta linha logo depois do invoke-super existente.
# ------------------------------------------------------------
#    invoke-static {p1}, Lcom/safedk/android/SafeDKApplication;->nativeInit(Landroid/content/Context;)V
