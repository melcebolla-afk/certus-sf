# Guardrails — certus-sf

## Datos / evidence — crítico

| Regla | Detalle |
|-------|---------|
| No borrar versiones `catalogs/*/v*` | Cada export añade; no wipe sin OK humano |
| Bárbol consensus/iccf | Export directo a `catalogs/consensus/`, `catalogs/iccf/` (atómico) |
| Paths producción | UCI `EvidencePath` o `*Path`; no hardcodear en C++ |
| Fixtures CI | No borrar `testdata/` sin OK humano |
| Schema catalog | Cambio breaking solo con sync repo evidence + bump `schema_version` |

## Runtime

- **Sin** MariaDB / API Bárbol en el motor.
- **Sin** ChessDB, Lichess cloud, OTB como capa evidencia.
- Syzygy: path operador (`SyzygyPath` SF estándar).
- NNUE: red Stockfish embebida; **no** importar train NNUE de evidence.

## Licencia

- Fork GPL-3.0; no relicenciar.
- No pegar código AGPL (Reckless, Viridithas).

## Cobertura capas

- Syzygy 6-man en producción.
- MatePath / Theory: **no** duplicar posiciones ≤6 piezas cubiertas por TB.

## Scope repo

- **No** train NNUE ni scripts self-play de evidence (salvo reutilizar builders de **capas**).
- **No** editar submodule `methodology/`.
- **No** modificar eval/search SF en Fase 0 baseline; cambios evidencia en fases FEAT-0001.

## Submodule Stockfish

- **No** submodule a upstream oficial; código en `stockfish/` dentro de certus-sf.
- Bump SF: script `scripts/merge-stockfish.sh`; no push a `official-stockfish/Stockfish`.

## Destructivo (preguntar humano)

- Force-push main
- Borrar `testdata/` fixtures o catálogos versionados
- Cambio breaking `catalog.json` schema sin sync evidence
- `git clean` / wipe de directorios `catalogs/*/v*` en producción
