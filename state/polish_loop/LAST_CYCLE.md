# Cycle 5 — 2026-09-18

## Focus

`vulkan-eye-capture` — live CSS is `-vulkan`; GL `CopyEye` cannot see that present.

## Did

- `vk_eye.hpp`: two distinct CPU frames required; same buffer is not ready.
- After each `RenderView`, copy last color RT (BeginRenderPass ≥640) or acquired swap image.
- Fence wait is on the **render** thread between eyes (8 ms). Present path still never `vkQueueWaitIdle`.
- Dual mailbox → `XrHostSubmitEyePixels`. Pose IPD still gated on two successful copies.
- GL copy is fallback only.

## Did not

- Claim stereo or HMD from offline green.
- Prove the RT layout (`COLOR_ATTACHMENT`) in a running CSS session.

## Tests

`cssvrmod_tests` — 41 passed, 0 failed (327 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`hmd-drives-viewangles` — look around is VR. In-game dual-paint walk still open.
