# Cycle 128 — 2026-09-19

## Focus

`turn-reset-on-stop` — stick-turn yaw must not survive cssvr_stop.

## Did

- Recovered cycle 127 tip `f462c8b`. CSS is not running. Log still has no `icvar ver=` / `register cssvr_start=`.
- `Turn_Reset` zeros yaw_off + snap latch. VK worker and GL swap call it when XR is off. Overlay clear already zeroed the peek; the live TurnState did not.
- Test: `input_turn_resets_on_stop`. Offline 151/151.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

`cssvrmod_tests` 151 passed, 0 failed (1099/1099 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
