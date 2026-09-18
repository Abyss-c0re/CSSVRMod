# Cycle 87 — 2026-09-19

## Focus

`icvar-vstdlib-factory` — CSS ICvar is CCvar in libvstdlib.so.

## Did

- Offline probe: `engine.so` CreateInterface(`VEngineCvar004`) is null (it has `VCvarQuery001`). `libvstdlib.so` returns CCvar.
- Probe ICvar from vstdlib after the engine factory. FindCommand/Register may live in libvstdlib.so — `SlotInEngine` would skip them.
- Offline 128/128.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

`cssvrmod_tests` 128 passed (1004/1004)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
