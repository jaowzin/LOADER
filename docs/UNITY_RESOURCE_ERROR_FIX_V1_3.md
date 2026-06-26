# Unity resource error fix v1.3

Erro observado:

```text
Not enough storage space to install required resources.
```

Neste contexto, normalmente não é espaço real. É Unity vendo o APK shell como caminho de código/resources.

A v1.3 mantém o modelo bootstrap/no-op, mas replica os caminhos do target também nos campos internos do LoadedApk:

```text
mAppDir
mResDir
mSplitAppDirs
mSplitResDirs
mLibDir
```

Logs esperados:

```text
KUBOOM_BOOT_V1_3: patched LoadedApk.mApplicationInfo paths
KUBOOM_BOOT_V1_3: patched LoadedApk.mAppDir
KUBOOM_BOOT_V1_3: patched LoadedApk.mResDir
KUBOOM_BOOT_V1_3: patched LoadedApk.mLibDir
KUBOOM_BOOT_V1_3: VERIFY OK loadClass(com.unity3d.player.UnityPlayerActivity)
```
