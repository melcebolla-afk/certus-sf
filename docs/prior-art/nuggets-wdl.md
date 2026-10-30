# prior-art — nuggets / ΔWDL

## Fuentes

- Ambanelli et al., *Fewer Draws, More Fun: Searching for Unbalanced Positions in Chess*, ICEC / Springer 2025. DOI: [10.1007/978-3-032-02555-5_1](https://doi.org/10.1007/978-3-032-02555-5_1). Postprint IRIS Unibo.
- Ambanelli et al., *Mining and extracting valuable positions for decisive chess games*, Entertainment Computing 58 (2026) 101196. DOI: [10.1016/j.entcom.2026.101196](https://doi.org/10.1016/j.entcom.2026.101196). Extiende el anterior con el índice ΔWDL y el pipeline de minería.
- Software (MIT): [amchess/NuggetsManagement](https://github.com/amchess/NuggetsManagement) — extracción WDL vía UCI (`UCI_ShowWDL`) + qualimetría offline.
- Motor auxiliar del estudio: [ShashChess](https://github.com/amchess/ShashChess) (Stockfish + modelo Shashin). **No** es dependencia de certus-sf.

## Qué aportan

**Problema:** tasas de tablas muy altas en torneos de motores y en correspondencia de élite (citan WCCC ICCF ~97–100%). Causa atribuida: juego casi perfecto NNUE → continuaciones de tablas incluso desde asimetrías.

**Nugget:** posición con win-probability ~75% (o ~25% para negras) y pérdida ~0 en el modelo WDL del motor (p.ej. `(W,D,L)=(50,50,0)` → winProb = W + D/2 = 75%). Umbral entre “aún tabloso” (~60%) y “trivialmente ganado” (>90%).

**ΔWDL (Indecision Index):** mide singularidades estratégicas — candidatas que el motor neural **no discrimina** de forma estadísticamente clara. Exponen límites de la convergencia data-driven.

**Pipeline:** PGN maestros → eval Stockfish/ShashChess → filtros (material, tensión táctica, anti-tablas, sensibilidad a error, tipología Shashin del centro) → set de nuggets → torneos de validación (menos tablas, mejor separación de Elo).

## Mapeo a certus-sf

| Idea del paper | Capas / producto Certus | Uso esperado |
|----------------|-------------------------|--------------|
| Motivación ICCF / tablas | Oráculo ICCF; `CertusStyle=Strict` vs SF (`docs/analysis/2026-10-09-iccf-style-strict-vs-mixed.md`) | Misma raíz de problema; prior art de producto |
| Banda WDL 75%/25% | WDL en `STRONG_CONSENSUS` / `EMPIRICAL_ICCF` (catálogos) | Mismo idioma probabilístico; **no** usar nuggets como capa de evidencia |
| ΔWDL / singularidades | Nodos donde NNUE no separa jugadas → valor de `marked_moves` / `frequent_moves` y Strict | Banco de prueba ideal para **divergencia** Strict vs Off/Mixed (no Elo blitz) |
| Nuggets como opening book | Lab: UHO + PGN ICCF + fastchess | Suite **complementaria** de match; no catálogo runtime |
| Fortalezas / tablas teóricas | Capa `THEORETICAL` (fortresses, Steingrimsson) | **Polo opuesto:** fortresses ≈ tablas; nuggets ≈ decisividad |
| ShashChess / Shashin en search | Fork `sf_19` + overlay Certus; sin train NNUE | **No portar** al binario; drift vs upstream |

## Qué no hacer

- No crear capa `THEORETICAL` “nugget” ni mezclar pepitas en `catalogs/` (contaminaría evidencia con opening book).
- No integrar ShashChess ni heurísticas Shashin en el runtime.
- No tratar el KPI del paper (“menos tablas / entretenimiento”) como KPI de producto (KPI Certus: divergencias interpretables en análisis ICCF).
- Si se reevalúan nuggets: versionar motor/profundidad; el paper admite que al mejorar los motores las pepitas se “resuelven”.

## Seguimiento posible (fuera de esta nota)

Fixture EPD/PGN de nuggets públicos (~12–50) + match lab Strict vs Off — solo si se abre un FEAT de lab explícito. Software de minería: paralelo conceptual a `builders/` / Bárbol, no al motor.
