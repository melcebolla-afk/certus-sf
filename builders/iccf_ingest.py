#!/usr/bin/env python3
"""Ingest ICCF-like stats → Certus EMPIRICAL_ICCF catalog (FEAT-0009)."""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import sys
from pathlib import Path

DEFAULT_MIN_N = 10
DEFAULT_MIN_ELO = 2000
DEFAULT_MIN_DATE = "2000-01-01"
DEFAULT_MIN_CONF = 0.5


def confidence_from_n(n: int) -> float:
    if n <= 0:
        return 0.0
    return float(min(0.85, max(0.35, 0.35 + 0.15 * math.log10(n))))


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("export_json", type=Path)
    ap.add_argument("out_dir", type=Path)
    ap.add_argument("--version", default="iccf-fixture-2026.08.29")
    ap.add_argument("--min-n", type=int, default=DEFAULT_MIN_N)
    ap.add_argument("--min-elo", type=int, default=DEFAULT_MIN_ELO)
    ap.add_argument("--min-date", default=DEFAULT_MIN_DATE)
    ap.add_argument("--min-confidence", type=float, default=DEFAULT_MIN_CONF)
    args = ap.parse_args()

    data = json.loads(args.export_json.read_text(encoding="utf-8"))
    if data.get("schema_version") != 1:
        print("bad export schema_version", file=sys.stderr)
        sys.exit(1)

    entries = []
    skipped = 0
    # Keep low-n rows in catalog so runtime gate tests miss; filter only trash.
    for row in data.get("rows", []):
        fen = row.get("fen")
        wdl = str(row.get("wdl", "")).lower()
        if wdl.startswith("w"):
            w = "w"
        elif wdl.startswith("d") or "1/2" in wdl:
            w = "d"
        elif wdl.startswith("l"):
            w = "l"
        else:
            skipped += 1
            continue
        n = int(row.get("n", 0))
        elo = int(row.get("elo", 0))
        date = str(row.get("date") or "1970-01-01")
        conf = confidence_from_n(n)
        entries.append(
            {
                "fen": fen,
                "wdl": w,
                "n": n,
                "elo": elo,
                "date": date,
                "confidence": round(conf, 4),
                "sources": [str(row.get("source") or "iccf")],
            }
        )

    catalog = {
        "schema_version": 1,
        "content_version": args.version,
        "layer": "EMPIRICAL_ICCF",
        "min_n": args.min_n,
        "min_elo": args.min_elo,
        "min_date": args.min_date,
        "min_confidence": args.min_confidence,
        "entries": entries,
    }
    args.out_dir.mkdir(parents=True, exist_ok=True)
    cat_path = args.out_dir / "catalog.json"
    raw = json.dumps(catalog, indent=2) + "\n"
    cat_path.write_text(raw, encoding="utf-8")
    digest = hashlib.sha256(raw.encode()).hexdigest()
    manifest = {
        "layer": "EMPIRICAL_ICCF",
        "schema_version": 1,
        "content_version": args.version,
        "checksum": f"sha256:{digest}",
        "coverage": {"positions": len(entries), "skipped_rows": skipped},
        "sources": ["iccf-export"],
    }
    (args.out_dir / "manifest.json").write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8"
    )
    print(f"wrote {cat_path} entries={len(entries)} skipped={skipped}")


if __name__ == "__main__":
    main()
