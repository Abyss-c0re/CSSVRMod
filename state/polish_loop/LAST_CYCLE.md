# Cycle 12 — 2026-09-18

## Focus

`menu3d-readable-labels` — 3D Vision panel was color bars.

## Did

- 5×7 glyphs + `Menu3d_RowLabel` / `RowValue` (EYE / SCALE / H / V / DONE).
- Raster draws VISION title and values on panels tall enough for text (live 1024×576).
- Tiny 64×48 test still only checks layout blocks.

## Did not

- HMD walk (user-gated).
- Claim stereo from offline green.

## Tests

`cssvrmod_tests` — 61 passed, 0 failed (413 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`honest-no-hmd-toast` — Cube toasts no-HMD; CSSVRMod only logs.
