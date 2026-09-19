# Cycle 158 — 2026-09-19

## Focus

`melee-vel-relative-to-hmd` — Lua Get*HandVelocityRelative; world vel used to punch while walking.

## Did

- Recovered cycle 157 tip `20d9e1f` (`c583215` on origin). CSS is not running. Log still has no `icvar ver=` / `register cssvr_start=`. Product commit `467ae0a`.
- Tick used raw hand world vel for melee gates. Walking with the HMD (hand.vel == hmd.vel) crossed the 75 u/s threshold. `MeleeVelRelative` subtracts HMD vel like Lua. No CSS damage dispatch.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

`cssvrmod_tests` 169/169 (1168 asserts). Offline green ≠ stereo/HMD.

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
