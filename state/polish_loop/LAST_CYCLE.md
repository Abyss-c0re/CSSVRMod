# Cycle 167 — 2026-09-19

## Focus

`melee-vel-per-hand` — Lua left/right relative vel are independent.

## Did

- Recovered cycle 166 tip `6262bc2` (`2d4b0e9` on origin). CSS is not running.
- One `HandVelState` finite-diff used to jump from the off-hand fist to the knife (~100u) and punch. Tick now picks left vs right state (`HandVelForHand`). VK/GL hooks reset both on stop. Commit `ce4f587`.
- Offline tests include `tick_melee_vel_is_per_hand` / `melee_vel_state_is_per_hand`.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.
- Live weapon query (still nullptr).

## Tests

`cssvrmod_tests` 175/175 (1193 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
