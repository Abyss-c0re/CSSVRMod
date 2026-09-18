# Cycle 22 — 2026-09-18

## Focus

`menu3d-laser-hit` — trigger clicked stick-focus, not the row the laser pointed at.

## Did

- Pure `Menu3d_RayHit` against the STAGE-locked Vision quad (faces +Z).
- Right-hand aim pose hovers focus; trigger applies that row. Stick still walks focus on a miss.

## Did not

- HMD walk (user-gated).
- Claim live laser UX from offline green.
- Raster a laser dot (next).

## Tests

`cssvrmod_tests` — 77 passed, 0 failed (540 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`menu3d-laser-dot` — draw a cursor on the panel at the hit UV.
