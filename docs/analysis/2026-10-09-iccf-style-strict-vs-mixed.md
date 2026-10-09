# Informe — Estilo Certus para análisis ICCF: Strict vs Mixed (High/Max)

**Fecha:** 2026-10-08 → 2026-10-09  
**Decisión operativa:** usar **`CertusStyle=Strict`** de forma general como oráculo Certus en análisis ICCF (contrapeso a Stockfish).  
**Ámbito:** comparación empírica de fuerza y de divergencia de bestmove frente a SF puro (`CertusStyle=Off`), sobre partidas ICCF del operador y libros UHO.

---

## 1. Objetivo y contexto

### 1.1 Uso previsto

Certus-sf se usa junto a Stockfish (SF) en análisis de correspondencia ICCF:

1. Si Certus y SF **coinciden** → revalidación (caso frecuente).
2. Si Certus **diverge** → hipótesis de investigación (posible línea que SF ha infravalorado, a menudo anclada a evidencia de catálogo).

No se busca maximizar Elo blitz, sino un contrapeso **útil** (divergencias interpretables) sin motor “roto”.

### 1.2 Sabores evaluados

| Modo | Idea |
|------|------|
| **SF-Off** | `CertusStyle=Off` — search/eval SF sin sesgo Certus (control). |
| **Mixed-High** | Sin filtro/force; preferred = marked else frequent; LMR−2 + ext en preferred; orden preferred en ID. |
| **Mixed-Max** | Como High + LMR≈0 en preferred + bonus MovePicker (+8000). |
| **Strict** | Filtro duro MarkedOnly/FreqOnly en nodos con hit; atajos raíz; LMR↓ + ext en preferred; en interiores solo se exploran preferred si hay catálogo. |

Referencias de producto: FEAT-0004, FEAT-0006, FEAT-0007; `docs/bootstrap/UCI-OPTIONS.md`.

### 1.3 Hipótesis de trabajo

- **H1 (fuerza):** Mixed High/Max no deben quedar debilitados respecto a Strict a nodos iguales.
- **H2 (contrapeso):** a más nodos, Mixed tenderá a coincidir con SF; Strict mantendrá más divergencias ancladas a preferred.
- **H3 (Max vs High):** Max, al concentrar más esfuerzo en preferred, podría divergir más de SF que High — **no confirmada** en estos tests.

---

## 2. Herramientas y entorno

### 2.1 Motor y evidencia

| Pieza | Ruta / valor |
|-------|----------------|
| Binario | `/home/mcebolla/certus-sf/stockfish/src/certus-sf` |
| EvidenceRoot | `/home/mcebolla/certus-sf/catalogs` |
| Consensus (marked) | `catalogs/consensus/v2026.10.05` (8594 entradas con `marked_moves`) |
| ICCF (frequent) | `catalogs/iccf/v2026.09.10` (schema v2, `frequent_moves`) |
| Preferred (definición) | marked legal; si vacío, frequent legal (igual que el motor Mixed/Strict) |
| Hardware de lab | máquina con ~20 CPUs lógicos; matches a `concurrency` 1–20; divergencia a 12 workers |

### 2.2 Orquestación de matches

| Herramienta | Uso |
|-------------|-----|
| **fastchess** (`/home/mcebolla/Engines/tools/fastchess`, alpha 1.8.2) | Torneos UCI: gauntlet / round-robin, openings EPD/PGN, `-repeat`, adjudicación, `nodes=` + TC de seguridad |
| Scripts bash | Ver §2.4 |

Opciones habituales de match:

- `Threads=1`, `Hash=64`
- Esfuerzo: `nodes=N` con `tc=300+5` (seguridad; el límite efectivo es nodos)
- Adjudicación: draw `movenumber=40 movecount=8 score=20`; resign `movecount=3 score=600 twosided=true`; `maxmoves=200`
- Book UHO: `order=random -srand 42`
- Book ICCF propio: `format=pgn plies=24 order=sequential -srand 42`

### 2.3 Orquestación de divergencia (bestmove)

