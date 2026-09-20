# Cycle 235 — 2026-09-20

## Focus

`stale-vk-pair-on-session-loss` — DropVkEyes on STOPPING/LOSS, not only cssvr_stop.

## Did

- Recovered cycle 234 tip `51246f1` (`8c3397e` on origin). CSS is not running. SteamVR is not running. WiVRn is up. Log still has no `icvar ver=` / `register cssvr_start=` / `renderview dual`.
- Hunt found: `DropVkEyes` was WantXr-only. Session STOPPING/LOSS while XR is still wanted kept `g_vk_eyes`, so the next `session_ok` could PushXrDual last-session rasters before two new world paints. WrapPresent now drops the pair whenever dual-paint should not run. Worker skips leftover mailbox unless `session_ok`. Offline 181/181.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

`cssvrmod_tests` 181 passed, 0 failed

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With OpenXR (WiVRn is already up), type `cssvr_start`. Dual paint stays HMD-gated.
