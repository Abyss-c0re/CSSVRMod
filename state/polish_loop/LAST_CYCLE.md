# Cycle 262 — 2026-09-20

## Focus

`xr-wait-needs-begin` — WaitFrame success must BeginFrame before the next Wait or EndSession.

## Did

- Recovered cycle 261 (uncommitted idle journal; tip `101d004` / product `618ccc1`). CSS is not running. SteamVR is not running. WiVRn is up. Log still has no `icvar ver=` / `register cssvr_start=` / `renderview dual`.
- Hunt: Tick / overlay / mailbox / copy / HMD / grip / begun-frame / exit-req already drop. Wait+Begin miss used to Wait again (illegal) and STOPPING EndSession without Begin.
- `XrFrame_SkipWait` / `XrFrame_BeginBeforeLeave`. Live retries Begin; LeaveRunning Begins then Ends. Dual paint stays HMD-gated.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

offline 189/189 (1325/1325 asserts). Not HMD-proven.

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With OpenXR (WiVRn is already up), type `cssvr_start`. Dual paint stays HMD-gated.
