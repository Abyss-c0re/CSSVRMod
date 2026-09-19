# Cycle 127 — 2026-09-19

## Focus

`panel-freeze-stick-turn` — Vision panel owns the right stick; locomotion turn must not spin the world under the quad.

## Did

- Recovered cycle 126 tip `a027028`. CSS is not running. Log still has no `icvar ver=` / `register cssvr_start=`.
- InputMap skips `Input_StickTurn` while `panel_visible`. Existing `yaw_off` still applies (heading kept). Move / jump / reload / use unchanged.
- Tests: `input_panel_freezes_stick_turn`, `tick_panel_freezes_stick_turn`. Offline 150/150.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

`cssvrmod_tests` 150 passed, 0 failed (1096/1096 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
