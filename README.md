# certus-sf

Fork de **Stockfish** (GPL-3) con las **capas de evidencia** de [Certus](https://github.com/melcebolla-afk/evidence) — motor UCI orientado a **ICCF / correspondencia**.

## Estado

- Bootstrap Certus copiado (`docs/bootstrap/`, `builders/`, `testdata/`).
- **Stockfish 18** (`sf_18`) en submodule `stockfish/` — Fase 0 hecha.
- Capas evidencia: spec [FEAT-0001](docs/specs/FEAT-0001-evidence-layers-sf.md); **Fase 1** (stores + UCI) hecha; Fases 2–4 pendiente.
- Sin train NNUE propio; red SF de serie.

## Build

```bash
make -j -C stockfish/src build ARCH=x86-64-sse41-popcnt
./stockfish/src/certus-sf

# Tests Fase 1
make -C stockfish/src evidence_probe ARCH=x86-64-sse41-popcnt
./tests/evidence_probe.sh
```

## Arranque Cursor

1. Abre este repo en Cursor.
2. Pega el contenido de **[CURSOR-START-PROMPT.md](CURSOR-START-PROMPT.md)** en un chat nuevo.

## Docs clave

| Ruta | Contenido |
|------|-----------|
| [CURSOR-START-PROMPT.md](CURSOR-START-PROMPT.md) | Prompt agente (copiar/pegar) |
| [docs/bootstrap/](docs/bootstrap/) | Export Certus: port map, resolver, formats |
| [docs/interop/](docs/interop/) | Contratos Bárbol → catalog |
| [docs/project/](docs/project/) | Inventario, guardrails, convenciones |

## Referencia hermana

Repo Rust oráculo: `/home/mcebolla/evidence` — `resolver.rs`, stores, UCI.

## Licencia

GPL-3.0-or-later. Ver [docs/bootstrap/LICENSE-NOTICE.md](docs/bootstrap/LICENSE-NOTICE.md).
