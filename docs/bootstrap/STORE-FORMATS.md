# Store formats — capas evidencia (compatible Certus)

## Convención paths

```text
{EvidenceRoot}/{layer}/v{content_version}/catalog.json
{EvidenceRoot}/{layer}/v{content_version}/manifest.json   # recomendado
```

UCI path = directorio `v…/` **o** ruta directa a `catalog.json`.

`EvidencePath` (opcional): raíz; auto-pick versión más reciente por mtime por subcapa.

## STRONG_CONSENSUS — `catalog.json`

Ver `interop/barbol-consensus-file.md`.

```json
{
  "schema_version": 1,
  "content_version": "consensus-YYYY.MM.DD",
  "layer": "STRONG_CONSENSUS",
  "min_confidence": 0.8,
  "entries": [{
    "fen": "...",
    "wdl": "w|d|l",
    "confidence": 0.92,
    "marked_moves": ["g1f3", "b1c3"],
    "sources": ["..."],
    "budgets": { "ab_nodes": 50000000, "mcts_nodes": 5000000 }
  }]
}
```

Runtime: ignorar entry si `confidence < min_confidence`.

## EMPIRICAL_ICCF — `catalog.json`

Ver `interop/barbol-iccf-file.md`.

Gates default Certus:

| Gate | Default |
|------|---------|
| min_n | 10 |
| min_elo | 2000 |
| min_date | 2000-01-01 |
| min_confidence | 0.5 |

## THEORETICAL — `catalog.json`

```json
{
  "schema_version": 1,
  "content_version": "YYYY.MM.DD",
  "layer": "THEORETICAL",
  "entries": [{
    "fen": "...",
    "result": "W|D|L",
    "note": "optional"
  }]
}
```

## PROVEN_MATE — `catalog.json` + `catalog.idx`

### JSON (fixtures / fallback)

```json
{
  "schema_version": 1,
  "content_version": "...",
  "layer": "PROVEN_MATE",
  "entries": [{
    "fen": "...",
    "plies": 3,
    "stm_wins": true
  }]
}
```

### Binary index `catalog.idx` (producción)

| Offset | Size | Field |
|--------|------|-------|
| 0 | 4 | magic `CMTE` |
| 4 | 4 | schema u32 = 1 |
| 8 | … | header (content_version string — ver Rust impl) |
| table | 16 × N | entries sorted by key u64 |

Entry (16 bytes):

| Field | Type |
|-------|------|
| key | u64 Zobrist placement+STM |
| plies | u8 |
| stm_wins | u8 |
| pad | 6 bytes |

Probe: binary search on sorted keys.

Referencia implementación: `crates/evidence-engine/src/mate_index.rs`.

## manifest.json (todas las capas)

```json
{
  "layer": "STRONG_CONSENSUS",
  "schema_version": 1,
  "content_version": "...",
  "checksum": "sha256:...",
  "coverage": { "positions": 12345 },
  "sources": ["..."],
  "built_at": "ISO8601"
}
```

## Staging Bárbol (domingo)

```text
evidence/from_barbol/consensus/vYYYY.MM.DD/
evidence/from_barbol/iccf/vYYYY.MM.DD/
```

Promote atómico: `vDATE.partial/` → rename → `vDATE/`.

Script: `barbol_layers_promote.py`.

## Syzygy (PROVEN_TB)

No catalog Certus — path UCI `SyzygyPath` estándar SF.

Regla ops: MatePath/Theory **no** duplicar ≤6 piezas cubiertas por TB 6-man.
