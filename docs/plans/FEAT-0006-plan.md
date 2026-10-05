# Plan — FEAT-0006 MixedEffort

## Fuera de alcance técnico (no construir)

- Filtrado parcial / force condicional.
- Medium o slider.
- Reordenar MovePicker interior.
- Tunear por blitz.

## Fases

### Phase 0 — Contrato docs

- Spec FEAT-0006; UCI-OPTIONS; RESOLVER-SPEC; nota FEAT-0004.
- Validación: default High documentado; Low = FEAT-0004.

### Phase 1 — Manager + search

- `MixedEffortMode` en evidence manager; parse/register UCI.
- `SearchMoveFilter::lmr_relief`; Mixed High: interior_depth, LMR 2048, boost MultiPV.
- `search.cpp`: partition preferred cada ID si Mixed High; Low/Strict una vez.
- Validación: probes.

### Phase 2 — Tests + cierre

- `consensus_search_probe` Low vs High; `evidence_probe.sh` grep default High.
- inventory, changelog, QA.
- Validación: `./tests/evidence_probe.sh`

## Validación global

```bash
make -j -C stockfish/src build evidence_probe golden_probe consensus_search_probe ARCH=x86-64-sse41-popcnt
./tests/evidence_probe.sh
```

## Cierre de fase

- Phase 0: **CERRADA** 2026-10-05 — spec + UCI-OPTIONS + RESOLVER-SPEC.
- Phase 1: **CERRADA** 2026-10-05 — manager, filter, search partition High.
- Phase 2: **CERRADA** 2026-10-05 — probes + `./tests/evidence_probe.sh: OK`.

## Cierre de la unidad

- Inventario + changelog + QA `docs/qa/FEAT-0006-qa.md`.
- PROJECT_STATE omitido (salida humana vía changelog).
