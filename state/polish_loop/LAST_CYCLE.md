# Cycle 228 — 2026-09-20

## Focus

`stale-vk-dual-drops-on-stop` — last-session L/R copies must not submit after cssvr_start.

## Did

- Recovered cycle 227 tip `1891899` (`7c98327` on origin). CSS is not running. SteamVR is not running.
- Dual latch (cycle 227) cleared the MONO banner, but `g_vk_eyes` still PushXrDual'd last-session rasters on the next start. Present drops the pair when XR is off; worker discards leftover mailbox. Offline 180/180.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

`cssvrmod_tests` 180 passed, 0 failed

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
