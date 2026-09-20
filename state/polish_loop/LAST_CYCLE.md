# Cycle 265 — 2026-09-20

## Focus

`xr-pump-stopping-retry-endsession` — STOPPING must retry Leave+EndSession on later pumps after Begin miss.

## Did

- Recovered cycle 264 tip `96b38aa` (record `a50d545`). CSS is not running. SteamVR is not running. WiVRn is up. Log still has no `icvar ver=` / `register cssvr_start=` / `renderview dual`.
- Hunt: Tick / overlay / mailbox / copy / HMD / grip / begun-frame / Wait-needs-Begin / EndSession-after-Leave / KeepWaited already drop. STOPPING fired once; Begin miss skipped EndSession; cssvr_stop froze pump (`session_ok` false).
- `XrFrame_RetryLeave` / `XrFrame_ShouldEndSession` / drain pump. Dual paint stays HMD-gated.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

offline 189/189 (1345/1345 asserts). Not HMD-proven.

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With OpenXR (WiVRn is already up), type `cssvr_start`. Dual paint stays HMD-gated.
