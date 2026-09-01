#!/usr/bin/env python3
"""Build THEORETICAL catalog: curated seed + optional insufficient-material expansion.

Usage:
  python3 builders/theory_build.py \\
    --seed testdata/theoretical/seed.json \\
    --out testdata/theoretical \\
    --version 2026.08.29 \\
    --expand-insufficient

Also copies to catalogs/theoretical/v{version}/ when --lab is set.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path


def fen_key(fen: str) -> str:
    parts = fen.split()
    if len(parts) < 2:
        raise ValueError(f"short FEN: {fen}")
    return f"{parts[0]} {parts[1]}"


def sq(file: int, rank: int) -> int:
    return rank * 8 + file


def algebraic(i: int) -> str:
    return f"{chr(ord('a') + (i % 8))}{1 + (i // 8)}"


def kings_adjacent(a: int, b: int) -> bool:
    af, ar = a % 8, a // 8
    bf, br = b % 8, b // 8
    return max(abs(af - bf), abs(ar - br)) <= 1


def board_fen(pieces: dict[int, str]) -> str:
    """pieces: square_index -> piece char (P/N/B/R/Q/K/p/n/b/r/q/k)."""
    rows = []
    for rank in range(7, -1, -1):
        empty = 0
        row = []
        for file in range(8):
            i = sq(file, rank)
            if i in pieces:
                if empty:
                    row.append(str(empty))
                    empty = 0
                row.append(pieces[i])
            else:
                empty += 1
        if empty:
            row.append(str(empty))
        rows.append("".join(row))
    return "/".join(rows)


def expand_kvk() -> list[dict]:
    """All legal K vs K (kings not adjacent), both STM."""
    out = []
    for wk in range(64):
        for bk in range(64):
            if wk == bk or kings_adjacent(wk, bk):
                continue
            placement = board_fen({wk: "K", bk: "k"})
            for stm in ("w", "b"):
                fen = f"{placement} {stm} - - 0 1"
                out.append(
                    {
                        "fen": fen,
                        "result": "D",
                        "note": "K vs K auto",
                        "ref": "FIDE insufficient material",
                    }
                )
    return out


def expand_minor_vs_k(piece: str, note: str) -> list[dict]:
    """Sample KB/KN vs K: piece on a–d/1–4 grid, kings in distant corners."""
    assert piece in ("B", "N", "BB", "NN")  # single letter for one piece
    out = []
    # two king pairs far apart
    king_pairs = [(sq(0, 0), sq(7, 7)), (sq(7, 0), sq(0, 7)), (sq(0, 7), sq(7, 0))]
    files = range(0, 4)
    ranks = range(0, 4)
    for wk, bk in king_pairs:
        for f in files:
            for r in ranks:
                p = sq(f, r)
                if p in (wk, bk):
                    continue
                pieces = {wk: "K", bk: "k", p: piece}
                placement = board_fen(pieces)
                for stm in ("w", "b"):
                    out.append(
                        {
                            "fen": f"{placement} {stm} - - 0 1",
                            "result": "D",
                            "note": note,
                            "ref": "FIDE insufficient material",
                        }
                    )
    return out


def load_seed(path: Path) -> tuple[str, list[dict]]:
    data = json.loads(path.read_text(encoding="utf-8"))
    base = data.get("content_version_base", "theory")
    entries = data.get("entries", [])
    return base, entries


def load_extra(path: Path) -> list[dict]:
    data = json.loads(path.read_text(encoding="utf-8"))
    if isinstance(data, list):
        return data
    return list(data.get("entries") or [])


def merge_entries(entries: list[dict]) -> list[dict]:
    by_key: dict[str, dict] = {}
    for e in entries:
        key = fen_key(e["fen"])
        # curated (non-auto) wins over auto on same key
        prev = by_key.get(key)
        if prev is None:
            by_key[key] = e
            continue
        prev_auto = "auto" in (prev.get("note") or "")
        cur_auto = "auto" in (e.get("note") or "")
        if prev_auto and not cur_auto:
            by_key[key] = e
        # else keep prev
    # stable order: key sorted
    return [by_key[k] for k in sorted(by_key.keys())]


def write_catalog(out_dir: Path, version: str, entries: list[dict]) -> Path:
    out_dir.mkdir(parents=True, exist_ok=True)
    catalog = {
        "schema_version": 1,
        "content_version": version,
        "layer": "THEORETICAL",
        "entries": [
            {
                "fen": e["fen"],
                "result": e["result"],
                "note": e.get("note", ""),
                **({"ref": e["ref"]} if e.get("ref") else {}),
            }
            for e in entries
        ],
    }
    # TheoreticalStore ignores unknown fields; strip ref from entries if we want strict — keep note only
    for e in catalog["entries"]:
        e.pop("ref", None)

    cat_path = out_dir / "catalog.json"
    text = json.dumps(catalog, indent=2) + "\n"
    cat_path.write_text(text, encoding="utf-8")
    digest = hashlib.sha256(text.encode()).hexdigest()
    manifest = {
        "schema_version": 1,
        "layer": "THEORETICAL",
        "content_version": version,
        "checksum": f"sha256:{digest}",
        "n_entries": len(entries),
        "source": "theory_build.py",
    }
    (out_dir / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return cat_path


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--seed", type=Path, default=Path("testdata/theoretical/seed.json"))
    ap.add_argument(
        "--extra",
        type=Path,
        action="append",
        default=[],
        help="Extra JSON seed fragment(s) (e.g. fortresses_theory.json); repeatable",
    )
    ap.add_argument("--out", type=Path, default=Path("testdata/theoretical"))
    ap.add_argument("--version", default=None, help="content_version (default: seed base)")
    ap.add_argument(
        "--expand-insufficient",
        action="store_true",
        help="Add auto KvK + sample KB/KN vs K draws",
    )
    ap.add_argument(
        "--lab",
        type=Path,
        default=None,
        help="Also write catalogs/theoretical/v{version}/ (e.g. catalogs/theoretical)",
    )
    ap.add_argument(
        "--min-pieces",
        type=int,
        default=0,
        help="If >0, drop entries with fewer pieces (lab: 7 with 6-man on)",
    )
    args = ap.parse_args()

    base, seed_entries = load_seed(args.seed)
    version = args.version or base
    entries = list(seed_entries)
    for extra in args.extra:
        entries.extend(load_extra(extra))
    if args.expand_insufficient:
        entries.extend(expand_kvk())
        entries.extend(expand_minor_vs_k("B", "KB vs K auto"))
        entries.extend(expand_minor_vs_k("N", "KN vs K auto"))
    if args.min_pieces > 0:
        kept = []
        for e in entries:
            try:
                import chess

                n = len(chess.Board(e["fen"]).piece_map())
            except Exception:
                placement = e["fen"].split()[0]
                n = sum(1 for c in placement if c.isalpha())
            if n >= args.min_pieces:
                kept.append(e)
        entries = kept
    merged = merge_entries(entries)
    write_catalog(args.out, version, merged)
    print(f"wrote {len(merged)} entries → {args.out}/catalog.json version={version}")

    if args.lab is not None:
        lab_dir = args.lab / f"v{version}"
        lab_dir.mkdir(parents=True, exist_ok=True)
        shutil.copy2(args.out / "catalog.json", lab_dir / "catalog.json")
        shutil.copy2(args.out / "manifest.json", lab_dir / "manifest.json")
        print(f"lab copy → {lab_dir}")


if __name__ == "__main__":
    main()
