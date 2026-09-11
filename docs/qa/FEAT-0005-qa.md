# QA — FEAT-0005 Stockfish 19 merge

## Matriz por fase

| Fase | Qué se probó | Cómo | Resultado | Pendiente humano |
|------|--------------|------|-----------|------------------|
| 0 | Tag `sf_19` clonable | `git ls-remote` / clone | OK `edb0d9db` | — |
| 1 | Merge árbol | `./scripts/merge-stockfish.sh sf_19` | OK overlay preservado | — |
| 2 | Parches Certus | grep `CERTUS_SF` + build | OK | — |
| 3 | Build + probes + CI script | comandos abajo | OK | — |
| 4 | Docs pin | inventory / UPSTREAM / changelog | OK | — |

## Criterios del spec

| CA / RF | Cubierto | Evidencia |
|---------|----------|-----------|
| CA-1 fuentes `sf_19` | sí | `stockfish/UPSTREAM` tag=sf_19 |
| CA-2 overlay | sí | `src/evidence` + `src/certus` |
| CA-3 build + probes | sí | build + `./tests/evidence_probe.sh: OK` |
| CA-4 docs | sí | inventory + changelog |
| RF-3 UCI identidad | sí | `id name certus-sf dev` + Stockfish 19 |

## Qué se validó / qué no

- Validado: merge, build, evidence/golden/consensus probes, script CI, UCI search (con stdin abierto hasta `bestmove`).
- No validado: fishtest / Elo vs SF18; GUI real ICCF.

## Evidencia ejecutable (gate blando)

- Suite aplicable: sí
- Comandos:

```bash
make -j -C stockfish/src build evidence_probe golden_probe consensus_search_probe ARCH=x86-64-sse41-popcnt
./tests/evidence_probe.sh
```

- Resultado: **ok** (`tests/evidence_probe.sh: OK`; UCI `go depth 6` → `bestmove e2e4`).

Nota: cerrar stdin justo tras `go` aborta la búsqueda en SF19 (también upstream vanilla); no es regresión Certus.

## Complejidad / YAGNI

- ¿Módulo nuevo? no (adaptación API NNUE + `Attacks::init` en probes).
- ¿Por si acaso? no.

## Resumen causal

Stockfish 19 (`sf_19` @ `edb0d9db`) sustituye el árbol vendored `sf_18`. Se reaplicaron los hooks de `TOUCHED_UPSTREAM.md` (Makefile, engine, misc, ucioption, thread, search). La API NNUE pasa de `Networks` (big+small) a una sola `Network`; `certus_eval` y el hook en `search.cpp` se adaptaron. Los probes llaman `Attacks::init()` (SF19 movió init fuera de `Bitboards`). Capas Certus y UCI producto se mantienen.

## Regresión

- Search/eval Certus: probes OK.
- Time-control Certus hooks: re-aplicados; no re-bench de blitz.

## Cierre

- Open questions bloqueantes: ninguna
- Evidencia ejecutable: ok
- Resumen causal validado por humano: pendiente
- `PROJECT_STATE`: omitido (changelog + inventory bastan)
- Inventario actualizado: sí
