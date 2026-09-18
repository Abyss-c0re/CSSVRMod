# Cycle 39 — 2026-09-18

## Focus

`persist-resize-last-win` — PollEvent persisted any window resize (splash could write 640×480).

## Did

- `SdlEventWinSizeFor` requires SDL window id == last CSS window.
- PollEvent no longer writes launch.cfg from a popup/splash resize.

## Did not

- HMD walk (user-gated). Dual-origin remains P0 but in-game.
- Menu / toast / print / help / launch.cfg seed. No force `-noborder`.

## Tests

`cssvrmod_tests` — 92 passed, 0 failed (726 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — close with no commit unless a real offline bug appears. Never queue `dual-renderview-ipd-origin`.
