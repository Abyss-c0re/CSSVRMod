# Cycle 64 — 2026-09-18

## Focus

`autoexec-plugin-load` — Steam Cloud wiped App 240 LaunchOptions, so the next Steam start would still have no hook.

## Did

- `--install` appends `plugin_load addons/cssvrmod/cssvrmod_plugin` to `cstrike/cfg/autoexec.cfg` (once). Steam does not Cloud-sync that file.
- Re-upserted LaunchOptions (best-effort; Cloud may wipe again).

## Did not

- HMD walk. Dual paint still unproven in-game.
- Menu / toast / print / help chrome. No force `-noborder`.
- Did not queue `dual-renderview-ipd-origin`.

## Tests

`cssvrmod_tests` — 116 passed, 0 failed (925 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — close with no commit unless a real offline bug appears. Live CSS is still the old unhooked process. Restart CSS (menu, not mid-map) so autoexec can `plugin_load`. Never queue `dual-renderview-ipd-origin`.
