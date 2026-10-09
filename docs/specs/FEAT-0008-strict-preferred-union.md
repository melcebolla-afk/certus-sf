# Spec — FEAT-0008 Strict preferred: Priority vs Union

## ID
`FEAT-0008-strict-preferred-union`

## Problema
En `CertusStyle=Strict`, el conjunto filtrado/preferred es **marked else frequent**: si hay consenso marked, las estadísticas ICCF (`frequent_moves`) no entran. Para análisis ICCF a veces se quiere que **ambas fuentes cuenten igual** (unión), sin que marked anule frequent.

## Decisión de producto (propuesta)
Nueva UCI combo (nombre provisional):

| Opción | Tipo | Default | Valores |
|--------|------|---------|---------|
| `StrictPreferred` | combo | **Union** | `Priority` \| `Union` |

Solo aplica si `CertusStyle=Strict`. Off/Mixed la ignoran (Mixed sigue con marked-else-frequent como hoy, fuera de este FEAT).

| Valor | Conjunto `P` (∩ legal) |
|-------|-------------------------|
| **Priority** (actual) | marked si no vacío; si no, frequent |
| **Union** | marked ∪ frequent (sin orden de preferencia entre fuentes) |

Capas siguen gated por `ConsensusSearch` / `IccfSearch`:

- `ConsensusSearch=Off` → marked no aporta a `P`
- `IccfSearch=Off` → frequent no aporta a `P`
- Ambos Off → sin filtro Certus (como hoy sin preferred)

## No objetivos
- Cambiar schema de catálogos.
- Unión en Mixed/Max (salvo FEAT futuro).
- Sustituir eval por WDL de consenso/ICCF.
- Preferencia ponderada (pesos) entre fuentes — solo igualdad vía unión.

## Criterios de aceptación
- CA-1: `uci` lista `option name StrictPreferred type combo default Union var Priority var Union`.
- CA-2: Strict + Priority → comportamiento legacy marked-else-frequent (filtro + preferred + atajos raíz).
- CA-3: Strict + Union + MarkedOnly + FreqOnly → en un nodo con marked={a,b} y frequent={b,c}, `P={a,b,c}`; solo se buscan esas; effort (LMR↓/ext) en las tres.
- CA-4: Strict + Union + un solo legal en `P` → atajo raíz force ese movimiento; si `|P|>1` → **no** forzar marked[0]; search entre `P`.
- CA-5: Off/Mixed + cualquier StrictPreferred → sin cambio de search.
- CA-6: probe/CI cubre Priority vs Union en fixture con marked y frequent solapados.

## Lógica Strict completa con `StrictPreferred=Union`

Asumiendo defaults: `ConsensusSearch=MarkedOnly`, `IccfSearch=FreqOnly`, no jaque, `pvIdx=0`.

### Por nodo (search principal)

1. Construir `marked_L` = marked ∩ legal (si ConsensusSearch activo y store ready).
2. Construir `freq_L` = frequent ∩ legal (si IccfSearch activo y store ready).
3. `P = marked_L ∪ freq_L` (sin duplicados; sin ranking marked≻frequent).
4. Si `P` vacío → SF completo en ese nodo (sin restrict, sin effort Certus).
5. Si `P` no vacío:
   - **Filtro:** solo movimientos ∈ `P`.
   - **Effort:** LMR−1 + ext+1 interiores en todo `P` (igual que preferred Strict hoy).
   - **Orden raíz:** preferred-first una vez (partition sobre `P`), no cada ID (Strict ≠ Mixed High).

Excepciones (igual que hoy):

| Situación | Comportamiento |
|-----------|----------------|
| En jaque | Sin filtro / sin effort Certus |
| `pvIdx > 0` (MultiPV) | Sin filtro / sin effort Certus |
| Qsearch | Sin filtro Certus (como hoy) |

### Atajos raíz (`prepare_root_search`)

| Condición | Priority (hoy) | **Union** |
|-----------|----------------|-----------|
| Consenso + marked legales | Force **marked[0]** (aunque haya varios marked) | **No** force solo por ser marked; ver fila siguiente |
| `|P| == 1` | Cubierto por marked[0] o frequent singleton | Force ese único movimiento de `P` |
| `|P| > 1` | Si hubo marked, ya forzó marked[0]; si solo frequent y size>1, search | **Search** entre `P` (sin elegir marked sobre frequent) |
| Solo frequent y size==1 | Force frequent | Force (idem, `|P|==1`) |

Info strings (recomendado): además de `marked=` / `frequent=`, opcional `preferred=` listando `P` en Union.

### Eval / capas
Sin cambio: TB > Mate > Theory; consenso/ICCF no sustituyen cp; SoftOnly thinning igual.

### Tabla resumen Strict

| Aspecto | Priority | Union |
|---------|----------|-------|
| Conjunto filtro/effort | marked else frequent | marked ∪ frequent |
| Preferencia entre fuentes | marked gana | ninguna |
| Force raíz multi-candidato | marked[0] si hay marked | solo si \|P\|=1 |
| Force raíz singleton | sí | sí |
| MultiPV / jaque / qsearch | sin filtro | sin filtro |
| MixedEffort | ignorado | ignorado |

## Riesgos
- Union ensancha el árbol vs Priority → menos “atajo”, más tiempo; divergencia vs SF puede cambiar.
- Force menos agresivo en raíz (ya no marked[0] automático) → distinto Elo/blitz; coherente con “igualdad”.

## Referencias
- FEAT-0002 / 0003 / 0004; `docs/bootstrap/UCI-OPTIONS.md`
- Informe ICCF: `docs/analysis/2026-10-09-iccf-style-strict-vs-mixed.md`
