# prior-art — consensus

## FEAT-0008

Runtime `ConsensusStore` (HashMap FEN → WDL + confidence), UCI `ConsensusPath`, precedencia bajo Theoretical.

Builder `builders/consensus_ingest.py`: ingesta export **barbol-lite** (oráculos DONE `ab`+`mcts` con acuerdo WDL) → `catalog.json` + `manifest.json`.

## Síntesis

- OQ-18 Bárbol-first en el **productor** del índice; Certus solo lee.
- **Decisión 2026-08-29:** Certus no llama a Bárbol ni a UCI externos para huecos (mismo modelo que Syzygy: hay tablas/índice → se usa; si no → miss).
- No se trata como `PROVEN_*`.
