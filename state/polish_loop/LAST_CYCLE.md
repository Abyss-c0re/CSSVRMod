# Cycle 49 — 2026-09-18

## Focus

`xr-session-ok-only-when-running` — log lied `xr submit fail #N session_ok` on warmup and after STOPPING.

## Did

- `session_ok` only after `xrBeginSession`. CreateSession is `session_created`.
- STOPPING / LOSS_PENDING / EXITING / instance-loss update the reason. LOSS toasts no-HMD once.
- `shouldRender=false` is a skip, not a fail. Warmup / stop do not increment `xr_fail`.
- Shutdown does not re-arm the no-HMD toast (one-shot per process).

## Did not

- HMD walk (user-gated). Dual paint still unproven in-game. No renderview/createmove in `/tmp/cssvrmod.log` (CSS not relaunched).
- Menu / toast / print / help / cfg chrome. No force `-noborder`.

## Tests

`cssvrmod_tests` — 104 passed, 0 failed (832 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — close with no commit unless a real offline bug appears. Never queue `dual-renderview-ipd-origin`.
