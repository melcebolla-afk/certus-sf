# Guardrails — certus-sf

## Datos / evidence — crítico

| Regla | Detalle |
|-------|---------|
| No borrar versiones `evidence/*/v*` | Promote añade; no wipe sin OK humano |
| Staging Bárbol | `evidence/from_barbol/` — solo promote con `barbol_layers_promote.py` |
| Paths producción | Configurar vía UCI `*Path`; no hardcodear paths de lab en C++ |
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

- Mantener upstream en `stockfish/`; merges selectivos desde `official-stockfish/Stockfish`.
- Tag SF documentado en inventario; bump tag = unidad de trabajo explícita.

## Destructivo (preguntar humano)

- Force-push main
- Borrar `testdata/` fixtures o catálogos versionados
- Cambio breaking `catalog.json` schema sin sync evidence
- `git clean` / wipe de directorios `evidence/*/v*` en producción
