# Cycle 40 — 2026-09-18

## Focus

`persist-win-prefer-larger` — a later splash CreateWindow stole `g_last_win`.

## Did

- First window is always the persist target (1×1 then SetWindowSize still works).
- A later window only replaces it if plausible and area >= current. Splash cannot steal.

## Did not

- HMD walk (user-gated). Dual-origin remains P0 but in-game.
- Menu / toast / print / help / cfg seed. No force `-noborder`.

## Tests

`cssvrmod_tests` — 93 passed, 0 failed (733 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — close with no commit unless a real offline bug appears. Never queue `dual-renderview-ipd-origin`.
