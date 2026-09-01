#!/usr/bin/env python3
"""Periodic MatePath catalog update (FEAT-0015).

Download (if changed) → filter mateIn1/2 ≥7 excluding catalog → mate_build --merge.

Default is merge-only (never wipes catalog). Use --replace only for full rebuild.

Examples:
  python3 builders/mate_repo_update.py --dry-run
  python3 builders/mate_repo_update.py --max-new 2000
  python3 builders/mate_repo_update.py --skip-download --max-new 500
"""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_DUMP = ROOT / "train/out/lichess_db_puzzle.csv.zst"
DEFAULT_DUMP_URL = "https://database.lichess.org/lichess_db_puzzle.csv.zst"
DEFAULT_CATALOG = ROOT / "evidence/mate/v2026.08.29/catalog.json"
DEFAULT_LAB = ROOT / "evidence/mate"
DEFAULT_OUT = ROOT / "train/out/mate_catalog_build"
DEFAULT_VERSION = "2026.08.29"


def download_dump(url: str, dest: Path) -> None:
    dest.parent.mkdir(parents=True, exist_ok=True)
    cmd = ["wget", "-N", "-P", str(dest.parent), url]
    expected = dest.parent / Path(url).name
    print(f"+ {' '.join(cmd)}", flush=True)
    subprocess.run(cmd, check=True)
    if expected.is_file() and expected.resolve() != dest.resolve():
        expected.replace(dest)


def run(cmd: list[str], dry: bool) -> None:
    print(f"+ {' '.join(cmd)}", flush=True)
    if dry:
        return
    subprocess.run(cmd, check=True)


def count_fens(path: Path) -> int:
    n = 0
    with path.open(encoding="utf-8") as fh:
        for line in fh:
            s = line.strip()
            if s and not s.startswith("#"):
                n += 1
    return n


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--dump", type=Path, default=DEFAULT_DUMP)
    ap.add_argument("--dump-url", default=DEFAULT_DUMP_URL)
    ap.add_argument("--catalog", type=Path, default=DEFAULT_CATALOG)
    ap.add_argument("--lab", type=Path, default=DEFAULT_LAB)
    ap.add_argument("--out", type=Path, default=DEFAULT_OUT)
    ap.add_argument("--version", default=DEFAULT_VERSION)
    ap.add_argument("--max-new", type=int, default=5000)
    ap.add_argument("--max-mate-in", type=int, default=2)
    ap.add_argument("--max-plies", type=int, default=3)
    ap.add_argument("--min-pieces", type=int, default=7)
    ap.add_argument("--skip-download", action="store_true")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument(
        "--replace",
        action="store_true",
        help="Full rebuild (no --merge / no exclude). Destructive vs incremental.",
    )
    ap.add_argument("--work-dir", type=Path, default=ROOT / "train/out")
    args = ap.parse_args()

    args.work_dir.mkdir(parents=True, exist_ok=True)
    delta = args.work_dir / "lichess_mates_delta.fens"
    stamp = args.work_dir / "mate_update.last"

    if not args.skip_download:
        if args.dry_run:
            print(f"+ wget -N {args.dump_url} → {args.dump}", flush=True)
        else:
            try:
                download_dump(args.dump_url, args.dump)
            except (subprocess.CalledProcessError, FileNotFoundError) as e:
                print(f"download failed: {e}", file=sys.stderr)
                if not args.dump.is_file():
                    return 2
                print("continuing with existing dump", file=sys.stderr)

    if not args.dump.is_file():
        print(f"missing dump: {args.dump}", file=sys.stderr)
        return 2

    filter_cmd = [
        sys.executable,
        str(ROOT / "builders/lichess_mate_filter.py"),
        "--csv",
        str(args.dump),
        "--out",
        str(delta),
        "--min-pieces",
        str(args.min_pieces),
        "--max-mate-in",
        str(args.max_mate_in),
        "--max-out",
        str(args.max_new),
    ]
    if not args.replace:
        if not args.catalog.is_file() and not args.dry_run:
            print(f"missing catalog for merge: {args.catalog}", file=sys.stderr)
            return 2
        filter_cmd.extend(["--exclude-catalog", str(args.catalog)])

    run(filter_cmd, args.dry_run)

    if args.dry_run:
        print(f"+ mate_build --merge … seed={delta}", flush=True)
        return 0

    n_new = count_fens(delta)
    print(f"delta fens: {n_new} → {delta}", flush=True)
    if n_new == 0:
        print("nothing new to merge; done", flush=True)
        stamp.write_text(f"n_new=0 catalog={args.catalog}\n", encoding="utf-8")
        return 0

    build_cmd = [
        "cargo",
        "run",
        "-q",
        "-p",
        "evidence-engine",
        "--example",
        "mate_build",
        "--release",
        "--",
        "--seed",
        str(delta),
        "--out",
        str(args.out),
        "--version",
        args.version,
        "--max-plies",
        str(args.max_plies),
        "--lab",
        str(args.lab),
    ]
    if not args.replace:
        build_cmd.extend(["--merge", str(args.catalog)])

    run(build_cmd, False)
    stamp.write_text(
        f"n_new={n_new} catalog={args.catalog} version={args.version}\n",
        encoding="utf-8",
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
