# Cycle 4 — 2026-09-18

## Focus

`hook-cviewrender-dual-paint` — locate CSS `CViewRender::RenderView` and paint left then right.

## Did

- Locator self-test: string `CViewRender::RenderView` → prologue → `movss [r13+0x40]` origin.
- CSS 64-bit `CViewSetup`: origin +0x40, angles +0x4c, fov +0x38 (from the copy in RenderView).
- Vtable slots (writable PT_LOAD) swapped only when they still point at that function.
- `DualPaint_Run`: `painted_dual` only after two paints **and** two framebuffer captures.
- Submit pose IPD stays 0 unless two distinct eye textures exist (heresy gate).
- Offline locator ran against live `client.so` on this machine.

## Did not

- Claim stereo or HMD from offline green.
- Prove the vtable patch in a running CSS session.

## Tests

`cssvrmod_tests` — 38 passed, 0 failed (313 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`vulkan-eye-capture` — live CSS is `-vulkan`; GL `CopyEye` cannot see that present. Dual paint stays gated until VK (or a real GL path) captures each eye.
