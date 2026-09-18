# Cycle 59 — 2026-09-18

## Focus

`console-strip-crlf` — `cssvr_start\n` became Help, so typed console start never enabled XR.

## Did

- Treat CR/LF as token whitespace in `CssvrParseConsole`.
- `cssvr_start\n` / `\r\n` now parse as Start.

## Did not

- HMD walk. Dual paint still unproven in-game.
- Menu / toast / print / help / cfg chrome. No force `-noborder`.
- Did not queue `dual-renderview-ipd-origin`.

## Tests

`cssvrmod_tests` — 114 passed, 0 failed (914 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — close with no commit unless a real offline bug appears. Live CSS is still the old unhooked Steam process. Never queue `dual-renderview-ipd-origin`.
