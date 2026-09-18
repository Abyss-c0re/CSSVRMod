# Cycle 53 — 2026-09-18

## Focus

`mono-uv-ipd-is-second-plane` — live path is MONO; per-eye UV IPD split one framebuffer.

## Did

- `CalibEye` no longer shifts L/R UV by eyescale. Same crop both eyes.
- Shared Vision pan/zoom stay. Dual IPD remains world origins only.

## Did not

- HMD walk (user-gated). Dual paint still unproven in-game.
- Menu / toast / print / help / cfg chrome. No force `-noborder`.

## Tests

`cssvrmod_tests` — 105 passed, 0 failed (846 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — close with no commit unless a real offline bug appears. Never queue `dual-renderview-ipd-origin`.
