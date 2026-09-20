# Cycle 254 — 2026-09-20

## Focus

`menu3d-grip-drop-on-session-loss` — Vision-panel grip_off must not yank the quad after STOPPING/LOSS / cssvr_stop.

## Did

- Recovered cycle 253 (uncommitted idle journal; tip `95eeb17` / product `a26c8c2`). CSS is not running. SteamVR is not running. WiVRn is up. Log still has no `icvar ver=` / `register cssvr_start=` / `renderview dual`.
- Hunt: Tick / overlay / VK pair / mailbox / inflight copy / HMD cache already drop. Grip drag did not: next session_ok applied last grip_off to a new hand pose.
- `Menu3d_KeepGrip` / `Menu3d_DropGrip`. Worker + present + LeaveRunning. Panel pose stays; next grab re-locks. Dual paint stays HMD-gated.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

offline 187/187 (1302/1302 asserts). Not HMD-proven.

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With OpenXR (WiVRn is already up), type `cssvr_start`. Dual paint stays HMD-gated.
