# Cycle 51 — 2026-09-18

## Focus

`dual-ipd-ignores-uv-eyescale` — Vision eyescale crushed world camera sep.

## Did

- Dual paint / submit pose use full `ipd_m`. eyescale stays UV-only (cycle 1 / pain point).
- Live calib `eye=0.13` would have put cameras ~8 mm apart (no stereo even after two paints).

## Did not

- HMD walk (user-gated). Dual paint still unproven in-game.
- Menu / toast / print / help / cfg chrome. No force `-noborder`.

## Tests

`cssvrmod_tests` — 105 passed, 0 failed (843 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — close with no commit unless a real offline bug appears. Never queue `dual-renderview-ipd-origin`.
