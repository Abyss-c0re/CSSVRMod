# Cycle 27 — 2026-09-18

## Focus

`menu3d-hand-row` — handedness was CLI-only (`--set` / env).

## Did

- Vision panel HAND row toggles L/R. Click writes `CSSVR_LEFT_HANDED` + launch.cfg so the next poll does not revert it.

## Did not

- HMD walk (user-gated).
- A way to snap a dragged panel back (next).
- Claim live hand-row UX from offline green.

## Tests

`cssvrmod_tests` — 81 passed, 0 failed (608 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`menu3d-reset-pose` — snap the world-locked panel back to default STAGE pose after a grip-drag.
