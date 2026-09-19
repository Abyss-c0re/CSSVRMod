# Cycle 170 — 2026-09-19

## Focus

`melee-vel-tracks-between-swings` — Lua samples relative vel every frame.

## Did

- Recovered cycle 169 tip `e925357` (`9d08ff1` on origin). CSS is not running.
- Fist finite-diff used to sample only while `melee_intent`. Live Tick is empty-handed, so the next punch inherited a stale delta. Per-hand states now update every tick. Commit `94d7b29`.
- Offline test `tick_melee_vel_tracks_between_swings`.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.
- Live weapon query (still nullptr).

## Tests

`cssvrmod_tests` 178/178 (1206 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
