# Cycle 111 — 2026-09-19

## Focus

`xr-abxy-lastinv` — XR only bound A; B/X/Y were documented but never polled, Y never fired lastinv.

## Did

- Recovered cycle 110 tip `d4273e7`. CSS not running. Log has no `icvar ver=` (splash toast only).
- Bind Touch B/X/Y. InputMap: B reload, X use, Y `lastinv`. ClientCmd edges lastinv on rising edge.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

135 passed / 0 failed (1038 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
