# Use este bloco SOMENTE se SafeDKApplication.smali NÃO tiver attachBaseContext(Context).
# Cole dentro da classe, fora de qualquer outro método.

.method protected attachBaseContext(Landroid/content/Context;)V
    .locals 0

    invoke-super {p0, p1}, Landroid/app/Application;->attachBaseContext(Landroid/content/Context;)V

    invoke-static {p1}, Lcom/safedk/android/SafeDKApplication;->nativeInit(Landroid/content/Context;)V

    return-void
.end method
