# Regras para AndroidManifest.xml

## Trocar package do loader

O package do APK shell/loader deve ser único para não conflitar com o app original instalado:

```xml
<manifest package="com.ctf.kuboom.loader">
```

## Não trocar nomes de classes do target

Activities, services, receivers e providers devem continuar com nomes de classe originais quando a ideia é resolvê-los pelo ClassLoader do target:

```xml
<application android:name="com.Nobodyshot.kuboom.KUBOOMApplication">
<activity android:name="com.Nobodyshot.kuboom.LauncherEntryActivity">
```

Não converter para:

```xml
com.ctf.kuboom.loader.KUBOOMApplication
```

## Trocar authorities de providers

Authorities precisam ser únicas, senão o Android recusa instalar junto com o app original:

```xml
android:authorities="com.ctf.kuboom.loader.firebaseinitprovider"
android:authorities="com.ctf.kuboom.loader.fileprovider"
```

Qualquer provider cujo authority ainda aponte para o package original pode causar conflito de instalação.

## FacebookContentProvider

Se estiver assim:

```xml
android:authorities="@7f140270"
```

isso é suspeito porque pode resolver para authority original. Para teste, usar literal único:

```xml
android:authorities="com.ctf.kuboom.loader.facebookcontentprovider"
```

## appComponentFactory

Em primeiro teste, remover:

```xml
android:appComponentFactory="androidx.core.app.CoreComponentFactory"
```

Motivo: pode fazer AndroidX carregar antes do bootstrap. Se o app precisar muito disso depois, reintroduzir quando ClassLoader estiver estável.

