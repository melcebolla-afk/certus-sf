# Plan — FEAT-0007 MixedEffort=Max

## Fuera de alcance técnico

- Extensión techo 3.
- Default Max.
- Filtrado parcial.
- Mutar mainHistory.

## Fases

### Phase 0 — Docs contrato

- Spec FEAT-0007; UCI-OPTIONS; RESOLVER-SPEC; nota FEAT-0006.
- Validación: Max documentado; default High.

### Phase 1 — Código

- `MixedEffortMode::Max`; parse/register.
- Filter: High|Max MultiPV/interior; mismo preferred marked-else-frequent; Max zero LMR + picker TLS + movepick hook.
- search.cpp: filter antes de MovePicker; repartition High|Max.
- Validación: probes.

### Phase 2 — Tests + cierre

- consensus_search_probe Max; evidence_probe.sh var Max.
- inventory, changelog, QA.

## Validación global

```bash
make -j -C stockfish/src build evidence_probe golden_probe consensus_search_probe ARCH=x86-64-sse41-popcnt
./tests/evidence_probe.sh
```

## Cierre de fase

- Phase 0: **CERRADA** 2026-10-06 — spec + UCI-OPTIONS + RESOLVER-SPEC.
- Phase 1: **CERRADA** 2026-10-06 — Max LMR0, picker bonus, High|Max ID; preferred marked-else-frequent (sin unión).
- Phase 2: **CERRADA** 2026-10-06 — probes + `./tests/evidence_probe.sh: OK`.

## Cierre de la unidad

- Inventario + changelog + QA `docs/qa/FEAT-0007-qa.md`.
- PROJECT_STATE omitido (changelog).
