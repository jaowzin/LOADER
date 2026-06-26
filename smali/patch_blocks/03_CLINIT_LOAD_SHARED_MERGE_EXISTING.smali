# Use este bloco se SafeDKApplication.smali JÁ tiver:
#   .method static constructor <clinit>()V
#
# Dentro do <clinit> existente:
#   - se estiver .locals 0, mude para .locals 1;
#   - se estiver .registers 0, mude para .registers 1;
#   - cole estas instruções perto do começo, antes de inicializações pesadas.

    const-string v0, "shared"
    invoke-static {v0}, Ljava/lang/System;->loadLibrary(Ljava/lang/String;)V
