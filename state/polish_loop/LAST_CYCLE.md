# Cycle 137 — 2026-09-19

## Focus

`wall-lock-hmd-teleport` — last-free hull used to yank the hand across the map after spawn/teleport.

## Did

- Wired Lua `ShouldReleaseWallLock` into Tick: if the safe sample is >100u from the HMD, drop last-free and keep the tracked pose (one frame of penetration beats a map-width snap). Nearby walls still clip.
- Offline 157/157. Not HMD-proven.

## Did not

- HMD walk. Stereo unproven. CSS is not running; log still has no `icvar ver=`.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

`cssvrmod_tests` 157 passed, 0 failed (1125/1125 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
