#!/usr/bin/env bash
# UCI + probe smoke for certus-sf Phase 1.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$ROOT/stockfish/src/certus-sf"
PROBE="$ROOT/stockfish/src/evidence_probe"

make -j -C "$ROOT/stockfish/src" build ARCH=x86-64-sse41-popcnt
make -j -C "$ROOT/stockfish/src" evidence_probe ARCH=x86-64-sse41-popcnt

"$PROBE" "$ROOT"

out="$(printf 'uci\nsetoption name ConsensusPath value %s/testdata/consensus\nisready\nquit\n' "$ROOT" | "$BIN" 2>&1)"
echo "$out" | grep -q 'id name certus-sf dev'
echo "$out" | grep -q 'option name ConsensusPath'
echo "$out" | grep -q 'ConsensusPath ready version='

echo "tests/evidence_probe.sh: OK"
