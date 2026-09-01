# Spec — FEAT-0002 consensus marked search

## Nombre

`FEAT-0002-consensus-marked-search`

## Contexto (hechos)

- **FEAT-0001 cerrado:** capas evidencia, resolver, root `marked` → `bestmove` sin search, `SoftOnly` en interiores quiet.
- Hoy, en nodos `Full`, hit `STRONG_CONSENSUS` **sustituye la eval estática** por WDL→utility (3 bandas: win/draw/loss).
- El árbol **siempre** expande todas las jugadas legales salvo el atajo de raíz.
- Discusión producto (2026-09-02): el WDL coarse del consenso puede **distorsionar** podas/orden frente a NNUE (+0.01 entre candidatas); el valor del consenso es **acotar variantes** (`marked_moves`), no reemplazar cp.

## Objetivo

- Separar roles: **consenso = filtro de ramas**; **NNUE = eval numérica** en búsqueda.
- Opción UCI `ConsensusSearch`: `Off` | `MarkedOnly`.
- En `MarkedOnly`, en nodos con hit de consenso y `marked_moves` no vacío, el minimax solo expande **marked ∩ legal**.
- **No** usar WDL del consenso como eval estática durante search (salvo capas duras que ya ganan precedencia: TB, mate, theory).

## No-objetivos

- Cambiar builders Bárbol ni formato `catalog.json`.
- Aplicar filtro marked a ICCF (solo `STRONG_CONSENSUS`).
- Sustituir NNUE por consenso WDL (rechazo explícito del “Nivel 2” para consenso).
- MultiPV / análisis paralelo completo (v1: comportamiento definido para `MultiPV=1`; MultiPV>1 → ver RF-7).
- Train NNUE, contempt/ResultBias (eliminado en FEAT-0001).

## Usuario / escenario

- **Quién:** operador ICCF / análisis con `EvidencePath` o `ConsensusPath` cargado.
- **Flujo actual:** consenso sesga eval estática o corta solo en raíz; muchas ramas “no humanas” se exploran igual.
- **Flujo esperado:** en posiciones con consenso fuerte, el motor **solo profundiza líneas marcadas**; la valoración sigue siendo la de Stockfish (NNUE) en esas líneas.

### Ejemplo

Posición **X** tras `P → x`; catálogo en X: `STRONG_CONSENSUS`, `marked_moves = [y, z]`.

| `ConsensusSearch` | Expansión desde X | Eval en X y bajo y/z |
|-------------------|-------------------|----------------------|
| `Off` | todas legales | NNUE (consenso no sustituye eval) |
| `MarkedOnly` | solo **y** y **z** si legales | NNUE (TB/mate/theory si ganan antes) |

Si solo **y** está marcada y es legal → una rama. El árbol **sigue** bajo **y** (no se detiene en y).

## Requisitos funcionales (RF)

### Opción UCI

- **RF-1:** `ConsensusSearch` combo `Off | MarkedOnly`, default **`Off`** (paridad con comportamiento actual salvo retirar WDL-eval de consenso — ver RF-4).
- **RF-2:** Declarar en `uci`; `setoption` case-insensitive; inválido → warning, sin cambio.

### Evaluación (search)

- **RF-3:** En `Certus::evaluate`, si la capa ganante sería **solo** `STRONG_CONSENSUS` (sin TB/mate/theory antes en precedencia), **no** devolver `ev.to_value()` del consenso → fallback **NNUE** (`Inference`).
- **RF-4:** TB, `PROVEN_MATE`, `THEORETICAL` siguen sustituyendo eval cuando ganan precedencia (sin cambio de FEAT-0001).
- **RF-5:** `EMPIRICAL_ICCF` no sustituye eval en search (igual que consenso tras este FEAT).

### Movegen (MarkedOnly)

- **RF-6:** Con `ConsensusSearch=MarkedOnly`, antes de iterar candidatas en `search()` / `qsearch()` (o hook equivalente), si `probe_consensus(pos)` devuelve entrada con `marked_moves` no vacío:
  - construir lista **legal_marked** (misma lógica que `certus_search.cpp` / raíz);
  - si `legal_marked` no vacío → **solo** esas jugadas como candidatas a expandir en ese nodo;
  - si vacío (marked ilegales) → **fallback** todas las legales (log opcional `marked-miss`).
- **RF-7:** `MultiPV>1`: v1 documentar **Off efectivo para filtro** o filtro solo en PV principal — decisión en plan; no bloquear CA con MultiPV=1.
- **RF-8:** Probe de consenso para filtro **independiente** de `SoftOnly` eval: hace falta consultar catálogo al expandir aunque eval interior sea NNUE-only.

### Raíz (FEAT-0010)

