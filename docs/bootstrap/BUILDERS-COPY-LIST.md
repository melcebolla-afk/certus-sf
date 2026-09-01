# Builders — lista certus-sf

**Canónico:** `docs/project/builders.md` y `docs/runbooks/catalogs-layers.md`

Salida de capas: **`catalogs/`** (no `evidence/` en disco).

| Script | Capa | Cron |
|--------|------|------|
| `mate_repo_update.py` | PROVEN_MATE | dom 04:00 |
| `theory_repo_update.py` | THEORETICAL | dom 04:45 |
| Bárbol `--certus-sf-export` | consensus + iccf | dom 04:15 |

`barbol_layers_promote.py` — **deprecado** (Bárbol escribe directo en `catalogs/`).

Paridad schema con `evidence/builders/` — portar diffs manualmente.
