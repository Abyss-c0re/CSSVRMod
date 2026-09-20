# Cycle 257 — 2026-09-20

## Focus

`xr-exit-on-cssvr-stop` — cssvr_stop must RequestExit and keep pumping while the session is running.

## Did

- Recovered cycle 256 tip `44bb6ad` (product `d305a87`). CSS is not running. SteamVR is not running. WiVRn is up. Log still has no `icvar ver=` / `register cssvr_start=` / `renderview dual`.
- Hunt: Tick / overlay / mailbox / copy / HMD / grip / begun-frame already drop. Pump gated WantXr only, so cssvr_stop froze PollEvents: STOPPING never ran and last rasters stayed on the HMD.
- `XrWorker_ShouldPump(wanted, running)` + `XrSession_RequestExit`. Worker `xrRequestExitSession`; EndSession still waits for STOPPING. Dual paint stays HMD-gated.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

offline 188/188 (1315/1315 asserts). Not HMD-proven.

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With OpenXR (WiVRn is already up), type `cssvr_start`. Dual paint stays HMD-gated.
