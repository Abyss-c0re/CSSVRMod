# Cycle 67 — 2026-09-18

## Focus

`hook-delay-openxr` — Steam pressure-vessel cannot resolve `libopenxr_loader.so.1`, so LD_PRELOAD / plugin_load never mapped the hook.

## Did

- Live CSS `2221372` still predates autoexec (19:20). LaunchOptions/plugin/VDF/autoexec are on disk. Hook NEEDED `libopenxr_loader.so.1`; PV overrides have the runtime under `openxr/`, not the Khronos loader. Loader exists only at `/run/host/usr/lib`.
- Stopped linking `openxr_loader` into `cssvrmod_hook` (already `XR_NO_PROTOTYPES` + `dlopen`). Search sonames, then `/run/host/usr/lib{,64}` and `/usr/lib{,64}`.
- Copied rebuilt hook into CSS `bin/linux64/`. Did not restart or inject the live process.

## Did not

- HMD walk. Stereo unproven. Offline green ≠ hook-in-this-process.
- Queue `dual-renderview-ipd-origin`.
- Print / help / menu / toast / cfg chrome.

## Tests

`cssvrmod_tests` — 117 passed, 0 failed (934 asserts). Built `CSSVR` + `cssvrmod_hook`. `readelf` NEEDED has no `libopenxr_loader`.

## Next

`idle-no-shell-ladder` — restart CSS from the menu (not mid-map) so autoexec/`addons/*.vdf` can load the hook. Dual paint stays HMD-gated.
