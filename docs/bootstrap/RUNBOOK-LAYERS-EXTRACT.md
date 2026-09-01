# Runbook extract — capas evidencia (sin train NNUE)

Extracto operativo desde `evidence/docs/runbooks/layers-and-train.md`.  
**Excluido:** self-play, train_tb_seed, PROD_EVAL, orchestrator.

## Premisa cobertura

Syzygy **6-man** siempre en producción. MatePath y Theory **no duplican** ≤6 piezas (TB gana por precedencia).

| Capa | Fuente | Contenido |
|------|--------|-----------|
| PROVEN_TB | Syzygy operador | ≤6 piezas WDL |
| PROVEN_MATE | Lichess puzzles filtrados + runtime ≤5 | ≥7 piezas |
| THEORETICAL | seed curado + fortresses EPD | Fuera TB |
| STRONG_CONSENSUS | Export Bárbol | marked_moves |
| EMPIRICAL_ICCF | Export Bárbol / ingest | Stats ICCF |
| INFERENCE | NNUE Stockfish | — |

## Layout EvidenceRoot

```text
evidence/
  from_barbol/consensus/vYYYY.MM.DD/
  from_barbol/iccf/vYYYY.MM.DD/
  consensus/v…/
  iccf/v…/
  theoretical/v…/
  mate/v…/
```

## Cadena domingo (cron)

| Hora | Script |
|------|--------|
| 04:00 | `mate_repo_update.py` |
| 04:00 | Bárbol → `from_barbol/consensus/` |
| 04:30 | `theory_repo_update.py` |
| 05:00 | `barbol_layers_promote.py` |
| 05:00 | Bárbol → `from_barbol/iccf/` (si aplica) |

## Comandos refresh manual

### Theoretical

```bash
python3 builders/theory_repo_update.py
python3 builders/theory_repo_update.py --skip-download
```

### MatePath

```bash
python3 builders/mate_repo_update.py
```

### Promote staging

```bash
python3 builders/barbol_layers_promote.py --dry-run
python3 builders/barbol_layers_promote.py
```

### Ingest standalone

```bash
python3 builders/consensus_ingest.py --help
python3 builders/iccf_ingest.py --help
```

## UCI paths (certus-sf)

Mismo esquema que Certus:

```text
setoption name SyzygyPath value /path/syzygy3:/path/syzygy6
setoption name ConsensusPath value /path/evidence/consensus/v2026.09.01
setoption name IccfPath value /path/evidence/iccf/v2026.09.01
setoption name TheoreticalPath value /path/evidence/theoretical/v2026.08.29
setoption name MatePath value /path/evidence/mate/v2026.09.01
setoption name EvidencePath value /path/evidence
setoption name EvidenceInfo value Root
setoption name ResultBias value Neutral
```

## Validación smoke

1. `isready` → info strings `*Path ready`.
2. FEN fixture → `info string evidence=…` en root.
3. Consensus FEN → bestmove marked si aplica.

## Sync desde repo Certus (evidence)

Si capas se generan en c5/evidence repo:

```bash
rsync -av /home/mcebolla/evidence/evidence/consensus/ ./evidence/consensus/
rsync -av /home/mcebolla/evidence/evidence/iccf/       ./evidence/iccf/
```

## Referencias

- Interop: `docs/interop/barbol-*.md`
- Guardrails datos: adaptar `docs/project/guardrails.md` (no destructivo en evidence/)
