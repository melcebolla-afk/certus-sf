# Spec — FEAT-0005-stockfish-19-merge

## Nombre

`FEAT-0005-stockfish-19-merge`

## Contexto (hechos)

- Baseline actual: Stockfish 18 (`sf_18` @ `cb3d4ee9`) vendored en `stockfish/`.
- Upstream publicó **Stockfish 19** (`sf_19` @ `edb0d9db`); fuentes en `official-stockfish/Stockfish`.
- Capas Certus viven en overlay `src/evidence/` + `src/certus/`; upstream tocado según `TOUCHED_UPSTREAM.md`.
- Runbook: `docs/runbooks/stockfish-merge.md` + `scripts/merge-stockfish.sh`.

## Objetivo

- Actualizar el árbol vendored a `sf_19`.
- Conservar comportamiento Certus (resolver, search hooks, UCI, probes).
- Build + tests evidencia verdes.

## No-objetivos

- Reentrenar / cambiar red NNUE Certus.
- Cambiar schema de catálogos o builders.
- Push a `official-stockfish/Stockfish`.
- Mezclar cambios locales no relacionados (mate builders / catalogs dirty).

## Usuario / escenario

- Quién: operador / GUI ICCF con `certus-sf`.
- Flujo esperado: mismo UCI y capas; motor base SF19 (NNUE/search upstream).

## Requisitos funcionales (RF)

- RF-1: `stockfish/UPSTREAM` pin `tag=sf_19` + commit real.
- RF-2: Parches de `TOUCHED_UPSTREAM.md` re-aplicados sobre SF19.
- RF-3: `id name certus-sf dev` y opciones evidencia intactas.

## Requisitos no funcionales (RNF)

- RNF-1: Compila con `ARCH=x86-64-sse41-popcnt`.
- RNF-2: `./tests/evidence_probe.sh` pasa.

## Criterios de aceptación

- CA-1: Código fuente `sf_19` disponible y mergeado en `stockfish/`.
- CA-2: Overlay `evidence/` + `certus/` preservado.
- CA-3: Build + evidence/golden/consensus probes OK.
- CA-4: Inventario + changelog actualizados.

## Compatibilidad / migración

- Drop-in UCI respecto a certus-sf actual; eval NNUE base cambia con SF19 (esperado).

## Dudas abiertas

- Ninguna bloqueante (procedimiento runbook existente).

## Aterrizaje en el repo

- `stockfish/` (upstream), `docs/project/inventory.md`, `docs/CHANGELOG.md`.

## Entregables

- Spec + plan + QA; design/OQ no aplican.
