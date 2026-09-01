# Inventario — certus-sf

Fuente de **entrada** para agentes. No usar `docs/PROJECT_STATE.md` como inventario.

## Qué es

**certus-sf:** fork Stockfish + capas evidencia Certus. Motor UCI ICCF. Sin train NNUE.

Referencia Rust: `/home/mcebolla/evidence` (Certus).

## Estructura

| Ruta | Rol |
|------|-----|
| `stockfish/` | Submodule upstream SF — código motor C++ |
| `stockfish/src/` | Fuentes SF; futuro `evidence/` para capas Certus |
| `stockfish/src/stockfish` | Binario release (build local, no versionado) |
| `methodology/` | Submodule método (no editar) |
| `docs/bootstrap/` | Export Certus: port, resolver spec, golden fixtures |
| `docs/project/` | Inventario, guardrails, convenciones |
| `docs/specs/` | Specs (`FEAT-0001-evidence-layers-sf.md`) |
| `docs/interop/` | Contratos Bárbol → catalog |
| `docs/plans|analysis|…/` | Unidades de trabajo |
| `builders/` | Ingest/promote capas (Python, desde evidence) |
| `testdata/` | Fixtures CI consensus/iccf/theory/mate/syzygy3 |
| `.cursor/rules/` | Reglas Cursor |

## Stockfish baseline

| Campo | Valor |
|-------|-------|
| Upstream | `https://github.com/official-stockfish/Stockfish.git` |
| Tag | `sf_18` (Stockfish 18) |
| Commit | `cb3d4ee9` |
| Layout | Submodule en `stockfish/` |
| Build | `make -j -C stockfish/src build ARCH=x86-64-sse41-popcnt` (o `x86-64-modern`, alias deprecado) |

## Stack

| Pieza | Tecnología |
|-------|------------|
| Motor | C++17 (Stockfish fork) |
| NNUE | Red Stockfish embebida (sin train propio) |
| Capas evidencia | C++ stores + resolver (port Certus) — pendiente |
| Builders | Python 3 |
| Tests | Golden `GOLDEN-FIXTURES.jsonl` + UCI smoke |
| Licencia | GPL-3.0-or-later |

## Capas producto

```text
PROVEN_TB > PROVEN_MATE > THEORETICAL > STRONG_CONSENSUS > EMPIRICAL_ICCF > INFERENCE(NNUE SF)
```

## Unidades

| ID | Estado |
|----|--------|
| Bootstrap export | **hecho** 2026-09-01 |
| Fase 0 Stockfish baseline | **hecho** 2026-09-01 (`sf_18`) |
| FEAT-0001 evidence layers SF | **en curso** — spec; Fases 1–4 pendientes |

## Fuera de alcance

- Train NNUE / certus_trainctl / TRAIN-002
- Motor Rust Certus (repo evidence)
