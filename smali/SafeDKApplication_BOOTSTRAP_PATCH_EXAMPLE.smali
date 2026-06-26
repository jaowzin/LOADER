;
; Patch smali para com/safedk/android/SafeDKApplication.smali
; ============================================================
;
; Este arquivo NÃO é para substituir a classe inteira cegamente.
; Ele mostra os blocos que devem ser colados/mesclados na classe real.
;
; Ordem:
;   1. Adicionar declaração nativeInit(Context)
;   2. Carregar System.loadLibrary("shared") no <clinit>
;   3. Chamar nativeInit(context) em attachBaseContext(context)
;
; Arquivo alvo típico após apktool:
;   smali_classes*/com/safedk/android/SafeDKApplication.smali
;

# -----------------------------------------------------------------------------
# 1) Cole dentro da classe, fora de qualquer outro método.
# -----------------------------------------------------------------------------
.method private static native nativeInit(Landroid/content/Context;)V
.end method

# -----------------------------------------------------------------------------
# 2A) Se NÃO existir <clinit>, adicione este método inteiro.
# -----------------------------------------------------------------------------
.method static constructor <clinit>()V
    .locals 1

    const-string v0, "shared"
    invoke-static {v0}, Ljava/lang/System;->loadLibrary(Ljava/lang/String;)V

    return-void
.end method

# -----------------------------------------------------------------------------
# 2B) Se JÁ existir <clinit>, NÃO crie outro. Cole só isto dentro dele:
#     Se o método tiver .locals 0, suba para .locals 1.
# -----------------------------------------------------------------------------
#    const-string v0, "shared"
#    invoke-static {v0}, Ljava/lang/System;->loadLibrary(Ljava/lang/String;)V

# -----------------------------------------------------------------------------
# 3A) Se NÃO existir attachBaseContext(Context), adicione este método inteiro.
# -----------------------------------------------------------------------------
.method protected attachBaseContext(Landroid/content/Context;)V
    .locals 0

    invoke-super {p0, p1}, Landroid/app/Application;->attachBaseContext(Landroid/content/Context;)V

    invoke-static {p1}, Lcom/safedk/android/SafeDKApplication;->nativeInit(Landroid/content/Context;)V

    return-void
.end method

# -----------------------------------------------------------------------------
# 3B) Se JÁ existir attachBaseContext(Context), NÃO crie outro.
#     Cole só isto logo depois do invoke-super:
# -----------------------------------------------------------------------------
#    invoke-static {p1}, Lcom/safedk/android/SafeDKApplication;->nativeInit(Landroid/content/Context;)V
