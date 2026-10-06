# QA — FEAT-0007 MixedEffort=Max

## Matriz por fase

| Fase | Qué se probó | Cómo | Resultado | Pendiente humano |
|------|--------------|------|-----------|------------------|
| 0 | Contrato Max + default High | spec / UCI-OPTIONS | OK | — |
| 1 | LMR0, mismo preferred que High, picker bonus, Strict ignora | `consensus_search_probe` | OK | — |
| 2 | UCI var Max + CI | `./tests/evidence_probe.sh` | OK | A/B ICCF High vs Max en GUI |

## Criterios del spec

| CA / RF | Cubierto | Evidencia |
|---------|----------|-----------|
| CA-1 default High + var Max | sí | UCI line MixedEffort |
| CA-2 Max no filtra | sí | probe allow a3 / italian a3 |
| CA-3 Max LMR0 + ext + MultiPV | sí | probe |
| CA-4 mismo preferred italian | sí | High=Max=d4 only (no bc4) |
| CA-5 Strict ignora Max | sí | probe zero_lmr false |

## Qué se validó / qué no

- Validado: probes unitarios + smoke UCI CI.
- No validado: partidas / nodos fijos MultiPV en GUI.

## Evidencia ejecutable (gate blando)

- Suite: sí — `./tests/evidence_probe.sh`
- Resultado: **ok** (`tests/evidence_probe.sh: OK`; `consensus_search_probe: all tests passed`)

## Complejidad / YAGNI

- Valor UCI Max + hook `movepick.cpp` nombrados en plan.
- Sin extensión techo 3.

## Resumen causal

Max cava el mismo preferred que High (marked else frequent) con LMR≈0 + bonus MovePicker. Default High; Max opt-in ICCF.

## Cierre

- Estado: **CERRADA** 2026-10-06
- inventory + CHANGELOG actualizados; PROJECT_STATE omitido.
