# Cycle 24 — 2026-09-18

## Focus

`menu3d-grip-move` — world-locked Vision panel could not be repositioned.

## Did

- Panel origin is STAGE-movable. Right-hand grab while the laser is on the quad offset-locks the panel to the hand.
- Laser / trigger / stick ignore the panel while gripping. Quad submit uses `Menu3d.pos`.

## Did not

- HMD walk (user-gated).
- Yaw the quad toward the HMD (next).
- Claim live grip UX from offline green.

## Tests

`cssvrmod_tests` — 79 passed, 0 failed (576 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`menu3d-face-hmd` — yaw the world-locked panel toward the headset so a dragged panel stays readable.
