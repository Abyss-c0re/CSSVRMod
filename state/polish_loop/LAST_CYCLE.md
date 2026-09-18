# Cycle 82 — 2026-09-19

## Focus

`vulkan-shim-like-sdl` — DXVK `dlopen`s `libvulkan.so.1`; GOT scan of the loader export misses GIPA present.

## Did

- Recovered cycle 81 tip `acc59cc`. CSS is not running (2370093 gone). Late-attach never logged — that process had the pre-81 hook.
- DXVK has no undefined `vk*` (runtime `dlopen` + GIPA). `bin/linux64` is first on `LD_LIBRARY_PATH`. Install now writes `libvulkan.css1` (renamed host loader) + a 14-byte-soname shim `libvulkan.so.1` that NEEDs the hook then the real loader. `VulkanSym` opens `libvulkan.css1` first (no recurse). Never copy the hook as vulkan on cc fail.
- Built shim into CSS `bin/linux64`. Dual paint stays HMD-gated. Did not invent print/help/menu/toast/cfg chrome.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.

## Tests

`cssvrmod_tests` — 125 passed, 0 failed (981 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — start CSS from the menu so the vulkan shim can interpose GIPA. Look for `vk: device ok` / `cmd wrap` / `icvar register`. Dual paint stays HMD-gated.
