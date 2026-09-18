# Cycle 48 — 2026-09-18

## Focus

`maps-basename-not-substring` — `client.so` needle matched `steamclient.so`.

## Did

- Maps path match requires `/client.so` (basename), not a substring.
- steamclient.so first in maps no longer steals the CSS module base.
- Same rule for engine.so.

## Did not

- HMD walk (user-gated). Dual paint still unproven in-game.
- Menu / toast / print / help / cfg chrome. No force `-noborder`.

## Tests

`cssvrmod_tests` — 99 passed, 0 failed (778 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — close with no commit unless a real offline bug appears. Never queue `dual-renderview-ipd-origin`.
