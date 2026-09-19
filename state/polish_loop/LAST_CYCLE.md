# Cycle 121 — 2026-09-19

## Focus

`menu-not-score` — menu click held `IN_SCORE` while toggling the Vision panel.

## Did

- Recovered cycle 120 tip `907286f`. CSS is not running. Log still has no `icvar ver=`.
- Menu is Vision only. Right stick-click is scoreboard (`+showscores` / TAB).

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

145 passed / 0 failed (1074 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
