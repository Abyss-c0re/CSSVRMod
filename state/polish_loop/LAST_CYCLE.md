# Cycle 133 — 2026-09-19

## Focus

`weapon-tip-wall` — barrel tip must not pass through a wall (Lua ApplyWeaponWallToHand).

## Did

- Recovered cycle 132 (uncommitted idle) tip `9d0144e`. CSS is not running. Log still has no `icvar ver=` / `register cssvr_start=`.
- Tick applies `ApplyWeaponTip` on the primary hand after the last-free hull, then slaves the gun. Wrist pulled back when the muzzle ray hits.
- Tests: `collision_weapon_tip_pulls_hand`, `tick_weapon_tip_blocks_muzzle`. Offline 155/155.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

`cssvrmod_tests` 155 passed, 0 failed (1108/1108 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
