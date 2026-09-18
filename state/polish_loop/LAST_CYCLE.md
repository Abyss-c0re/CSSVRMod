# Cycle 31 — 2026-09-18

## Focus

`launch-seed-handed` — hook only saw `CSSVR_LEFT_HANDED` after XR init, so first Tick was right-only.

## Did

- `Settings_LeftHandedSeed` / parse launch.cfg text. Env wins.
- `CSSVR.sh` exports from `cssvr_launch.cfg` before exec. `CSSVR` setenv before spawn.

## Did not

- HMD walk (user-gated). Dual-origin remains P0 but in-game.
- Menu chrome.
- Claim live left-hand start from offline green.

## Tests

`cssvrmod_tests` — 85 passed, 0 failed (635 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`dual-renderview-ipd-origin` — still HMD-gated. Do not add `menu3d-*`. Prefer a non-menu offline gap or wait for a headset walk.
