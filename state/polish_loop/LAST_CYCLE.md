# Cycle 25 — 2026-09-18

## Focus

`menu3d-face-hmd` — dragged Vision panel stayed +Z, so it could sit edge-on.

## Did

- Yaw-only billboard: local +Z points at the HMD on the STAGE XZ plane.
- Laser plane and quad pose share that yaw. Default in front of origin stays identity.

## Did not

- HMD walk (user-gated).
- Pitch the panel or persist yaw.
- Claim live billboard UX from offline green.

## Tests

`cssvrmod_tests` — 80 passed, 0 failed (587 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`menu3d-primary-hand` — laser + grip from the primary hand, not hardcoded right.
