# Cycle 266 — 2026-09-20

## Focus

`xr-destroy-session-on-loss` — LOSS_PENDING / EXITING must DestroySession so Init can CreateSess again.

## Did

- Recovered cycle 265 tip `9ee38a5` (record `3adce59`). CSS is not running. SteamVR is not running. WiVRn is up. Log still has no `icvar ver=` / `register cssvr_start=` / `renderview dual`.
- Hunt: Tick / overlay / mailbox / copy / HMD / grip / begun-frame / Wait-needs-Begin / EndSession-after-Leave / KeepWaited / STOPPING retry already drop. LOSS left a dead `g_sess` with `g_info.session` true, so Init skipped CreateSess.
- `XrSession_ShouldDestroy` + DropLostSession. Dual paint stays HMD-gated.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

offline 189/189 (1348/1348 asserts). Not HMD-proven.

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With OpenXR (WiVRn is already up), type `cssvr_start`. Dual paint stays HMD-gated.
