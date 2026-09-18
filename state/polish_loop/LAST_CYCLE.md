# Cycle 37 — 2026-09-18

## Focus

`help-launch-env` — `--help` hid CSSVR_LAUNCH and that resize persists.

## Did

- `--help` documents CSSVR_LAUNCH (default ~/.config/gvrmod/cssvr_launch.cfg).
- `--width`/`--height` lines say resize persists to launch.cfg.

## Did not

- HMD walk (user-gated). Dual-origin remains P0 but in-game.
- Menu chrome / toasts / another print-* line. No force `-noborder`.

## Tests

`cssvrmod_tests` — 91 passed, 0 failed (711 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`seed-launch-cfg` — first run should write default launch.cfg so `--print` is not `missing`. Never queue `dual-renderview-ipd-origin`.
