# Cycle 280 — 2026-09-20

## Focus

`idle-no-shell-ladder` — no product commit unless a real offline bug appears.

## Did

- Recovered cycle 279 record `99e94c4` (product `18be126`). CSS is not running. SteamVR is not running. WiVRn is still down. Log unchanged since splash/late-attach; still no `icvar ver=` / `register cssvr_start=` / `renderview dual`.
- Hunt: ICvar probe keeps going until cmds (splash wrap ≠ register). Tick / overlay / mailbox / copy / HMD / grip / session pairing already drop. pose_x stays 0. Remaining gaps HMD-gated.
- No code. Dual paint stays HMD-gated.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

no rebuild (no code). Last offline 189/189 (1360/1360 asserts). Not HMD-proven.

## Next

`idle-no-shell-ladder` — start OpenXR (WiVRn/SteamVR) first, then CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. Dual paint stays HMD-gated.
