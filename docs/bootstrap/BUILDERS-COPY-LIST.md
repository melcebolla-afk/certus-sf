# Builders — lista para certus-sf

Scripts **reutilizables tal cual** (salvo ajustar `ROOT` / paths). Copiar desde `evidence/builders/`.

## Capas evidencia (P0)

| Script | Capa | Función |
|--------|------|---------|
| `consensus_ingest.py` | STRONG_CONSENSUS | Export Bárbol / JSON intermedio → `catalog.json` |
| `iccf_ingest.py` | EMPIRICAL_ICCF | Stats ICCF rows → `catalog.json` |
| `theory_build.py` | THEORETICAL | seed.json + extras → `catalog.json` |
| `theory_repo_update.py` | THEORETICAL | Cron: download EPD + merge seed + build |
| `fortresses_import.py` | THEORETICAL | Chess-EPDs fortresses → JSON intermedio |
| `mate_repo_update.py` | PROVEN_MATE | Lichess puzzles → MatePath (+ `catalog.idx`) |
| `lichess_mate_filter.py` | PROVEN_MATE | Filtro piece_count ≥7, temas mate |
| `lichess_theory_candidates.py` | THEORETICAL | Candidatos Lichess → curación humana |
| `barbol_layers_promote.py` | ops | `evidence/from_barbol/` → `evidence/{consensus,iccf}/` |

## Ops relacionados (P1 — opcional en v1)

| Script | Notas |
|--------|-------|
| `lab_launch.py` | Adaptar: lanzar binario **SF fork** en lugar de `evidence-engine` |
| `nightly_smoke.py` | Adaptar paths binario |

## No copiar (train / Rust engine)

`bench_search.py`, `bench_tt_smp.py`, `bench_nps_profile.py`, `certus_trainctl.sh`, `post_selfplay_*`, `cycle_budget.py`, `filter_labels.py`, `train/*`, `write_prod_meta.py` (salvo metadata evidencia si se necesita).

## Layout evidence en disco

```text
evidence/
  from_barbol/          # staging Bárbol domingo
    consensus/vYYYY.MM.DD/
    iccf/vYYYY.MM.DD/
  consensus/v…/
  iccf/v…/
  theoretical/v…/
  mate/v…/
```

## Cron sugerido (desde RUNBOOK-LAYERS-EXTRACT.md)

| Hora | Script |
|------|--------|
| 04:00 | `mate_repo_update.py` |
| 04:30 | `theory_repo_update.py` |
| 05:00 | `barbol_layers_promote.py` |

Bárbol escribe consensus/iccf en `from_barbol/` (ver interop).

## Validación post-build

```bash
python3 builders/consensus_ingest.py --help
python3 builders/barbol_layers_promote.py --dry-run
# Tras generar catalog:
# certus-sf UCI: setoption name ConsensusPath value testdata/consensus
```
