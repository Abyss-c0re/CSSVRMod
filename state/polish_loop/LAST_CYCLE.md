# Cycle 147 — 2026-09-19

## Focus

`offhand-hull-knuckle-offset` — Lua processHand samples the off-hand sphere 2.5u along forward.

## Did

- Recovered cycle 146 tip `bdf41cf` (`855938d` on origin). CSS is not running. Log still has no `icvar ver=` / `register cssvr_start=`. Product commit `4111583`.
- Wrist-only off-hand hull left the knuckles in the wall. Tick now samples `HandCollisionSample` (2.5u) on the off-hand; gun hand still uses AdjustCollisionsBox. Still a sphere, not a gun box.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

`cssvrmod_tests` 163/163 (1145 asserts). Offline green ≠ stereo/HMD.

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
