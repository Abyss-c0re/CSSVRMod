# Cycle 56 — 2026-09-18

## Focus

`install-steam-launch-options` — `--install` copied the hook but Steam App 240 LaunchOptions stayed empty, so a normal Steam start never preloaded it.

## Did

- `--install` upserts `Software/Valve/Steam/apps/240` LaunchOptions (`LD_PRELOAD` + `CSSVR_XR=0 %command%`).
- Keeps the user's extras. Skips `"240" "hex"` blobs. Idempotent.
- Applied to the live userdata file this turn.

## Did not

- HMD walk. Dual paint still unproven in-game.
- Menu / toast / print / help chrome. No force `-noborder`.
- Did not queue `dual-renderview-ipd-origin`.

## Tests

`cssvrmod_tests` — 112 passed, 0 failed (894 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — close with no commit unless a real offline bug appears. Steam may revert LaunchOptions on exit if it still has the old file in memory; re-run `--install` after Steam restarts if the next CSS start has no hook. Never queue `dual-renderview-ipd-origin`.
