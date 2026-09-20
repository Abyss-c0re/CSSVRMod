# Cycle 238 — 2026-09-20

## Focus

`overlay-drop-on-session-loss` — last stick/buttons must not keep walking on STOPPING/LOSS.

## Did

- Recovered cycle 237 tip `0d21246` (`b55594b` on origin). CSS is not running. SteamVR is not running. WiVRn is up. Log still has no `icvar ver=` / `register cssvr_start=` / `renderview dual`.
- Hunt found: CreateMove Peek gated WantXr only, so last stick/buttons kept walking while the headset was off. Present now `UserCmd_NoteSessionOk(run)` and clears overlay unless session_ok. Offline 184/184.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

`cssvrmod_tests` 184 passed, 0 failed

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With OpenXR (WiVRn is already up), type `cssvr_start`. Dual paint stays HMD-gated.
