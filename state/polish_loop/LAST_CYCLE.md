# Cycle 11 — 2026-09-18

## Focus

`standalone-settings` — desktop host for Vision knobs + launch prefs.

## Did

- `settings.hpp`: apply eyescale / H / V / scale / backend / map / noborder.
- Persist launch prefs in `~/.config/gvrmod/cssvr_launch.cfg`; calib still live-reloads.
- `CSSVR --settings` prints. `CSSVR --set K V` writes and exits. `--play` spawns after a set.
- Saved backend/map apply on the next Start (Cube or CSSVR).

## Did not

- A GTK/Qt window (CLI host is the product surface this cycle).
- Claim stereo or HMD from offline green.

## Tests

`cssvrmod_tests` — 60 passed, 0 failed (407 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`dual-paint-hmd-walk` — remaining P0 is in-game proof, not more offline stereo claims.
