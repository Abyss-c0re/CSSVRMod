# Cycle 154 — 2026-09-19

## Focus

`fist-melee-knuckle-origin` — Lua DEFAULT_OFFSET: fist traces from 5u along hand forward.

## Did

- Recovered cycle 153 tip `4111583` (`b6413cb` on origin). CSS is not running. Log still has no `icvar ver=` / `register cssvr_start=`. Product commit `70e2284`.
- Wrist-only 5u fist reach left the knuckles short of a wall 6u ahead. Tick now uses `MeleeHandOrigin` for fist; knife stays at the wrist. No CSS damage dispatch. Off-hand still swings from primary.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

`cssvrmod_tests` 165/165 (1151 asserts). Offline green ≠ stereo/HMD.

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
