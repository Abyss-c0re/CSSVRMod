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

Product start is CubeUI (gVRMod) → New Game → **CSSVRMod** → Start Game. That execs `scripts/CSSVR.sh` / `install/CSSVR`. Do not add a second desktop entry.
