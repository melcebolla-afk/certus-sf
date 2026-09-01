# Spec — FEAT-0001 evidence layers SF

## Contexto (hechos)

- **Fase 0 hecha:** Stockfish 18 (`sf_18`, commit `cb3d4ee9`) en submodule `stockfish/`; build release verde.
- Bootstrap Certus en `docs/bootstrap/` (RESOLVER-SPEC, EVIDENCE-PORT, GOLDEN-FIXTURES, UCI-OPTIONS).
- Oráculo Rust: `/home/mcebolla/evidence` — paridad de clase evidencia, no de search.

## Objetivo

Integrar en el fork SF la **cadena única de evidencia Certus** antes del NNUE, con UCI y tests de paridad, sin reentrenar NNUE ni duplicar Syzygy.

## No-objetivos

- Train NNUE, self-play, orchestrator c1, port completo del motor Rust.
- ChessDB / Lichess cloud / OTB como capa.
- Cambiar heurísticas de search SF salvo hooks documentados (root consensus, thinning).

## Requisitos funcionales (RF)

- **RF-1:** Stores C++ en `stockfish/src/evidence/` — load `catalog.json`, MatePath `catalog.idx` (mmap); key = placement + STM.
- **RF-2:** Resolver con precedencia `TB > MATE > THEORY > CONSENSUS > ICCF > NNUE` (RESOLVER-SPEC).
- **RF-3:** Opciones UCI según `docs/bootstrap/UCI-OPTIONS.md`; reload invalida TT/generation.
- **RF-4:** Root STRONG_CONSENSUS + marked moves legales → `bestmove` sin search (FEAT-0010).
- **RF-5:** `info string evidence=…` / `marked=…` según `EvidenceInfo`.
- **RF-6:** Thinning HardOnly en nodos quiet midgame (>6 piezas, !in_check).

## Requisitos no funcionales (RNF)

- **RNF-1:** GPL-3.0; NOTICE Stockfish + Certus.
- **RNF-2:** Runtime sin MariaDB/API Bárbol.
- **RNF-3:** Golden `GOLDEN-FIXTURES.jsonl` + `testdata/` — misma clase evidencia que Certus (± tolerancia utility).

## Criterios de aceptación

- **CA-1:** Probe hit/miss unitario por capa con fixtures `testdata/*`.
- **CA-2:** Golden fixtures pasan (clase evidencia).
- **CA-3:** UCI smoke: load paths + `go depth 1` + parse `info string evidence` en posiciones conocidas.
- **CA-4:** `id name certus-sf dev` (hasta decisión producto).

## Fases (plan)

| Fase | Alcance |
|------|---------|
| 0 | Baseline SF — **hecho** |
| 1 | UCI options + stores (sin eval en search) | **hecho** 2026-09-01 |
| 2 | Primera capa en eval (ICCF o Consensus) + golden |
| 3 | Resolver completo + search hooks + ResultBias |
| 4 | CI + runbooks builders |

## Dudas abiertas

- Nombre producto final UCI (`certus-sf dev` vs otro).
- Delta documentado si thinning SF difiere de Certus en nodos interiores (preferir paridad Certus).

## Aterrizaje en el repo

- Código: `stockfish/src/evidence/*`, hooks en `evaluate.cpp`, `uci.cpp`, `search.cpp`.
- Docs: `docs/prior-art/evidence-port.md`, `docs/plans/FEAT-0001-plan.md` (al abrir plan formal).
