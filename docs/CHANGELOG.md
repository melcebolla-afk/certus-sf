# Changelog — certus-sf

## Unreleased

- **Fase 4:** CI GitHub (`.github/workflows/ci.yml`); runbook `docs/runbooks/ci.md`.
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
