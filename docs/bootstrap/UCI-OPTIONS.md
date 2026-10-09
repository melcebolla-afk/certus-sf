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
| `EvidencePath` | empty | sí | raíz; auto-pick versión más reciente por fecha en nombre de carpeta / `content_version` (no mtime) |

Vacío → clear capa; inválido → `info string warning …` sin crash.

## Producto / protocolo

| Option | Type | Default | Values |
|--------|------|---------|--------|
| `CertusStyle` | combo | **Strict** | Off, Mixed, Strict |
| `MixedEffort` | combo | **High** | Low, High, Max — solo si `CertusStyle=Mixed` |
| `EvidenceInfo` | combo | Root | Off, Root, All |
| `ConsensusSearch` | combo | MarkedOnly | Off, MarkedOnly — filtro solo si `CertusStyle=Strict` |
| `IccfSearch` | combo | FreqOnly | Off, FreqOnly — filtro solo si `CertusStyle=Strict` |
| `UCI_ShowWDL` | check | true | (SF puede tener ya) |

`CertusStyle` (FEAT-0004):

| Valor | Comportamiento |
|-------|----------------|
| `Off` | SF puro: sin filtro, sin bias, sin atajos, eval sin capas evidencia |
| `Mixed` | Sin filtro ni force. Intensidad: `MixedEffort` (**High** default). |
| `Strict` (**default**) | Filtro MarkedOnly/FreqOnly + atajos raíz + LMR↓ + extensión en preferred; oráculo ICCF recomendado |

`MixedEffort` (FEAT-0006/0007) — ignorado si `CertusStyle` no es Mixed:

| Valor | Comportamiento |
|-------|----------------|
| `Low` | Mixed FEAT-0004: orden raíz una vez + LMR−1 preferred; sin ext; sin boost `pvIdx>0` |
| `High` (**default**) | Orden preferred cada iteración ID + LMR−2 + ext+1 interiores + boost MultiPV; **sin** filtro ni force |
| `Max` (FEAT-0007) | Como High (mismo preferred: marked else frequent) + LMR≈0 + bonus orden MovePicker (+8000); **sin** filtro ni force |

`ConsensusSearch=MarkedOnly`: en **Strict**, nodos con consenso + `marked_moves` → solo marked ∩ legal. Raíz Strict: atajo FEAT-0010 + score NNUE.

`IccfSearch=FreqOnly`: en **Strict**, filtra frequent ∩ legal; raíz Strict + 1 frequent → atajo.

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
