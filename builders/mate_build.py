#!/usr/bin/env python3
"""Offline PROVEN_MATE catalog builder (port mate_build.rs — no Rust/cargo).

Verify seed FENs with exhaustive mate probe; write catalog.json + catalog.idx + manifest.

  python3 builders/mate_build.py --seed testdata/mate/seed.fens --out /tmp/mate_build \\
    --version 2026.08.29 --max-plies 5 --lab catalogs/mate

  # Bulk bootstrap (many FENs):
  python3 builders/mate_build.py --seed train/out/lichess_mates_delta.fens \\
    --out train/out/mate_catalog_build --merge catalogs/mate/v2026.08.29/catalog.json \\
    --max-plies 3 --jobs 32 --lab catalogs/mate
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import sys
import time
from concurrent.futures import ProcessPoolExecutor, as_completed
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


def _verify_fen(fen: str, max_plies: int) -> tuple[str, dict | None, str | None]:
    """Return (status, entry_or_none, err_or_none). status: ok|skip|bad."""
    try:
        board = chess.Board(fen)
    except ValueError as e:
        return ("bad", None, str(e))
    hit = probe_mate_ungated(board, max_plies)
    if hit is None:
        return ("skip", None, None)
    return (
        "ok",
        {
            "fen": fen,
            "plies": hit.plies,
            "stm_wins": hit.stm_wins,
        },
        None,
    )


def _worker(payload: tuple[int, str, int]) -> tuple[int, str, dict | None, str | None]:
    line_no, fen, max_plies = payload
    status, entry, err = _verify_fen(fen, max_plies)
    return (line_no, status, entry, err)


def _iter_seed(path: Path):
    with path.open(encoding="utf-8") as fh:
        for i, line in enumerate(fh, 1):
            fen = line.strip()
            if not fen or fen.startswith("#"):
                continue
            yield i, fen


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--seed", type=Path, required=True, help="FEN list (.fens), one per line")
    ap.add_argument("--out", type=Path, required=True, help="Output dir for catalog.json")
    ap.add_argument("--merge", type=Path, default=None, help="Existing catalog.json to merge")
    ap.add_argument("--version", default="2026.08.29")
    ap.add_argument("--max-plies", type=int, default=5)
    ap.add_argument("--lab", type=Path, default=None, help="Copy to lab/v{version}/")
    ap.add_argument(
        "--jobs",
        type=int,
        default=1,
        help="Parallel workers (1 = sequential). Use N≈cores for bulk bootstrap.",
    )
    ap.add_argument(
        "--progress-every",
        type=int,
        default=5000,
        help="Log progress every N candidates processed (0 = silent)",
    )
    args = ap.parse_args()

    if args.jobs < 1:
        print("--jobs must be >= 1", file=sys.stderr)
        return 2

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

    work: list[tuple[int, str, int]] = []
    for line_no, fen in _iter_seed(args.seed):
        key = fen_key(fen)
        if key in by_key:
            continue
        work.append((line_no, fen, args.max_plies))

    print(
        f"candidates to probe: {len(work)} (jobs={args.jobs} max_plies={args.max_plies})",
        flush=True,
    )

    skipped = 0
    added = 0
    done = 0
    t0 = time.perf_counter()

    def _consume(line_no: int, status: str, entry: dict | None, err: str | None) -> None:
        nonlocal skipped, added, done
        done += 1
        if status == "bad":
            print(f"line {line_no}: bad fen: {err}", flush=True)
            skipped += 1
        elif status == "skip":
            print(
                f"line {line_no}: no forced mate within {args.max_plies} plies — skip",
                flush=True,
            )
            skipped += 1
        else:
            assert entry is not None
            key = fen_key(entry["fen"])
            if key not in by_key:
                by_key[key] = entry
                added += 1
        if args.progress_every > 0 and done % args.progress_every == 0:
            elapsed = time.perf_counter() - t0
            rate = done / elapsed if elapsed > 0 else 0.0
            eta = (len(work) - done) / rate if rate > 0 else 0.0
            print(
                f"progress {done}/{len(work)} added={added} skipped={skipped} "
                f"{rate:.1f} pos/s eta={eta / 60:.1f}min",
                flush=True,
            )

    if not work:
        print("nothing new to probe", flush=True)
    elif args.jobs == 1:
        for line_no, fen, max_plies in work:
            status, entry, err = _verify_fen(fen, max_plies)
            _consume(line_no, status, entry, err)
    else:
        # Chunk submissions to bound peak memory on multi-million seeds.
        chunk = max(args.jobs * 64, 256)
        with ProcessPoolExecutor(max_workers=args.jobs) as pool:
            for start in range(0, len(work), chunk):
                batch = work[start : start + chunk]
                futures = [pool.submit(_worker, item) for item in batch]
                for fut in as_completed(futures):
                    line_no, status, entry, err = fut.result()
                    _consume(line_no, status, entry, err)

    elapsed = time.perf_counter() - t0
    if work:
        print(
            f"probe done in {elapsed:.1f}s ({len(work) / elapsed:.1f} pos/s) "
            f"added={added} skipped={skipped}",
            flush=True,
        )

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
        "jobs": args.jobs,
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
    # Required for some platforms when using ProcessPoolExecutor under spawn.
    sys.exit(main())
