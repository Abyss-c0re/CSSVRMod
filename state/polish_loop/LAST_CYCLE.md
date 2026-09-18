# Cycle 81 — 2026-09-19

## Focus

`sdl-late-attach-present` — SDL2-mapped hook must late-attach present + ICvar (not LD_PRELOAD).

## Did

- Recovered cycle 80 idle. Live CSS is pid `2370093` (started 00:45). **Hook is in maps** via `bin/linux64/libSDL2-2.0.so.0` → `libcssvrmod_hook.so`. Window persist 1280×720. shaderapivk + dxvk + libvulkan mapped.
- 11+ min, no `vk: device`, no `cmd wrap`, no `icvar register`. DXVK `dlopen`s libvulkan; hook exports are not global. Plugin `CssvrLateAttach` never runs on the client.
- `SDL_PollEvent` one-shots `CssvrLateAttach` after the first window (present GOT patch + ProbeLiveEngine / ICvar). Copied rebuilt hook into CSS `bin/linux64`.
- Dual paint stays HMD-gated. Did not invent print/help/menu/toast/cfg chrome.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- A libvulkan.so.1 shim (next if GOT patch misses after restart).

## Tests

`cssvrmod_tests` — 124 passed, 0 failed (973 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — restart CSS from the menu so the new hook can late-attach. Dual paint stays HMD-gated.
