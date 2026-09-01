# Interop — fichero de consenso Bárbol → Certus

**Audiencia:** repo Bárbol (spec FEAT productora del artefacto).  
**Consumidor:** Certus (`ConsensusPath` / capa `STRONG_CONSENSUS`).  
**Fecha de contrato:** 2026-08-29.  
**Repo Certus de referencia:** este documento en `docs/interop/barbol-consensus-file.md`.

---

## 1) Objetivo

Bárbol genera **offline** un fichero (o árbol versionado) de consenso multi-arquitectura que Certus **solo lee**.  
Certus **no** llama a Bárbol, **no** abre MariaDB/API, y **no** lanza engines UCI para cubrir huecos (mismo modelo que Syzygy: hay dato → se usa; no hay → miss).

---

## 2) Cuándo hay consenso (regla de negocio)

Para una **posición** (FEN canónico: placement + side-to-move):

| Condición | ¿Emitir entrada de consenso? |
|-----------|------------------------------|
| Hay **uno o más** movimientos **marked** (acuerdo multi-familia según reglas Bárbol) | **Sí** |
| Cero marked | **No** (no escribir la posición en el fichero) |

### Movimientos marked

- Un marked es un movimiento UCI (`e2e4`, `e7e8q`, …) que el proceso de consenso de Bárbol considera **válido bajo acuerdo** (p. ej. top acordado entre αβ-NNUE y MCTS-NN a presupuesto).
- **Puede haber más de un marked** en la misma posición: varias jugadas **igualmente válidas** a efectos de consenso (no se fuerza top-1 único).
- Certus **acepta** `marked_moves.length >= 1` y **acepta** `length >= 2` sin degradar la capa.

### Definición operativa de `marked_moves` (cerrada — Certus 2026-08-29)

**1 = A** (intersección fail-closed).

Para una posición con ≥1 oráculo **DONE** `ab_nnue` y ≥1 **DONE** `mcts_nn`:

```text
marked_moves = { UCI | es bestmove de ≥1 DONE ab_nnue
                      Y es bestmove de ≥1 DONE mcts_nn }
```

| Resultado | Acción |
|-----------|--------|
| Intersección no vacía | Emitir entrada; multi-marked = todas las UCI de la intersección |
| Intersección vacía | **No emitir** la posición (no inventar consenso) |

**Rechazado:** B (casi nunca multi-marked) y C (unión de bestmoves con solo acuerdo WDL — trata como “marked” jugadas que una clase no eligió).

WDL de posición: debe ser coherente con el acuerdo de las clases que participan; si las clases no acuerdan WDL de posición en la banda que Bárbol documente → no emitir (aunque hubiera intersección de jugadas; fail-closed).

### Universo de posiciones del primer builder (cerrado — Certus 2026-08-29)

**2 = C** (scope explícito).

- El builder v1 **exige** scope: lista de `expansion_root_id`, fichero de FENs, y/o flags CLI equivalentes.
- Sin scope → **error** o dry-run vacío (documentar uno; preferible **error** en modo no-dry-run).
- **No** escanear toda la BD por defecto (evita catalogs gigantes accidentales).
- Modo `--all` / scan global (**opción A**) = ampliación **posterior**, no requisito del primer FEAT.

### WDL de la posición

Además de los marked, cada entrada lleva un **WDL de posición** (`w` / `d` / `l`) coherente con el acuerdo (resultado esperado con juego de consenso, no “eval de un solo motor”).

---

## 3) Artefacto que debe producir Bárbol

### Opción A — Catálogo Certus directo (preferida si Bárbol escribe el formato final)

Directorio o fichero legible por Certus:

```text
ConsensusRoot/
  catalog.json      # obligatorio
  manifest.json     # recomendado
```

UCI Certus: `setoption name ConsensusPath value <ConsensusRoot|catalog.json>`

### Opción B — Export intermedio + ingest Certus

Si Bárbol prefiere un export propio, debe ser transformable 1:1 al `catalog.json` de abajo (Certus ya tiene `builders/consensus_ingest.py` para un export “barbol-lite”; puede adaptarse). **El contrato normativo para el motor es el `catalog.json`.**

---

## 3.1) Dónde dejar el fichero (ruta de salida)

Certus **no** busca un path mágico en el repo Bárbol: el operador apunta `ConsensusPath` al directorio (o `catalog.json`) que Bárbol generó.  
Bárbol debe documentar y respetar una **ruta de salida estable** para no romper scripts/CI.

### Drop dominical (lab Certus — 2026-09-01)

