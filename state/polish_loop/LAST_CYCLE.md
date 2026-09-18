# Cycle 58 — 2026-09-18

## Focus

`cmd-wrap-on-vk-present` — Vulkan present waited on RenderView locate before probing the engine, so `cssvr_start` could never attach.

## Did

- `WrapPresent` probes the engine until the console wrap is ready (does not wait on `CViewRender`).
- Do not freeze the wrap plan until slot 106 is on, or it is present and not `engine.so`.

## Did not

- HMD walk. Dual paint still unproven in-game.
- Menu / toast / print / help / cfg chrome. No force `-noborder`.
- Did not queue `dual-renderview-ipd-origin`.

## Tests

`cssvrmod_tests` — 114 passed, 0 failed (908 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — close with no commit unless a real offline bug appears. Live CSS is still the old unhooked Steam process. Never queue `dual-renderview-ipd-origin`.
