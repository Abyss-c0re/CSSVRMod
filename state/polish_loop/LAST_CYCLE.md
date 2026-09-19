# Cycle 97 — 2026-09-19

## Focus

`icvar-retry-after-wrap` — splash cmd-wrap must not freeze ICvar register.

## Did

- `EngineProbe_PresentDone(wrap, icvar)` — present keeps `ProbeLiveEngine` until `cssvr_*` is on CCvar.
- Register the static ConCommands once; retry FindCommand only. Quiet the per-present miss log.
- Offline 129/129. CSS still not running — `9196a74` never mapped; no `icvar ver=` yet.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

`cssvrmod_tests` 129 passed, 0 failed.

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
