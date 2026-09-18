# Cycle 78 — 2026-09-19

## Focus

`cbuf-steal-safe` — do not 12-byte inline-hook CSS `Cbuf_AddText` (RIP-relative lea).

## Did

- Recovered cycle 77 idle. Tip was `cd799b7` (Cbuf hook from a parallel cycle). Live CSS still pid `2221372` (19:20, ~5h). No hook in maps.
- Slot 106 thunk is `mov rdi,rsi; jmp 0x4ce720`. That target starts `push rbp; lea rcx,[rip+disp]; mov rbp,rsp; push r14`. Stealing 12 bytes splits the push and relocates the lea — every Cbuf_AddText would crash.
- `EngineCmd_CbufStealOk` allowlists relocatable insns only. `HookCbufAddText` refuses the live CSS prologue. Copied rebuilt hook into CSS `bin/linux64`.
- Dual paint / RT copy stay HMD-gated. Did not invent print/help/menu/toast/cfg chrome. Did not restart CSS.

## Did not

- HMD walk. Stereo unproven.
- Queue `dual-renderview-ipd-origin`.
- A relocating Cbuf trampoline (typed `cssvr_start` still Unknown until then).

## Tests

`cssvrmod_tests` — 121 passed, 0 failed (962 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`idle-no-shell-ladder` — close with no commit unless a real offline bug appears. Restart CSS from the menu (not mid-map) so autoexec/`addons/*.vdf` can load. Dual paint stays HMD-gated.
