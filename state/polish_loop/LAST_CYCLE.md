# Cycle 45 — 2026-09-18

## Focus

`vk-present-clears-dual-gate` — ViewHookOnSwap ran only on GL swap.

## Did

- `DualPaintFrameGate` — one dual attempt per present; clear on swap.
- `WrapPresent` calls `ViewHookOnSwap` so the next CViewRender may dual-paint again.
- Tiny views still do not consume the gate.

## Did not

- HMD walk (user-gated). Dual paint still unproven in-game.
- Menu / toast / print / help / cfg chrome. No force `-noborder`.

## Tests

`cssvrmod_tests` — 97 passed, 0 failed (763 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — close with no commit unless a real offline bug appears. Never queue `dual-renderview-ipd-origin`.
