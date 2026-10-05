# Spec — FEAT-0006 MixedEffort

## ID
`FEAT-0006-mixed-effort-boost`

## Contexto (hechos)

- `CertusStyle=Mixed` no filtra ni fuerza `bestmove`. Solo reorder raíz **una vez** antes del iterative deepening, LMR−1 en preferred, sin extensión interior, sin boost si `pvIdx>0`.
- Uso objetivo: análisis ICCF (MultiPV, think largo), no Elo blitz.
- Mixed Low (comportamiento FEAT-0004) prioriza poco; Strict sí cava el corredor pero excluye jugadas.

## Objetivo

- Dar a Mixed un dial de **esfuerzo** (nunca exclusión): High cava preferred más a fondo en raíz e interiores.
- Conservar Low como Mixed FEAT-0004 para A/B y rollback.
- Default **High**.

## No-objetivos

- Filtro, force raíz, o “Strict light”.
- Sustituir eval por consensus/ICCF.
- Valor Medium / slider numérico.
- Reordenar MovePicker interior.
- Tunear por partidas 1+5.

## Usuario / escenario

- Quién: analista ICCF con `CertusStyle=Mixed`.
- Flujo esperado: High empuja marked/frequent (orden + LMR + ext + MultiPV) sin ocultar tácticas fuera del catálogo. Low = Mixed anterior.

## Requisitos funcionales (RF)

- RF-1: UCI `MixedEffort` combo `Low` | `High`, default **High**.
- RF-2: Solo aplica si `CertusStyle=Mixed`. Off y Strict ignoran el valor.
- RF-3: Ni Low ni High ponen `restrict_moves` ni atajos de raíz.
- RF-4 High: reorder preferred al inicio de **cada** iteración ID; LMR−2 preferred; ext+1 interiores preferred (`extension < 2`); boost también con `pvIdx>0`.
- RF-5 Low: reorder raíz una vez; LMR−1; sin ext; sin boost si `pvIdx>0` (FEAT-0004).
- RF-6 Strict: sin cambios de palanca (LMR−1 + ext + filtro/force; `pvIdx>0` sin filtro).

## Criterios de aceptación

- CA-1: `uci` lista `option name MixedEffort type combo default High`.
- CA-2: Mixed + cualquier MixedEffort permite jugadas no preferred (`allow_search_move`).
- CA-3: Mixed High: preferred interiores LMR−2 y ext+1; boost con `pvIdx>0`.
- CA-4: Mixed Low: LMR−1, sin ext, sin boost `pvIdx>0`.
- CA-5: Strict + MixedEffort=High no cambia filtro ni LMR/ext de Strict.

## Fuera de alcance / no tocar

- Schema de catálogos, NNUE, Off eval path.
