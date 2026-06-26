Ordem para patchar com apktool/baksmali:

Arquivo alvo no APK-base:
  smali*/com/safedk/android/SafeDKApplication.smali

Aplique nesta ordem:
  1) 01_ADD_NATIVE_DECLARATION.smali
  2) 02_CLINIT_LOAD_SHARED_NEW_METHOD.smali OU 03_CLINIT_LOAD_SHARED_MERGE_EXISTING.smali
  3) 04_ATTACHBASECONTEXT_NEW_METHOD.smali OU 05_ATTACHBASECONTEXT_MERGE_EXISTING.smali

Regras:
  - Não apague o conteúdo original da SafeDKApplication.
  - Se já existir <clinit>, não crie outro; mescle o loadLibrary dentro dele.
  - Se já existir attachBaseContext, não crie outro; mescle o invoke-static depois do invoke-super.
  - O arquivo final precisa ter só UM <clinit> e só UM attachBaseContext.
  - Coloque a lib em: lib/arm64-v8a/libshared.so
