# Cycle 246 — 2026-09-20

## Focus

`copy-drop-on-session-loss` — in-flight swapchain harvest on STOPPING/LOSS, not only mailbox.

## Did

- Recovered cycle 245 tip `32565a6` (product `04ad0b0`). CSS is not running. SteamVR is not running. WiVRn is up. Log still has no `icvar ver=` / `register cssvr_start=` / `renderview dual`.
- Hunt: mailbox leftover is dropped, but a GPU copy started last session (or ppm-only during STOPPING) still HarvestCopy→PushXrFrame after READY.
- `XrCopy_Take(keep, started_for_xr, epoch)`. Stamp `copy_for_xr` / `copy_epoch` when kicking the copy.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

offline 186/186 (1285/1285 asserts). Not HMD-proven.

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. Dual paint stays HMD-gated.
