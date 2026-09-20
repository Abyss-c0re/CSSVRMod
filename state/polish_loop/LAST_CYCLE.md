# Cycle 230 — 2026-09-20

## Focus

`title-mono-on-stop` — window title must not stay CSS after cssvr_stop.

## Did

- Recovered cycle 229 tip `0843ce0` (`1f042d3` on origin). CSS is not running. SteamVR is not running.
- Dual latch dropped (cycle 227) but HarvestCopy returned before the MONO stamp when XR was off, so the title stayed `CSS`. `Banner_ChromeLabel` + ViewHookOnSwap now set MONO. Offline 181/181.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

`cssvrmod_tests` 181 passed, 0 failed

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