| Herramienta | Uso |
|-------------|-----|
| **Python 3** + **python-chess** | Extracción de FENs desde PGN; lookup preferred en `catalog.json` |
| `certus_sf_disagreement_iccf.py` | Lanza SF-Off / High / Max (y variantes) con `go nodes N` en paralelo (`ProcessPoolExecutor`) |
| `certus_strict_diverge_merge.py` | Solo Strict sobre las mismas FENs; merge con resultados previos |
| Pipelines bash | Encadenan 4M → 8M → 16M y escriben informes |

Métrica principal: acuerdo/desacuerdo de **bestmove UCI** a nodos fijos (no MultiPV completo).

### 2.4 Scripts y artefactos de lab

| Script | Rol |
|--------|-----|
| `Engines/tools/certus_vs_berserk_gauntlet.bash` | Gauntlet vs Berserk (TC 30+5) — contexto inicial |
| `Engines/tools/certus_vs_berserk_gauntlet_nodes.bash` | Gauntlet vs Berserk a `nodes=4e6` |
| `Engines/tools/certus_flavors_rr_nodes.bash` | RR Strict/High/Max, UHO, `nodes=4e6` |
| `Engines/tools/certus_high_vs_max_iccf.bash` | High vs Max sobre PGN ICCF del operador |
| `Engines/tools/certus_sf_disagreement_iccf.py` | Divergencia SF/High/Max (extensible) |
| `Engines/tools/certus_sf_disagree_pipeline.bash` | Pipeline 4M/8M/16M preferred FENs |
| `Engines/tools/certus_strict_diverge_merge.py` | Añade Strict a resultados de divergencia |
| `Engines/tools/certus_strict_full_pipeline.bash` | Divergencia+Strict + RR ICCF fuerza |

**Salidas típicas (lab, no versionadas):** `/tmp/certus_*`  
Ejemplos:  
`/tmp/certus_flavors_rr_nodes_20261008_215642`,  
`/tmp/certus_high_vs_max_iccf_20261009_080315`,  
`/tmp/certus_sf_disagree_pipe_20261009_124055`,  
`/tmp/certus_strict_full_20261009_153455`.

### 2.5 Datos de posiciones ICCF del operador

| Recurso | Detalle |
|---------|---------|
| PGN | `player161891.pgn` (repo; ~268 partidas ICCF, jugador Cebolla Benedito) |
| Extracción | FENs tras N plies de la línea principal |
| Filtro preferred | Intersección FEN (placement+STM+castling+ep) con consensus/iccf catalogs |

---

## 3. Experimentos realizados

### 3.1 Matches de fuerza (resumen)

| ID | Diseño | Controles | Volumen | Resultado clave |
|----|--------|-----------|---------|-----------------|
| **E1** | Gauntlet Berserk vs Strict / High / Max | TC 30+5, UHO, 5×repeat | 10 juegos/sabor | Certus ~65–70% vs Berserk; poco discriminante entre sabores |
| **E2** | Igual E1 a nodos | `nodes=4e6`, mismo book | ~parcial + gate | Confirma que TC sesga Strict (ahorro en forzadas); nodos más justos |
| **E3** | RR Strict ↔ High ↔ Max | UHO, `nodes=4e6`, concurrency 10, 50 rounds×repeat | **300** juegos | Empate estadístico; Strict +7±15 Elo |
| **E4** | High vs Max | PGN ICCF `plies=24`, `nodes=4e6`, concurrency 20 | **472** juegos | High 50.32% (20–435–17); ~92% tablas |
| **E5** | RR Strict/High/Max | PGN ICCF `plies=24`, `nodes=4e6`, concurrency 20, 40 rounds×repeat | **240** juegos | High +6.5±11; Strict −2; Max −4; ~91–94% tablas |

**Conclusión fuerza:** a igualdad de nodos, los tres sabores son **equiparables**. Ninguno está descompensado. Las diferencias Elo están dentro del intervalo de confianza habitual.

### 3.2 Divergencia vs SF (bestmove)

**Universo de FENs (tras filtros):**

1. Partida ICCF → FENs a plies=24 con preferred → **166**.
2. Añadidas FENs tempranas plies ∈ {8,10,12} con preferred → **+343**.
3. Total **509** FENs (se descartaron FENs sin marked/frequent: ahí High/Max≈SF y el ruido domina).

