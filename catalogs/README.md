# catalogs/ — EvidenceRoot de certus-sf

Catálogos versionados que consume el motor vía UCI `EvidencePath` (o `*Path` por capa).

**No confundir con:**

- `stockfish/src/evidence/` — código C++ del resolver
- `testdata/` — fixtures pequeños para CI (`evidence_probe`, `golden_probe`)
- `/home/mcebolla/evidence/` — oráculo Rust (referencia; no es el EvidenceRoot de este repo)

## Layout

```text
catalogs/
  consensus/v2026.09.01/{catalog.json,manifest.json}   ← Bárbol (export directo)
  iccf/v2026.09.01/…                                   ← Bárbol
  mate/v2026.08.29/{catalog.json,catalog.idx,manifest.json}
  theoretical/v2026.08.29/…
```

Convención de carpeta: **`v{YYYY.MM.DD}`** (ver `docs/bootstrap/STORE-FORMATS.md`).

Bárbol escribe **directamente** en `catalogs/consensus/` y `catalogs/iccf/` (sin staging `from_barbol/` ni promote).

## UCI (lab)

```text
setoption name EvidencePath value /home/mcebolla/certus-sf/catalogs
isready
position fen ...
go depth 20
```

El motor elige la versión más reciente por capa (mtime de `catalog.json`).

Capas individuales:

```text
setoption name ConsensusPath value /home/mcebolla/certus-sf/catalogs/consensus/v2026.09.01
```

## Actualización automática (cron)

Ver `docs/runbooks/catalogs-layers.md`.

| Hora (domingo) | Script | Capa |
|----------------|--------|------|
| 04:00 | `builders/mate_repo_update.py` | mate |
| 04:15 | Bárbol (`chess_idea`, `--certus-sf-export`) | consensus + iccf |
| 04:45 | `builders/theory_repo_update.py` | theoretical |

Logs: `train/out/*.log`

## Bootstrap inicial (opcional)

Si `catalogs/mate/` está vacío, el cron de mate necesita un catálogo base para merge:

```bash
mkdir -p catalogs/mate/v2026.08.29
cp -a testdata/mate/catalog.json testdata/mate/catalog.idx testdata/mate/manifest.json \
  catalogs/mate/v2026.08.29/
```

Teoría usa `testdata/theoretical/seed.json` como semilla curada en cada run.

## Git

Los catálogos en `catalogs/*/v*/` se versionan en este repo (~pocos MB). Tras clone:

```text
setoption name EvidencePath value /path/to/certus-sf/catalogs
```

Regeneración / ampliación: cron y builders (§ Actualización automática).

## Guardrails

- No borrar `catalogs/*/v*` sin OK humano.
- Export Bárbol: escritura atómica (directorio completo antes de visible).
- Cambios breaking de `schema_version` → sync con repo `evidence` + motor.
