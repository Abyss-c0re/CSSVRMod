#!/usr/bin/env bash
# CSSVRMod host — find CSS, LD_PRELOAD hook, keep Steam runtime out of the host.
# Also invoked by CubeUI (gVRMod launcher) when category CSSVRMod is started.
set +e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="${CSSVR_BIN:-$ROOT/install/CSSVR}"
HOOK="${CSSVR_HOOK:-$ROOT/install/libcssvrmod_hook.so}"
export CSSVR_ROOT="$ROOT"
export CSSVR_HOOK="$HOOK"

unset LD_LIBRARY_PATH
unset STEAM_RUNTIME
unset STEAM_RUNTIME_LIBRARY_PATH
export DISPLAY="${DISPLAY:-:0}"
export XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-/run/user/$(id -u)}"
export SDL_VIDEODRIVER="${SDL_VIDEODRIVER:-x11}"
# Primary hand before CSSVR/hook so first Tick is not right-only.
if [[ -z "${CSSVR_LEFT_HANDED:-}" ]]; then
  launch="${CSSVR_LAUNCH:-${HOME}/.config/gvrmod/cssvr_launch.cfg}"
  if [[ -r "$launch" ]]; then
    v=$(awk '$1=="left_handed"||$1=="lefthanded"{print $2; exit}' "$launch" 2>/dev/null)
    if [[ -n "$v" ]]; then export CSSVR_LEFT_HANDED="$v"; fi
  fi
fi
# engine.so NEEDED libcurl-gnutls.so.4 — not on Arch. CSSVR also searches this.
if [[ -z "${CSSVR_LIBDIR:-}" ]]; then
  for d in "$ROOT/.scratch/fakelibs" \
           "${HOME}/Dev/GMod/gVRMod/.scratch/cssvrmod/fakelibs"; do
    if [[ -e "$d/libcurl-gnutls.so.4" ]]; then export CSSVR_LIBDIR="$d"; break; fi
  done
fi
if [[ -z "${XR_RUNTIME_JSON:-}" ]]; then
  for c in /usr/share/openxr/1/openxr_wivrn.json /usr/local/share/openxr/1/openxr_wivrn.json \
           /usr/share/openxr/1/openxr_monado.json; do
    if [[ -r "$c" ]]; then export XR_RUNTIME_JSON="$c"; break; fi
  done
fi

if [[ ! -x "$BIN" ]]; then
  echo "CSSVRMod: missing $BIN — cmake --build build --target CSSVR" >&2
  exit 1
fi
exec "$BIN" "$@"
