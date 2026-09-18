# Cycle 72 — 2026-09-18

## Focus

`steam-launch-hook-basename` — `SteamLaunchHasHook` used the full path. `.steam/steam` vs `.local/share/Steam` (same file) would prepend a second `LD_PRELOAD` on re-install.

## Did

- Live CSS `2221372` still predates autoexec (19:20). No cssvrmod in maps.
- Match LaunchOptions on hook basename so symlink spellings do not stack `LD_PRELOAD`.
- Did not restart or inject CSS.

## Did not

- HMD walk. Stereo unproven. Offline green ≠ hook-in-this-process.
- Queue `dual-renderview-ipd-origin`.
- Print / help / menu / toast / cfg chrome.

## Tests

`cssvrmod_tests` — 118 passed, 0 failed (945 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — restart CSS from the menu (not mid-map) so autoexec/`addons/*.vdf` can load the hook. Dual paint stays HMD-gated.
