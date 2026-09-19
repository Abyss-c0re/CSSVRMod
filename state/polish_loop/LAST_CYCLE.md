# Cycle 109 — 2026-09-19

## Focus

`input-turn-accumulates` — stick-turn added one frame of yaw onto HMD look and dropped it.

## Did

- Recovered cycle 108 tip `6b13a0b`. CSS still not running; log has no `icvar ver=`.
- `TurnState` persists locomotion yaw; snap latches like Cube. Tick/hooks hold the state.
- Offline: two seconds of hold → -90 then -180; held snap stays one step.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

133 passed / 0 failed (1026 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. Game-dir hook was stale vs install/. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
