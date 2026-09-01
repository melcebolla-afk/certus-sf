# Interop — fichero EMPIRICAL_ICCF Bárbol → Certus

**Audiencia:** repo Bárbol (FEAT que exporta stats ICCF al formato Certus).  
**Consumidor:** Certus (`IccfPath` / capa `EMPIRICAL_ICCF`).  
**Fecha de contrato:** 2026-08-29.  
**Repo Certus:** este documento en `docs/interop/barbol-iccf-file.md`.  
**Contexto Certus:** FEAT-0009 cerrada en runtime; solo lee disco.

---

## 1) Objetivo

Bárbol ya dispone (o extenderá) de un pipeline que, a partir de un **fichero de partidas ICCF**, genera estadísticas por posición.  
Esta FEAT debe **emitir el artefacto que Certus carga** con `IccfPath`, sin que Certus llame a Bárbol, MariaDB ni red.

Modelo: igual que Syzygy / consenso — hay índice → se usa; no hay → miss.

---

## 2) Origen de datos (normativo)

| Permitido | Prohibido |
|-----------|-----------|
| Partidas / dumps **ICCF** (correspondencia) | ChessDB, Lichess cloud, OTB, Mega Database, opening explorers |

Las stats deben ser **solo ICCF**. Si el pipeline interno mezcla fuentes, filtrar antes de exportar.

---

## 3) Cuándo emitir una entrada

Para cada FEN (clave Certus = **placement + side-to-move**):

| Condición | Acción |
|-----------|--------|
| Hay agregación W/D/L con `n` partidas tras filtros de calidad Bárbol | **Candidata** a emitir |
| `n`, elo, fecha no cumplen los **gates del catalog** que Bárbol elija escribir | Puede omitirse en export **o** incluirse y dejar que Certus haga miss en probe (preferible **omitir** basura) |

Certus en runtime aplica gates del `catalog.json` (`min_n`, `min_elo`, `min_date`, `min_confidence`). Defaults Certus v1:

| Gate | Default |
|------|---------|
| `min_n` | 10 |
| `min_elo` | 2000 |
| `min_date` | `2000-01-01` |
| `min_confidence` | 0.5 |

Bárbol puede usar los mismos umbrales al filtrar, o más estrictos; debe **documentar** los valores escritos en el catalog.

### WDL empírico

- `w` / `d` / `l` según mayoría (o regla documentada: p. ej. argmax de conteos W/D/L desde el punto de vista del **side-to-move**).
- Documentar en la FEAT Bárbol cómo se define el resultado desde STM (imprescindible para no invertir W↔L).

### Confidence sugerida (compatible Certus)

```text
confidence = clamp(0.35 + 0.15 * log10(n), 0.35, 0.85)
```

Otra fórmula OK si se documenta y el valor queda en `[0,1]`.

---

## 4) Artefacto normativo (`catalog.json`)

### Drop dominical (lab Certus — 2026-09-01)

Misma raíz que consenso:

```text
/home/mcebolla/evidence/evidence/from_barbol/iccf/v{YYYY.MM.DD}/
  catalog.json
  manifest.json
```

| Regla | Valor |
|-------|--------|
| Carpeta | `vYYYY.MM.DD` (fecha del domingo) — **no** `viccf-YYYY.MM.DD` |
| Atómico | `…/vDATE.partial/` → `mv` → `…/vDATE/` |
| `layer` en catalog | exactamente `EMPIRICAL_ICCF` |

Ver también § drop en `docs/interop/barbol-consensus-file.md`.

### Layout de salida (dónde dejarlo)

```text
{BARBOL_EXPORT_ROOT}/certus/iccf/v{content_version}/
  catalog.json      # obligatorio
  manifest.json     # recomendado
```

Producción / EvidenceRoot Certus:

```text
{EvidenceRoot}/iccf/v{content_version}/catalog.json
```

UCI:

```text
setoption name IccfPath value /ruta/…/v{content_version}
```

(`IccfPath` = directorio con `catalog.json` **o** ruta al fichero.)

`{BARBOL_EXPORT_ROOT}` configurable (env/flag); no hardcodear path de lab del operador.

**Comando lab Bárbol (EMPIRICAL_ICCF → staging Certus):** desde la raíz del repo Bárbol:

