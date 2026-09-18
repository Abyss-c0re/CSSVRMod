# Cycle 32 — 2026-09-18

## Focus

`settings-win-size` — desktop `-w`/`-h` were hardcoded 1920×1080.

## Did

- Persist `width`/`height` in launch.cfg (`--set width 1280`). Clamp 640–3840 × 480–2160.
- Spawn and `--width`/`--height` apply the knobs. Still framed (no force `-noborder`).

## Did not

- HMD walk (user-gated). Dual-origin remains P0 but in-game.
- Menu chrome / toasts.

## Tests

`cssvrmod_tests` — 85 passed, 0 failed (649 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`dual-renderview-ipd-origin` — still HMD-gated. Do not add `menu3d-*` or another toast. Prefer a non-menu offline gap or wait.
