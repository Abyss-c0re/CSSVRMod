# Cycle 124 — 2026-09-19

## Focus

`clientcmd-release-on-stop` — overlay drop left `+attack`/`+duck` held after `cssvr_stop`.

## Did

- Recovered cycle 123 tip `338bf46`. CSS is not running. Log still has no `icvar ver=`.
- Present path releases ClientCmd edges when XR is off. Prev overlay cleared.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

148 passed / 0 failed (1090 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
