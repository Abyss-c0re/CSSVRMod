# Cycle 117 — 2026-09-19

## Focus

`stick-click-duck` — `kInDuck` / SDL ctrl existed; XR never crouched.

## Did

- Recovered cycle 116 tip `09eff72`. CSS is not running. Log still has no `icvar ver=`.
- Left thumbstick click → `IN_DUCK`. Cube uses that slot for sprint; CSS has none. A stays jump, B stays reload.
- VK ClientCmd edge `+duck`/`-duck` so present still ducks when CreateMove is live.
- OpenXR binds `/user/hand/left/input/thumbstick/click`.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Remap A/B to Cube crouch/jump (would steal reload).
- Invent print/help/menu/toast/cfg chrome.

## Tests

138 passed / 0 failed (1051 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
