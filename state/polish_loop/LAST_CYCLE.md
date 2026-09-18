# Cycle 35 — 2026-09-18

## Focus

`persist-resize` — a user-resized framed window was forgotten next launch.

## Did

- SDL resize / SetWindowSize / destroy writes clamped `width`/`height` to launch.cfg.
- Minimize/garbage (below 640×480) is ignored. Debounce 400 ms; destroy forces a last write.
- Launch-only save — resize must not rewrite Vision calib.

## Did not

- HMD walk (user-gated). Dual-origin remains P0 but in-game.
- Menu chrome / toasts. No force `-noborder`.

## Tests

`cssvrmod_tests` — 89 passed, 0 failed (700 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`print-launch-path` — `--print` should show which launch.cfg size came from. Never queue `dual-renderview-ipd-origin`.
