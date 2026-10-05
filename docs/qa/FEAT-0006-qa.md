# QA — FEAT-0006 MixedEffort

## Matriz por fase

| Fase | Qué se probó | Cómo | Resultado | Pendiente humano |
|------|--------------|------|-----------|------------------|
| 0 | Contrato UCI default High | spec + UCI-OPTIONS | OK | — |
| 1 | Palancas Low/High vs Strict | `consensus_search_probe` | OK | — |
| 2 | UCI + CI smoke | `./tests/evidence_probe.sh` | OK | A/B análisis ICCF GUI |

## Criterios del spec

| CA / RF | Cubierto | Evidencia |
|---------|----------|-----------|
| CA-1 MixedEffort default High | sí | `option name MixedEffort type combo default High` |
| CA-2 Mixed no filtra | sí | probe High/Low allow a3 |
| CA-3 High LMR-2 + ext + MultiPV | sí | probe |
| CA-4 Low = FEAT-0004 | sí | probe |
| CA-5 Strict ignora High | sí | probe LMR-1 + restrict |

## Qué se validó / qué no

- Validado: probes, UCI lista, Strict force smoke del script CI.
- No validado: posiciones ICCF reales MultiPV a nodos fijos (humano en GUI).

## Evidencia ejecutable (gate blando)

- Suite aplicable: sí
- Comando: `./tests/evidence_probe.sh`
- Resultado: **ok** (`tests/evidence_probe.sh: OK`; `consensus_search_probe: all tests passed`)

## Complejidad / YAGNI

- Flag UCI `MixedEffort` nombrada en spec/plan.
- `certus.mk`: reglas de `.o` de probes al nombre plano que usa el linker (CI).

## Resumen causal

Mixed FEAT-0004 casi no priorizaba preferred. High cava más (orden cada ID, LMR−2, ext, MultiPV) **sin** excluir jugadas. Low restaura el Mixed anterior. Default High para análisis ICCF.

## Cierre

- Estado: **CERRADA** 2026-10-05
- inventory + CHANGELOG actualizados; PROJECT_STATE omitido.
