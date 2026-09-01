# Prompt Cursor — certus-sf (copiar y pegar)

Abre un **chat nuevo** en el workspace `/home/mcebolla/certus-sf` y pega **todo el bloque** entre las líneas `---` (inclusive el texto, sin este encabezado).

---

Eres el agente de **desarrollo de producto** del repo **certus-sf** (`/home/mcebolla/certus-sf`).

## Misión

Fork de **Stockfish** (GPL-3) + **capas de evidencia Certus** (sin reentrenar NNUE). Producto: motor UCI creíble para **ICCF / correspondencia** — evaluación por certeza, no solo `cp`.

**Fuera de alcance:** train NNUE, self-play Certus, orchestrator c1, port del motor Rust completo, TRAIN-*.

## Estado del repo (ya hecho)

| Qué | Dónde |
|-----|--------|
| Methodology submodule | `methodology/` — leer `INDEX.md`, `rhythm.md`, `prompts.md` |
| Cursor rules | `.cursor/rules/` |
| Bootstrap Certus | `docs/bootstrap/` (EVIDENCE-PORT, RESOLVER-SPEC, STORE-FORMATS, UCI-OPTIONS, GOLDEN-FIXTURES.jsonl) |
| Builders capas | `builders/` → salida en `catalogs/` (ver `docs/project/builders.md`) |
| EvidenceRoot lab | `catalogs/` — Bárbol export directo consensus/iccf; cron mate/theory |
| Fixtures CI | `testdata/{consensus,iccf,theoretical,mate,syzygy3}/` |
| Interop Bárbol | `docs/interop/` |
| Inventario borrador | `docs/project/inventory.md` |

**Pendiente:** código Stockfish en el repo (Fase 0).

## Referencia oráculo (Rust — no copiar search)

Repo hermano: **`/home/mcebolla/evidence`** (remoto `https://github.com/melcebolla-afk/evidence.git`).

Portar diseño desde:

- `crates/evidence-engine/src/resolver.rs` — precedencia, thinning, hard_evidence
- `consensus.rs`, `iccf.rs`, `theoretical.rs`, `mate_index.rs` — load/probe
- `uci.rs` — opciones + info strings

Datos lab: **`/home/mcebolla/certus-sf/catalogs/`** (EvidenceRoot UCI). Oráculo Rust: `/home/mcebolla/evidence` (solo referencia de paridad).

## Capas

```text
PROVEN_TB        → SF Syzygy (YA) — no duplicar
PROVEN_MATE      → PORTAR MatePath (catalog.json + catalog.idx)
THEORETICAL      → PORTAR
STRONG_CONSENSUS → PORTAR + root marked bestmove
EMPIRICAL_ICCF   → PORTAR (hard_evidence=false)
INFERENCE        → NNUE SF (YA)
```

Precedencia: `TB > MATE > THEORY > CONSENSUS > ICCF > NNUE`

Leer **`docs/bootstrap/RESOLVER-SPEC.md`** y **`docs/bootstrap/EVIDENCE-PORT.md`** antes de codear.

## Metodología

- Ritmo: `methodology/rhythm.md` — completo si no trivial; carril corto si ≤3 ficheros sin contrato.
- Plans: `docs/plans/FEAT-####-plan.md`
- Specs: `docs/specs/`
- Actualizar `docs/project/inventory.md` al cerrar unidades.
- **No editar** `methodology/` (submodule).

## Plan — ejecutar en orden

### Fase 0 — Stockfish baseline (AHORA)

1. Añadir Stockfish oficial como base:
   - Opción A: fork git subtree en raíz o `src/`
   - Opción B: submodule `stockfish/` apuntando a `official-stockfish/Stockfish`
2. Compilar release (`make build ARCH=x86-64-modern` o equivalente).
3. Verificar binario ≈ upstream (bench/`go depth 1`).
4. Commit: `baseline: stockfish @<tag>` — **sin cambios de eval**.
5. Actualizar `docs/project/inventory.md` con layout real + tag SF.

### Fase 1 — UCI + stores (sin eval en search)

- Opciones UCI: ver `docs/bootstrap/UCI-OPTIONS.md`
- C++ `src/evidence/`: load `catalog.json`, MatePath `catalog.idx` (mmap)
- Key = placement + STM (como Certus)
- Tests: probe hit/miss con `testdata/*`

### Fase 2 — Una capa en eval

- Hook eval SF antes de NNUE; primera capa ICCF o Consensus
- Golden: `docs/bootstrap/GOLDEN-FIXTURES.jsonl`

### Fase 3 — Resolver completo + search hooks

- Cadena completa, thinning HardOnly, root consensus marked, EvidenceInfo, ResultBias

### Fase 4 — Ops + CI

- Builders ya en `builders/`; documentar refresh en `docs/runbooks/`

## Guardrails

- GPL-3.0; NOTICE Stockfish + Certus (ver `docs/bootstrap/LICENSE-NOTICE.md`)
- Runtime sin MariaDB/API Bárbol
- No ChessDB/Lichess cloud/OTB como capa
- Mate/Theory: no duplicar finales ≤6 piezas (Syzygy 6-man)
- **No** mezclar train NNUE de evidence en este repo

## Entregables **esta sesión**

1. Leer `docs/bootstrap/EVIDENCE-PORT.md` + `RESOLVER-SPEC.md`; resumir plan en 5 líneas.
2. **Fase 0 completa:** SF en repo + build verde.
3. `docs/specs/FEAT-0001-evidence-layers-sf.md` (alcance v1 corto).
4. Completar `docs/project/inventory.md` y `docs/project/conventions.md` (C++, SF, builders).
5. Esbozo `docs/project/guardrails.md` (paths evidence, no destructivo).

## Comandos útiles

```bash
cd /home/mcebolla/certus-sf
# Tras Fase 0:
make -j -C src build ARCH=x86-64-modern   # ajustar path según layout

# Fixtures (sin motor aún):
ls testdata/consensus/catalog.json
python3 builders/barbol_layers_promote.py --dry-run
```

## Nombre UCI

Usar **`id name certus-sf dev`** hasta decisión producto (Certus reservado al Rust).

Empieza leyendo bootstrap, confirma el plan, e **implementa Fase 0** (traer Stockfish y compilar). No saltes a capas evidencia hasta baseline verde.

---
