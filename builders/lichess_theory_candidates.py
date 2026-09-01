#!/usr/bin/env python3
"""Export Lichess endgame FENs (≥7) as Theory *candidates* (FEAT-0015).

Never writes theoretical/ catalog. Human must curate seed.json (result/note/ref).
Skips mate-only puzzles that belong in MatePath.

  python3 builders/lichess_theory_candidates.py \\
    --csv train/out/lichess_db_puzzle.csv.zst \\
    --out train/out/theory_candidates.fens --max-out 500
"""

from __future__ import annotations

import argparse
import csv
import sys
from pathlib import Path

try:
    import chess
except ImportError:
    print("missing dependency: pip install chess", file=sys.stderr)
    sys.exit(2)


def open_csv(path: Path):
    if path.suffix == ".zst":
        import subprocess

        proc = subprocess.Popen(
            ["zstd", "-dq", "-c", str(path)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        assert proc.stdout is not None
        return proc, proc.stdout
    return None, path.open(encoding="utf-8", newline="")


def theme_parts(themes: str) -> set[str]:
    return {t.strip().lower() for t in themes.replace(",", " ").split() if t.strip()}


def is_mate_theme(parts: set[str]) -> bool:
    if "mate" in parts:
        return True
    return any(t.startswith("matein") or t.endswith("mate") for t in parts)


def apply_first(fen: str, moves: str) -> str | None:
    ucis = moves.split()
    if not ucis:
        return None
    try:
        board = chess.Board(fen)
        board.push_uci(ucis[0])
    except (ValueError, chess.InvalidMoveError, chess.IllegalMoveError):
        return None
    return board.fen()


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--csv", type=Path, required=True)
    ap.add_argument("--out", type=Path, required=True)
    ap.add_argument("--min-pieces", type=int, default=7)
    ap.add_argument("--max-out", type=int, default=500)
    ap.add_argument("--require-endgame", action="store_true", default=True)
    ap.add_argument("--no-require-endgame", action="store_false", dest="require_endgame")
    args = ap.parse_args()

    if not args.csv.is_file():
        print(f"missing csv: {args.csv}", file=sys.stderr)
        return 2

    proc, fh = open_csv(args.csv)
    seen: set[str] = set()
    kept = 0
    scanned = 0
    try:
        reader = csv.DictReader(fh)
        args.out.parent.mkdir(parents=True, exist_ok=True)
        with args.out.open("w", encoding="utf-8") as out:
            out.write(
                "# theory CANDIDATES only — curate into testdata/theoretical/seed.json; "
                "never auto-WDL\n"
            )
            for row in reader:
                scanned += 1
                parts = theme_parts(row.get("Themes") or "")
                if args.require_endgame and "endgame" not in parts:
                    continue
                if is_mate_theme(parts):
                    continue
                fen = (row.get("FEN") or "").strip()
                moves = row.get("Moves") or ""
                if not fen:
                    continue
                solved = apply_first(fen, moves)
                if solved is None:
                    continue
                board = chess.Board(solved)
                if len(board.piece_map()) < args.min_pieces:
                    continue
                key = " ".join(solved.split()[:2])
                if key in seen:
                    continue
                seen.add(key)
                out.write(solved + "\n")
                kept += 1
                if args.max_out and kept >= args.max_out:
                    break
    finally:
        fh.close()
        if proc is not None:
            proc.wait(timeout=30)

    print(f"scanned={scanned} candidates={kept} → {args.out}", file=sys.stderr)
    return 0 if kept else 1


if __name__ == "__main__":
    sys.exit(main())
