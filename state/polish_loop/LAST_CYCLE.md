# Cycle 168 — 2026-09-19

## Focus

`floor-ceiling-passthrough` — Lua processHand drops floor/ceiling clips.

## Did

- Recovered cycle 167 tip `ce4f587` (`ba34796` on origin). CSS is not running.
- Sweep still depens at a floor+solid joint (`floor_solid_depen`). Tick now runs `DropFloorCeilingLock` so the fist is not yanked off the ground. Vertical walls still lock. Commit `e925357`.
- Offline tests include `collision_floor_passthrough_when_desired_solid` / `tick_floor_does_not_yank_fist`.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.
- Live weapon query (still nullptr).

## Tests

`cssvrmod_tests` 177/177 (1202 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
