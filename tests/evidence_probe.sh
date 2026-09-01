#!/usr/bin/env bash
# UCI + probe + golden resolver smoke for certus-sf.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$ROOT/stockfish/src/certus-sf"
PROBE="$ROOT/stockfish/src/evidence_probe"
GOLDEN="$ROOT/stockfish/src/golden_probe"

make -j -C "$ROOT/stockfish/src" build ARCH=x86-64-sse41-popcnt
make -j -C "$ROOT/stockfish/src" evidence_probe golden_probe consensus_search_probe ARCH=x86-64-sse41-popcnt

python3 "$ROOT/builders/test_mate_build.py"

"$ROOT/stockfish/src/consensus_search_probe" "$ROOT"

"$PROBE" "$ROOT"
"$GOLDEN" "$ROOT"

out="$(printf 'uci\nsetoption name ConsensusPath value %s/testdata/consensus\nisready\nposition fen rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2\ngo depth 1\nquit\n' "$ROOT" | "$BIN" 2>&1)"
echo "$out" | grep -q 'info string evidence=STRONG_CONSENSUS'
echo "$out" | grep -q 'info string marked=b1c3'
echo "$out" | grep -q 'bestmove b1c3'
echo "$out" | grep -qv 'info depth 1'

out="$(printf 'uci\nsetoption name ConsensusPath value %s/testdata/consensus\nisready\nquit\n' "$ROOT" | "$BIN" 2>&1)"
echo "$out" | grep -q 'id name certus-sf dev'
echo "$out" | grep -q 'option name ConsensusPath'
echo "$out" | grep -q 'option name ConsensusSearch'
echo "$out" | grep -q 'ConsensusPath ready version='

echo "tests/evidence_probe.sh: OK"
