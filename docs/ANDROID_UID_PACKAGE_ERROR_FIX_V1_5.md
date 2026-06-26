# v1.5 — Android UID/package fix

Erro observado:

```text
java.lang.SecurityException: Package com.Nobodyshot.kuboom does not belong to <uid>
```

Causa: a v1.4 alterava campos de identidade do pacote (`ApplicationInfo.packageName`, `LoadedApk.mPackageName`, `ContextImpl.mBasePackageName`, `ContextImpl.mOpPackageName`) para o pacote alvo. Android moderno valida se o package name pertence ao UID chamador. Como o UID é do loader, o framework rejeita.

Correção v1.5:

- preserva package/UID do loader;
- mantém redirecionamento de caminhos (`mAppDir`, `mResDir`, splits, libs);
- mantém `mClassLoader` apontando para o alvo;
- remove alteração efetiva de `mOpPackageName` e similares.

Nota: se o loader for casca mínima, o erro anterior de resource pode voltar. Para Unity, o caminho estável é usar APK original como base real do loader, mantendo `resources.arsc`, `assets/bin/Data` e libs Unity.
