#!/usr/bin/env python3
"""Ingest Bárbol-lite DONE oracles → Certus STRONG_CONSENSUS catalog (FEAT-0008).

Requires ≥1 ab + ≥1 mcts DONE per FEN with agreeing WDL and ≥1 marked
move after intersection (see docs/interop/barbol-consensus-file.md).
Disagreements and empty marked sets are skipped (no false consensus).
"""

from __future__ import annotations

import argparse
import hashlib
import json
import sys
from collections import defaultdict
from pathlib import Path

MIN_AB = 50_000_000
MIN_MCTS = 5_000_000


def fen_key(fen: str) -> str:
    parts = fen.split()
    if len(parts) < 2:
        raise ValueError(f"short fen: {fen}")
    return f"{parts[0]} {parts[1]}"


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("export_json", type=Path)
    ap.add_argument("out_dir", type=Path)
    ap.add_argument("--version", default="consensus-fixture-2026.08.29")
    ap.add_argument("--min-confidence", type=float, default=0.8)
    args = ap.parse_args()

    data = json.loads(args.export_json.read_text(encoding="utf-8"))
    if data.get("schema_version") != 1:
        print("bad export schema_version", file=sys.stderr)
        sys.exit(1)

    by_fen: dict[str, dict[str, list]] = defaultdict(lambda: {"ab": [], "mcts": []})
    for o in data.get("oracles", []):
        if o.get("status") != "DONE":
            continue
        fam = o.get("family")
        if fam not in ("ab", "mcts"):
            continue
        key = fen_key(o["fen"])
        by_fen[key][fam].append(o)

    entries = []
    skipped = 0
    for fen_k, fams in sorted(by_fen.items()):
        if not fams["ab"] or not fams["mcts"]:
            skipped += 1
            continue
        ab = max(fams["ab"], key=lambda x: x.get("budget_nodes", 0))
        mcts = max(fams["mcts"], key=lambda x: x.get("budget_nodes", 0))
        wdl_ab = str(ab["wdl"]).lower()
        wdl_mcts = str(mcts["wdl"]).lower()
        if wdl_ab != wdl_mcts:
            skipped += 1
            continue
        ab_n = int(ab.get("budget_nodes", 0))
        mcts_n = int(mcts.get("budget_nodes", 0))
        conf = 0.92 if ab_n >= MIN_AB and mcts_n >= MIN_MCTS else 0.85
        if conf < args.min_confidence:
            skipped += 1
            continue
        w = wdl_ab
        if w.startswith("w"):
            wdl = "w"
        elif w.startswith("d") or "1/2" in w:
            wdl = "d"
        elif w.startswith("l") or w == "0-1":
            wdl = "l"
        else:
            skipped += 1
            continue
        ab_m = {str(m) for m in (ab.get("marked_moves") or [])}
        mcts_m = {str(m) for m in (mcts.get("marked_moves") or [])}
        if ab_m and mcts_m:
            marked = sorted(ab_m & mcts_m)
        else:
            marked = sorted(ab_m | mcts_m)
        # Production rule (interop): no marked → no consensus entry.
        if not marked:
            skipped += 1
            continue
        entries.append(
            {
                "fen": ab.get("fen", fen_k + " - - 0 1"),
                "wdl": wdl,
                "confidence": conf,
                "marked_moves": marked,
                "sources": [
                    f"ab:budget={ab_n}",
                    f"mcts:budget={mcts_n}",
                ],
                "budgets": {"ab_nodes": ab_n, "mcts_nodes": mcts_n},
            }
        )
    catalog = {
        "schema_version": 1,
        "content_version": args.version,
        "layer": "STRONG_CONSENSUS",
        "min_confidence": args.min_confidence,
        "entries": entries,
    }
    args.out_dir.mkdir(parents=True, exist_ok=True)
    cat_path = args.out_dir / "catalog.json"
    raw = json.dumps(catalog, indent=2) + "\n"
    cat_path.write_text(raw, encoding="utf-8")
    digest = hashlib.sha256(raw.encode()).hexdigest()
    manifest = {
        "layer": "STRONG_CONSENSUS",
        "schema_version": 1,
        "content_version": args.version,
        "checksum": f"sha256:{digest}",
        "coverage": {"positions": len(entries), "skipped_fen": skipped},
        "sources": ["barbol-lite-export"],
    }
    (args.out_dir / "manifest.json").write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8"
    )
    print(f"wrote {cat_path} entries={len(entries)} skipped_fen={skipped}")


if __name__ == "__main__":
    main()
