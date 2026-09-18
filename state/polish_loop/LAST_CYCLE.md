# Cycle 47 — 2026-09-18

## Focus

`vk-live-gl-copy-not-capture` — CopyEye counted a GL blit as a world capture on the Vulkan path.

## Did

- `DualCapture_Accept` — when a VK device is live, only `VkCaptureEye` counts.
- CopyEye returns false on GL fallback if `VkEye_DeviceLive()`.
- GL-only launch still uses the blit. No new toast.

## Did not

- HMD walk (user-gated). Dual paint still unproven in-game.
- Menu / toast / print / help / cfg chrome. No force `-noborder`.

## Tests

`cssvrmod_tests` — 99 passed, 0 failed (778 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — close with no commit unless a real offline bug appears. Never queue `dual-renderview-ipd-origin`.
