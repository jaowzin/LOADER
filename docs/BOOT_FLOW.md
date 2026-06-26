# Fluxo de boot da libshared

## Fluxo desejado

```text
Android inicia pacote loader
↓
Carrega Application declarada no manifest
↓
Application.<clinit>() chama System.loadLibrary("shared")
↓
JNI_OnLoad roda
↓
RegisterNatives tenta registrar nativeInit(...)
↓
Application.attachBaseContext(context)
↓
nativeInit(context)
↓
bootstrap(context)
↓
PackageManager.getApplicationInfo(targetPackage)
↓
extrai sourceDir, splitSourceDirs, nativeLibraryDir do target instalado
↓
monta dexPath base.apk:split1.apk:split2.apk...
↓
cria PathClassLoader do target
↓
achar LoadedApk do loader em ActivityThread.mPackages
↓
patch LoadedApk.mClassLoader
↓
patch LoadedApk.mApplicationInfo paths
↓
patch ContextImpl.mPackageInfo best-effort
↓
verifica loadClass(TARGET_ACTIVITY)
↓
Android instancia providers/activities pelo ClassLoader novo
```

## Por que precisa ser cedo

Providers do manifest são instalados antes da Activity. Se o ClassLoader não for trocado antes dos providers, o crash típico é:

```text
ClassNotFoundException: com.google.firebase.provider.FirebaseInitProvider
ClassNotFoundException: androidx.startup.InitializationProvider
```

Por isso o ponto ideal é `Application.<clinit>` + `attachBaseContext`.

## Onde a shared não atua

A shared atual não:

- cria namespace nativo customizado;
- copia assets/libs;
- implementa lógica do app;
- substitui Hilt/Dagger;
- resolve provider se a Application não carrega;
- garante resources se o Android usar caches antes do patch.

