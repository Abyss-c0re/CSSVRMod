# Cycle 69 — 2026-09-18

## Focus

`xr-loader-pv-jsoncpp` — host `libopenxr_loader.so.1` NEEDs `libjsoncpp.so.27` with no RPATH. Steam PV has jsoncpp only under `/run/host`, not cache/overrides, so `dlopen` of the loader still fails after cycle 67.

## Did

- Live CSS `2221372` still predates autoexec (19:20). No cssvrmod in maps.
- Preload jsoncpp (soname, `/run/host`, sibling of the loader path) `RTLD_GLOBAL` before each loader `dlopen`.
- Copied rebuilt hook into CSS `bin/linux64/`. Did not restart or inject.

## Did not

- HMD walk. Stereo unproven. Offline green ≠ hook-in-this-process / XR session.
- Queue `dual-renderview-ipd-origin`.
- Print / help / menu / toast / cfg chrome.

## Tests

`cssvrmod_tests` — 118 passed, 0 failed (943 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — restart CSS from the menu (not mid-map) so autoexec/`addons/*.vdf` can load the hook. Dual paint stays HMD-gated.
