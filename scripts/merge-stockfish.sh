#!/usr/bin/env bash
# Merge a new upstream Stockfish release into certus-sf (vendored stockfish/).
# Usage: ./scripts/merge-stockfish.sh [tag]   # default: read stockfish/UPSTREAM tag
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SF="$ROOT/stockfish"
UPSTREAM_REPO="${UPSTREAM_REPO:-https://github.com/official-stockfish/Stockfish.git}"
WORKDIR="${TMPDIR:-/tmp}/certus-sf-merge-$$"

TAG="${1:-}"
if [[ -z "$TAG" ]]; then
  TAG="$(grep '^tag=' "$SF/UPSTREAM" | cut -d= -f2)"
fi

echo "==> Fetch $UPSTREAM_REPO tag $TAG"
git clone --depth 1 --branch "$TAG" "$UPSTREAM_REPO" "$WORKDIR/upstream"

echo "==> Preserve certus overlay"
CERTUS_BACK="$WORKDIR/certus-overlay"
mkdir -p "$CERTUS_BACK"
rsync -a "$SF/src/evidence/" "$CERTUS_BACK/evidence/"
rsync -a "$SF/src/certus/" "$CERTUS_BACK/certus/"
cp "$SF/UPSTREAM" "$CERTUS_BACK/UPSTREAM" 2>/dev/null || true

echo "==> Replace upstream tree (keep certus paths out of rsync delete)"
rsync -a --delete \
  --exclude '/src/evidence/' \
  --exclude '/src/certus/' \
  "$WORKDIR/upstream/" "$SF/"

echo "==> Restore certus overlay"
rsync -a "$CERTUS_BACK/evidence/" "$SF/src/evidence/"
rsync -a "$CERTUS_BACK/certus/" "$SF/src/certus/"

COMMIT="$(git -C "$WORKDIR/upstream" rev-parse HEAD)"
cat > "$SF/UPSTREAM" <<EOF
# Stockfish upstream pin (certus-sf vendored fork)
repo=$UPSTREAM_REPO
tag=$TAG
commit=$COMMIT
describe=$(git -C "$WORKDIR/upstream" describe --tags --always 2>/dev/null || echo "$TAG")
EOF

echo "==> Re-apply documented upstream touches (manual if conflicts):"
echo "    stockfish/src/certus/TOUCHED_UPSTREAM.md"
echo "    - Makefile: -include certus/certus.mk + objclean CERTUS_CLEAN line"
echo "    - engine.h / engine.cpp / misc.cpp"
echo ""
echo "Updated UPSTREAM → tag=$TAG commit=${COMMIT:0:8}"
echo "Next: make -C stockfish/src build evidence_probe && ./tests/evidence_probe.sh"

rm -rf "$WORKDIR"
