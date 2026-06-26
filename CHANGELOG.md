# CHANGELOG

## v1.3
- Corrige caso em que UnityPlayerActivity sobe, mas Unity mostra:
  `Not enough storage space to install required resources.`
- Além de ApplicationInfo, agora a shared também tenta patchar campos privados do LoadedApk:
  - `mAppDir`
  - `mResDir`
  - `mSplitAppDirs`
  - `mSplitResDirs`
  - `mLibDir`
- Tag de log alterada para `KUBOOM_BOOT_V1_3`.

## v1.2
- Workflow sem android-actions/setup-android.

## v1.1
- Adicionado workflow e blocos smali.

## v1.0
- Bootstrap/no-op para com.Nobodyshot.kuboom.
