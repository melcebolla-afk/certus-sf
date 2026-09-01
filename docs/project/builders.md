# Builders — capas evidencia (certus-sf)

Scripts Python en `builders/`; salida en **`catalogs/`** (EvidenceRoot del motor).

Runbook cron: `docs/runbooks/catalogs-layers.md`  
CI local/remoto: `docs/runbooks/ci.md`

## Cron activo (certus-sf)

| Script | Capa | Destino |
|--------|------|---------|
| `mate_repo_update.py` | PROVEN_MATE | `catalogs/mate/v…/` |
| `theory_repo_update.py` | THEORETICAL | `catalogs/theoretical/v…/` |

**Bárbol** exporta directo a `catalogs/consensus/` y `catalogs/iccf/` (sin promote).

## Ingest manual (ad hoc)

| Script | Capa |
|--------|------|
| `consensus_ingest.py` | STRONG_CONSENSUS |
| `iccf_ingest.py` | EMPIRICAL_ICCF |
| `theory_build.py` | THEORETICAL |
| `fortresses_import.py` | THEORETICAL (EPD → JSON) |
| `lichess_mate_filter.py` | mate (filtro previo a mate_build) |
| `mate_build.py` | PROVEN_MATE (probe + catalog.json + catalog.idx) |
| `mate_repo_update.py` | cron mate (wget + filter + mate_build --merge) |

## Deprecado

| Script | Notas |
|--------|-------|
| `barbol_layers_promote.py` | Solo migración legacy `from_barbol/` |

## Paths compartidos

`builders/catalog_paths.py`:

- `CATALOGS` → `catalogs/`
- `TRAIN_OUT` → `train/out/`

Dependencia Python: `pip install -r builders/requirements.txt` (`python-chess`).

## Paridad con repo evidence

Misma forma `catalog.json` / `schema_version`. Cambios breaking → portar diff desde `evidence/builders/`.
