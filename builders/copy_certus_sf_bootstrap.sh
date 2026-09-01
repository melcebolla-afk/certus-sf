#!/usr/bin/env bash
# Copy certus-sf bootstrap export into a new repo checkout.
# Usage: builders/copy_certus_sf_bootstrap.sh /path/to/certus-sf
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEST="${1:?usage: copy_certus_sf_bootstrap.sh DEST_REPO_ROOT}"

BOOT="$ROOT/docs/export/certus-sf-bootstrap"
mkdir -p "$DEST/docs/bootstrap" "$DEST/builders" "$DEST/testdata"

echo "==> docs/bootstrap"
cp -a "$BOOT/." "$DEST/docs/bootstrap/"

echo "==> builders (capas)"
for f in consensus_ingest iccf_ingest theory_build theory_repo_update \
         fortresses_import mate_repo_update lichess_mate_filter \
         lichess_theory_candidates barbol_layers_promote; do
  cp "$ROOT/builders/${f}.py" "$DEST/builders/"
done

echo "==> testdata fixtures"
for d in consensus iccf theoretical mate syzygy3; do
  if [[ -d "$ROOT/testdata/$d" ]]; then
    cp -a "$ROOT/testdata/$d" "$DEST/testdata/"
  fi
done

echo "==> interop + prior-art (optional)"
mkdir -p "$DEST/docs/interop" "$DEST/docs/prior-art"
cp "$ROOT/docs/interop/"*.md "$DEST/docs/interop/" 2>/dev/null || true
for f in consensus iccf resolver syzygy; do
  [[ -f "$ROOT/docs/prior-art/${f}.md" ]] && cp "$ROOT/docs/prior-art/${f}.md" "$DEST/docs/prior-art/"
done

echo "==> copy script (self)"
cp "$ROOT/builders/copy_certus_sf_bootstrap.sh" "$DEST/builders/"

echo "Done. Next:"
echo "  1) Fork Stockfish into $DEST"
echo "  2) Read $DEST/docs/bootstrap/STARTUP-PROMPT.md in Cursor"
echo "  3) See $DEST/docs/bootstrap/REPO-SETUP.md"
