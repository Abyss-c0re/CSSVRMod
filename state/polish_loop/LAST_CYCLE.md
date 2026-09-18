# Cycle 43 — 2026-09-18

## Focus

`engine-so-noload-path` — ProbeLiveEngine used short-name NOLOAD then `dlopen(nullptr)`.

## Did

- `Module_SoHandle` / `Module_SoPlan` for any path-loaded Source .so. Never the main exe.
- ProbeLiveEngine opens `engine.so` + `client.so` via install path + maps.
- Look / trace factories no longer come from the launcher.

## Did not

- HMD walk (user-gated). Dual paint still unproven in-game.
- Menu / toast / print / help / cfg chrome. No force `-noborder`.

## Tests

`cssvrmod_tests` — 95 passed, 0 failed (752 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — close with no commit unless a real offline bug appears. Never queue `dual-renderview-ipd-origin`.
