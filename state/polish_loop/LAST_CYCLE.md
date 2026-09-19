# Cycle 122 — 2026-09-19

## Focus

`dual-only-when-session-ok` — hook IPD-offset the desktop with no HMD / after stop.

## Did

- Recovered cycle 121 tip `09c43bf`. CSS is not running. Log still has no `icvar ver=`.
- Dual paint + HMD look only when XR wanted and `session_ok`. Stale HMD pose ignored when not running.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

146 passed / 0 failed (1081 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
