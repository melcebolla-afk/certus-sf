#!/usr/bin/env python3
"""Import Chess-EPDs fortresses.epd → Theory seed entries (FEAT-0016).

Only positions labeled draw / fortress / eval = draw.
Default --min-pieces 7 (Syzygy 6-man always on).

  python3 builders/fortresses_import.py \\
    --epd train/out/fortresses.epd \\
    --out train/out/fortresses_theory.json
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

try:
    import chess
except ImportError:
    print("missing dependency: pip install chess", file=sys.stderr)
    sys.exit(2)

DRAW_MARKERS = ("draw", "fortress", "eval = draw", "eval=draw")
REF = "Chess-EPDs/fortresses.epd"


def is_draw_ops(ops: str) -> bool:
    low = ops.lower()
    return any(m in low for m in DRAW_MARKERS)


def parse_epd_line(line: str) -> tuple[str, str] | None:
    line = line.strip().replace("\x00", "")
    if not line or line.startswith("#"):
        return None
    parts = line.split(";")
    head = parts[0].strip()
    ops = ";".join(parts[1:]).strip()
    fields = head.split()
    if len(fields) < 2:
        return None
    placement, stm = fields[0], fields[1]
    castle = fields[2] if len(fields) > 2 else "-"
    ep = fields[3] if len(fields) > 3 else "-"
    return f"{placement} {stm} {castle} {ep} 0 1", ops


def fen_key(fen: str) -> str:
    parts = fen.split()
    return f"{parts[0]} {parts[1]}" if len(parts) >= 2 else fen


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--epd", type=Path, required=True)
    ap.add_argument("--out", type=Path, required=True)
    ap.add_argument("--min-pieces", type=int, default=7)
    ap.add_argument("--exclude-catalog", type=Path, default=None)
    args = ap.parse_args()

    if not args.epd.is_file():
        print(f"missing epd: {args.epd}", file=sys.stderr)
        return 2

    exclude: set[str] = set()
    if args.exclude_catalog and args.exclude_catalog.is_file():
        data = json.loads(args.exclude_catalog.read_text(encoding="utf-8"))
        for e in data.get("entries") or []:
            fen = (e.get("fen") or "").strip()
            if fen:
                exclude.add(fen_key(fen))

    seen: set[str] = set()
    entries: list[dict] = []
    scanned = 0
    skip_ops = 0
    skip_pieces = 0
    skip_known = 0
    skip_bad = 0

    for raw in args.epd.read_text(encoding="utf-8", errors="replace").splitlines():
        parsed = parse_epd_line(raw)
        if not parsed:
            continue
        scanned += 1
        fen, ops = parsed
        if not is_draw_ops(ops):
            skip_ops += 1
            continue
        try:
            board = chess.Board(fen)
        except ValueError:
            skip_bad += 1
            continue
        if len(board.piece_map()) < args.min_pieces:
            skip_pieces += 1
            continue
        key = fen_key(board.fen())
        if key in seen:
            continue
        if key in exclude:
            skip_known += 1
            continue
        seen.add(key)
        note = "fortress/draw from EPD"
        m = re.search(r"c0\s+([^;]+)", ops, re.I)
        if m:
            note = m.group(1).strip().strip('"')[:120] or note
        entries.append(
            {
                "fen": board.fen(),
                "result": "D",
                "note": note,
                "ref": REF,
            }
        )

    args.out.parent.mkdir(parents=True, exist_ok=True)
    payload = {
        "content_version_base": "fortresses-epd",
        "source": args.epd.name,
        "entries": entries,
    }
    args.out.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    print(
        f"scanned={scanned} kept={len(entries)} "
        f"skip_ops={skip_ops} skip_pieces={skip_pieces} "
        f"skip_known={skip_known} skip_bad={skip_bad} → {args.out}",
        file=sys.stderr,
    )
    return 0 if entries else 1


if __name__ == "__main__":
    sys.exit(main())
