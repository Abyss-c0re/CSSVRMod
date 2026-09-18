# Cycle 28 — 2026-09-18

## Focus

`menu3d-reset-pose` — a grip-moved Vision panel had no way home.

## Did

- `Menu3d_ResetPose` snaps STAGE pos + yaw back to default.
- Trigger on the VISION title (not a row) resets. Stick-focus click still applies a row on a miss.

## Did not

- HMD walk (user-gated).
- A HOME glyph on the title when moved (next).
- Claim live reset UX from offline green.

## Tests

`cssvrmod_tests` — 82 passed, 0 failed (618 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`menu3d-home-hint` — show HOME on the title when the panel is off default so the reset target is readable.
