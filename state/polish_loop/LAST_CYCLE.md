# Cycle 23 — 2026-09-18

## Focus

`menu3d-laser-dot` — no cursor on the Vision panel at the laser UV.

## Did

- UV→pixel + cyan crosshair rastered on the world-locked quad when the STAGE ray hits it.
- Submit re-locates the right-hand aim so the VK (submit-then-poll) path is not a frame late.

## Did not

- HMD walk (user-gated).
- Claim live laser UX from offline green.
- Grip-reposition the panel (next).

## Tests

`cssvrmod_tests` — 78 passed, 0 failed (556 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`menu3d-grip-move` — hold grip to drag the world-locked Vision panel in STAGE.
