# Cycle 129 — 2026-09-19

## Focus

`pose-session-reset-on-stop` — hand vel and last-free hull must not survive cssvr_stop.

## Did

- Recovered cycle 128 tip `69f87bc`. CSS is not running. Log still has no `icvar ver=` / `register cssvr_start=`.
- `HandVel_Reset` / `Wall_Reset`. VK worker and GL swap clear them with turn when XR is off. Restart no longer finite-diffs a fake swing or yanks hands to the old map pose.
- Tests: `melee_hand_vel_resets_on_stop`, `collision_wall_resets_on_stop`. Offline 153/153.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

`cssvrmod_tests` 153 passed, 0 failed (1103/1103 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
