# Product spec extract — capas evidencia (desde Certus PRODUCT-SPEC)

Documento completo: `evidence/docs/PRODUCT-SPEC.md`.  
Este extract es la **norma de producto** para certus-sf v1.

## Objetivo producto

Motor UCI cuya evaluación pasa por **Evidence Resolver** con capas fuertes y precedencia fija. Foco **análisis / correspondencia (ICCF)** — no TCEC como métrica primaria.

## Capas admitidas v1

Solo estas producen evaluación etiquetada como evidencia:

| Código | Actualizable offline |
|--------|----------------------|
| PROVEN_TB | Syzygy path |
| PROVEN_MATE | Índice + suite mates |
| THEORETICAL | Catálogo versionado |
| STRONG_CONSENSUS | Rebuild Bárbol/builder |
| EMPIRICAL_ICCF | Rebuild ICCF stats |

INFERENCE = NNUE Stockfish. **Nunca** evidencia.

## Precedencia

```text
PROVEN_TB > PROVEN_MATE > THEORETICAL > STRONG_CONSENSUS > EMPIRICAL_ICCF > INFERENCE
```

## Gates EMPIRICAL_ICCF

- Origen solo ICCF (no OTB/Lichess/ChessDB).
- min_n, min_elo, min_date, min_confidence (ver STORE-FORMATS.md).

## Consenso

- Solo posiciones con ≥1 `marked_move` legal en catalog.
- Multi-marked permitido; root fuerza primer marked legal.
- Builder Bárbol-first; runtime solo lectura.

## No-objetivos v1 certus-sf

- Sustituir panel Bárbol / MariaDB runtime.
- ChessDB, Lichess cloud, OTB como capa.
- Train NNUE / self-play producto.
- UCI_Chess960 (YAGNI ICCF).
- Libro de aperturas.

## Usuario

Operador análisis CC: configura paths, interpreta `info string evidence=…`, ajusta `ResultBias`.

## Actualización capas

Cada capa: dataset + manifest + builder offline + loader `setoption` reload.

Actualizar capa **no** requiere recompilar motor (salvo schema incompatible).

## Relación Bárbol

Runtime sin API Bárbol. Builders offline consumen export. Ver `docs/interop/`.

## Licencia

GPL-3.0-or-later. Stockfish base GPL-3.

## OQ cerradas relevantes (2026-08-29)

| OQ | Decisión |
|----|----------|
| OQ-5 | ICCF sí; ChessDB/OTB no |
| OQ-8 | reload setoption |
| OQ-12 | EvidenceInfo Off\|Root\|All |
| OQ-16 | ResultBias |
| OQ-19 | evidence en info string; score UCI estándar |
| OQ-23 | repos separados consumidor |

## Éxito v1 certus-sf

- Binario SF + capas carga fixtures Certus.
- Golden tests verdes.
- Ops: builders refrescan capas igual que Certus lab.
- Análisis ICCF: profundidad SF + hits consensus/theory/iccf visibles en UCI.
