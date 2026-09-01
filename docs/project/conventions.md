# Convenciones — certus-sf

## Docs

- Layout: `methodology/layout.md`
- IDs: `FEAT-####` / `BUG-####`
- Plans: `docs/plans/<ID>-plan.md`
- Specs: `docs/specs/<ID>-<slug>.md`

## Código C++ (Stockfish fork)

- Upstream en submodule `stockfish/`; tag documentado en `docs/project/inventory.md`.
- Cambios Certus: nuevo directorio `stockfish/src/evidence/` (stores + resolver).
- Hooks en archivos SF existentes: `evaluate.cpp`, `uci.cpp` / `ucioption.cpp`, `search.cpp`.
- Estilo: seguir el del árbol SF tocado (C++17, sin excepciones).
- GPL-3.0; atribución SF + diseño Certus en NOTICE.
- Prior-art: documentar deltas en `docs/prior-art/`.

## Build

```bash
make -j -C stockfish/src build ARCH=x86-64-sse41-popcnt
# Binario: stockfish/src/stockfish
```

ARCH `x86-64-modern` sigue funcionando (alias deprecado → sse41-popcnt).

## Stores evidencia

- **Mismos formatos** que Certus (`docs/bootstrap/STORE-FORMATS.md`).
- Lookup key: placement + STM (Zobrist SF `Position::key()` sin distinguir reloj si Certus no lo hace).
- Paths vía UCI (`*Path`); no hardcodear lab en C++.
- No cambiar schema sin bump `schema_version` + acuerdo con repo evidence.

## Builders Python

- `builders/` — copiados de evidence; raíz repo = `Path(__file__).resolve().parents[1]`.
- Generan `catalog.json` / `catalog.idx` compatibles con Certus Rust y certus-sf C++.
- Promote Bárbol: `builders/barbol_layers_promote.py`.

## Tests

- Golden: `docs/bootstrap/GOLDEN-FIXTURES.jsonl`
- Fixtures: `testdata/{consensus,iccf,theoretical,mate,syzygy3}/`
- Smoke UCI: `uci` → `isready` → `go depth 1` → `bestmove`

## UCI producto

- Nombre provisional: `id name certus-sf dev` (Certus reservado al Rust).
- Ver opciones objetivo: `docs/bootstrap/UCI-OPTIONS.md`.

## Repos relacionados

| Repo | Rol |
|------|-----|
| `evidence` (Certus) | Oráculo Rust, fábrica datos, mismos builders origen |
| `methodology` | Submodule método |
| `official-stockfish/Stockfish` | Upstream motor |

## Commits

- Español o inglés; foco en «por qué».
- Separar: baseline SF / evidence layer / docs / builders.
- Baseline SF: `baseline: stockfish @<tag>` sin cambios de eval.