**Protocolo por FEN y presupuesto de nodos:**

```text
go nodes N   para   SF-Off, Mixed-High, Mixed-Max [, Strict]
N ∈ {4_000_000, 8_000_000, 16_000_000}
Threads=1 Hash=64 EvidencePath=catalogs
```

**Etapas:**

| Etapa | Qué se midió |
|-------|----------------|
| D1 | SF/High/Max @ 4M sobre 236 FENs plies=24 (sin filtrar preferred) — baseline temprano |
| D2 | Pipeline preferred: High/Max/SF @ 4M (166 reutilizadas + 343 nuevas), luego 8M y 16M sobre 509 |
| D3 | Strict @ 4M/8M/16M sobre las mismas 509 (merge con D2) |

**Análisis estadístico:** tasas de acuerdo, leads únicos, McNemar (pares discordantes High vs Max / Strict vs High), fracción de bestmoves Certus ∈ preferred cuando divergen.

---

## 4. Resultados detallados

### 4.1 Fortaleza

#### E3 — UHO RR (nodes=4M, 300 juegos)

| Rank | Motor | Elo | Score | Draw% |
|------|-------|-----|-------|-------|
| 1 | Strict | +6.95 ± 15.18 | 51.0% | 80.0% |
| 2 | Mixed-High | −1.74 ± 17.70 | 49.8% | 73.0% |
| 3 | Mixed-Max | −5.21 ± 15.58 | 49.2% | 79.0% |

#### E4 — ICCF High vs Max (nodes=4M, 472 juegos)

- High **50.32%** (20–435–17), Elo +2.2 ± 3.2 (IC incluye 0).
- Draw ratio ~98% (pares).

#### E5 — ICCF RR Strict/High/Max (nodes=4M, 240 juegos)

| Rank | Motor | Elo | Score | Draw% |
|------|-------|-----|-------|-------|
| 1 | Mixed-High | +6.52 ± 11.18 | 50.9% | 91.2% |
| 2 | Strict | −2.17 ± 9.51 | 49.7% | 93.8% |
| 3 | Mixed-Max | −4.34 ± 10.39 | 49.4% | 92.5% |

### 4.2 Divergencia vs SF — 509 FENs preferred

| Nodos | High=SF | Max=SF | Strict=SF | High≠SF | Max≠SF | Strict≠SF |
|------:|--------:|-------:|----------:|--------:|-------:|----------:|
| 4M | 79.8% | 79.4% | 78.2% | 20.2% | 20.6% | 21.8% |
| 8M | 83.7% | 82.7% | 78.2% | 16.3% | 17.3% | **21.8%** |
| 16M | 85.1% | 84.3% | 79.6% | 14.9% | 15.7% | **20.4%** |

**Leads únicos (≠SF y los otros dos =SF):**

| Nodos | Strict-only | High-only | Max-only | Todos Certus iguales ≠SF |
|------:|------------:|----------:|---------:|-------------------------:|
| 4M | 5.1% | 2.2% | 3.7% | 9.0% |
| 8M | 5.9% | 2.8% | 2.9% | 7.7% |
| 16M | 5.9% | 2.4% | 2.6% | 7.3% |

**Anclaje a catálogo:** cuando Strict≠SF, Strict∈preferred en **~99%** de los casos (4M/8M/16M).

**McNemar Strict vs High (desacuerdo con SF):**

| Nodos | Strict-only≠SF | High-only≠SF | p (aprox.) |
|------:|---------------:|-------------:|------------|
| 4M | 37 | 29 | ~0.39 (n.s.) |
| 8M | 48 | 20 | **~0.001** |
| 16M | 47 | 19 | **~0.001** |

**High vs Max:** sin diferencia significativa en ningún presupuesto (tasas casi idénticas; McNemar n.s.).

### 4.3 Hallazgo estructural (por qué Strict diverge más a más nodos)

- **Mixed:** prior suave. Al aumentar nodos, el search “puro” (como SF) recupera peso → más acuerdo con SF-Off.
- **Strict:** en nodos con hit de catálogo, **solo busca preferred**. El bestmove queda atado a la evidencia aunque SF, a más nodos, prefiera otra jugada. Por eso la tasa Strict≠SF **no se erosiona** como la de Mixed.

