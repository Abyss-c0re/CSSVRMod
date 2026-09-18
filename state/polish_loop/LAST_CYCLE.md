# Cycle 42 — 2026-09-18

## Focus

`client-base-maps` — `dlopen("client.so", NOLOAD)` misses CSS's path-loaded module.

## Did

- Parse `/proc/self/maps` for `client.so` (prefer r-x, keep full path).
- `Module_ClientBase` tries short name, install path, then maps path + `dladdr`.
- RenderView + CreateMove hooks share it. Log `renderview install try` so a miss is not silent.

## Did not

- HMD walk (user-gated). Dual paint still unproven in-game.
- Menu / toast / print / help / cfg chrome. No force `-noborder`.

## Tests

`cssvrmod_tests` — 94 passed, 0 failed (741 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — close with no commit unless a real offline bug appears. Never queue `dual-renderview-ipd-origin`.
