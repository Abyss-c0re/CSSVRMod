# Cycle 145 — 2026-09-19

## Focus

`gun-hull-offset` — wrist-only hull left the gun body in the wall.

## Did

- Wired Lua `AdjustCollisionsBox` on the primary hand: sweep the 10u (3u melee) gun sample, apply that delta back to the wrist. Off-hand stays a wrist sphere. Nearby walls / teleport release / 40u clamp unchanged.
- Offline 161/161. Not HMD-proven.

## Did not

- HMD walk. Stereo unproven. CSS is not running; log still has no `icvar ver=`.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

`cssvrmod_tests` 161 passed, 0 failed (1139/1139 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
