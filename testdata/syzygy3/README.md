# Syzygy WDL fixtures (3-man subset)

Small `.rtbw` files for automated `PROVEN_TB` tests (from [sesse](http://tablebase.sesse.net/syzygy/3-4-5/)).

Point UCI `SyzygyPath` or env `SYZYGY_PATH` at this directory for local smoke:

```bash
SYZYGY_PATH=testdata/syzygy3 cargo test -p syzygy
printf 'uci\nsetoption name SyzygyPath value %s\nposition fen 8/8/8/8/8/8/4Q3/4K1k1 w - - 0 1\ngo\nquit\n' \
  "$(pwd)/testdata/syzygy3" | cargo run -q -p evidence-engine
```

Not a full 3-4-5 set — only enough for KQK / KRK / etc. smoke.
