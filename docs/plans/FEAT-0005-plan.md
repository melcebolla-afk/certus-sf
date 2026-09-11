# Plan — FEAT-0005 Stockfish 19 merge

## Fuera de alcance técnico (no construir)

- Refactors de evidence/search no exigidos por el merge.
- Actualizar builders/mate o catálogos dirty del working tree.
- Cambios de producto CertusStyle / opciones UCI.

## Fases

### Phase 0 — Preparación

- Validar tag `sf_19` y fuentes clonables.
- Backup parches Certus vs `sf_18`.
- Validación: tag remoto `edb0d9db`; patches en `/tmp/certus-sf-patches/`.

### Phase 1 — Merge árbol upstream

- `./scripts/merge-stockfish.sh sf_19`
- Validación: `stockfish/UPSTREAM` → `sf_19`; `src/evidence` y `src/certus` intactos.

### Phase 2 — Re-aplicar TOUCHED_UPSTREAM

- Makefile, engine.h/cpp, misc.cpp, ucioption.cpp, thread.cpp, search.cpp.
- Validación: grep `CERTUS_SF` / `certus_` en ficheros listados.

### Phase 3 — Build + tests

- `make -j -C stockfish/src build evidence_probe golden_probe consensus_search_probe ARCH=x86-64-sse41-popcnt`
- `./tests/evidence_probe.sh`
- Validación: exit 0; UCI `uci` muestra `certus-sf` + `sf_19` en identidad si aplica.

### Phase 4 — Docs cierre

- inventory, changelog, QA.
- Validación: pin documentado = UPSTREAM.

## Validación global

```bash
./scripts/merge-stockfish.sh sf_19   # ya hecho en Phase 1
make -j -C stockfish/src build evidence_probe golden_probe consensus_search_probe ARCH=x86-64-sse41-popcnt
./tests/evidence_probe.sh
```

## Cierre de fase

- Phase 0: **CERRADA** 2026-09-11 — tag `sf_19` @ `edb0d9db` disponible.
- Phase 1: **CERRADA** 2026-09-11 — merge script OK.
- Phase 2: **CERRADA** 2026-09-11 — hooks + `Networks`→`Network` + probes `Attacks::init`.
- Phase 3: **CERRADA** 2026-09-11 — build + `./tests/evidence_probe.sh: OK`.
- Phase 4: **CERRADA** 2026-09-11 — inventory/changelog/QA.

## Cierre de la unidad

- Inventario + changelog actualizados; QA en `docs/qa/FEAT-0005-qa.md`.
- PROJECT_STATE omitido (salida humana vía changelog).
