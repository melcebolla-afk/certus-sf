# Runbook — capas evidencia (`catalogs/`)

EvidenceRoot de **certus-sf**: `/home/mcebolla/certus-sf/catalogs`

Formato de stores: `docs/bootstrap/STORE-FORMATS.md`  
Contratos Bárbol: `docs/interop/barbol-*.md`

## Responsabilidades

| Capa | Origen | Destino |
|------|--------|---------|
| STRONG_CONSENSUS | Bárbol domingo | `catalogs/consensus/v…/` |
| EMPIRICAL_ICCF | Bárbol domingo | `catalogs/iccf/v…/` |
| PROVEN_MATE | `builders/mate_repo_update.py` | `catalogs/mate/v…/` |
| THEORETICAL | `builders/theory_repo_update.py` | `catalogs/theoretical/v…/` |

No se usa `barbol_layers_promote.py` (Bárbol publica directo en `catalogs/`).

## Crontab (usuario lab)

Bloque `# certus-sf` en crontab. Bárbol lo gestiona el operador en `chess_idea`.

```cron
# certus-sf — actualización capas (domingo, +15 min vs pipeline evidence legacy)

# MatePath — 04:00 (wget Lichess si cambió + merge hasta 5000)
0 4 * * 0 cd /home/mcebolla/certus-sf && /usr/bin/python3 builders/mate_repo_update.py --max-new 5000 >> train/out/mate_update.log 2>&1

# THEORETICAL — 04:45 (fortresses.epd + seed)
45 4 * * 0 cd /home/mcebolla/certus-sf && /usr/bin/python3 builders/theory_repo_update.py >> train/out/theory_update.log 2>&1

# Bárbol → catalogs/consensus + catalogs/iccf — ~04:15 (configurado en chess_idea)
# 15 4 * * 0 cd /home/mcebolla/chess_idea && ./venv/bin/python tools/ops_certus_consensus_from_barbol.py --certus-sf-export >> ...
```

Desactivar en crontab los jobs equivalentes bajo `# certus` en repo `evidence` (mate/theory/promote).

## Comandos manuales

```bash
cd /home/mcebolla/certus-sf

# Dry-run
python3 builders/mate_repo_update.py --dry-run --max-new 100
python3 builders/theory_repo_update.py --dry-run

# Bulk bootstrap MatePath (una vez; cron semanal sigue en --max-new 5000)
# python3 builders/mate_repo_update.py --skip-download --max-new 2000000 --jobs 32

# EMPIRICAL_ICCF → catalogs/iccf/v…/ (schema v2 + frequent_moves) — desde Bárbol
# cd /home/mcebolla/chess_idea && \
# ./venv/bin/python tools/iccf_pgn_stats_load.py \
#   --pgn-file … --min-date … --min-elo … \
#   --certus-sf-export \
#   --certus-min-n-move 3 \
#   --certus-min-share 0.05 \
#   --certus-max-frequent-moves 8

# Tras actualizar catálogos — smoke motor
make -j -C stockfish/src build ARCH=x86-64-sse41-popcnt
printf 'setoption name EvidencePath value %s/catalogs\nisready\nposition startpos\ngo depth 1\nquit\n' "$PWD" \
  | ./stockfish/src/certus-sf
```

## Dependencias

| Script | Requiere |
|--------|----------|
| `mate_repo_update.py` | `wget`, Python 3, `python-chess` (`builders/requirements.txt`); `builders/mate_build.py` |
| `theory_repo_update.py` | `curl`, Python 3; solo builders locales |
| Bárbol export | `chess_idea` venv, rutas `--certus-sf-export` → `catalogs/` |

## Logs

| Fichero | Job |
|---------|-----|
| `train/out/mate_update.log` | mate cron |
| `train/out/theory_update.log` | theory cron |
| `chess_idea` / Bárbol log | consensus + iccf (operador) |

## Validación post-domingo

```bash
ls -lt catalogs/consensus/ catalogs/iccf/ catalogs/mate/ catalogs/theoretical/
./tests/evidence_probe.sh   # CI fixtures (independiente)
```

Comprobar UCI:

```bash
printf 'setoption name EvidencePath value %s/catalogs\nisready\nquit\n' /home/mcebolla/certus-sf \
  | ./stockfish/src/certus-sf
# → líneas *Path ready version=… entries=…
```
