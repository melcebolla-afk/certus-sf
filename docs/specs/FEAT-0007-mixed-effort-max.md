# Spec — FEAT-0007 MixedEffort=Max

## ID
`FEAT-0007-mixed-effort-max`

## Contexto (hechos)

- FEAT-0006: `MixedEffort` Low | High (default High). High = reorder cada ID, LMR−2, ext+1, boost MultiPV; preferred = marked else frequent.
- Uso: análisis ICCF. Max es opt-in encima de High, sin filtro ni force.

## Objetivo

- Añadir `MixedEffort=Max`: LMR≈0 en preferred, preferred = marked ∪ frequent, bonus de orden en MovePicker (+8000).
- Default sigue **High**.
- Sin techo de extensión 3.

## No-objetivos

- Filtrar / forzar bestmove.
- Mutar `mainHistory`.
- Cambiar score UCI.
- Default Max.
- Extensión techo 3.

## Requisitos funcionales

- RF-1: UCI `MixedEffort` combo `Low` | `High` | `Max`, default **High**.
- RF-2: Max hereda High (reorder cada ID, ext+1 `extension < 2`, boost `pvIdx>0`, sin restrict).
- RF-3: Max → `reductionUnits = 0` en preferred tras bias.
- RF-4: Max preferred = unión legal marked ∪ frequent (dedupe).
- RF-5: Max → bonus +8000 en `MovePicker::score` para preferred (TLS; no history tables).
- RF-6: Low/High/Strict sin cambios de contrato FEAT-0006 salvo que el combo liste Max.

## Criterios de aceptación

- CA-1: `uci` lista `MixedEffort` default High con var Max.
- CA-2: Max no `restrict_moves`; permite no-preferred.
- CA-3: Max preferred LMR→0 + ext+1; boost `pvIdx>0`.
- CA-4: En posición con marked={d4} y frequent⊃{bc4,d4,nc3}, Max prefiere las tres; High solo d4.
- CA-5: Strict ignora Max (filtro + LMR−1).

## Fuera de alcance

- Schema catálogos, NNUE, Off eval.
