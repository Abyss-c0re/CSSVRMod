# Cycle 264 — 2026-09-20

## Focus

`xr-leave-keeps-waited-on-begin-miss` — LeaveRunning must not clear waited if Begin failed to close Wait.

## Did

- Recovered cycle 263 tip `1cabe06` (record `5d3a51e`). CSS is not running. SteamVR is not running. WiVRn is up. Log still has no `icvar ver=` / `register cssvr_start=` / `renderview dual`.
- Hunt: Tick / overlay / mailbox / copy / HMD / grip / begun-frame / Wait-needs-Begin / EndSession-after-Leave already drop. LeaveRunning still cleared waited on Begin fail, so CanEndSession EndSession'd an open Wait.
- `XrFrame_KeepWaited`. Dual paint stays HMD-gated.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

offline 189/189 (1335/1335 asserts). Not HMD-proven.

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With OpenXR (WiVRn is already up), type `cssvr_start`. Dual paint stays HMD-gated.
