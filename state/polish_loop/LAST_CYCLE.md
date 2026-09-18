# Cycle 10 — 2026-09-18

## Focus

`melee-live-hands` — knife / fist from hand velocity using the live hull.

## Did

- `MeleeSweepHit`: swing hull along reach; start-solid is not a hit.
- `HandVelOrDelta`: XR vel if present, else pose finite difference.
- Tick uses the sweep when `trace` is set (no more mock-only `world_melee_hit`).
- hook_gl / hook_vk keep a right-hand vel tracker.

## Did not

- Deal CSS knife damage via a guessed entity API.
- Claim live melee from offline green.

## Tests

`cssvrmod_tests` — 58 passed, 0 failed (388 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`standalone-settings` — desktop host for eyescale / H / V / scale (still Cube + 3D panel).
