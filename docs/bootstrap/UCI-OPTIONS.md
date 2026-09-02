# UCI options — certus-sf (desde Certus)

Implementar además de opciones SF estándar. Referencia: `crates/evidence-engine/src/uci.rs`.

## Capas evidencia (string path)

| Option | Default | Reload | Notas |
|--------|---------|--------|-------|
| `SyzygyPath` | (SF) | sí | PROVEN_TB — ya en SF |
| `TheoreticalPath` | empty | sí + TT age | dir o catalog.json |
| `ConsensusPath` | empty | sí + TT age | dir o catalog.json |
| `IccfPath` | empty | sí + TT age | dir o catalog.json |
| `MatePath` | empty | sí + TT age | dir; prefer `catalog.idx` |
| `EvidencePath` | empty | sí | raíz auto-version (mtime max) |

Vacío → clear capa; inválido → `info string warning …` sin crash.

## Producto / protocolo

| Option | Type | Default | Values |
|--------|------|---------|--------|
| `EvidenceInfo` | combo | Root | Off, Root, All |
| `ConsensusSearch` | combo | MarkedOnly | Off, MarkedOnly |
| `IccfSearch` | combo | FreqOnly | Off, FreqOnly |
| `UCI_ShowWDL` | check | true | (SF puede tener ya) |

`ConsensusSearch=MarkedOnly` (default): en nodos con consenso + `marked_moves`, el search principal solo expande marked ∩ legal (eval NNUE). Raíz: atajo FEAT-0010 independiente; emite `info depth N score cp <NNUE>` antes del `bestmove`.

`IccfSearch=FreqOnly` (default): en nodos con hit ICCF + `frequent_moves` (catalog schema v2), filtra a frequent ∩ legal. **Nunca** fuerza `bestmove`. Si consenso MarkedOnly aplica en el mismo nodo, el consenso tiene prioridad.

## No implementar en fork (v1)

| Option Certus | Motivo |
|---------------|--------|
| `EvalFile` | NNUE embebido SF / EvalFile SF |
| `EvidencePath` solo | OK si se implementa |
| Train-specific | N/A |

## Info strings emitidos

### Ready / setoption

```text
info string TheoreticalPath ready version=… entries=…
info string ConsensusPath ready version=… entries=…
info string IccfPath ready version=… entries=…
info string MatePath ready version=… entries=…
info string SyzygyPath ready files=… max_pieces=…
```

### Durante search (EvidenceInfo)

```text
info string evidence=EMPIRICAL_ICCF confidence=0.74 version=iccf-2026.08.29
info string evidence_hits PROVEN_TB=3 STRONG_CONSENSUS=1 …   # All mode
info string marked=g1f3,b1c3
```

### Score

Usar `format_score` SF; mate `score mate N`; WDL permille si `UCI_ShowWDL`.

## id name

Decidir nombre producto (distinto de `Certus` Rust si conviene). Ejemplo:

```text
id name CertusSF dev
id author …
```

## Comandos

Mismos que SF: `uci`, `isready`, `position`, `go`, `stop`, `setoption`, `ucinewgame`.

`ucinewgame`: clear histories SF + opcional reset evidence stats.

## Tests UCI mínimos

Copiar escenarios de tests Rust `uci.rs` (consensus root marked, iccf hit, theory draw, tb hit) adaptados a binario SF.
