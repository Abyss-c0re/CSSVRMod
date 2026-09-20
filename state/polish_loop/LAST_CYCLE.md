# Cycle 245 — 2026-09-20

## Focus

`mailbox-drop-on-session-loss` — leftover XR mailbox on STOPPING/LOSS, not only skip-while-!ok.

## Did

- Recovered cycle 244 tip `8f5cd5d` (product `39e7aa3`). CSS is not running. SteamVR is not running. WiVRn is up. Log still has no `icvar ver=` / `register cssvr_start=` / `renderview dual`.
- Hunt: cycle 235 skipped worker submit while !session_ok, but a mailbox dual sitting through STOPPING→READY submitted last-session rasters. Harvest still pushed on `XrWanted` during STOPPING.
- `XrMailbox_Keep` / `Drop` / epoch `Accept`. Present DropMailbox with DropVkEyes. Harvest gated on session_ok. Worker rejects old epoch.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

offline 186/186 (1281/1281 asserts). Not HMD-proven.

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. Dual paint stays HMD-gated.
