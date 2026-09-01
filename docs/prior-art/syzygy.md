# prior-art — syzygy

## Qué hay

Crate `syzygy`: `TbStore` sobre `shakmaty_syzygy::Tablebase<Chess>` con feature `mmap`, probe `probe_wdl_after_zeroing`, conversión `board::Position` → shakmaty vía FEN.

## Síntesis (FEAT-0002)

- Adoptado **shakmaty / shakmaty-syzygy** (Niklas Fiekas; GPL) en lugar de Pyrrhic FFI, tras OQ-2 → GPL-3.0.
- No se usa DTZ como evidencia (OQ-9 WDL only).
- `board` propio se mantiene; shakmaty solo para probe (YAGNI: no migrar movegen aún).

## Referencias

- https://docs.rs/shakmaty-syzygy
- Ronald de Man — Syzygy tablebases
- Producto: PRODUCT-SPEC RF-3.1
