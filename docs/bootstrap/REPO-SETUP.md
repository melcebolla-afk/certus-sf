# Repo setup — certus-sf

## Layout recomendado

```text
certus-sf/
├── stockfish/              # upstream (submodule o fork root)
│   ├── src/
│   │   ├── evaluate.cpp    # hook principal evidencia
│   │   ├── uci.cpp
│   │   ├── search.cpp      # root consensus marked
│   │   └── evidence/       # NUEVO: stores + resolver C++
│   └── ...
├── docs/
│   ├── bootstrap/          # export Certus (este paquete)
│   ├── project/
│   │   ├── inventory.md
│   │   ├── guardrails.md
│   │   └── conventions.md  # adaptar desde evidence
│   ├── specs/
│   ├── plans/
│   ├── interop/
│   └── prior-art/
├── builders/               # Python ingest/promote (desde evidence)
├── testdata/               # fixtures CI (desde evidence)
├── evidence/               # opcional: datos lab versionados o .gitignore + sync
├── tests/
│   └── evidence/           # golden + smoke UCI
├── methodology/            # submodule git → evidence methodology tag
├── LICENSE                 # GPL-3.0
├── NOTICE                  # Stockfish + Certus evidence
└── README.md
```

## Stockfish como base

**Opción A (recomendada):** repo = fork GitHub de `official-stockfish/Stockfish`; código en raíz o subdir `stockfish/`.

**Opción B:** submodule:

```bash
git submodule add https://github.com/official-stockfish/Stockfish.git stockfish
```

Mantener remote `upstream` para merges selectivos (no obligatorio seguir cada commit).

Tag inicial sugerido: último **release estable** documentado en README del fork.

## Methodology (igual que Certus)

Desde repo `evidence`:

```bash
# Submodule al paquete metodología (mismo remoto que evidence usa)
git submodule add -b main https://github.com/melcebolla-afk/evidence.git methodology-src
# O mejor: si methodology vive como subtree en evidence, copiar tag:
# git clone --depth 1 --filter=blob:none --sparse evidence && ...
```

**Práctica simple:** copiar directorio `methodology/` del tag `1.4.1` (VERSION en evidence) como submodule apuntando al repo del paquete metodología si existe independiente; si no, **copiar** `methodology/` desde evidence y documentar versión en `docs/project/inventory.md`.

Cursor rules: copiar/adaptar `.cursor/rules/` desde evidence (`00-methodology`, `50-project-guardrails`, `55-project-inventory`, `60-project-conventions`) — **sin** reglas de train NNUE ni engine-prior-art Rust.

## Licencia

- Proyecto **GPL-3.0-or-later** (compatible Stockfish).
- `NOTICE`: "This program includes code from Stockfish (GPL-3) and evidence layer design from Certus/evidence (GPL-3)."
- No relicenciar a permisiva.

## CI mínimo (objetivo)

1. Build release SF.
2. Golden evidence tests (`GOLDEN-FIXTURES.jsonl`).
3. UCI smoke: `uci` + load fixture paths + `go depth 1` + parse `info string evidence`.

## Sync datos con repo Certus

Durante transición:

```bash
# rsync capas lab desde evidence (c5)
rsync -av evidence/consensus/ certus-sf/evidence/consensus/
rsync -av evidence/iccf/       certus-sf/evidence/iccf/
```

O apuntar UCI paths al mismo `EvidenceRoot` en disco compartido.

## GitHub

- Repo nuevo separado de `evidence`.
- README: propósito ICCF, link a Certus como referencia, build instructions.
- No submodule de todo `evidence` (solo methodology o docs puntuales).
