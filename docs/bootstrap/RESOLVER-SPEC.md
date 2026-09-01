# Evidence Resolver — spec normativa (desde Certus)

Fuente canónica Rust: `crates/evidence-engine/src/resolver.rs`  
Producto completo: `docs/PRODUCT-SPEC.md` §6 (extract en `PRODUCT-EVIDENCE-EXTRACT.md`).

## Taxonomía

| Clase | Código UCI | hard_evidence | Notas |
|-------|------------|---------------|-------|
| Tablebase | `PROVEN_TB` | **true** | Syzygy WDL — SF nativo |
| Mate verificado | `PROVEN_MATE` | **true** | Índice offline + ≤5 runtime |
| Teoría | `THEORETICAL` | **true** | Catálogo curado W/D/L |
| Consenso | `STRONG_CONSENSUS` | **true** | Multi-arquitectura; marked_moves |
| ICCF | `EMPIRICAL_ICCF` | **false** | Empírico; techo humano, no prueba |
| Inferencia | `INFERENCE` | false | NNUE SF — **nunca** reportar como evidencia |

## Precedencia (estricta)

```text
PROVEN_TB > PROVEN_MATE > THEORETICAL > STRONG_CONSENSUS > EMPIRICAL_ICCF > INFERENCE
```

- Primera capa aplicable **gana**; no promediar capas.
- Misma capa: reglas documentadas en ingest (consenso: min_confidence).

## Probe modes

### Full (default root, quiesce, soft eval)

`hard_layers=true`, `soft_layers=true`:

1. TB (si piezas ≤ max TB)
2. Mate index (+ exhaustivo ≤5 si configurado)
3. Theoretical
4. Consensus (confidence ≥ min)
5. ICCF (gates min_n, min_elo, min_date, min_confidence)
6. NNUE

### HardOnly thin (interior midgame quiet)

Condición Certus:

```text
!soft_layers && piece_count > 6 && !in_check
```

Solo **Syzygy** en capas duras; si miss → utility 0 / Inference sin ICCF/NNUE en ese probe específico.

Ver test: `hard_only_thin_midgame_skips_consensus`.

### SoftOnly fast path

Certus interior: solo NNUE (sin capas). SF ya optimiza eval NNUE — equivalente: skip evidence probe en nodos interiores **salvo** hard cutoffs.

**Decisión certus-sf (fork SF):** interiores quiet midgame → `SoftOnly` (NNUE directo). `HardOnly` queda en el resolver para tests; no se usa en search.

## Root STRONG_CONSENSUS (FEAT-0010)

Si en raíz:

- `consensus.probe(pos)` hit
- `marked_moves` no vacío
- al menos un marked **legal**

→ **No search** (o search fallback solo si ningún marked legal):

- `bestmove` = primer marked legal (orden catalog)
- `info string marked=uci,uci,...`

## ResultBias (post-resolver)

Opción UCI `ResultBias`: NeedWin | Neutral | PreferDraw

- Modifica **utility** para TM/search; **no** cambia `evidence_class`.
- Referencia: `result_bias.rs`

## EvidenceInfo

| Valor | Comportamiento |
|-------|----------------|
| Off | Sin info string evidencia |
| Root | Solo raíz |
| All | Nodos con hit (caro) |

## Key lookup

FEN canonical → **placement + side-to-move** Zobrist.

Certus: `board::hash_placement_stm(pos)`.

Castling rights, en passant, halfmove, fullmove **no** distinguen hit (alineado interop Bárbol).

## Utility / score UCI

- WDL capas: mapear a cp utility Certus (ver `ConsensusWdl::utility`, TB constants).
- Mate: `score mate N` UCI cuando aplique.
- Emitir `UCI_ShowWDL` coherente si SF ya lo soporta.

## Fuentes excluidas (normativo)

No capa evidencia desde: ChessDB, Lichess cloud, OTB, eval single-engine, self-play, NNUE.

## Regresión

Cada regla debe tener entrada en `GOLDEN-FIXTURES.jsonl` o test unitario C++.
