# GitHub Actions build

Workflow incluído:

```text
.github/workflows/build-libshared.yml
```

Ele faz:

1. checkout do repositório;
2. instala Android SDK/NDK `26.3.11579264`;
3. executa `tools/build_local.sh`;
4. valida o ELF gerado com `file` e `readelf`;
5. envia artifact:

```text
libshared-kuboom-bootstrap-arm64-v8a/libshared.so
```

## Estrutura esperada no repositório

```text
repo/
├── .github/
│   └── workflows/
│       └── build-libshared.yml
├── src/
│   └── libshared_kuboom_bootstrap_v1.c
└── tools/
    └── build_local.sh
```

## Uso

No GitHub:

```text
Actions → Build libshared KUBOOM bootstrap → Run workflow
```

Depois baixe o artifact e coloque no APK shell/base:

```text
lib/arm64-v8a/libshared.so
```
