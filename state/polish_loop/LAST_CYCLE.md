# Cycle 108 — 2026-09-19

## Focus

`input-stick-right-turns-right` — stick-right increased Source yaw (look left). Cube subtracts.

## Did

- Recovered cycle 107 tip `9ce03a9`. CSS still not running; log has no `icvar ver=`.
- InputMap snap/smooth turn now matches gVRMod: +thumbstick decreases yaw.
- Offline test: stick right → negative yaw; snap left → +30.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

131 passed / 0 failed (1021 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
