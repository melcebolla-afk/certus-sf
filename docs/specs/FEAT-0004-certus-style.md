# Spec — FEAT-0004 CertusStyle

## ID
`FEAT-0004-certus-style`

## Problema
Un solo sabor de search Certus (filtro duro + atajos raíz) no sirve igual para correspondencia agresiva y para uso “SF con sesgo” (p. ej. ICCF blitz/análisis). Hace falta un interruptor de estilo.

## Decisión de producto (2026-09-09)
UCI `CertusStyle`: `Off` | `Mixed` | `Strict`. **Default: `Mixed`.**

| Modo | Search | Atajos raíz force | Eval evidencia (TB/mate/theory) |
|------|--------|-------------------|----------------------------------|
| **Off** | SF puro | No | No (NNUE SF directo) |
| **Mixed** | Todas legales; **orden + effort en raíz** hacia marked/frequent; sin profundidad Certus en interiores | No | Sí (como ahora, SoftOnly thinning) |
| **Strict** | Filtro MarkedOnly/FreqOnly (como hoy vía `ConsensusSearch`/`IccfSearch`) + orden/effort raíz + **menos LMR / +extensión** en preferred | Sí (consenso / ICCF singleton) | Sí |

`ConsensusSearch` / `IccfSearch` solo aplican filtro cuando `CertusStyle=Strict`.

## No objetivos
- Fine-tune NNUE.
- Sustituir eval por WDL consensus/ICCF.
- Cambiar schema de catálogos.

## Criterios de aceptación
- CA-1: `uci` lista `CertusStyle` default `Mixed`.
- CA-2: `Off` → `allow_search_move` no bloquea; `prepare_root_search` no fuerza.
- CA-3: `Strict` + MarkedOnly → filtro como FEAT-0002.
- CA-4: `Mixed` → no filtra; preferred en raíz reordenadas primero.
