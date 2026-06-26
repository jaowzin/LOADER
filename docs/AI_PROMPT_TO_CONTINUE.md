# Prompt pronto para outra IA continuar

Cole isto para outra IA junto com este pacote:

```text
Estou trabalhando em um CTF/lab Android com uma libshared.so de bootstrap estilo DPFF.

Objetivo benigno: documentar/continuar uma casca de loader que, em runtime, troca ClassLoader/caminhos do pacote loader para apontar para o app alvo instalado. Não quero menu, cheat, auth, hook, patch de memória ou bypass. Só bootstrap e diagnóstico.

Arquivos principais:
- src/libshared_kuboom_bootstrap_v1_DOCUMENTED.c
- docs/BOOT_FLOW.md
- docs/SMALI_DEX_RULES.md
- docs/ERRORS_AND_NEXT_STEPS.md
- smali/KUBOOMApplication_HILT_PATCH.smali

Contexto:
- DPFF funcionou com este conceito.
- KUBOOM target package: com.Nobodyshot.kuboom
- Loader package usado: com.ctf.kuboom.loader
- Application manifest: com.Nobodyshot.kuboom.KUBOOMApplication
- Launcher: com.Nobodyshot.kuboom.LauncherEntryActivity

Problema atual:
- Application simples caiu com Hilt: “Hilt Activity must be attached to an @HiltAndroidApp Application”.
- Application Hilt-like caiu quando deixei só classes.dex pequeno, porque as classes obfuscadas Hilt/droom foram apagadas.

Regra para continuar:
- Não apagar classes2.dex/classes3.dex/classes4.dex/classes5.dex no KUBOOM.
- Manter dex/smali originais e substituir apenas KUBOOMApplication.smali, ou criar bootstrap antes da Application real.
- A shared atual registra nativeInit overloads individualmente e patcha LoadedApk.mClassLoader.

Preciso que você analise os logs e proponha o próximo patch mantendo o escopo benigno/CTF.
```

