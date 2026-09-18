# Cycle 17 — 2026-09-18

## Focus

`incomplete-dual-capture-toast` — hook painted both eyes; copies missed; only a log.

## Did

- Pure `DualCapture_ToastDecide`: toast after 8 incomplete frames; never abort VR.
- `HookedRenderView` fires desktop toast + title `NO CAP`.
- Two successful copies stay quiet (label `2CAP` is not a stereo claim).

## Did not

- HMD walk (user-gated).
- Claim stereo from offline green or from two captures.

## Tests

`cssvrmod_tests` — 71 passed, 0 failed (489 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`createmove-locate-miss-toast` — analog overlay locate fail is still log-only.
