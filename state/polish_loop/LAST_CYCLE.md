# Cycle 33 — 2026-09-18

## Focus

`print-spawn-wh` — `--print` hid the persisted desktop `-w`/`-h`.

## Did

- `--print` writes `cssvr: spawn -w N -h N` from argv (exec truth) or planned opts if CSS is missing.
- Also dumps space-joined argv so `+mat_setvideomode` is auditable without spawn.

## Did not

- HMD walk (user-gated). Dual-origin remains P0 but in-game.
- Menu chrome / toasts. No force `-noborder`.

## Tests

`cssvrmod_tests` — 86 passed, 0 failed (661 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`print-spawn-chrome` — `--print` should say framed vs `-noborder`. Never queue `dual-renderview-ipd-origin`.
