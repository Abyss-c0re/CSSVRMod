# Cycle 1 — 2026-09-18

## Focus

Stop same-frame **projection pose IPD**. User: two planes, no stereo, black frames.

## Did

- Confessed: this is still a mono CSS present, not a VRMOD.
- `CalibEye.pose_x` is always 0. xrEndFrame uses identity VIEW pose + HMD FOV.
- UV `eyescale` crop remains for Vision dials only.
- Stood up `state/polish_loop/` and a 13-minute audit scheduler.
- Offline tests after the submit law change.

## Did not

- Dual RenderView (next_focus).
- Standalone settings UI.
- Live IEngineTrace collisions.

## Tests

`cssvrmod_tests` — see journal after build.

## Next

`dual-renderview-ipd-origin` — hook CSS view origin, paint twice, then pose IPD is legal.
