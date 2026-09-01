#!/usr/bin/env python3
"""Periodic THEORETICAL update from Chess-EPDs fortresses.epd (FEAT-0016).

Download EPD if changed → import draw/fortress ≥7 → merge with curated seed → lab.

  python3 builders/theory_repo_update.py --dry-run
  python3 builders/theory_repo_update.py --skip-download
"""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_EPD_URL = (
    "https://raw.githubusercontent.com/ChrisWhittington/Chess-EPDs/master/fortresses.epd"
)
DEFAULT_EPD = ROOT / "train/out/fortresses.epd"
DEFAULT_SEED = ROOT / "testdata/theoretical/seed.json"
DEFAULT_LAB = ROOT / "evidence/theoretical"
DEFAULT_OUT = ROOT / "train/out/theory_catalog_build"
DEFAULT_VERSION = "2026.08.29"


def run(cmd: list[str], dry: bool) -> None:
    print(f"+ {' '.join(cmd)}", flush=True)
    if dry:
        return
    subprocess.run(cmd, check=True)


def download_epd(url: str, dest: Path) -> None:
    dest.parent.mkdir(parents=True, exist_ok=True)
    # curl -z: only download if newer than local file
    cmd = ["curl", "-fsSL", "-z", str(dest), "-o", str(dest), url]
    print(f"+ {' '.join(cmd)}", flush=True)
    subprocess.run(cmd, check=True)
    if not dest.is_file() or dest.stat().st_size == 0:
        # -z may leave empty on some curl builds; force once
        subprocess.run(["curl", "-fsSL", "-o", str(dest), url], check=True)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--epd", type=Path, default=DEFAULT_EPD)
    ap.add_argument("--epd-url", default=DEFAULT_EPD_URL)
    ap.add_argument("--seed", type=Path, default=DEFAULT_SEED)
    ap.add_argument("--lab", type=Path, default=DEFAULT_LAB)
    ap.add_argument("--out", type=Path, default=DEFAULT_OUT)
    ap.add_argument("--version", default=DEFAULT_VERSION)
    ap.add_argument("--min-pieces", type=int, default=7)
    ap.add_argument("--skip-download", action="store_true")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--work-dir", type=Path, default=ROOT / "train/out")
    args = ap.parse_args()

    args.work_dir.mkdir(parents=True, exist_ok=True)
    fragment = args.work_dir / "fortresses_theory.json"
    stamp = args.work_dir / "theory_update.last"
    catalog = args.lab / f"v{args.version}" / "catalog.json"

    if not args.skip_download:
        if args.dry_run:
            print(f"+ curl -z {args.epd_url} → {args.epd}", flush=True)
        else:
            try:
                download_epd(args.epd_url, args.epd)
            except (subprocess.CalledProcessError, FileNotFoundError) as e:
                print(f"download failed: {e}", file=sys.stderr)
                if not args.epd.is_file():
                    return 2
                print("continuing with existing epd", file=sys.stderr)

    if not args.epd.is_file():
        print(f"missing epd: {args.epd}", file=sys.stderr)
        return 2

    import_cmd = [
        sys.executable,
        str(ROOT / "builders/fortresses_import.py"),
        "--epd",
        str(args.epd),
        "--out",
        str(fragment),
        "--min-pieces",
        str(args.min_pieces),
    ]
    # Full rebuild from seed+epd each time (EPD is tiny); exclude unused
    run(import_cmd, args.dry_run)

    build_cmd = [
        sys.executable,
        str(ROOT / "builders/theory_build.py"),
        "--seed",
        str(args.seed),
        "--extra",
        str(fragment),
        "--out",
        str(args.out),
        "--version",
        args.version,
        "--min-pieces",
        str(args.min_pieces),
        "--lab",
        str(args.lab),
    ]
    run(build_cmd, args.dry_run)

    if not args.dry_run:
        stamp.write_text(
            f"epd={args.epd} catalog={catalog} version={args.version}\n",
            encoding="utf-8",
        )
    return 0


if __name__ == "__main__":
    sys.exit(main())
