# Manifiesto de reutilización — evidence → certus-sf

## Copiar al nuevo repo (obligatorio)

| Origen (evidence) | Destino (certus-sf) | Notas |
|-------------------|---------------------|-------|
| `docs/export/certus-sf-bootstrap/*` | `docs/bootstrap/` | Este export |
| `builders/consensus_ingest.py` | `builders/` | Ingest export Bárbol → catalog |
| `builders/iccf_ingest.py` | `builders/` | Ingest ICCF rows → catalog |
| `builders/theory_build.py` | `builders/` | Build THEORETICAL catalog |
| `builders/theory_repo_update.py` | `builders/` | Cron theory refresh |
| `builders/fortresses_import.py` | `builders/` | EPD fortresses |
| `builders/mate_repo_update.py` | `builders/` | MatePath Lichess pipeline |
| `builders/lichess_mate_filter.py` | `builders/` | Filtro mates ≥7 piezas |
| `builders/lichess_theory_candidates.py` | `builders/` | Candidatos theory (curación) |
| `builders/barbol_layers_promote.py` | `builders/` | Promote domingo staging→activo |
| `docs/interop/*.md` | `docs/interop/` | Contratos Bárbol |
| `testdata/consensus/` | `testdata/consensus/` | CI |
| `testdata/iccf/` | `testdata/iccf/` | CI |
| `testdata/theoretical/` | `testdata/theoretical/` | CI |
| `testdata/mate/` | `testdata/mate/` | CI (si existe fixture) |
| `testdata/syzygy3/` | `testdata/syzygy3/` | CI TB (opcional 3-man) |
| `docs/prior-art/consensus.md` | `docs/prior-art/` | Si existe |
| `docs/prior-art/iccf.md` | `docs/prior-art/` | Si existe |
| `docs/prior-art/resolver.md` | `docs/prior-art/` | Si existe |

## Referenciar (no duplicar código)

| Artefacto | Uso |
|-----------|-----|
| `crates/evidence-engine/src/resolver.rs` | **Spec ejecutable** — port a C++ |
| `consensus.rs`, `iccf.rs`, `theoretical.rs`, `mate_index.rs` | Formato probe + load |
| `crates/evidence-engine/src/uci.rs` | Opciones UCI + strings info |
| `docs/PRODUCT-SPEC.md` | Spec completa producto original |
| `docs/runbooks/layers-and-train.md` | Ops capas (extract en bootstrap) |
| `evidence/` en disco | Datos reales lab/prod |

## No migrar al fork SF

| Origen | Motivo |
|--------|--------|
| `crates/search`, `crates/board`, `crates/nnue` | SF reemplaza search/eval NNUE |
| `train/`, `certus_trainctl`, TRAIN-* docs | Sin train NNUE propio |
| `builders/bench_*`, `uci_match` (Elo) | Opcional más tarde; no bloquea v1 |
| `builders/pgo_build.sh` | Usar build SF nativo |
| Profiling `CERTUS_TT_PROFILE` / `CERTUS_NPS_PROFILE` | Específico Rust |

## Compatibilidad de artefactos (crítico)

Los `catalog.json` y `catalog.idx` generados por builders **deben** cargar en:

1. Certus Rust (regresión durante transición)
2. certus-sf C++ (producto)

Clave de lookup: **Zobrist placement + side-to-move** (ignorar castling/ep/clock en hit — igual que Certus).

## Methodology + Cursor

Copiar desde evidence:

- `methodology/` @ tag documentado
- `.cursor/rules/00-methodology.mdc` (adaptar paths)
- `.cursor/rules/50-project-guardrails.mdc`
- `.cursor/rules/55-project-inventory.mdc`
- `.cursor/rules/60-project-conventions.mdc` (quitar prior-art engines Rust; añadir “SF base + evidence port”)

## Dependencias Python builders

Revisar imports en cada script (`pathlib`, stdlib; algunos pueden asumir `ROOT = parents[1]`). Ajustar `ROOT` al layout certus-sf.

No requieren PyTorch para **solo capas** (train scripts excluidos).
