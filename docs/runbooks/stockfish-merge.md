# Runbook — merge Stockfish upstream

## Modelo

- **`stockfish/`** — árbol Stockfish **vendored** en certus-sf (no submodule al repo oficial).
- **`stockfish/src/evidence/`** — código producto Certus (stores, resolver futuro).
- **`stockfish/src/certus/`** — capa integración mínima + **`certus.mk`** (build overlay).
- **`stockfish/UPSTREAM`** — pin de tag/commit upstream.

Nada se envía a `official-stockfish/Stockfish`. Los merges son **locales** en este repo.

## Archivos upstream tocados (mínimo)

Ver checklist en [`stockfish/src/certus/TOUCHED_UPSTREAM.md`](../../stockfish/src/certus/TOUCHED_UPSTREAM.md):

| Fichero | Cambio |
|---------|--------|
| `src/Makefile` | `-include certus/certus.mk` + línea `objclean` con `$(CERTUS_CLEAN)` |
| `src/engine.h` | `Certus::EngineExtension certus_` + accessor |
| `src/engine.cpp` | `certus_.register_options(...)` |
| `src/misc.cpp` | `#ifdef CERTUS_SF` → `Certus::engine_identity()` |

Todo lo demás (binario `certus-sf`, fuentes evidence, tests) vive en **overlay** `certus.mk`.

## Procedimiento

1. Elegir tag upstream (p. ej. `sf_19`): https://github.com/official-stockfish/Stockfish/releases
2. Ejecutar:

```bash
chmod +x scripts/merge-stockfish.sh
./scripts/merge-stockfish.sh sf_19
```

3. Re-aplicar parches de `TOUCHED_UPSTREAM.md` si el merge los pisó (el script **no** los re-aplica automáticamente).
4. Validar:

```bash
make -j -C stockfish/src build evidence_probe ARCH=x86-64-sse41-popcnt
./tests/evidence_probe.sh
```

5. Actualizar `stockfish/UPSTREAM`, `docs/project/inventory.md`, changelog.

## Conflictos habituales

- **`Makefile`**: conservar `-include certus/certus.mk` y la segunda línea de `objclean`.
- **`engine.cpp`**: reinsertar una línea `certus_.register_options` tras `EvalFileSmall`.
- **NNUE / eval**: resolver como upstream; no mezclar train Certus.

## Alternativa manual

```bash
git clone --depth 1 --branch sf_XX https://github.com/official-stockfish/Stockfish.git /tmp/sf
rsync -a --delete --exclude src/evidence --exclude src/certus /tmp/sf/ stockfish/
# restaurar evidence/ + certus/ desde git
```
