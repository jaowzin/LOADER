# Changelog

## v1.1

- Adicionado workflow GitHub Actions:
  - `.github/workflows/build-libshared.yml`
- Adicionados blocos smali separados em:
  - `smali/patch_blocks/`
- Atualizado `SafeDKApplication_BOOTSTRAP_PATCH_EXAMPLE.smali` com instruções de merge para `<clinit>` e `attachBaseContext`.
- Adicionado `docs/GITHUB_WORKFLOW.md`.

## v1.0

- Adaptação inicial bootstrap/no-op para KUBOOM CTF.
- Alvo: `com.Nobodyshot.kuboom`.
- Activity de verificação: `com.unity3d.player.UnityPlayerActivity`.
- Classe primária JNI: `com/safedk/android/SafeDKApplication`.
