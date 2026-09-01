#!/usr/bin/env python3
"""Filter Lichess puzzle CSV → FEN list for MatePath (FEAT-0014).

Premisa: Syzygy 6-man siempre on → solo posiciones con piece_count >= --min-pieces
(default 7) y tema mate*.

Formato Lichess: FEN = posición *antes* del primer UCI en `Moves` (jugada del rival).
El filtro aplica ese UCI y emite el FEN del lado que resuelve (donde debe haber mate).

Requires: `pip install chess` (see train/requirements.txt).

Download (offline, not in CI):
  wget https://database.lichess.org/lichess_db_puzzle.csv.zst

Usage:
  python3 builders/lichess_mate_filter.py \\
    --csv lichess_db_puzzle.csv.zst \\
    --out train/out/lichess_mates_ge7.fens \\
    --min-pieces 7 --max-mate-in 2 --max-out 50000

  python3 builders/lichess_mate_filter.py \\
    --csv testdata/mate/lichess_sample.csv --out /tmp/t.fens
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

MATE_THEME_MARKERS = (
    "mate",
    "matein1",
    "matein2",
    "matein3",
    "matein4",
    "matein5",
    "backrankmate",
    "smotheredmate",
    "bodenmate",
    "anastasiamate",
)


def has_mate_theme(themes: str) -> bool:
    parts = {t.strip().lower() for t in themes.replace(",", " ").split() if t.strip()}
    if not parts:
        return False
    if "mate" in parts:
        return True
    return any(t.startswith("matein") or t in MATE_THEME_MARKERS for t in parts)


def mate_in_ok(themes: str, max_mate_in: int) -> bool:
    if max_mate_in <= 0:
        return True
    parts = {t.strip().lower() for t in themes.replace(",", " ").split() if t.strip()}
    return any(f"matein{i}" in parts for i in range(1, max_mate_in + 1))


def puzzle_solver_fen(fen: str, moves: str) -> str | None:
    """Apply first UCI (opponent) → FEN for the side that solves the puzzle."""
    ucis = moves.split()
    if not ucis:
        return None
    try:
        board = chess.Board(fen)
        board.push_uci(ucis[0])
    except (ValueError, chess.InvalidMoveError, chess.IllegalMoveError):
        return None
    return board.fen()


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


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--csv", type=Path, required=True, help="Lichess puzzle CSV or .csv.zst")
    ap.add_argument("--out", type=Path, required=True, help="Output .fens (one FEN per line)")
    ap.add_argument("--min-pieces", type=int, default=7)
    ap.add_argument("--max-out", type=int, default=0, help="0 = unlimited")
    ap.add_argument(
        "--max-mate-in",
        type=int,
        default=0,
        help="If >0, keep only themes mateIn1..mateInN (skips bare 'mate')",
    )
    ap.add_argument("--min-rating", type=int, default=0)
    ap.add_argument("--min-popularity", type=int, default=0)
    ap.add_argument(
        "--exclude-catalog",
        type=Path,
        default=None,
        help="Skip FENs already in this MatePath catalog.json (placement+STM)",
    )
    args = ap.parse_args()

    exclude: set[str] = set()
    if args.exclude_catalog is not None:
        if not args.exclude_catalog.is_file():
            print(f"missing exclude catalog: {args.exclude_catalog}", file=sys.stderr)
            return 2
        import json

        data = json.loads(args.exclude_catalog.read_text(encoding="utf-8"))
        for e in data.get("entries") or []:
            fen = (e.get("fen") or "").strip()
            if fen:
                exclude.add(" ".join(fen.split()[:2]))
        print(f"exclude catalog keys: {len(exclude)}", file=sys.stderr)

    if not args.csv.is_file():
        print(f"missing csv: {args.csv}", file=sys.stderr)
        return 2

    proc, fh = open_csv(args.csv)
    seen: set[str] = set()
    kept = 0
    scanned = 0
    bad_move = 0
    try:
        reader = csv.DictReader(fh)
        if not reader.fieldnames or "FEN" not in reader.fieldnames:
            print("CSV missing FEN column", file=sys.stderr)
            return 2
        if "Moves" not in reader.fieldnames:
            print("CSV missing Moves column", file=sys.stderr)
            return 2
        args.out.parent.mkdir(parents=True, exist_ok=True)
        with args.out.open("w", encoding="utf-8") as out:
            out.write(
                f"# lichess mate filter min_pieces={args.min_pieces} "
                f"max_mate_in={args.max_mate_in} apply_first_move=1 "
                f"source={args.csv.name}\n"
            )
            for row in reader:
                scanned += 1
                fen = (row.get("FEN") or "").strip()
                themes = row.get("Themes") or ""
                moves = row.get("Moves") or ""
                if not fen or not has_mate_theme(themes):
                    continue
                if not mate_in_ok(themes, args.max_mate_in):
                    continue
                solved = puzzle_solver_fen(fen, moves)
                if solved is None:
                    bad_move += 1
                    continue
                board = chess.Board(solved)
                if len(board.piece_map()) < args.min_pieces:
                    continue
                try:
                    rating = int(row.get("Rating") or 0)
                except ValueError:
                    rating = 0
                try:
                    pop = int(row.get("Popularity") or 0)
                except ValueError:
                    pop = 0
                if rating < args.min_rating or pop < args.min_popularity:
                    continue
                key = " ".join(solved.split()[:2])
                if key in seen or key in exclude:
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

    print(
        f"scanned={scanned} kept={kept} bad_move={bad_move} → {args.out}",
        file=sys.stderr,
    )
    return 0  # empty keep is success for callers (mate_repo_update)


if __name__ == "__main__":
    sys.exit(main())
