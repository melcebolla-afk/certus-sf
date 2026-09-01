"""Shared paths for certus-sf layer builders and cron jobs."""

from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CATALOGS = ROOT / "catalogs"
TRAIN_OUT = ROOT / "train/out"

VERSION_RE = re.compile(r"^v(\d{4}\.\d{2}\.\d{2})$")


def layer_root(layer: str) -> Path:
    return CATALOGS / layer


def newest_version_dir(layer: str) -> Path | None:
    """Newest `catalogs/{layer}/vYYYY.MM.DD/` with catalog.json (mtime)."""
    root = layer_root(layer)
    if not root.is_dir():
        return None
    candidates: list[tuple[float, str, Path]] = []
    for p in root.iterdir():
        if not p.is_dir() or not VERSION_RE.match(p.name):
            continue
        catalog = p / "catalog.json"
        if not catalog.is_file():
            continue
        candidates.append((catalog.stat().st_mtime, p.name, p))
    if not candidates:
        return None
    candidates.sort(key=lambda x: (x[0], x[1]), reverse=True)
    return candidates[0][2]


def newest_catalog(layer: str, *, fallback: Path | None = None) -> Path:
    d = newest_version_dir(layer)
    if d is not None:
        return d / "catalog.json"
    if fallback is not None:
        return fallback
    return layer_root(layer) / "catalog.json"