```bash
./venv/bin/python tools/iccf_pgn_stats_load.py \
  --pgn-file … --min-date … --min-elo … \
  --certus-export
```

Con `--certus-export` la salida por defecto es:

```text
/home/mcebolla/evidence/evidence/from_barbol/iccf/v{YYYY.MM.DD}/
  catalog.json
  manifest.json
```

(Ajustar PGN/gates al dump del operador.) Flujo completo: `docs/runbooks/layers-and-train.md` §1.5.

### Schema `catalog.json`

```json
{
  "schema_version": 1,
  "content_version": "iccf-YYYY.MM.DD",
  "layer": "EMPIRICAL_ICCF",
  "min_n": 10,
  "min_elo": 2000,
  "min_date": "2000-01-01",
  "min_confidence": 0.5,
  "entries": [
    {
      "fen": "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
      "wdl": "d",
      "n": 42,
      "elo": 2280,
      "date": "2018-06-01",
      "confidence": 0.74,
      "sources": ["iccf"]
    }
  ]
}
```

| Campo | Obligatorio | Notas |
|-------|-------------|--------|
| `schema_version` | sí | `1` |
| `content_version` | sí | versionable |
| `layer` | sí | exactamente `EMPIRICAL_ICCF` |
| `min_n` / `min_elo` / `min_date` / `min_confidence` | recomendado | gates runtime Certus |
| `entries[].fen` | sí | FEN legal |
| `entries[].wdl` | sí | `w`\|`d`\|`l` |
| `entries[].n` | sí | nº partidas (o posiciones-agregadas) que sustentan el WDL |
| `entries[].elo` | recomendado | elo medio/mínimo de la muestra (documentar cuál) |
| `entries[].date` | recomendado | `YYYY-MM-DD` (p. ej. mediana / min fecha de partidas) |
| `entries[].confidence` | sí | float |
| `entries[].sources` | recomendado | p. ej. `["iccf"]` |

### `manifest.json` (recomendado)

```json
{
  "layer": "EMPIRICAL_ICCF",
  "schema_version": 1,
  "content_version": "iccf-YYYY.MM.DD",
  "checksum": "sha256:…",
  "coverage": { "positions": 12345 },
  "sources": ["iccf-pgn-stats"],
  "built_at": "2026-08-29T12:00:00Z"
}
```

---

## 5) Semántica en Certus

```text
… > STRONG_CONSENSUS > EMPIRICAL_ICCF > INFERENCE
```

- Hit → `evidence=EMPIRICAL_ICCF` (empírico; **no** `PROVEN_*`; `hard_evidence=false`).
- Miss → siguiente capa / Inference.
- Certus **no** regenera stats ni consulta BD.

---

## 6) Alcance sugerido FEAT Bárbol

**Incluir:**

- Reutilizar el desarrollo existente de stats desde PGN/fichero ICCF.
- Export a `catalog.json` (+ `manifest`) conforme §4.
- Path §4 (`…/certus/iccf/v{content_version}/`).
- Documentar: definición WDL desde STM, significado de `elo`/`date`/`n`, gates usados.
- Scope controlable (CLI/fichero de FENs o “todas las posiciones del stats run”); evitar wipe de EvidenceRoot de producción sin OK.

**Excluir:**

- Acoplar runtime Certus a Bárbol.
- Incluir OTB/ChessDB/Lichess en este artefacto.
- Que Certus ejecute el builder.

**CA sugeridos:**

- CA-1: Fixture pequeño → `catalog.json` válido carga en Certus (`IccfPath`).
- CA-2: Entrada con `n` bajo aparece omitida **o** Certus hace miss con gates default.
- CA-3: `layer=EMPIRICAL_ICCF`, checksum en manifest.
- CA-4: Solo fuente ICCF documentada.
- CA-5: Path de salida §4 documentado.

---

## 7) Opción B — export intermedio

Si Bárbol prefiere un JSON propio, debe mapear 1:1 a §4. Certus tiene `builders/iccf_ingest.py` (formato `rows[]` con fen/wdl/n/elo/date) como referencia de ingest; **el contrato del motor sigue siendo `catalog.json`.**

---

## 8) Contacto de schema

Bump de `schema_version` o cambio de semántica STM/WDL → acordar en ambos repos.
