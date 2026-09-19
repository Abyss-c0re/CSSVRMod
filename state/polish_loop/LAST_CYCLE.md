# Cycle 110 — 2026-09-19

## Focus

`vk-clientcmd-edges-use-melee` — VK present dropped +use / +attack2 / +showscores that GL already edged.

## Did

- Recovered cycle 109 tip `725fc45`. CSS not running. Log has no `icvar ver=` (a splash at 07:04 died before cmd wrap).
- Shared `ClientCmd_ApplyEdges` so VK matches GL: melee, use, score, plus analog only when CreateMove is down.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- Invent print/help/menu/toast/cfg chrome.

## Tests

134 passed / 0 failed (1034 asserts)

## Next

`idle-no-shell-ladder` — start CSS from the menu so ICvar can log `ver=` / `cssvr_start=`. With SteamVR on, type `cssvr_start`. Dual paint stays HMD-gated.
