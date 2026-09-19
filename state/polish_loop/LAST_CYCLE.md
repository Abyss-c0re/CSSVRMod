# Cycle 159 — 2026-09-19

## Focus

`empty-primary-not-a-gun` — Lua processHand is a fist until holdingGun; live Tick has no weapon query.

## Did

- Recovered cycle 158 tip `467ae0a` (`9d7b38b` on origin). CSS is not running. Log still has no `icvar ver=` / `register cssvr_start=`. Product commit `7329f7d`.
- Empty primary used the 10u gun box and 18u barrel tip. Live hook always passes `wep=nullptr`, so a fist was yanked off walls a gun-length ahead. Gun hull + tip stay when `WeaponIsGun`.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

`cssvrmod_tests` 170/170 (1170 asserts). Offline green ≠ stereo/HMD.

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
