# Cycle 157 — 2026-09-19

## Focus

`offhand-fist-melee-origin` — Lua left is always fist; off-hand trigger must hull the off-hand.

## Did

- Recovered cycle 156 tip `70e2284` (`c69703b` on origin). CSS is not running. Log still has no `icvar ver=` / `register cssvr_start=`. Product commit `20d9e1f`.
- Fist melee used the primary pose even when `melee_intent` came from the off-hand trigger. Tick now uses `MeleeSwingPose` / `MeleeSwingHandId`: knife stays on the gun hand; fist hulls the off-hand knuckles. No CSS damage dispatch.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

`cssvrmod_tests` 167/167 (1161 asserts). Offline green ≠ stereo/HMD.

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
