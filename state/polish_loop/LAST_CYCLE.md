# Cycle 236 — 2026-09-20

## Focus

`hmd-cache-drop-on-session-loss` — last locate must not drive look after stop / STOPPING / LOSS.

## Did

- Recovered cycle 235 tip `ab81d37` (`8788c77` on origin). CSS is not running. SteamVR is not running. WiVRn is up. Log still has no `icvar ver=` / `register cssvr_start=` / `renderview dual`.
- Hunt found: `XrHostLastHmd` kept the last locate across cssvr_stop and session loss, so the next `session_ok` dual paints inherited last-session yaw until a new locate. ViewHookOnSwap now clears the cache when dual-paint should not run. Invalid HMD keeps the game view. Offline 182/182.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

`cssvrmod_tests` 182 passed, 0 failed

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With OpenXR (WiVRn is already up), type `cssvr_start`. Dual paint stays HMD-gated.
