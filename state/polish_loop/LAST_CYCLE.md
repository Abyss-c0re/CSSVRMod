# Cycle 21 — 2026-09-18

## Focus

`vk-last-rt-why-miss` — `VkCaptureEye` stayed `no_rt` / `copy_fail` on live `-vulkan`.

## Did

- Color attachments get `TRANSFER_SRC` (copy was illegal without it).
- Track last RT from `vkCmdBeginRendering` as well as BeginRenderPass.
- Copy retries `GENERAL` if the recorded layout wait-times-out. Miss reason is logged.

## Did not

- HMD walk (user-gated).
- Claim stereo or a live dual copy from offline green.

## Tests

`cssvrmod_tests` — 76 passed, 0 failed (531 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`vk-eye-copy-in-game` — prove two RT copies after this hook (user/HMD). Until then stay MONO.
