# Cycle 57 — 2026-09-18

## Focus

`console-unrestricted-cmd` — typed `cssvr_start` goes through ClientCmd_Unrestricted (slot 106), not restricted ClientCmd (slot 7).

## Did

- Wrap both CEngineClient slots. 7 stays restricted; 106 is the console path (`mov rdi,rsi; jmp Cbuf_AddText`).
- Same filter; each slot keeps its own original.

## Did not

- HMD walk. Dual paint still unproven in-game.
- Menu / toast / print / help / cfg chrome. No force `-noborder`.
- Did not queue `dual-renderview-ipd-origin`.

## Tests

`cssvrmod_tests` — 113 passed, 0 failed (903 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — close with no commit unless a real offline bug appears. Next CSS start needs the new hook (Steam LaunchOptions + `--install` copy). Never queue `dual-renderview-ipd-origin`.
