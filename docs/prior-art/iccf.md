# prior-art — iccf

## FEAT-0009

Runtime `IccfStore` + UCI `IccfPath`. Capa `EMPIRICAL_ICCF` bajo consenso; gates `min_n`/`min_elo`/`min_date`; `hard_evidence=false`.

Builder `builders/iccf_ingest.py` desde export estático de stats.

## Síntesis

- Techo empírico humano (OQ-5): solo ICCF; no ChessDB/OTB/cloud.
- Mismo modelo Syzygy: hay índice → se usa; miss → Inference.
- Certus no llama a Bárbol en runtime.