Job Bárbol **domingo**: dejar artefactos listos para Certus aquí (staging; no pisa la versión UCI activa):

```text
/home/mcebolla/evidence/evidence/from_barbol/
  consensus/v{YYYY.MM.DD}/
    catalog.json      # obligatorio
    manifest.json     # obligatorio en job auto
  iccf/v{YYYY.MM.DD}/
    catalog.json
    manifest.json
```

Reglas:

| Regla | Valor |
|-------|--------|
| Fecha en el path | domingo del job, `YYYY.MM.DD` (ISO) |
| Prefijo de carpeta | siempre `v` + fecha → `v2026.09.07` (**no** `viccf-…`, **no** `consensus-` en el nombre de carpeta) |
| `content_version` en JSON | **igual** a la fecha del path (`2026.09.07`); opcional prefijo semántico solo *dentro* del JSON si hace falta (`iccf-2026.09.07`), pero el **directorio** sigue siendo `v2026.09.07` |
| Publicación atómica | escribir `v{YYYY.MM.DD}.partial/` → al terminar `mv` a `v{YYYY.MM.DD}/` |
| No borrar | otras `v…` sin OK; Certus promoverá a `evidence/consensus/` e `evidence/iccf/` (E-05/E-06) |

Contrato de ficheros: este documento + `docs/interop/barbol-iccf-file.md`.

### Convención normativa de layout (EvidenceRoot)

Alineada al PRODUCT-SPEC de Certus (`EvidenceRoot/consensus/v{content_version}/`):

```text
{EvidenceRoot}/
  consensus/
    v{content_version}/
      catalog.json      # obligatorio
      manifest.json     # recomendado (checksum, built_at, coverage)
```

Ejemplo concreto:

```text
/var/lib/evidence/consensus/v2026.08.29/catalog.json
/var/lib/evidence/consensus/v2026.08.29/manifest.json
```

UCI:

```text
setoption name ConsensusPath value /var/lib/evidence/consensus/v2026.08.29
```

(`ConsensusPath` puede ser el directorio `v…` o la ruta directa a `catalog.json`.)

### Convención recomendada **dentro del repo / export Bárbol**

Si el builder corre en el árbol Bárbol (lab/CI), escribir por defecto:

```text
{BARBOL_EXPORT_ROOT}/certus/consensus/v{content_version}/
  catalog.json
  manifest.json
```

- `{BARBOL_EXPORT_ROOT}`: configurable (env/flag del builder; p. ej. `export/` o `artifacts/` en el repo Bárbol). **No** hardcodear un path de producción del operador.
- `{content_version}`: el mismo string que va en `catalog.json` / `manifest.json` (p. ej. `2026.08.29` o `2026.08.29+sha`).
- Cada rebuild crea (o reemplaza de forma atómica) **su** `v{content_version}/`; no borrar otras versiones sin OK explícito.
- Publicación a un `EvidenceRoot` de lab/producción = **copia o rsync** de ese directorio (o symlink), no lectura live de la BD Bárbol desde Certus.

### Resumen operativo

| Rol | Qué hace |
|-----|----------|
| Builder Bárbol | Escribe `{BARBOL_EXPORT_ROOT}/certus/consensus/v{content_version}/{catalog,manifest}.json` |
| Operador / deploy | Coloca ese directorio bajo `{EvidenceRoot}/consensus/v{content_version}/` (o apunta directo al export) |
| Certus | Solo `ConsensusPath` → lee; si falta el path o el FEN → miss |

**Prohibido:** que Certus asuma un path fijo del checkout Bárbol (`~/barbol/...`) sin `ConsensusPath`.

**Comando lab Bárbol (conocido 2026-08-29):** desde la raíz del repo Bárbol:

```bash
./venv/bin/python tools/certus_consensus_export.py --dry-run
./venv/bin/python tools/certus_consensus_export.py --export-root export
```

Flujo completo Certus: `docs/runbooks/layers-and-train.md` §1.

---

## 4) Schema `catalog.json` (normativo para Certus)

```json
{
  "schema_version": 1,
  "content_version": "consensus-YYYY.MM.DD",
  "layer": "STRONG_CONSENSUS",
  "min_confidence": 0.8,
  "entries": [
    {
      "fen": "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
      "wdl": "d",
      "confidence": 0.92,
      "marked_moves": ["g1f3", "b1c3"],
      "sources": ["ab:…", "mcts:…"],
      "budgets": { "ab_nodes": 50000000, "mcts_nodes": 5000000 }
    }
  ]
}
```

### Campos

