# Cycle 8 — 2026-09-18

## Focus

`live-xr-input-proven` — analog stick into `CUserCmd`, not only `+forward`.

## Did

- `usercmd.hpp`: detect vptr vs no-vptr via sane viewangles; apply forward/side/up/buttons.
- Locate `ClientModeShared::CreateMove` by RTTI + `xmm0` dt + `rsi` cmd (slot 22 on this CSS).
- Vtable hook applies the last `InputMap` overlay. Movement `ClientCmd` skipped if hooked.
- Offline locator ran on live `client.so`.

## Did not

- Claim in-game XR controls from offline green.
- Hook `IEngineTrace` (next).

## Tests

`cssvrmod_tests` — 50 passed, 0 failed (366 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`ienginetrace-live` — last-free hull on live hands.
