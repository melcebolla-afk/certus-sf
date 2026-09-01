# Inventario — certus-sf

Fuente de **entrada** para agentes. No usar `docs/PROJECT_STATE.md` como inventario.

## Qué es

**certus-sf:** fork Stockfish + capas evidencia Certus. Motor UCI ICCF. Sin train NNUE.

Referencia Rust: `/home/mcebolla/evidence` (Certus).

## Estructura

| Ruta | Rol |
|------|-----|
| `stockfish/` | Stockfish vendored (baseline en `UPSTREAM`); merge vía runbook |
| `stockfish/src/evidence/` | Stores evidencia Certus (código producto) |
| `stockfish/src/certus/` | Integración mínima + `certus.mk` (overlay build) |
| `stockfish/src/certus-sf` | Binario release (build local, no versionado) |
| `methodology/` | Submodule método (no editar) |
| `docs/bootstrap/` | Export Certus: port, resolver spec, golden fixtures |
| `docs/project/` | Inventario, guardrails, convenciones |
| `docs/specs/` | Specs (`FEAT-0001-evidence-layers-sf.md`) |
| `docs/interop/` | Contratos Bárbol → catalog |
| `docs/plans|analysis|…/` | Unidades de trabajo |
| `catalogs/` | EvidenceRoot — catálogos versionados (lab/prod local; ver README) |
| `train/out/` | Logs cron + artefactos temporales builders |
| `builders/` | Ingest/actualización capas (Python); ver `docs/project/builders.md` |
| `testdata/` | Fixtures CI consensus/iccf/theory/mate/syzygy3 |
| `.cursor/rules/` | Reglas Cursor |

## Stockfish baseline

| Campo | Valor |
|-------|-------|
| Upstream | `https://github.com/official-stockfish/Stockfish.git` |
| Tag | `sf_18` (Stockfish 18) |
| Commit | `cb3d4ee9` |
| Layout | Vendored en `stockfish/` + overlay `src/certus/` |
| Merge | `docs/runbooks/stockfish-merge.md`, `scripts/merge-stockfish.sh` |
| Build | `make -j -C stockfish/src build ARCH=x86-64-sse41-popcnt` (o `x86-64-modern`, alias deprecado) |

## Stack

| Pieza | Tecnología |
|-------|------------|
| Motor | C++17 (Stockfish fork) |
| NNUE | Red Stockfish embebida (sin train propio) |
| Capas evidencia | C++ stores + `evidence/resolver.cpp` (precedencia Certus); hook eval vía `certus/certus_eval` |
| Builders | Python 3 |
| Tests | `evidence_probe`, `golden_probe`, `./tests/evidence_probe.sh` (CI) |
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
| Fase 1 UCI + evidence stores | **hecho** 2026-09-01 |
| Fase 2 resolver + eval hook + golden | **hecho** 2026-09-01 |
| Fase 3 search hooks + info strings | **hecho** 2026-09-01 |
| Fase 4 CI GitHub + runbooks | **hecho** 2026-09-01 |
| FEAT-0001 evidence layers SF | **cerrado** 2026-09-01 |
| FEAT-0002 consensus marked search | **hecho** 2026-09-02 |

## Fuera de alcance

- Train NNUE / certus_trainctl / TRAIN-002
- Motor Rust Certus (repo evidence)
