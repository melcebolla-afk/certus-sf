# Runbook — CI certus-sf

Workflow: `.github/workflows/ci.yml`

## Qué ejecuta

En cada push/PR a `main`:

```bash
pip install -r builders/requirements.txt
./tests/evidence_probe.sh
```

Incluye:

| Paso | Qué valida |
|------|------------|
| `make build` | Binario `certus-sf` |
| `evidence_probe` | Load/probe stores (`testdata/*`) |
| `golden_probe` | Resolver vs `GOLDEN-FIXTURES.jsonl` (incl. Syzygy3) |
| `test_mate_build.py` | Paridad Python mate_build / idx |
| UCI smoke | Consenso raíz → `marked=` + `bestmove` sin depth |

## Local (misma suite que CI)

```bash
cd /home/mcebolla/certus-sf
pip install -r builders/requirements.txt
./tests/evidence_probe.sh
```

## Fallos habituales

| Síntoma | Causa probable |
|---------|----------------|
| `make: g++: not found` | Instalar `g++ make` |
| `golden_probe: tb_krk_win` skip | Falta `testdata/syzygy3/*.rtbw` en checkout |
| `test_mate_build` ImportError | `pip install -r builders/requirements.txt` |
| UCI smoke sin `marked=` | Regresión `certus_search` o fixture consensus |

## Relacionado

- Capas lab: `docs/runbooks/catalogs-layers.md`
- Merge Stockfish: `docs/runbooks/stockfish-merge.md`
