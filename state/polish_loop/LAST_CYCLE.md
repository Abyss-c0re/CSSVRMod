# Cycle 160 — 2026-09-19

## Focus

`empty-fire-keeps-hmd-look` — no gun means no snap-on-fire; live Tick has no weapon query.

## Did

- Recovered cycle 159 tip `7329f7d` (`b1ce28b` on origin). CSS is not running. Log still has no `icvar ver=` / `register cssvr_start=`. Product commit `ae56d53`.
- Empty primary still ran `GunFromHand` with the default 12u muzzle. Trigger fire snap-aimed along the fist instead of the HMD. `GunFromHand` / `ResolveMuzzle` now only when `WeaponIsGun`.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

`cssvrmod_tests` 171/171 (1176 asserts). Offline green ≠ stereo/HMD.

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
