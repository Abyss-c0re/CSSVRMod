# Cycle 227 — 2026-09-20

## Focus

`dual-latch-drops-on-stop` — last dual capture must not skip MONO after cssvr_stop.

## Did

- Recovered cycle 226 tip `94d7b29` (`6d1d57f` on origin). CSS is not running. SteamVR is not running.
- Dual latch (`g_have_eyes` / `g_note_dual`) survived `cssvr_stop` / warmup, so Banner skipped MONO on the next session_ok until two new paints. `DualPaint_Latch` drops it when ShouldRun is false; ViewHookOnSwap and shutdown clear it. Offline 179/179.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

`cssvrmod_tests` 179 passed, 0 failed

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
