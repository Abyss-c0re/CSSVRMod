# Cycle 112 — 2026-09-19

## Focus

`icvar-vstdlib-noload-lazy` — NOW+NOLOAD can miss CSS libvstdlib (no SONAME); ICvar never logged.

## Did

- Recovered cycle 111 tip `4074014`. CSS not running. Log still has no `icvar ver=`.
- `Module_SoHandle` uses `RTLD_LAZY|RTLD_NOLOAD`. ICvar basename matches maps `(deleted)`. Sibling vstdlib from mapped engine if FindCssInstall misses.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

136 passed / 0 failed (1041 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
