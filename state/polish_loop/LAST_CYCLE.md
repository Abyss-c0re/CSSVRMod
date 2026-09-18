# Cycle 29 — 2026-09-18

## Focus

`menu3d-home-hint` — title-click reset was invisible when the panel was moved.

## Did

- `Menu3d_OffHome` (1 cm from default STAGE pos).
- Raster HOME on the title (right side) only when off-home so the reset target is readable.

## Did not

- HMD walk (user-gated).
- Highlight the title while the laser is on it (next).
- Claim live reset UX from offline green.

## Tests

`cssvrmod_tests` — 83 passed, 0 failed (622 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`menu3d-title-hot` — light the VISION header when the laser is on the title so reset/home is an obvious hit.