- **RF-9:** Atajo raíz (`prepare_root_search`: consenso + marked → `bestmove` sin search) **se mantiene** independiente de `ConsensusSearch` (producto correspondencia).
- **RF-10:** Opcional v1: si `ConsensusSearch=Off`, mantener atajo raíz; si en futuro `ConsensusSearch=MarkedOnly` sin atajo raíz, escalar a humano (no bloqueante v1).

### Telemetría

- **RF-11:** `EvidenceInfo` sin cambio: `evidence=` / `marked=` en raíz; `evidence_hits` en `All` si se registra probe (no requiere WDL-eval).

## Requisitos no funcionales (RNF)

- **RNF-1:** Regresión: `./tests/evidence_probe.sh` verde; golden resolver sin cambio de precedencia de **clase** en probes directos.
- **RNF-2:** Sin MariaDB/API Bárbol en runtime.
- **RNF-3:** Hook mínimo en `search.cpp` (preferir función en `certus_search.cpp`).

## Salvaguardas (normativas)

| ID | Regla |
|----|--------|
| S-1 | Sin `marked_moves` → no filtrar (todas legales). |
| S-2 | `marked_moves` sin legal → fallback todas legales. |
| S-3 | **En jaque (`in_check`)** → no filtrar marked (todas legales). Motivo: defensas tácticas no siempre en catálogo. |
| S-4 | Capturas / qsearch: v1 **no filtrar** en qsearch (solo `search()` principal quiet/non-quiet según plan). |
| S-5 | Catálogo no cargado o miss → todas legales. |

## Criterios de aceptación

- **CA-1:** `uci` lista `ConsensusSearch` default `Off`.
- **CA-2:** Fixture `testdata/consensus`, FEN con 2 marked legales, `ConsensusSearch=MarkedOnly`, `go depth 4`: árbol no elige jugada **no marcada** en ese nodo (test instrumentado: contador de expansiones o posición donde solo marked son razonables — ver plan).
- **CA-3:** Misma FEN, `ConsensusSearch=Off`: todas legales exploradas (o equivalente a SF sin filtro).
- **CA-4:** Nodo con solo hit consenso (sin TB/mate/theory): eval estática coincide con NNUE (± tolerancia 0), no WDL utility.
- **CA-5:** TB/mate/theory fixtures siguen sustituyendo eval cuando aplican.
- **CA-6:** Raíz consenso + marked → `bestmove` primer marked sin `info depth` (regresión FEAT-0010).
- **CA-7:** Entrada en `GOLDEN-FIXTURES.jsonl` o test C++ para “consenso no altera utility en eval hook”.
- **CA-8:** Inventario + CHANGELOG actualizados.

## Compatibilidad / migración

- **Breaking:** con default `Off`, tras RF-3 el comportamiento **cambia** respecto a FEAT-0001: consenso ya no sesga eval estática (mejora alineada con producto). Documentar en CHANGELOG.
- Operadores que querían WDL-eval de consenso: no hay opción v1 (rechazado); solo marked filtro + NNUE.

## Dudas abiertas

| ID | Pregunta | Propuesta v1 |
|----|----------|--------------|
| OQ-1 | ¿Filtro en PV sola o todos los nodos con hit? | Todos los nodos con hit+marked (salvo S-3/S-4). |
| OQ-2 | ¿Umbral `min_confidence` del catálogo para filtrar? | Usar gates ya en `ConsensusStore` (si miss, no filtrar). |
| OQ-3 | `MultiPV>1` | Filtro solo `MultiPV` línea 1 o desactivar filtro. |
| OQ-4 | ¿Fix UCI `EvidenceInfo` combo (`var Off`…)? | Carril paralelo / bugfix; no bloquea FEAT-0002. |

## Aterrizaje en el repo

| Área | Ficheros |
|------|----------|
| UCI | `evidence/evidence_manager.cpp` (`ConsensusSearch`) |
| Eval | `certus/certus_eval.cpp` — skip consensus `to_value()` |
| Search | `certus/certus_search.cpp` — `legal_marked`, filtro candidatas |
| Hook | `search.cpp` — llamada antes del bucle de moves (y salvaguarda jaque) |
| Tests | `tests/` o extensión `golden_probe`; fixture FEN consenso |
| Docs | `docs/bootstrap/UCI-OPTIONS.md`, `RESOLVER-SPEC.md` § consenso search |

Consultar `docs/project/inventory.md`.

## Entregables

- [ ] `docs/analysis/FEAT-0002-consensus-marked-search-analysis.md` (opcional corto)
- [ ] `docs/plans/FEAT-0002-plan.md`
- [ ] Implementación + tests
- [ ] `docs/qa/FEAT-0002-qa.md` al cerrar

## Referencias

- FEAT-0010 root marked — `certus_search.cpp::prepare_root_search`
- `docs/bootstrap/RESOLVER-SPEC.md` — precedencia; actualizar § consenso eval vs search
- `testdata/consensus/` — FEN e4 e5, marked `b1c3`, `g1f3`
