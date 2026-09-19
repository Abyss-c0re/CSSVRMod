# Cycle 164 — 2026-09-19

## Focus

`melee-finite-diff-relative-to-hmd` — live OpenXR never fills pose.vel; world delta used to punch while walking.

## Did

- Recovered cycle 163 tip `ae56d53` (`515879c` on origin). CSS is not running. Log still has no `icvar ver=` / `register cssvr_start=`. Product commit `6262bc2`.
- Cycle 158 subtracted HMD vel, but `XrHostPollInput` never writes `pose.vel`. Live melee finite-diffed world hand pos, so walking with still hands crossed 75 u/s. `HandVelOrDelta` now diffs `hand.pos − hmd.pos`. No CSS damage dispatch.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

`cssvrmod_tests` 173/173 (1182 asserts). Offline green ≠ stereo/HMD.

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
