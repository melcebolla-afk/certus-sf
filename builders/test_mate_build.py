#!/usr/bin/env python3
"""Regression tests for mate_build / certus_hash / mate_idx (run from repo root)."""

from __future__ import annotations

import json
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from certus_hash import hash_placement_stm_fen
from mate_idx import load_catalog_entries, write_idx_from_json
from mate_probe import probe_mate_ungated

import chess

ROOT = Path(__file__).resolve().parents[1]
BUILDERS = Path(__file__).resolve().parent
FAILURES = 0


def check(ok: bool, msg: str) -> None:
    global FAILURES
    if not ok:
        print(f"FAIL: {msg}", file=sys.stderr)
        FAILURES += 1
    else:
        print(f"OK: {msg}")


def test_probe_mate() -> None:
    b = chess.Board("6k1/8/6K1/8/8/8/8/7Q w - - 0 1")
    hit = probe_mate_ungated(b, 5)
    check(hit is not None and hit.stm_wins and hit.plies == 1, "mate in one Qh8#")

    b = chess.Board("7k/6Q1/6K1/8/8/8/8/8 b - - 0 1")
    hit = probe_mate_ungated(b, 5)
    check(hit is not None and not hit.stm_wins and hit.plies == 0, "STM checkmated plies=0")

    b = chess.Board()
    check(probe_mate_ungated(b, 5) is None, "startpos no mate")

    b = chess.Board("4r3/1k6/pp3P2/1b5p/3R1p2/P1R2P2/1P4PP/6K1 b - - 0 35")
    hit = probe_mate_ungated(b, 5)
    check(hit is not None and hit.stm_wins and hit.plies == 3, "ungated heavy mate plies=3")


def test_idx_hash_parity() -> None:
    cat = ROOT / "testdata/mate/catalog.json"
    idx = ROOT / "testdata/mate/catalog.idx"
    check(cat.is_file() and idx.is_file(), "testdata mate catalog present")

    entries, version = load_catalog_entries(cat)
    check(version == "2026.08.29", "content_version")
    check(len(entries) == 3, "three idx entries")

    data = json.loads(cat.read_text(encoding="utf-8"))
    for e in data["entries"]:
        key = hash_placement_stm_fen(e["fen"])
        check(key in entries, f"hash key in idx map for {e['fen'][:30]}")
        plies, wins = entries[key]
        check(plies == e["plies"] and wins == e["stm_wins"], f"entry payload {e['fen'][:20]}")

    with tempfile.TemporaryDirectory() as td:
        tmp_cat = Path(td) / "catalog.json"
        shutil.copy2(cat, tmp_cat)
        rebuilt = write_idx_from_json(tmp_cat)
        a = idx.read_bytes()
        b = rebuilt.read_bytes()
        check(a == b, f"idx byte-identical ({len(a)} bytes)")


def test_mate_build_seed() -> None:
    seed = ROOT / "testdata/mate/seed.fens"
    with tempfile.TemporaryDirectory() as td:
        out = Path(td) / "out"
        cmd = [
            sys.executable,
            str(BUILDERS / "mate_build.py"),
            "--seed",
            str(seed),
            "--out",
            str(out),
            "--version",
            "2026.08.29",
            "--max-plies",
            "5",
        ]
        proc = subprocess.run(cmd, capture_output=True, text=True)
        check(proc.returncode == 0, f"mate_build exit 0 ({proc.stderr.strip()})")
        built = json.loads((out / "catalog.json").read_text(encoding="utf-8"))
        ref = json.loads((ROOT / "testdata/mate/catalog.json").read_text(encoding="utf-8"))
        check(len(built["entries"]) == len(ref["entries"]), "seed build entry count")
        ref_keys = {" ".join(e["fen"].split()[:2]) for e in ref["entries"]}
        got_keys = {" ".join(e["fen"].split()[:2]) for e in built["entries"]}
        check(ref_keys == got_keys, "seed build same placement keys")


def test_mate_build_merge() -> None:
    seed = ROOT / "testdata/mate/seed.fens"
    ref = ROOT / "testdata/mate/catalog.json"
    with tempfile.TemporaryDirectory() as td:
        out = Path(td) / "out"
        cmd = [
            sys.executable,
            str(BUILDERS / "mate_build.py"),
            "--seed",
            str(seed),
            "--out",
            str(out),
            "--merge",
            str(ref),
            "--version",
            "2026.08.29",
            "--max-plies",
            "5",
        ]
        proc = subprocess.run(cmd, capture_output=True, text=True)
        check(proc.returncode == 0, "merge build exit 0")
        built = json.loads((out / "catalog.json").read_text(encoding="utf-8"))
        check(len(built["entries"]) == 3, "merge adds nothing duplicate")
        check((out / "catalog.idx").is_file(), "merge build wrote idx")


def main() -> int:
    print("test_mate_build.py", flush=True)
    test_probe_mate()
    test_idx_hash_parity()
    test_mate_build_seed()
    test_mate_build_merge()
    if FAILURES:
        print(f"\n{FAILURES} failure(s)", file=sys.stderr)
        return 1
    print("\nall mate_build tests passed", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
