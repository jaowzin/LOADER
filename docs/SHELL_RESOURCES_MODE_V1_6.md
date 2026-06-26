# v1.6 — Shell Resources Mode

Esta versão corrige o caso em que a casca mínima cai em:

```text
Resources$NotFoundException: String resource ID #0x0
```

Causa: o processo continua sendo `com.Nobodyshot.loader`, mas a v1.5/v1.3 redirecionava também os resources para o APK alvo `com.Nobodyshot.kuboom`. Quando Unity faz lookup de string usando o package do Context (`com.Nobodyshot.loader`) contra a tabela de resources do alvo (`com.Nobodyshot.kuboom`), `getIdentifier` retorna `0`.

Mudança da v1.6:

- Mantém `mClassLoader` apontando para o alvo.
- Mantém `mAppDir`, `mSplitAppDirs` e `mLibDir` apontando para o alvo.
- Mantém `sourceDir`, `splitSourceDirs` e `nativeLibraryDir` apontando para o alvo.
- NÃO altera `packageName`, `mBasePackageName` ou `mOpPackageName`.
- NÃO altera `publicSourceDir`, `splitPublicSourceDirs`, `mResDir` ou `mSplitResDirs`.

Com isso, strings Java básicas, como `game_view_content_description`, precisam existir no loader.

Confirme no loader:

```text
res/values/strings.xml contém game_view_content_description
lib/arm64-v8a/libshared.so contém KUBOOM_BOOT_V1_6
```