| Campo | Obligatorio | Notas |
|-------|-------------|--------|
| `schema_version` | sí | `1` |
| `content_version` | sí | string versionable / fecha |
| `layer` | sí | exactamente `STRONG_CONSENSUS` |
| `min_confidence` | no (default 0.8) | Certus ignora entradas con `confidence` &lt; este umbral |
| `entries[].fen` | sí | FEN legal; clave de lookup = **placement + STM** (castling/ep/reloj no distinguen hit) |
| `entries[].wdl` | sí | `w` \| `d` \| `l` (también acepta win/draw/loss) |
| `entries[].confidence` | sí | float ∈ [0,1] |
| `entries[].marked_moves` | sí (producción) | array UCI, **≥ 1**; orden = preferencia opcional (primero = sugerido), todos válidos |
| `entries[].sources` | recomendado | trazabilidad familias/oráculos |
| `entries[].budgets` | opcional | nodos/presupuesto usado |

### Invariantes que Bárbol debe garantizar al emitir

1. Solo posiciones con `marked_moves.length >= 1`.
2. Todos los `marked_moves` son legales en ese FEN.
3. Sin duplicados en `marked_moves`.
4. Si hay desacuerdo irreconciliable entre familias → **no emitir** la posición (no inventar consenso).
5. No incluir ChessDB / OTB / cloud como fuente de esta capa.

### `manifest.json` (recomendado)

```json
{
  "layer": "STRONG_CONSENSUS",
  "schema_version": 1,
  "content_version": "consensus-YYYY.MM.DD",
  "checksum": "sha256:…",
  "coverage": { "positions": 12345 },
  "sources": ["barbol-oracles-done"],
  "built_at": "2026-08-29T12:00:00Z"
}
```

---

## 5) Semántica en Certus (consumidor)

```text
PROVEN_TB > PROVEN_MATE > THEORETICAL > STRONG_CONSENSUS > … > INFERENCE
```

- **Hit:** FEN en catálogo y `confidence >= min_confidence` → `evidence=STRONG_CONSENSUS`; utility desde `wdl`.
- **Miss:** sin path, FEN ausente, o confidence baja → no se usa la capa.
- **Marked moves (FEAT-0010):** en la **raíz** UCI, si hay ≥1 marked **legal**, Certus **fuerza** `bestmove` = primer marked legal (orden del catálogo) y emite `info string marked=…`. Sin marked usable → search fallback. Marked ilegales se ignoran.
- Certus **no** reanaliza ni completa huecos.

---

## 6) Alcance sugerido de la FEAT en Bárbol

**Objetivo:** job/builder offline que, a partir de oráculos DONE (αβ + MCTS u otras familias acordadas), escribe `catalog.json` (+ `manifest.json`) conforme a §4.

**Incluir:**

- Definición operativa **1=A** (intersección bestmoves ab∩mcts; §2).
- Universo **2=C** (scope explícito; sin `--all` obligatorio en v1).
- Emisión solo si ≥1 marked tras intersección.
- Soporte explícito de **varios marked** igualmente válidos.
- Versionado `content_version` + checksum.
- Export a `{BARBOL_EXPORT_ROOT}/certus/consensus/v{content_version}/` (§3.1); **sin** acoplar el runtime UCI de Certus.

**Excluir:**

- Que Certus invoque Bárbol o engines.
- Capas ICCF / TB (otros artefactos).

**Criterios de aceptación (para la FEAT Bárbol):**

- CA-1: Fixture con 1 marked → entrada válida en catalog.
- CA-2: Fixture con 2+ marked legales → ambos en `marked_moves`; Certus puede cargar el fichero.
- CA-3: Posición sin marked → ausente del catalog.
- CA-4: Manifest con checksum y `content_version`.
- CA-5: Documentado el path de salida §3.1 y que el consumidor es `ConsensusPath` de Certus.

---

## 7) Ejemplo mínimo (2 marked)

Posición: `1.e4 e5`, blancas.

```json
{
  "fen": "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
  "wdl": "d",
  "confidence": 0.92,
  "marked_moves": ["g1f3", "b1c3"],
  "sources": ["ab:stockfish-family", "mcts:lc0-family"]
}
```

Interpretación: hay consenso de que la posición es tablas-ish (`d`) y que **Nf3 y Nc3** son jugadas de consenso igualmente admisibles.

---

## 8) Contacto de schema

Cambios breaking de `schema_version` o de la regla “≥1 marked” → acordar en ambos repos antes de bump.  
Extensiones aditivas (`marked_moves` ya previsto) preferibles a romper `wdl`/`fen`.
