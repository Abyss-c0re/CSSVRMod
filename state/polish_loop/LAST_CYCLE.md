# Cycle 34 — 2026-09-18

## Focus

`print-spawn-chrome` — `--print` hid framed vs `-noborder`.

## Did

- `--print` writes `cssvr: spawn chrome framed` or `cssvr: spawn chrome -noborder`.
- Argv is exec truth; CSS-missing falls back to planned/persisted noborder.

## Did not

- HMD walk (user-gated). Dual-origin remains P0 but in-game.
- Menu chrome / toasts. No force `-noborder`.

## Tests

`cssvrmod_tests` — 87 passed, 0 failed (670 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`persist-resize` — remember a user-resized framed window in launch.cfg. Never queue `dual-renderview-ipd-origin`.