Esto encaja con el objetivo de contrapeso: divergencia = “evidencia vs search NNUE”, no ruido High vs Max.

---

## 5. Conclusiones

1. **Fuerza:** Strict, Mixed-High y Mixed-Max son **comparables** a `nodes=4e6` (UHO e ICCF). No hay motor débil entre ellos.
2. **Mixed-High ≈ Mixed-Max** como contrapeso a SF: mismas tasas de divergencia; Max no aporta leads sistemáticos adicionales en este dominio/presupuesto.
3. **Strict es el mejor contrapeso a SF** entre los sabores probados:
   - mantiene ~20–22% de desacuerdo a 4M–16M;
   - Mixed cae a ~15% a 16M;
   - divergencias Strict casi siempre ∈ preferred.
4. **Uso recomendado ICCF (operativo):**
   - Oráculo Certus principal: **`CertusStyle=Strict`** (+ `EvidencePath` actualizado).
   - Contraste: Stockfish / `CertusStyle=Off`.
   - Opcional: Mixed-High como segundo motor “casi-SF con sesgo suave” si se quiere una tercera opinión menos restrictiva.
   - Mixed-Max: no priorizar en el flujo habitual.
5. **Default UCI:** adoptado **`CertusStyle=Strict`** como default del binario (2026-10-09), alineado con el uso ICCF. `MixedEffort` sigue default High e ignorado bajo Strict.

---

## 6. Limitaciones

- Bestmove a MultiPV=1; no se midió cobertura MultiPV ni calidad humana de cada lead.
- Preferred lookup por FEN de catálogo (4 campos); coherente con stores, pero posiciones raras fuera de catálogo no entran en D2/D3.
- Matches con adjudicación y books fijos; Elo con IC amplios en muestras de cientos de partidas.
- Berserk/TC sirvió de contexto; el criterio de decisión ICCF se basa en nodos + divergencia en FENs del operador.
- No se etiquetó cada lead Strict≠SF como “mejoría real” vs SF (eso sigue siendo juicio de analista).

---

## 7. Reproducción rápida

```bash
# Divergencia (ejemplo 16M, FENs preferred ya generadas en lab)
python3 /home/mcebolla/Engines/tools/certus_sf_disagreement_iccf.py \
  --positions-json /tmp/certus_sf_disagree_expanded/fens_all_preferred.json \
  --bin /home/mcebolla/certus-sf/stockfish/src/certus-sf \
  --evidence /home/mcebolla/certus-sf/catalogs \
  --nodes 16000000 --workers 12 --outdir /tmp/diverge_16M

# Match RR ICCF
/home/mcebolla/Engines/tools/fastchess \
  -engine cmd=.../certus-sf name=certus-Strict option.CertusStyle=Strict ... \
  -engine cmd=... name=certus-Mixed-High option.CertusStyle=Mixed option.MixedEffort=High ... \
  -engine cmd=... name=certus-Mixed-Max option.CertusStyle=Mixed option.MixedEffort=Max ... \
  -each proto=uci tc=300+5 nodes=4000000 \
  -tournament roundrobin -rounds 40 -repeat -concurrency 20 \
  -openings file=player161891.pgn format=pgn plies=24 order=sequential -srand 42
```

Config UCI recomendada para análisis ICCF (contrapeso):

```text
setoption name EvidencePath value /home/mcebolla/certus-sf/catalogs
setoption name CertusStyle value Strict
setoption name ConsensusSearch value MarkedOnly
setoption name IccfSearch value FreqOnly
```

---

## 8. Referencias internas

- Specs: `docs/specs/FEAT-0004-certus-style.md`, `FEAT-0006-mixed-effort-boost.md`, `FEAT-0007-mixed-effort-max.md`
- UCI: `docs/bootstrap/UCI-OPTIONS.md`
- Catálogos: `catalogs/README.md`, `docs/runbooks/catalogs-layers.md`
- Decisión: `docs/decisions/current-known-decisions.md` (entrada 2026-10-09)
