# Cycle 85 — 2026-09-19

## Focus

`icvar-dispatch-args` — ICvar Dispatch must keep `cssvr_set` key/value.

## Did

- Recovered cycle 84 idle. Tip `8d34b00`. CSS is not running. Log unchanged since 01:27.
- `8d34b00` registered `cssvr_set` but Dispatch passed only `m_pszName`. `ICvar_DispatchLine` rebuilds `name` + CCommand ArgS (argc/argv0size). Insane blobs fall back to the verb so `cssvr_start` still fires. Copied rebuilt hook into CSS `bin/linux64`.
- Dual paint stays HMD-gated. Did not invent print/help/menu/toast/cfg chrome.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.

## Tests

`cssvrmod_tests` — 126 passed, 0 failed (989 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — start CSS from the menu so `8d34b00`+Dispatch can log `icvar ver=` / `register cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
