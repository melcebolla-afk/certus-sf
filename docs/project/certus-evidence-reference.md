# Referencia repo Certus (evidence)

Oráculo de diseño y regresión durante el port a certus-sf.

## Paths locales

| Recurso | Ruta |
|---------|------|
| Repo | `/home/mcebolla/evidence` |
| Resolver | `crates/evidence-engine/src/resolver.rs` |
| Stores | `crates/evidence-engine/src/{consensus,iccf,theoretical,mate_index}.rs` |
| UCI | `crates/evidence-engine/src/uci.rs` |
| PRODUCT-SPEC | `docs/PRODUCT-SPEC.md` |
| Runbook capas | `docs/runbooks/layers-and-train.md` |
| Datos lab | `evidence/consensus/`, `evidence/iccf/`, … |

## Sync datos (opcional)

```bash
rsync -av /home/mcebolla/evidence/evidence/consensus/ /home/mcebolla/certus-sf/evidence/consensus/
rsync -av /home/mcebolla/evidence/evidence/iccf/       /home/mcebolla/certus-sf/evidence/iccf/
```

## Re-export bootstrap

Si evidence actualiza el export:

```bash
/home/mcebolla/evidence/builders/copy_certus_sf_bootstrap.sh /home/mcebolla/certus-sf
```
