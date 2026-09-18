# Cycle 36 — 2026-09-18

## Focus

`print-launch-path` — `--print` hid which launch.cfg width/height came from.

## Did

- `--print` and `--settings` write `cssvr: launch.cfg PATH ok|missing`.
- CSSVR_LAUNCH override is the path shown (same file persist-resize writes).

## Did not

- HMD walk (user-gated). Dual-origin remains P0 but in-game.
- Menu chrome / toasts. No force `-noborder`.

## Tests

`cssvrmod_tests` — 90 passed, 0 failed (706 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`help-launch-env` — `--help` should mention CSSVR_LAUNCH / resize persist. Never queue `dual-renderview-ipd-origin`.
