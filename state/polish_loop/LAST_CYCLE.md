# Cycle 46 — 2026-09-18

## Focus

`identical-eyes-stay-mono` — two copies of one RT still unlocked pose IPD.

## Did

- `VkEye_WorldsDiffer` — byte-compare L/R; same pixels are one world view.
- `DualPaint_AcceptPair` / view hook / WrapPresent stay MONO unless worlds differ.
- No new toast.

## Did not

- HMD walk (user-gated). Dual paint still unproven in-game.
- Menu / toast / print / help / cfg chrome. No force `-noborder`.

## Tests

`cssvrmod_tests` — 98 passed, 0 failed (772 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — close with no commit unless a real offline bug appears. Never queue `dual-renderview-ipd-origin`.
