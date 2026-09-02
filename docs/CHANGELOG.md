# Changelog — certus-sf

## Unreleased

- **mate_build / mate_probe:** early-exit mate-in-1, checks-first, `--jobs` paralelo + progreso; `mate_repo_update --jobs` (bootstrap masivo).
- **FEAT-0003b:** Consensus `entries[].wdl` opcional al cargar (default `d`); entradas solo con fen/confidence/marked_moves ya no se descartan.
- **FEAT-0003:** ICCF schema v2 `frequent_moves`; UCI `IccfSearch` default `FreqOnly` — filtra ramas, nunca fuerza bestmove; consenso manda si ambos aplican.
- **FEAT-0002:** `ConsensusSearch` default `MarkedOnly`; filtro `marked_moves` en search; consenso/ICCF no sustituyen eval (NNUE); atajo raíz emite score NNUE.
- **`catalogs/`** EvidenceRoot: layout versionado, builders retarget, cron mate/theory, runbook `docs/runbooks/catalogs-layers.md`. Bárbol → export directo (sin promote).
- **Fase 3:** root STRONG_CONSENSUS → `bestmove` sin search; `info string evidence=` / `marked=`; thinning SoftOnly (interior → NNUE); `evidence_hits` con `EvidenceInfo All`. ResultBias eliminado.
- **Fase 2:** `evidence/resolver.cpp` (TB > Mate > Theory > Consensus > ICCF > NNUE); hook en `Search::Worker::evaluate`; `golden_probe` vs `GOLDEN-FIXTURES.jsonl`.
- **Fase 1:** evidence stores + UCI; binario `certus-sf`; layout merge-friendly (`certus/` overlay).
- **Fase 0:** Stockfish 18 submodule (`sf_18` @ `cb3d4ee9`); build release verde.
- Spec `FEAT-0001-evidence-layers-sf.md`; inventario/guardrails/convenciones actualizados.
- Bootstrap Certus: `docs/bootstrap/`, `builders/` capas, `testdata/` fixtures, interop, prior-art.
- `CURSOR-START-PROMPT.md` — prompt arranque agente.

## 2026-09-01

- Repo inicial + methodology submodule @ v1.4.1 + LAYOUT + adaptador Cursor.
