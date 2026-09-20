# Cycle 268 — 2026-09-20

## Focus

`xr-init-no-hmd-does-not-latch` — GetSystem no_hmd must not burn the init-fail cap before the HMD appears.

## Did

- Recovered cycle 267 tip `eeb1782` (record `f390980`). CSS is not running. SteamVR is not running. WiVRn is up. Log still has no `icvar ver=` / `register cssvr_start=` / `renderview dual`.
- Hunt: Tick / overlay / mailbox / copy / HMD / grip / begun-frame / Wait-needs-Begin / EndSession / KeepWaited / STOPPING retry / DropLostSession / init-fail reset already drop. Splash no_hmd still incremented the cap so four presents blocked CreateSess.
- `XrSession_CountInitFail`. Dual paint stays HMD-gated.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

offline 189/189 (1360/1360 asserts). Not HMD-proven.

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With OpenXR (WiVRn is already up), type `cssvr_start`. Dual paint stays HMD-gated.
