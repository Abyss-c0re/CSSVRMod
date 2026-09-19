# Cycle 123 — 2026-09-19

## Focus

`overlay-clears-on-stop` — last stick/buttons kept walking after `cssvr_stop`.

## Did

- Recovered cycle 122 tip `cea9dc2`. CSS is not running. Log still has no `icvar ver=`.
- PeekOverlay / Turn_PeekYawOff refuse when XR is off. Shutdown clears the overlay.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

147 passed / 0 failed (1086 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
