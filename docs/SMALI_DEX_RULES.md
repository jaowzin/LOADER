# Regras de smali/dex para KUBOOM

## Erro que aconteceu

O APK final ficou com apenas:

```text
classes.dex  = Application pequena
```

E perdeu:

```text
classes2.dex
classes3.dex
classes4.dex
classes5.dex
```

Com isso, a Application Hilt-like não carregou porque suas superclasses obfuscadas não estavam presentes:

```text
Ldroom/daro/a/sw/l;
Ldroom/daro/a/iz/a;
```

## Regra para Hilt

Para KUBOOM, não apagar os dex/smali originais até entender exatamente onde estão todas as classes Hilt/Dagger/obfuscadas.

O caminho seguro:

```text
1. Decompilar APK original.
2. Manter smali/, smali_classes2/, smali_classes3/, smali_classes4/, smali_classes5/.
3. Substituir apenas a KUBOOMApplication.smali original pelo patch.
4. Colocar lib/arm64-v8a/libshared.so.
5. Rebuildar e assinar.
```

## Onde colocar a Application

Se você usa a Application simples que estende `android.app.Application`, ela pode ir em `smali/`, mas cairá no erro de Hilt.

Se você usa a Application Hilt-like, coloque no mesmo dex/smali onde a Application original estava, mantendo as dependências do mesmo app.

## Conferência no APK final

Abrir o APK final como zip e confirmar:

```text
classes.dex
classes2.dex
classes3.dex
classes4.dex
classes5.dex
lib/arm64-v8a/libshared.so
AndroidManifest.xml
```

Se só tiver `classes.dex` pequeno, está errado para KUBOOM.

