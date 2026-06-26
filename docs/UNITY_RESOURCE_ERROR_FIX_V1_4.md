# KUBOOM_BOOT_V1_4 — Fix String resource ID #0x0

Sintoma observado:

```text
Unable to start activity ComponentInfo{com.Nobodyshot.loader/com.unity3d.player.UnityPlayerActivity}
android.content.res.Resources$NotFoundException: String resource ID #0x0
```

Causa provável em loader shell:

- O Manifest do shell usa package diferente, por exemplo `com.Nobodyshot.loader`.
- A tabela de resources do APK-base ainda foi gerada para o package original `com.Nobodyshot.kuboom`.
- Unity chama `Resources.getIdentifier(..., "string", context.getPackageName())`.
- Se `context.getPackageName()` retorna o package do loader, o ID volta `0`.
- Em seguida o Unity chama `getString(0)` e cai com `String resource ID #0x0`.

Correção da v1.4:

- patch em `ApplicationInfo.packageName` para `com.Nobodyshot.kuboom`;
- patch best-effort em `LoadedApk.mPackageName`;
- patch best-effort em `ContextImpl.mBasePackageName` e `ContextImpl.mOpPackageName`;
- mantém os patches de paths/classloader da v1.3.

Logs esperados:

```text
KUBOOM_BOOT_V1_4 patched LoadedApk.mPackageName -> target
KUBOOM_BOOT_V1_4 patched ContextImpl.mBasePackageName -> target
KUBOOM_BOOT_V1_4 patched ContextImpl.mOpPackageName -> target
```
