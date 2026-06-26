# Erros conhecidos e próximos passos

## INSTALL_FAILED_INVALID_APK: base.apk code is missing

Causa provável:

```text
APK não tem classes.dex, só classes5.dex ou nenhum dex principal.
```

Correção:

```text
Garantir que o APK final tenha classes.dex.
```

## NoSuchMethodError: nativeInit(Context)

Causa:

```text
RegisterNatives tentou registrar assinatura que não existe no smali.
```

Correção já aplicada na source:

```text
Registrar cada overload de nativeInit separadamente.
Limpar exceções para assinaturas ausentes.
```

## Hilt Activity must be attached to an @HiltAndroidApp Application

Causa:

```text
Application stub simples não preserva a Application real/Hilt.
```

Correção provável:

```text
Não usar Application simples.
Preservar herança original ou bootstrap antes da Application real.
```

## ClassNotFoundException: KUBOOMApplication

Pode ter duas causas:

1. A classe realmente não está no dex.
2. A classe está no dex, mas sua superclasse não está. O Android reporta como se a própria classe não existisse.

Para KUBOOM, a segunda aconteceu quando os dex originais foram removidos.

Correção:

```text
Manter os dex/smali originais, principalmente os que contêm droom/daro/... e Hilt/Dagger.
```

## VERIFY FAIL LauncherEntryActivity

Causa:

```text
A shared rodou, mas o PathClassLoader do target não resolve a Activity.
```

Verificar:

```text
TARGET_PACKAGE correto?
KUBOOM original instalado?
sourceDir/splitSourceDirs logados?
Launcher real é LauncherEntryActivity ou UnityPlayerActivity?
Splits faltando?
```

## UnsatisfiedLinkError

Causa:

```text
ClassLoader Java funcionou, mas namespace/caminho nativo não achou alguma .so.
```

Verificar:

```text
target nativeLibraryDir logado corretamente?
A lib existe no target?
Android nativeloader criou namespace para o novo classloader?
```

A shared atual só passa `nativeLibraryDir` no PathClassLoader e patcha ApplicationInfo. Não implementa namespace nativo avançado.

