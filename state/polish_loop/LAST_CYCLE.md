# Cycle 44 — 2026-09-18

## Focus

`hook-retry-until-client-mapped` — first VK present can run before client.so is mapped.

## Did

- `HookInstall_Transient("no_client_base")` — retry, do not toast (splash is not a miss).
- Locate client.so once; retry ClientBase + patch on later presents.
- CreateMove same. WrapPresent / GL swap no longer one-shot.

## Did not

- HMD walk (user-gated). Dual paint still unproven in-game.
- Menu / toast / print / help / cfg chrome. No force `-noborder`.

## Tests

`cssvrmod_tests` — 96 passed, 0 failed (760 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — close with no commit unless a real offline bug appears. Never queue `dual-renderview-ipd-origin`.
