# GitHub workflow corrigido — v1.2

O workflow v1.1 usava `android-actions/setup-android@v3`. Em alguns runners ele tenta preparar pacotes antigos de SDK Tools e pode falhar antes da compilação nativa.

A versão v1.2 remove essa dependência. Agora o workflow:

1. tenta usar NDK já presente no runner;
2. tenta `/usr/local/lib/android/sdk/ndk/26.3.11579264`;
3. se não encontrar, baixa diretamente `android-ndk-r26d-linux.zip` do repositório oficial do Android;
4. compila `src/libshared_kuboom_bootstrap_v1.c` com `aarch64-linux-android23-clang`;
5. sobe artifact `libshared-kuboom-bootstrap-arm64-v8a` contendo `libshared.so`.

Arquivo:

```text
.github/workflows/build-libshared.yml
```

Resultado esperado:

```text
out/arm64-v8a/libshared.so
```
