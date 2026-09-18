# Cycle 26 — 2026-09-18

## Focus

`menu3d-primary-hand` — Vision laser/grip/trigger were hardcoded to the right hand.

## Did

- Primary hand index + axis helpers. Laser, grip, and menu trigger follow `left_handed`.
- Persist via `CSSVR --set left_handed 1` / `CSSVR_LEFT_HANDED`. Tick input uses the same live flag.

## Did not

- HMD walk (user-gated).
- A 3D-panel HAND row (next).
- Claim live left-hand UX from offline green.

## Tests

`cssvrmod_tests` — 81 passed, 0 failed (600 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`menu3d-hand-row` — toggle left/right on the Vision panel so handedness is not CLI-only.
