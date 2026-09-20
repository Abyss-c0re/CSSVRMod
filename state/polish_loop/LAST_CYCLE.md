# Cycle 237 — 2026-09-20

## Focus

`worker-pump-on-session-loss` — skip leftover submit, still PollEvents so READY after STOPPING is seen.

## Did

- Recovered cycle 236 tip `60d1c6b` (`93663c0` on origin). CSS is not running. SteamVR is not running. WiVRn is up. Log still has no `icvar ver=` / `register cssvr_start=` / `renderview dual`.
- Hunt found: cycle 235 skipped the worker mailbox on STOPPING/LOSS without pumping events, so READY after EndSession was never seen. Worker now PollEvents while XR is wanted and still refuses leftover submit. Offline 183/183.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

`cssvrmod_tests` 183 passed, 0 failed

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With OpenXR (WiVRn is already up), type `cssvr_start`. Dual paint stays HMD-gated.
