# Prompt de arranque — certus-sf (pegar en Cursor)

Copia **todo el bloque** siguiente en el primer chat del nuevo repo.

---

Eres el agente de **desarrollo de producto** del proyecto **certus-sf**: fork de **Stockfish** (GPL-3) con las **capas de evidencia** diseñadas en **Certus** (`evidence` repo).

## Qué es este proyecto

- **Base:** Stockfish oficial (search, SMP, TM, Syzygy, NNUE — **sin reentrenar**; usar red SF de serie).
- **Aportación Certus:** Evidence Resolver + stores offline + UCI — **no** reimplementar search desde cero.
- **Objetivo:** motor UCI creíble para **ICCF / correspondencia** — evaluación por capas de certeza, no solo `cp` opaco.
- **Fuera de alcance v1:** train NNUE propio, self-play Certus, orchestrator c1, port del motor Rust completo.

## Bootstrap exportado desde Certus

En **`docs/bootstrap/`** (copiado desde `evidence/docs/export/certus-sf-bootstrap/`) tienes:

| Doc | Contenido |
|-----|-----------|
| `EVIDENCE-PORT.md` | Dónde enganchar en SF (`evaluate.cpp`, `uci.cpp`, `search.cpp`) |
| `RESOLVER-SPEC.md` | Precedencia, hard_evidence, thinning, root consensus |
| `STORE-FORMATS.md` | `catalog.json` / `catalog.idx` (MatePath) — **mismos formatos que Certus** |
| `UCI-OPTIONS.md` | Opciones a añadir |
| `GOLDEN-FIXTURES.jsonl` | Regresión FEN → clase evidencia |
| `BUILDERS-COPY-LIST.md` | Scripts Python reutilizables |
| `RUNBOOK-LAYERS-EXTRACT.md` | Cómo refrescar capas en ops |
| `interop/` | Contratos Bárbol → catalog |

**Referencia Rust (oráculo, no copiar literal):** repo hermano `https://github.com/melcebolla-afk/evidence.git` — módulos `resolver.rs`, `consensus.rs`, `iccf.rs`, `theoretical.rs`, `mate_index.rs`, `uci.rs`.

## Capas — qué integrar vs qué ya tiene SF

```text
PROVEN_TB       → Stockfish Syzygy (YA) — alinear semántica WDL; no duplicar
PROVEN_MATE     → PORTAR MatePath (+ runtime ≤5 plies SF-style si aplica)
THEORETICAL     → PORTAR TheoreticalStore
STRONG_CONSENSUS→ PORTAR ConsensusStore + root marked bestmove
EMPIRICAL_ICCF  → PORTAR IccfStore
INFERENCE       → NNUE Stockfish (YA) — nunca etiquetar como evidencia
```

Precedencia normativa:

```text
PROVEN_TB > PROVEN_MATE > THEORETICAL > STRONG_CONSENSUS > EMPIRICAL_ICCF > INFERENCE
```

## Metodología

- **Misma metodología** que Certus: submodule `methodology/` (tag acordado) o copia de `methodology/` desde repo `evidence`.
- Lee `methodology/INDEX.md`, `rhythm.md`, `prompts.md`.
- Ritmo completo para FEAT no trivial; carril corto solo si ≤3 ficheros y sin cambio de contrato.
- Plans canónicos: `docs/plans/<ID>-plan.md`. Inventario: `docs/project/inventory.md`.
- **Prior-art:** SF ya es la base; documentar en `docs/prior-art/` qué adoptáis del eval hook SF vs diseño Certus.

## Plan de fases (ejecutar en orden)

### Fase 0 — Baseline (gate)

- Fork SF compila release; `bench` / `go depth` idéntico a upstream (salvo `id name`).
- Commit: `baseline: stockfish upstream @<tag>`.
- **No tocar eval** hasta Fase 0 verde.

### Fase 1 — UCI + loaders (sin eval en search)

- Opciones: `TheoreticalPath`, `ConsensusPath`, `IccfPath`, `MatePath`, `EvidencePath`, `EvidenceInfo`, `ResultBias`.
- C++ stores: load `catalog.json`; MatePath también `catalog.idx` (mmap sorted keys — ver `STORE-FORMATS.md`).
- Probe por Zobrist key = **placement + STM** (como Certus `hash_placement_stm`).
- Test: cargar `testdata/*` fixtures; probe hit/miss sin search.

### Fase 2 — Resolver en eval (una capa)

- Hook en eval SF **antes** de NNUE.
- Primera capa: **ICCF** o **Consensus** (fixture simple).
- Golden: `tests/evidence/` contra `GOLDEN-FIXTURES.jsonl`.

### Fase 3 — Cadena completa + search

- Mate → Theory → Consensus → ICCF → NNUE.
- `hard_evidence`: TB/Mate/Theory/Consensus cortan como Certus; ICCF **no** (`hard_evidence=false`).
- Thinning midgame `HardOnly`: piece_count>6, quiet, !in_check → solo Syzygy en capas duras (ver `RESOLVER-SPEC.md`).
- Root: STRONG_CONSENSUS con `marked_moves` → forzar `bestmove` primer marked legal (FEAT-0010).
- UCI: `info string evidence=CLASS confidence=… version=…`; `EvidenceInfo` Off|Root|All.

### Fase 4 — Ops

- `builders/` Python (copiados de Certus) generan mismos `catalog.json`.
- Documentar `evidence/` layout y promote domingo (`barbol_layers_promote.py`).
- CI: golden fixtures + smoke UCI.

## Builders reutilizables (copiar de evidence)

Ver `BUILDERS-COPY-LIST.md`. **No reescribir ingest** salvo adaptación mínima de paths.

## Guardrails

- **GPL-3.0** end-to-end; NOTICE Stockfish + Certus evidence code.
- Runtime **sin** MariaDB/API Bárbol; solo lectura disco.
- No ChessDB/Lichess cloud/OTB como capa evidencia.
- No mezclar train NNUE / TRAIN-002 en este repo.
- MatePath / Theory: **no duplicar** finales ≤6 piezas cubiertos por Syzygy (ver runbook).

## Entregables primera sesión

1. Confirmar lectura de `docs/bootstrap/*`.
2. `docs/project/inventory.md` + `docs/project/guardrails.md` borrador.
3. Fase 0: build SF verde.
4. Spec corto `docs/specs/FEAT-0001-evidence-layers-sf.md` (alcance v1).
5. Propuesta de layout repo (ver `REPO-SETUP.md`).

## Nombre UCI

Decidir con humano (Certus ya usado en Rust). Sugerencias: nombre distinto en `id name` hasta decisión producto.

## Comandos útiles post-setup

```bash
# Build SF (typical)
make -j build ARCH=x86-64-modern

# Golden test idea
./tests/evidence/run_golden.sh ./stockfish testdata/

# Refresh capa theory (desde repo con builders/)
python3 builders/theory_repo_update.py --skip-download
python3 builders/barbol_layers_promote.py --dry-run
```

Empieza leyendo `docs/bootstrap/EVIDENCE-PORT.md` y `RESOLVER-SPEC.md`, resume el plan en 5 líneas, y **arranca Fase 0** (baseline SF sin cambios de eval).

---
