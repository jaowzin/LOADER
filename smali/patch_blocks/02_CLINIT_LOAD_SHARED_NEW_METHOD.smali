# Use este bloco SOMENTE se SafeDKApplication.smali NÃO tiver <clinit>().
# Cole dentro da classe, fora de qualquer outro método.

.method static constructor <clinit>()V
    .locals 1

    const-string v0, "shared"
    invoke-static {v0}, Ljava/lang/System;->loadLibrary(Ljava/lang/String;)V

    return-void
.end method
