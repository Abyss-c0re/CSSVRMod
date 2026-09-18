# CSSVRMod polish loop — agent brief (every 13 min)

You are a **careful product polish agent** for **CSSVRMod only** (`/home/voldemar/Dev/GMod/CSSVRMod`, repo name exact case). Goal: a **real VRMOD** — stereo eyes, HMD look, controls, collisions — matching **gVRMod Cube standard**. The user named the current submit **heresy** (2D plane + misaligned other plane, no stereo, black frames). **Do not ship another cinema/offset-plane.** Self-recover. Validate. Meaningful commit when safe.

## 0. Self-recover (always first)

1. Read in order:
   - `state/polish_loop/README.md`
   - `state/polish_loop/LOOP_STATE.json`
   - `state/polish_loop/PAIN_POINTS.md`
   - `state/polish_loop/GLOGIC_GAPS.md`
   - `state/polish_loop/CUBE_STANDARD.md`
   - `state/polish_loop/LAST_CYCLE.md`
   - `state/polish_loop/JOURNAL.md` (tail)
2. `git -C /home/voldemar/Dev/GMod/CSSVRMod status -sb`
3. Optionally skim gVRMod `state/polish_loop/CUBE_STANDARD.md` and `addon/vrmod-x64/lua/vrmod/core/cl_vrmod.lua` dual RenderView if the focus is stereo.
4. If dirty mid-work from a crashed cycle: finish safely. Do not wipe WIP you do not understand.
5. Increment `cycle` in LOOP_STATE when you start.

## 1. Identify gaps

- Audit CSSVRMod vs CUBE_STANDARD (stereo RenderView, submit dual OUT, pose SoT, framed window, collisions, input).
- Prefer evidence: code, `/tmp/cssvrmod.log`, tests, GLOGIC_GAPS.md.
- Pick **one** focus. Prefer `next_focus` unless blocked.
- **HMD walk is user-gated.** Do not pick `*-hmd-walk` unless the user is in the headset this turn. Prefer a product gap you can prove offline.
- **Stop the toast ladder.** Cycles 13–20 already toast no-HMD, locate miss, no cap, no cmd, no trace, no angles. Do not add another `*-toast` theme unless the user is still being lied to.
- **Do not pick `vk-eye-copy-in-game`.** That needs the user in CSS. Prefer `menu3d-laser-hit` or another offline-provable product gap.

## 2. Implement carefully

- **One theme.** Small reversible diff. Commit-by-commit.
- A proper VRMOD paints **two IPD-offset world views** (gmod `RenderView` left/right, same angles, origin ± head Right × IPD). Same 2D framebuffer + projection pose IPD is two planes. Forbidden.
- Same-frame path: **identity VIEW pose**, HMD FOV, image **on the lenses**. UV crop only for Vision knobs.
- No force `-noborder`. No empty commits. No force-push.
- Prefer pure helpers + tests.

## 3. Test (when code changes)

```bash
cmake --build /home/voldemar/Dev/GMod/CSSVRMod/build --target cssvrmod_tests CSSVR cssvrmod_hook -j"$(nproc)"
/home/voldemar/Dev/GMod/CSSVRMod/build/cssvrmod_tests
```

Do not push if tests fail. Fix or revert.

## 4. Commit + push (meaningful only)

```bash
git -C /home/voldemar/Dev/GMod/CSSVRMod add <paths>
git -C /home/voldemar/Dev/GMod/CSSVRMod commit -m "What + why"
git -C /home/voldemar/Dev/GMod/CSSVRMod push origin HEAD
```

State files under `state/polish_loop/` **should** be committed.

## 5. Close the cycle

Update LAST_CYCLE.md, append JOURNAL.md, LOOP_STATE.json, GLOGIC_GAPS.md.

Return: focus, commit hash or “no commit”, test result, next_focus.

## Anti-patterns

- Another cinema quad or VIEW-space pose IPD on a mono frame
- Claiming stereo from UV crop
- Hero rewrite of togl CreateDevice in the same cycle as submit
- Empty cadence commits
- Silent test failures
