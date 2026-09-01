#!/usr/bin/env python3
"""Offline PROVEN_MATE catalog builder (port mate_build.rs — no Rust/cargo).

Verify seed FENs with exhaustive mate probe; write catalog.json + catalog.idx + manifest.

  python3 builders/mate_build.py --seed testdata/mate/seed.fens --out /tmp/mate_build \\
    --version 2026.08.29 --max-plies 5 --lab catalogs/mate
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import sys
from datetime import datetime, timezone
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from certus_hash import fen_key
from mate_idx import write_idx_from_json
from mate_probe import probe_mate_ungated

import chess


def load_merge_catalog(path: Path) -> dict[str, dict]:
    data = json.loads(path.read_text(encoding="utf-8"))
    out: dict[str, dict] = {}
    for e in data.get("entries") or []:
        fen = (e.get("fen") or "").strip()
        if fen:
            out[fen_key(fen)] = e
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--seed", type=Path, required=True, help="FEN list (.fens), one per line")
    ap.add_argument("--out", type=Path, required=True, help="Output dir for catalog.json")
    ap.add_argument("--merge", type=Path, default=None, help="Existing catalog.json to merge")
    ap.add_argument("--version", default="2026.08.29")
    ap.add_argument("--max-plies", type=int, default=5)
    ap.add_argument("--lab", type=Path, default=None, help="Copy to lab/v{version}/")
    args = ap.parse_args()

    by_key: dict[str, dict] = {}
    if args.merge is not None:
        if not args.merge.is_file():
            print(f"read --merge {args.merge}: missing", file=sys.stderr)
            return 2
        by_key = load_merge_catalog(args.merge)
        print(f"merge base: {len(by_key)} entries from {args.merge}", flush=True)

    if not args.seed.is_file():
        print(f"read seed: missing {args.seed}", file=sys.stderr)
        return 2

    skipped = 0
    added = 0
    for i, line in enumerate(args.seed.read_text(encoding="utf-8").splitlines(), 1):
        fen = line.strip()
        if not fen or fen.startswith("#"):
            continue
        key = fen_key(fen)
        if key in by_key:
            continue
        try:
            board = chess.Board(fen)
        except ValueError as e:
            print(f"line {i}: bad fen: {e}", flush=True)
            skipped += 1
            continue
        hit = probe_mate_ungated(board, args.max_plies)
        if hit is None:
            print(f"line {i}: no forced mate within {args.max_plies} plies — skip", flush=True)
            skipped += 1
            continue
        by_key[key] = {
            "fen": fen,
            "plies": hit.plies,
            "stm_wins": hit.stm_wins,
        }
        added += 1

    if not by_key:
        print("no verified mates; abort", file=sys.stderr)
        return 1

    entries = sorted(by_key.values(), key=lambda e: e["fen"])
    catalog = {
        "schema_version": 1,
        "content_version": args.version,
        "layer": "PROVEN_MATE",
        "entries": entries,
    }

    args.out.mkdir(parents=True, exist_ok=True)
    cat_path = args.out / "catalog.json"
    body = json.dumps(catalog, indent=2, sort_keys=False) + "\n"
    cat_path.write_text(body, encoding="utf-8")

    try:
        idx_path = write_idx_from_json(cat_path)
        print(f"wrote idx → {idx_path}", flush=True)
    except OSError as e:
        print(f"warning: idx build failed: {e}", flush=True)

    digest = hashlib.sha256(body.encode("utf-8")).hexdigest()
    updated = datetime.now(timezone.utc).strftime("%Y-%m-%d")
    manifest = {
        "schema_version": 1,
        "layer": "PROVEN_MATE",
        "content_version": args.version,
        "checksum": f"sha256:{digest}",
        "n_entries": len(entries),
        "max_plies": args.max_plies,
        "source": "mate_build",
        "updated": updated,
        "merged": args.merge is not None,
        "added": added,
    }
    (args.out / "manifest.json").write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8"
    )

    print(
        f"wrote {len(entries)} mates (added {added}, skipped {skipped}) → {cat_path} "
        f"version={args.version}",
        flush=True,
    )

    if args.lab is not None:
        dest = args.lab / f"v{args.version}"
        dest.mkdir(parents=True, exist_ok=True)
        shutil.copy2(cat_path, dest / "catalog.json")
        shutil.copy2(args.out / "manifest.json", dest / "manifest.json")
        idx_src = args.out / "catalog.idx"
        if idx_src.is_file():
            shutil.copy2(idx_src, dest / "catalog.idx")
        print(f"lab copy → {dest}", flush=True)

    return 0


if __name__ == "__main__":
    sys.exit(main())
