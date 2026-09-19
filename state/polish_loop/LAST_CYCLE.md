# Cycle 118 — 2026-09-19

## Focus

`stick-turn-yaw-wrap` — unbounded stick yaw passed ±720 so ViewAnglesSane dropped look.

## Did

- Recovered cycle 117 tip `c8a10d6`. CSS is not running. Log still has no `icvar ver=`.
- `AngleNormalize` wraps stick-turn `yaw_off` and Look/InputMap yaw. After ~6 spins, SetViewAngles / usercmd used to refuse.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

140 passed / 0 failed (1056 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
