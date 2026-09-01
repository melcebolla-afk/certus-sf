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

## Do not modify for certus

- `evaluate.cpp`, `search.cpp`, `uci.cpp` (until planned Fase 2/3 hooks)
- NNUE, Syzygy, thread pool

## Merge procedure

See `docs/runbooks/stockfish-merge.md` and `scripts/merge-stockfish.sh`.
