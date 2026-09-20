# Cycle 244 — 2026-09-20

## Focus

`tick-state-drop-on-session-loss` — worker Tick state on STOPPING/LOSS, not only cssvr_stop.

## Did

- Recovered cycle 243 (uncommitted idle journal; tip still `9fc43ef` / product `347f563`). CSS is not running. SteamVR is not running. WiVRn is up. Log still has no `icvar ver=` / `register cssvr_start=` / `renderview dual`.
- Hunt: VK worker reset yaw / last-free / hand vel only when `!XrWanted()`. STOPPING/LOSS skipped submit but kept Tick state, so the next session_ok inherited last heading and could yank hands / fake a melee swing. Overlay / VK pair / HMD cache already dropped (235–238).
- `Tick_KeepState` / `Tick_DropState`; VK worker drops after pump; GL swap matches.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

offline 185/185 (1268/1268 asserts). Not HMD-proven.

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. Dual paint stays HMD-gated.
