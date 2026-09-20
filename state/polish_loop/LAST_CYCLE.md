# Cycle 267 — 2026-09-20

## Focus

`xr-init-fails-reset-on-destroy` — LOSS destroy must clear the SubmitPixels init-fail latch so CreateSess can retry.

## Did

- Recovered cycle 266 tip `20060d4` (record `e2cd0bc`). CSS is not running. SteamVR is not running. WiVRn is up. Log still has no `icvar ver=` / `register cssvr_start=` / `renderview dual`.
- Hunt: Tick / overlay / mailbox / copy / HMD / grip / begun-frame / Wait-needs-Begin / EndSession / KeepWaited / STOPPING retry / DropLostSession already drop. Static `init_fails > 3` still blocked CreateSess after LOSS.
- `XrSession_AllowInit` / `XrSession_InitFailsAfter`. Dual paint stays HMD-gated.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

offline 189/189 (1355/1355 asserts). Not HMD-proven.

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With OpenXR (WiVRn is already up), type `cssvr_start`. Dual paint stays HMD-gated.
