# Use este bloco se SafeDKApplication.smali JÁ tiver:
#   .method protected attachBaseContext(Landroid/content/Context;)V
#
# Dentro do método existente, cole logo DEPOIS do invoke-super:
#   invoke-super {p0, p1}, Landroid/app/Application;->attachBaseContext(Landroid/content/Context;)V
#
# Não precisa aumentar .locals porque usa p1.

    invoke-static {p1}, Lcom/safedk/android/SafeDKApplication;->nativeInit(Landroid/content/Context;)V
