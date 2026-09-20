# Cycle 256 — 2026-09-20

## Focus

`xr-endframe-on-abort` — begun OpenXR frame must EndFrame on swapchain miss / STOPPING, not only shutdown.

## Did

- Recovered cycle 255 (uncommitted idle journal; tip `9635b54` / product `b062f4a`). CSS is not running. SteamVR is not running. WiVRn is up. Log still has no `icvar ver=` / `register cssvr_start=` / `renderview dual`.
- Hunt: Tick / overlay / mailbox / copy / HMD cache / Vision grip already drop. A Wait+Begin could stay open: swapchain miss returned without EndFrame; STOPPING called EndSession first. Next READY Wait+Begin is illegal.
- `XrFrame_EndOnAbort`. LeaveRunning EndFrames first; STOPPING EndSession after. Submit miss EndFrames. Dual paint stays HMD-gated.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

offline 188/188 (1309/1309 asserts). Not HMD-proven.

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With OpenXR (WiVRn is already up), type `cssvr_start`. Dual paint stays HMD-gated.
