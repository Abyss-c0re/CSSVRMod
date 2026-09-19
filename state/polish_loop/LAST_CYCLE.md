# Cycle 120 — 2026-09-19

## Focus

`menu-steals-combat` — Vision panel trigger/grab used to fire and melee.

## Did

- Recovered cycle 119 tip `c781e92`. CSS is not running. Log still has no `icvar ver=`.
- Panel-visible steals trigger/grab from attack and melee. Jump/reload/use stay. Laser reason `focus_primary`.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

144 passed / 0 failed (1071 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
