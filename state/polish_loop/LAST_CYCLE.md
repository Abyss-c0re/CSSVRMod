# Cycle 52 — 2026-09-18

## Focus

`dual-pose-ipd-is-second-plane` — world IPD + VIEW pose IPD = two planes.

## Did

- `StereoView_SubmitPoseX` is always 0. The blit sits on the lenses (cycle 1).
- Dual IPD stays in the two world origins only. Pose IPD on those rasters is heresy.

## Did not

- HMD walk (user-gated). Dual paint still unproven in-game.
- Menu / toast / print / help / cfg chrome. No force `-noborder`.

## Tests

`cssvrmod_tests` — 105 passed, 0 failed (844 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — close with no commit unless a real offline bug appears. Never queue `dual-renderview-ipd-origin`.
