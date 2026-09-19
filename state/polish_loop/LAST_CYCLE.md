# Cycle 141 — 2026-09-19

## Focus

`hand-correction-clamp` — last-free hull used to snap the hand 70u when rest was solid.

## Did

- Wired Lua `ApplyHandCorrection`: dead-zone hull noise (<0.35u) and cap yanks at 40u. Nearby walls still clip; teleport release still drops last-free past 100u from the HMD.
- Offline 159/159. Not HMD-proven.

## Did not

- HMD walk. Stereo unproven. CSS is not running; log still has no `icvar ver=`.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

`cssvrmod_tests` 159 passed, 0 failed (1133/1133 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
