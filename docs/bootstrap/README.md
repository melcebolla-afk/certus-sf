# Export bootstrap — Certus → certus-sf (Stockfish fork)

**Generado:** 2026-09-01 desde repo `evidence` (Certus).  
**Propósito:** copiar este directorio al **nuevo repo** (`certus-sf` o nombre TBD) y usar `STARTUP-PROMPT.md` como primer mensaje en Cursor.

## Contenido del export

| Fichero | Uso |
|---------|-----|
| **[STARTUP-PROMPT.md](STARTUP-PROMPT.md)** | Prompt para arrancar el proyecto en Cursor (pegar en chat nuevo) |
| [REPO-SETUP.md](REPO-SETUP.md) | Estructura repo, submodule SF, methodology, git |
| [REUSE-MANIFEST.md](REUSE-MANIFEST.md) | Qué copiar desde `evidence` vs referenciar |
| [BUILDERS-COPY-LIST.md](BUILDERS-COPY-LIST.md) | Scripts Python de capas (ingest/promote) |
| [EVIDENCE-PORT.md](EVIDENCE-PORT.md) | Mapa Certus Rust → ficheros Stockfish C++ |
| [RESOLVER-SPEC.md](RESOLVER-SPEC.md) | Reglas normativas del Evidence Resolver |
| [STORE-FORMATS.md](STORE-FORMATS.md) | Formatos `catalog.json`, `catalog.idx`, manifests |
| [UCI-OPTIONS.md](UCI-OPTIONS.md) | Opciones UCI a implementar en el fork |
| [PRODUCT-EVIDENCE-EXTRACT.md](PRODUCT-EVIDENCE-EXTRACT.md) | Spec de producto (solo capas evidencia) |
| [RUNBOOK-LAYERS-EXTRACT.md](RUNBOOK-LAYERS-EXTRACT.md) | Operación capas (sin train NNUE) |
| [GOLDEN-FIXTURES.jsonl](GOLDEN-FIXTURES.jsonl) | Tests de regresión FEN → evidence class |
| [interop/](interop/) | Contratos Bárbol → motor |
| [LICENSE-NOTICE.md](LICENSE-NOTICE.md) | GPL + atribución Stockfish |

## Copia rápida al nuevo repo

```bash
# Desde el checkout de evidence (ajusta rutas):
CERTUS_EVIDENCE=/home/mcebolla/evidence
CERTUS_SF=/path/to/certus-sf   # tu repo nuevo

# 1) Bootstrap docs + fixtures
cp -a "$CERTUS_EVIDENCE/docs/export/certus-sf-bootstrap/." "$CERTUS_SF/docs/bootstrap/"

# 2) Builders de capas (ver BUILDERS-COPY-LIST.md)
mkdir -p "$CERTUS_SF/builders"
for f in consensus_ingest iccf_ingest theory_build theory_repo_update \
         fortresses_import mate_repo_update lichess_mate_filter \
         lichess_theory_candidates barbol_layers_promote; do
  cp "$CERTUS_EVIDENCE/builders/${f}.py" "$CERTUS_SF/builders/"
done

# 3) Testdata mínimo (CI golden)
cp -a "$CERTUS_EVIDENCE/testdata/consensus" "$CERTUS_SF/testdata/"
cp -a "$CERTUS_EVIDENCE/testdata/iccf" "$CERTUS_SF/testdata/"
cp -a "$CERTUS_EVIDENCE/testdata/theoretical" "$CERTUS_SF/testdata/"
cp -a "$CERTUS_EVIDENCE/testdata/mate" "$CERTUS_SF/testdata/" 2>/dev/null || true
cp -a "$CERTUS_EVIDENCE/testdata/syzygy3" "$CERTUS_SF/testdata/" 2>/dev/null || true

# 4) Interop + prior-art evidencia (opcional)
mkdir -p "$CERTUS_SF/docs/interop" "$CERTUS_SF/docs/prior-art"
cp "$CERTUS_EVIDENCE/docs/interop/"*.md "$CERTUS_SF/docs/interop/"
cp "$CERTUS_EVIDENCE/docs/prior-art/"{consensus,iccf,resolver,syzygy}.md "$CERTUS_SF/docs/prior-art/" 2>/dev/null || true

# 5) Methodology (submodule — ver REPO-SETUP.md)
```

## Relación con repo Certus (`evidence`)

| Repo | Rol |
|------|-----|
| **evidence** | Referencia Rust, oráculo de regresión, **fábrica de datos** (builders), congelado o mínimo mantenimiento |
| **certus-sf** | **Producto ICCF**: binario Stockfish + capas evidencia |

Los **artefactos de capas** (`catalog.json`, `catalog.idx`) deben ser **byte-compatible** entre ambos motores durante la transición.

## Próximo paso

1. Crear repo GitHub + workspace Cursor.  
2. Fork/clonar Stockfish como base.  
3. Copiar este export.  
4. Pegar **`STARTUP-PROMPT.md`** en el agente de Cursor del nuevo repo.
