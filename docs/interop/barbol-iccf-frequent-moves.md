# Interop addendum — EMPIRICAL_ICCF + `frequent_moves` (schema v2)

**Fecha:** 2026-09-02  
**Base:** `docs/interop/barbol-iccf-file.md`  
**Consumidor:** certus-sf / Certus (`IccfPath`)

## Cambio

Campo **`frequent_moves`** (UCI frecuentes ICCF), misma idea que `marked_moves` del consenso pero **sin forzar bestmove**.

| Campo | Capa | ¿Forzar raíz? | ¿Filtrar search? |
|-------|------|---------------|------------------|
| `marked_moves` | `STRONG_CONSENSUS` | Sí | `ConsensusSearch=MarkedOnly` |
| `frequent_moves` | `EMPIRICAL_ICCF` | **No** | `IccfSearch=FreqOnly` |

## Schema

- `schema_version`: **1** (legacy, sin filtro) o **2** (con `frequent_moves` opcional por entrada).
- Root v2: `min_n_move`, `min_share`, `max_frequent_moves` (documentación del export).
- Entrada: `frequent_moves: string[]`, opcional `move_stats: [{uci,n,share}]`.
- Sin `frequent_moves` / vacío → hit WDL OK; **no filtrar**.

## Semántica motor (certus-sf)

- Eval: ICCF **no** sustituye NNUE.
- `IccfSearch=Off` (default): no filtra.
- `IccfSearch=FreqOnly`: candidatas = `frequent_moves ∩ legal`; vacío → todas; no jaque; no qsearch; MultiPV `pvIdx>0` sin filtro.
- Si también hay consenso+marked activo → **consenso manda**.

Ver export Bárbol FEAT-0123.

## Comando export (EMPIRICAL_ICCF → certus-sf)

Desde la raíz del repo Bárbol (`chess_idea`):

```bash
./venv/bin/python tools/iccf_pgn_stats_load.py \
  --pgn-file … --min-date … --min-elo … \
  --certus-sf-export \
  --certus-min-n-move 3 \
  --certus-min-share 0.05 \
  --certus-max-frequent-moves 8
```

Salida: `catalogs/iccf/v{YYYY.MM.DD}/catalog.json` (+ `manifest.json`) bajo `CERTUS_SF_CATALOGS_ROOT` (default `/home/mcebolla/certus-sf/catalogs`).
