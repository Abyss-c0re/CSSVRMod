# Cycle 258 — 2026-09-20

## Focus

`xr-exit-req-no-fail-latch` — failed xrRequestExitSession must not latch; cssvr_stop retries.

## Did

- Recovered cycle 257 tip `4aaba7a` (product `d109cbc`). CSS is not running. SteamVR is not running. WiVRn is up. Log still has no `icvar ver=` / `register cssvr_start=` / `renderview dual`.
- Hunt: Tick / overlay / mailbox / copy / HMD / grip / begun-frame / WantXr pump already drop. Cycle 257 latched g_exit_req on any RequestExit call; a fail skipped every later cssvr_stop so last rasters could stay on the HMD.
- `XrSession_LatchExitReq`. Live latches only on XR_SUCCESS. Dual paint stays HMD-gated.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

offline 188/188 (1319/1319 asserts). Not HMD-proven.

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With OpenXR (WiVRn is already up), type `cssvr_start`. Dual paint stays HMD-gated.
