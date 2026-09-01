#!/usr/bin/env python3
"""DEPRECATED — Bárbol exporta directo a catalogs/{consensus,iccf}/.

Legacy E-05/E-06: promote staging → active EvidenceRoot.

  evidence/from_barbol/{consensus,iccf}/vYYYY.MM.DD/
    → catalogs/{consensus,iccf}/vYYYY.MM.DD/

No está en el cron de certus-sf. Conservado solo por migración one-off.

  python3 builders/barbol_layers_promote.py --dry-run --staging /path/from_barbol
"""

from __future__ import annotations

import argparse
import re
import shutil
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from catalog_paths import CATALOGS

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_STAGING = CATALOGS / "from_barbol"
DEFAULT_EVIDENCE = CATALOGS
LAYERS = ("consensus", "iccf")
VERSION_RE = re.compile(r"^v(\d{4}\.\d{2}\.\d{2})$")


def is_complete(version_dir: Path) -> bool:
    return (version_dir / "catalog.json").is_file()


def list_versions(layer_staging: Path) -> list[Path]:
    if not layer_staging.is_dir():
        return []
    out: list[Path] = []
    for p in sorted(layer_staging.iterdir()):
        if not p.is_dir():
            continue
        if p.name.endswith(".partial"):
            continue
        if not VERSION_RE.match(p.name):
            print(f"skip unexpected dir: {p}", flush=True)
            continue
        if not is_complete(p):
            print(f"skip incomplete (no catalog.json): {p}", flush=True)
            continue
        out.append(p)
    return out


def promote_one(src: Path, dest: Path, *, dry: bool, force: bool) -> str:
    """Returns status: promoted|skipped|forced."""
    dest_catalog = dest / "catalog.json"
    if dest_catalog.is_file() and not force:
        return "skipped"
    if dry:
        print(f"+ promote {src} → {dest}", flush=True)
        return "forced" if force and dest_catalog.is_file() else "promoted"
    dest.parent.mkdir(parents=True, exist_ok=True)
    tmp = dest.parent / f"{dest.name}.partial"
    if tmp.exists():
        shutil.rmtree(tmp)
    shutil.copytree(src, tmp)
    if dest.exists():
        if not force:
            shutil.rmtree(tmp)
            return "skipped"
        bak = dest.parent / f"{dest.name}.old"
        if bak.exists():
            shutil.rmtree(bak)
        dest.rename(bak)
        tmp.rename(dest)
        shutil.rmtree(bak)
        return "forced"
    tmp.rename(dest)
    return "promoted"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--staging", type=Path, default=DEFAULT_STAGING)
    ap.add_argument("--evidence-root", type=Path, default=DEFAULT_EVIDENCE)
    ap.add_argument(
        "--layers",
        nargs="+",
        choices=list(LAYERS),
        default=list(LAYERS),
        help="Which layers to promote (default: both)",
    )
    ap.add_argument(
        "--only-latest",
        action="store_true",
        help="Promote only the newest vYYYY.MM.DD per layer",
    )
    ap.add_argument("--force", action="store_true", help="Replace dest if already present")
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()

    staging: Path = args.staging
    evidence: Path = args.evidence_root
    if not staging.is_dir():
        print(f"staging missing (nothing to do): {staging}", flush=True)
        return 0

    counts = {"promoted": 0, "skipped": 0, "forced": 0}
    for layer in args.layers:
        vers = list_versions(staging / layer)
        if args.only_latest and vers:
            vers = [vers[-1]]
        if not vers:
            print(f"{layer}: no complete versions under {staging / layer}", flush=True)
            continue
        for src in vers:
            dest = evidence / layer / src.name
            status = promote_one(src, dest, dry=args.dry_run, force=args.force)
            print(f"{layer} {src.name}: {status} → {dest}", flush=True)
            counts[status] = counts.get(status, 0) + 1

    print(
        f"done promoted={counts['promoted']} forced={counts['forced']} skipped={counts['skipped']}",
        flush=True,
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
