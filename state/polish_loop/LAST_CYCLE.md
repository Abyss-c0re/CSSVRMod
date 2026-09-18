# Cycle 41 — 2026-09-18

## Focus

`submit-uv-drop-dual` — dual-eye blit still called `CalibEye` with the mono default.

## Did

- `BlitToSwapchain` takes `painted_dual` and uses `CalibSubmitCrop`.
- Dual submit no longer adds a second UV IPD plane on top of origin IPD.
- Vision H/V/scale pan still applies equally to both eyes.

## Did not

- HMD walk (user-gated). Dual paint still unproven in-game.
- Menu / toast / print / help / cfg chrome. No force `-noborder`.

## Tests

`cssvrmod_tests` — 93 passed, 0 failed (735 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — close with no commit unless a real offline bug appears. Never queue `dual-renderview-ipd-origin`.
