# Cycle 54 — 2026-09-18

## Focus

`dual-origin-honors-swap-eyes` — Vision SWAP did nothing after pose/UV IPD went to 0.

## Did

- Dual world origins swap when `swap_eyes` is set.
- VIEW pose and UV stay identity (no second plane).

## Did not

- HMD walk (user-gated). Dual paint still unproven in-game.
- Menu / toast / print / help / cfg chrome. No force `-noborder`.

## Tests

`cssvrmod_tests` — 106 passed, 0 failed (851 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — close with no commit unless a real offline bug appears. Never queue `dual-renderview-ipd-origin`.
