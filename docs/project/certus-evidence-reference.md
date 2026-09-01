# Referencia — datos evidencia certus-sf

## EvidenceRoot (este repo)

```text
/home/mcebolla/certus-sf/catalogs/
  consensus/v…/
  iccf/v…/
  mate/v…/
  theoretical/v…/
```

UCI:

```text
setoption name EvidencePath value /home/mcebolla/certus-sf/catalogs
```

## Oráculo Rust (referencia, no runtime)

| Recurso | Ruta |
|---------|------|
| Repo | `/home/mcebolla/evidence` |
| Resolver spec | `crates/evidence-engine/src/resolver.rs` |
| `mate_build.rs` | referencia de paridad para `builders/mate_build.py` |

## CI vs lab

| Ruta | Uso |
|------|-----|
| `testdata/` | Golden, `evidence_probe`, CI |
| `catalogs/` | Lab/producción local, cron, Bárbol |

No mezclar.

## Bootstrap histórico (evidence → certus-sf)

Si hace falta copia one-off desde el lab Rust:

```bash
rsync -av /home/mcebolla/evidence/evidence/mate/        /home/mcebolla/certus-sf/catalogs/mate/
rsync -av /home/mcebolla/evidence/evidence/theoretical/ /home/mcebolla/certus-sf/catalogs/theoretical/
```

Consensus/iccf: Bárbol escribe directo en `catalogs/` (`--certus-sf-export`).
