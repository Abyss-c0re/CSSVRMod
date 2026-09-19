# Cycle 107 — 2026-09-19

## Focus

`tick-primary-hand-gun-slave` — left-handed Tick kept the gun and knife on the right hand.

## Did

- Recovered cycle 106 tip `459c146`. CSS still not running; log has no `icvar ver=`.
- Tick gun / aim / melee now follow the primary hand (`CSSVR_LEFT_HANDED` / HAND row). Laser already did.
- Offline test: left pose owns the AK muzzle; knife melee `Hand::Left`.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

130 passed / 0 failed (1018 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
