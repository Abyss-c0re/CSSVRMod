# Cycle 50 — 2026-09-18

## Focus

`maps-elf-base-not-exec` — maps fallback used the RX segment as `client.so` base.

## Did

- First maps line is the ELF load base (vaddr 0). Hook RVAs are ELF vaddrs.
- CSS `client.so` first PT_LOAD is R; RX is +0x644000. exec+RVA never hits RenderView.
- Kernel ` (deleted)` suffix still matches and is stripped from the path.

## Did not

- HMD walk (user-gated). Dual paint still unproven in-game.
- Menu / toast / print / help / cfg chrome. No force `-noborder`.

## Tests

`cssvrmod_tests` — 104 passed, 0 failed (835 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — close with no commit unless a real offline bug appears. Never queue `dual-renderview-ipd-origin`.
