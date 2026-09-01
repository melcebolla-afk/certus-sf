# prior-art — resolver

## Qué hay

Precedencia TB > Mate > Theory > Consensus > ICCF > NNUE. `evaluate_layers(hard, soft)` / `EvalNeed`:

- **HardOnly** midgame quiet (`piece_count > 6`, no jaque): solo Syzygy / early out (FEAT-0034); search puede omitir HardOnly en ese caso.
- **SoftOnly**: NNUE sin re-probe duro; ICCF solo si Full o ≤6/jaque.
- **Full**: stack completo (qsearch / raíz).
- Syzygy: no llamar probe si `piece_count > max_pieces`.

## Pendiente

NNUE más barata / incremental / SIMD (**FEAT-0035**, meta NPS hacia SF; estudio SF → síntesis GPL, no copiar AGPL); bloom MatePath; magics (0032) después.
