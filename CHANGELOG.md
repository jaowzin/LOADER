# Changelog

## v1.4

- Corrige o caso Unity `Resources$NotFoundException: String resource ID #0x0` em shell com package diferente.
- Adiciona patch best-effort de identidade de pacote:
  - `ApplicationInfo.packageName = com.Nobodyshot.kuboom`
  - `LoadedApk.mPackageName = com.Nobodyshot.kuboom`
  - `ContextImpl.mBasePackageName = com.Nobodyshot.kuboom`
  - `ContextImpl.mOpPackageName = com.Nobodyshot.kuboom`
- Mantém patches da v1.3 para `mAppDir`, `mResDir`, splits, libs e `mClassLoader`.

## v1.3

- Patch best-effort de paths internos do LoadedApk para Unity resources/assets.

## v1.2

- Workflow sem `android-actions/setup-android@v3`.

## v1.1

- Adiciona workflow e blocos smali.

## v1.0

- Bootstrap/no-op inicial para KUBOOM.
