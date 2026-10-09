# Plan — FEAT-0008 StrictPreferred Priority/Union

## Alcance
UCI `StrictPreferred` Priority|Union; Strict filter/effort/root force según spec.

## Fases
1. Enum + UCI + `make_search_move_filter` + root force Union (`|P|==1`)
2. Probes + `evidence_probe.sh` + docs UCI/RESOLVER/CHANGELOG
3. Lab: divergencia + RR fuerza vs High/Max/Strict-Priority

## Validación
`./tests/evidence_probe.sh`; CA-1–CA-6 del spec.
