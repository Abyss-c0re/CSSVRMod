# Cycle 7 — 2026-09-18

## Focus

`engine-setviewangles-selftest` — movement/bullets follow look only after a real Get/Set pair.

## Did

- `ViewAnglesSane` + `ViewAnglesRoundtripOk` (write probe, read back, restore).
- Live probe: slots 19/20 only if `dladdr` says `engine.so` **and** the roundtrip works.
- `EngineSetViewAngles` no-ops unless `angles_ok`.
- RenderView hook writes cyclopean HMD look to the engine after that gate.

## Did not

- Guess a vtable index without a roundtrip.
- Claim HMD look or stereo from offline green.

## Tests

`cssvrmod_tests` — 46 passed, 0 failed (350 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`live-xr-input-proven` — movement / fire from XR in-game (CreateMove / ClientCmd still stubs).
