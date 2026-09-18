# Agent notes — CSSVRMod

Repo name is **`CSSVRMod`** (that letter case). Sibling of `Abyss-c0re/gVRMod`.

## Version control

After meaningful changes: `git status` / `git diff` / commit / `git push origin` on the current branch. No force-push of shared history.

## Test before push

```bash
cmake -S . -B build
cmake --build build --target cssvrmod_tests CSSVR cssvrmod_hook -j"$(nproc)"
./build/cssvrmod_tests
```

## Cube launcher

Product start is CubeUI (gVRMod) → New Game → **CSSVRMod** → Start Game. That execs `scripts/CSSVR.sh` / `install/CSSVR`.

## 13-minute polish loop

Durable brief: `state/polish_loop/AGENT_PROMPT.md`. Cadence: one theme vs gVRMod Cube standard, tests, meaningful commit. Never claim stereo from a mono present.
