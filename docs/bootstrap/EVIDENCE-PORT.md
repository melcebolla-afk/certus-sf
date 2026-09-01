# Evidence port map — Certus Rust → Stockfish C++

Referencia SF: `official-stockfish/Stockfish` (estructura puede variar por versión; ajustar paths al tag elegido).

## Principio

Insertar **Evidence Resolver** en el camino de evaluación **antes** del NNUE SF. Syzygy TB ya existe en SF → mapear a `PROVEN_TB` sin segunda implementación.

## Mapa de módulos

| Certus (Rust) | Stockfish (C++) | Acción |
|---------------|-----------------|--------|
| `resolver.rs` | `src/evidence/resolver.cpp` (nuevo) | Port lógica precedencia |
| `consensus.rs` | `src/evidence/consensus.cpp` | Load catalog + probe |
| `iccf.rs` | `src/evidence/iccf.cpp` | Load catalog + gates |
| `theoretical.rs` | `src/evidence/theoretical.cpp` | Load catalog + probe |
| `mate_index.rs` | `src/evidence/mate_store.cpp` | mmap `catalog.idx` + JSON fallback |
| `mate.rs` (≤5 plies) | SF mate search / TB | Reusar SF donde exista |
| `syzygy::TbStore` | `syzygy.cpp` / TB probe | **Ya existe** — tag `PROVEN_TB` |
| NNUE `evaluate()` | `nnue/` + eval | **Ya existe** — tag `INFERENCE` |
| `uci.rs` setoption | `uci.cpp` | Añadir options + reload |
| Root consensus force | `search.cpp` root move | Port FEAT-0010 |
| `result_bias.rs` | `evidence/result_bias.cpp` | Post-resolver utility tweak |
| `hash_placement_stm` | `Position::key()` / Zobrist | Usar key SF **sin** repetir clock si Certus no lo hace |

## Hook eval (conceptual)

```cpp
// evaluate.cpp (pseudocódigo)
Value Eval::evaluate(const Position& pos) {
  if (auto ev = evidence::probe_hard_layers(pos))
    return ev.value;  // TB ya cubierto por SF probe antes — unificar
  if (auto ev = evidence::probe_mate_index(pos))
    return ev.value;
  if (auto ev = evidence::probe_theoretical(pos))
    return ev.value;
  if (auto ev = evidence::probe_consensus(pos))
    return ev.value;
  if (auto ev = evidence::probe_iccf(pos))
    return ev.value;
  return NNUE::evaluate(pos);  // INFERENCE
}
```

**Importante:** SF ya combina NNUE + TB + mate en eval. Refactorizar para **una** cadena de precedencia Certus, no dos pipelines paralelos.

## Thinning midgame (search interior)

Certus `evaluate_layers(pos, hard=true, soft=false)` en nodos quiet midgame (>6 piezas, !in_check):

- Solo **Syzygy** en capas duras; skip mate index / theory / consensus en ese probe.
- Implementar flag en resolver: `ProbeMode::HardOnlyThin` vs `Full`.

Archivo referencia: `resolver.rs` líneas ~104–108, tests `hard_only_thin_midgame_skips_consensus`.

## Root STRONG_CONSENSUS

Certus `uci.rs`: si root `StrongConsensus` y marked moves legales → `bestmove` = primero marked, sin search.

SF: hook en inicio de search o `Search::RootMoves` antes de iterative deepening.

## UCI info strings

Certus formato (referencia `format_root_evidence` / `uci.rs`):

```text
info string evidence=STRONG_CONSENSUS confidence=0.92 version=consensus-2026.08.29
info string marked=g1f3,b1c3
```

## TT / reload

Certus: `setoption` path → reload store + `tt.new_search()` / bump generation.

SF: invalidar TT o marcar generation al reload de capas (documentar en FEAT).

## Tests de paridad

Usar `GOLDEN-FIXTURES.jsonl` + mismos `testdata/` que Certus. Objetivo: **misma clase evidencia** en mismas FENs (utility puede diferir ± tolerancia por escala SF).

## Prior-art SF

Documentar en `docs/prior-art/evidence-port.md`:

- Dónde SF hace TB probe hoy
- Cómo SF integra WDL en UCI (`UCI_ShowWDL`)
- Qué **no** portar (red SF, search heuristics)

## Ficheros SF a tocar (checklist)

- [ ] `src/evaluate.cpp` / `evaluate.h`
- [ ] `src/uci.cpp` / `ucioption.cpp`
- [ ] `src/search.cpp` (root marked)
- [ ] `src/Makefile` / `meson.build` — añadir `src/evidence/*`
- [ ] `tests/` — golden evidence
