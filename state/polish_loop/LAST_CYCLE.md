# Cycle 263 — 2026-09-20

## Focus

`xr-shutdown-endsession-after-leave` — Shutdown must LeaveRunning (close Wait/Begin) before EndSession.

## Did

- Recovered cycle 262 tip `3ec4d6c` (product `b44e9c5`). CSS is not running. SteamVR is not running. WiVRn is up. Log still has no `icvar ver=` / `register cssvr_start=` / `renderview dual`.
- Hunt: Tick / overlay / mailbox / copy / HMD / grip / begun-frame / exit-req / Wait-needs-Begin already drop. Shutdown still EndSession first, so a Wait-without-Begin pair leaked.
- `XrFrame_CanEndSession`. Shutdown LeaveRunning then EndSession. Dual paint stays HMD-gated.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

offline 189/189 (1329/1329 asserts). Not HMD-proven.

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With OpenXR (WiVRn is already up), type `cssvr_start`. Dual paint stays HMD-gated.
