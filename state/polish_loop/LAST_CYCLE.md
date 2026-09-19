# Cycle 113 — 2026-09-19

## Focus

`look-includes-stick-turn` — RenderView SetViewAngles(HMD) clobbered CreateMove locomotion yaw.

## Did

- Recovered cycle 112 tip `83030c9`. CSS not running. Log still has no `icvar ver=`.
- `Look_Decide` adds persisted stick-turn yaw. Hooks `Turn_NoteYawOff`; RenderView peeks it so world cameras and engine yaw match Cube snap/smooth.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

137 passed / 0 failed (1044 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
