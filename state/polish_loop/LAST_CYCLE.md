# Cycle 60 — 2026-09-18

## Focus

`find-css-from-self` — the in-process hook found CSS only via HOME Steam paths, so RenderView locate could miss inside a Steam runtime.

## Did

- `FindCssInstall` tries `/proc/self/exe` parent and cwd after env, before HOME.
- `CssRootFromExe` is the pure helper.

## Did not

- HMD walk. Dual paint still unproven in-game.
- Menu / toast / print / help / cfg chrome. No force `-noborder`.
- Did not queue `dual-renderview-ipd-origin`.

## Tests

`cssvrmod_tests` — 115 passed, 0 failed (918 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — close with no commit unless a real offline bug appears. Live CSS is still the old unhooked Steam process. Never queue `dual-renderview-ipd-origin`.
