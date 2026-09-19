# Cycle 119 — 2026-09-19

## Focus

`aim-pitch-normalize` — VectorAngles look-up is 270°; ViewAnglesSane dropped snap-on-fire.

## Did

- Recovered cycle 118 tip `34eb480`. CSS is not running. Log still has no `icvar ver=`.
- AimViewAngles / Look_Decide wrap pitch (270 → -90). Firing while aiming up used to skip usercmd and SetViewAngles.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

142 passed / 0 failed (1061 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
