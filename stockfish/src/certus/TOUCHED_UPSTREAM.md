# Upstream files touched by certus-sf

Keep these diffs **minimal** when merging new Stockfish releases.
All product code: `src/evidence/` + `src/certus/` (except this doc).

## Required patches

### `src/Makefile`

1. After `VPATH = syzygy:nnue:nnue/features` add:

```makefile
-include certus/certus.mk
```

2. In target `objclean`, append (after upstream `rm` line):

```makefile
	@rm -f $(CERTUS_CLEAN) ./evidence/*.o
```

(`CERTUS_CLEAN` is defined in `certus/certus.mk`.)

### `src/engine.h`

- `#include "certus/certus_engine.h"`
- Member: `Certus::EngineExtension certus_;`
- Optional accessor: `certus()` for eval/search hooks (Fase 2+)

### `src/engine.cpp`

- `#include "certus/certus_engine.h"` (not evidence headers directly)
- After `EvalFileSmall` option: `certus_.register_options(options, [this] { search_clear(); });`

### `src/misc.cpp`

- `#ifdef CERTUS_SF` / `#include "certus/certus_engine.h"`
- `engine_info()`: call `Certus::engine_identity(to_uci)` when `CERTUS_SF` defined

### `src/ucioption.cpp`

- Combo options: emit `default` from `currentValue` and `var` for each token in `defaultValue` (EvidenceInfo, ConsensusSearch).

### `src/search.cpp`

- `#ifdef CERTUS_SF` / `#include "certus/certus_eval.h"` + `certus_search.h`
- `Search::Worker::evaluate`: delegate to `Certus::evaluate` (evidence resolver → NNUE fallback)
- `CERTUS_SET_EVAL_NEED` before static eval (SoftOnly en NonPV quiet >6 piezas → NNUE)
- `Certus::allow_search_move` in main search move loop (FEAT-0002 MarkedOnly)
- `start_searching`: `Certus::prepare_root_search` (consensus marked shortcut)
- Before `onBestmove`: `Certus::finish_search_evidence` (EvidenceInfo All)

### `src/thread.cpp`

- `#ifdef CERTUS_SF` / `#include "certus/certus_eval.h"`
- In `ThreadPool::start_thinking` per-thread job: `Certus::bind_tb_config(tbConfig)`

### `src/engine.cpp`

- `#ifdef CERTUS_SF` / `#include "certus/certus_eval.h"`
- In `Engine::go`: `Certus::bind_evidence(certus_.evidence())`

## Do not modify for certus (yet)

- `evaluate.cpp`, `uci.cpp` (eval hook via certus_eval; UCI root consensus Fase 3)
- NNUE, Syzygy, thread pool

## Merge procedure

See `docs/runbooks/stockfish-merge.md` and `scripts/merge-stockfish.sh`.
