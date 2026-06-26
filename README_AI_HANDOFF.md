# Shared bootstrap KUBOOM — handoff

Este pacote contém uma adaptação bootstrap/no-op da `libshared.so` para o alvo CTF KUBOOM analisado.

## Alvo

```text
package: com.Nobodyshot.kuboom
application: com.safedk.android.SafeDKApplication
launcher activity: com.unity3d.player.UnityPlayerActivity
ABI: arm64-v8a
engine: Unity/IL2CPP
```

## O que a shared faz

- lê o package atual do loader;
- encontra `com.Nobodyshot.kuboom` via PackageManager;
- lê `sourceDir`, `splitSourceDirs` e `nativeLibraryDir` do alvo instalado;
- cria `PathClassLoader` para base/splits/libs do alvo;
- troca `LoadedApk.mClassLoader` do loader;
- ajusta `ApplicationInfo` do loader para os caminhos do alvo;
- valida `loadClass(com.unity3d.player.UnityPlayerActivity)`.

## O que ela não faz

Não contém menu, hooks, patch de memória, alterações de gameplay, anti-cheat bypass ou lógica de mod. É apenas bootstrap de ambiente para laboratório/CTF.

## Arquivo principal

```text
src/libshared_kuboom_bootstrap_v1.c
```

## Build

Linux/macOS:

```bash
export ANDROID_NDK_HOME=/caminho/android-ndk
chmod +x tools/build_local.sh
./tools/build_local.sh
```

Windows:

```bat
set ANDROID_NDK_HOME=C:\Androidndroid-ndk-r26d
toolsuild_local.bat
```

Saída:

```text
out/arm64-v8a/libshared.so
```

Colocar no APK shell/base:

```text
lib/arm64-v8a/libshared.so
```

## Integração recomendada para APK original como base

1. Manter todos os `classes*.dex`, `assets`, `res` e `lib/arm64-v8a` do APK-base.
2. Alterar package do shell para algo como `com.ctf.kuboom.loader`.
3. Trocar `provider authorities` que usam `com.Nobodyshot.kuboom.*` para `com.ctf.kuboom.loader.*`.
4. Adicionar visibilidade de pacote se necessário:

```xml
<queries>
    <package android:name="com.Nobodyshot.kuboom" />
</queries>
```

5. Patchar `com.safedk.android.SafeDKApplication` para:
   - `System.loadLibrary("shared")`;
   - declarar `nativeInit(Context)`;
   - chamar `nativeInit(context)` em `attachBaseContext`.

Veja `smali/SafeDKApplication_BOOTSTRAP_PATCH_EXAMPLE.smali`.


## GitHub Actions

Workflow pronto:

```text
.github/workflows/build-libshared.yml
```

No GitHub use:

```text
Actions → Build libshared KUBOOM bootstrap → Run workflow
```

O artifact gerado será:

```text
libshared-kuboom-bootstrap-arm64-v8a/libshared.so
```

## Smali pronto para SafeDKApplication

Blocos separados:

```text
smali/patch_blocks/01_ADD_NATIVE_DECLARATION.smali
smali/patch_blocks/02_CLINIT_LOAD_SHARED_NEW_METHOD.smali
smali/patch_blocks/03_CLINIT_LOAD_SHARED_MERGE_EXISTING.smali
smali/patch_blocks/04_ATTACHBASECONTEXT_NEW_METHOD.smali
smali/patch_blocks/05_ATTACHBASECONTEXT_MERGE_EXISTING.smali
```

Exemplo consolidado:

```text
smali/SafeDKApplication_BOOTSTRAP_PATCH_EXAMPLE.smali
```

## Logs esperados

Veja `EXPECTED_LOGS.txt`.

---

## Nota v1.2 — workflow corrigido

Se a build do GitHub Actions falhar durante `setup-android` ou `sdkmanager` antes de compilar a lib, use o workflow v1.2 incluído neste pacote. Ele não usa `android-actions/setup-android@v3`; resolve o NDK localmente ou baixa diretamente o Android NDK r26d.

Smali direto para a SafeDKApplication:

```text
smali/SafeDKApplication_INSERT_THIS_PATCH.smali
```
