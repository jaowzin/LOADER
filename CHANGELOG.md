# Changelog

## v1.5

- Corrige `SecurityException: Package com.Nobodyshot.kuboom does not belong to <uid>`.
- Desativa patch de identidade do pacote introduzido na v1.4.
- Preserva package/UID do loader.
- Mantém patches de caminhos do target e ClassLoader.

## v1.4

- Tentava patchar package identity para corrigir resource lookup do Unity.
- Em Android moderno pode falhar por validação UID/package.
