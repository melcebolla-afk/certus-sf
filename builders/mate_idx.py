"""Write MatePath catalog.idx (CMTE) — paridad mate_index.rs write_idx_file."""

from __future__ import annotations

import json
import struct
from pathlib import Path

from certus_hash import hash_placement_stm_fen

IDX_MAGIC = b"CMTE"
IDX_SCHEMA = 1
ENTRY_SIZE = 16


def load_catalog_entries(catalog_json: Path) -> tuple[dict[int, tuple[int, bool]], str]:
    data = json.loads(catalog_json.read_text(encoding="utf-8"))
    if data.get("schema_version") != 1:
        raise ValueError(f"unsupported schema_version {data.get('schema_version')}")
    layer = data.get("layer") or ""
    if layer and layer != "PROVEN_MATE":
        raise ValueError(f"unexpected layer {layer}")
    version = data["content_version"]
    out: dict[int, tuple[int, bool]] = {}
    for e in data.get("entries") or []:
        fen = (e.get("fen") or "").strip()
        if not fen:
            continue
        key = hash_placement_stm_fen(fen)
        plies = int(e["plies"])
        stm_wins = bool(e.get("stm_wins", True))
        out[key] = (plies, stm_wins)
    return out, version


def write_idx_file(idx_path: Path, content_version: str, entries: dict[int, tuple[int, bool]]) -> None:
    sorted_items = sorted(entries.items(), key=lambda x: x[0])
    tmp = idx_path.with_suffix(".idx.tmp")
    ver_b = content_version.encode("utf-8")
    with tmp.open("wb") as w:
        w.write(IDX_MAGIC)
        w.write(struct.pack("<I", IDX_SCHEMA))
        w.write(struct.pack("<Q", len(sorted_items)))
        w.write(struct.pack("<I", len(ver_b)))
        w.write(ver_b)
        for key, (plies, stm_wins) in sorted_items:
            w.write(struct.pack("<Q", key))
            w.write(bytes([plies & 0xFF, 1 if stm_wins else 0, 0, 0, 0, 0, 0, 0]))
    tmp.replace(idx_path)


def write_idx_from_json(catalog_json: Path) -> Path:
    entries, version = load_catalog_entries(catalog_json)
    if catalog_json.name == "catalog.json":
        idx_path = catalog_json.parent / "catalog.idx"
    else:
        idx_path = catalog_json.with_suffix(".idx")
    write_idx_file(idx_path, version, entries)
    return idx_path
