#pragma once
// Client-console verbs after a normal CSS launch (hook preloaded or plugin_load).
#include <cstring>

namespace cssvr {

enum class CssvrCmd { None, Start, Stop, Toggle, Menu, Status, Set, Help };

struct CssvrConsole {
  CssvrCmd cmd = CssvrCmd::None;
  char key[32]{};
  char val[64]{};
};

inline bool CssvrIsVerb(const char* s) {
  if (!s || !s[0]) return false;
  return std::strncmp(s, "cssvr", 5) == 0;
}

inline bool CssvrParseConsole(const char* line, CssvrConsole* out) {
  if (!out) return false;
  *out = {};
  if (!line) return false;
  while (*line == ' ' || *line == '\t') ++line;
  if (!CssvrIsVerb(line)) return false;
  char tok[8][64]{};
  int nt = 0, ti = 0;
  for (const char* p = line; *p && nt < 8; ++p) {
    if (*p == ' ' || *p == '\t') {
      if (ti) {
        tok[nt++][ti] = 0;
        ti = 0;
      }
      continue;
    }
    if (ti < 63) tok[nt][ti++] = *p;
  }
  if (ti && nt < 8) {
    tok[nt][ti] = 0;
    nt++;
  }
  if (nt < 1) return false;
  const char* a0 = tok[0];
  const char* a1 = nt > 1 ? tok[1] : "";
  const char* a2 = nt > 2 ? tok[2] : "";
  if (std::strcmp(a0, "cssvr_start") == 0 || (std::strcmp(a0, "cssvr") == 0 && std::strcmp(a1, "start") == 0))
    out->cmd = CssvrCmd::Start;
  else if (std::strcmp(a0, "cssvr_stop") == 0 || (std::strcmp(a0, "cssvr") == 0 && std::strcmp(a1, "stop") == 0))
    out->cmd = CssvrCmd::Stop;
  else if (std::strcmp(a0, "cssvr_toggle") == 0 ||
           (std::strcmp(a0, "cssvr") == 0 && std::strcmp(a1, "toggle") == 0))
    out->cmd = CssvrCmd::Toggle;
  else if (std::strcmp(a0, "cssvr_menu") == 0 || (std::strcmp(a0, "cssvr") == 0 && std::strcmp(a1, "menu") == 0))
    out->cmd = CssvrCmd::Menu;
  else if (std::strcmp(a0, "cssvr_help") == 0 || (std::strcmp(a0, "cssvr") == 0 && std::strcmp(a1, "help") == 0))
    out->cmd = CssvrCmd::Help;
  else if (std::strcmp(a0, "cssvr_set") == 0 || (std::strcmp(a0, "cssvr") == 0 && std::strcmp(a1, "set") == 0)) {
    out->cmd = CssvrCmd::Set;
    const char* k = std::strcmp(a0, "cssvr_set") == 0 ? a1 : a2;
    const char* v = std::strcmp(a0, "cssvr_set") == 0 ? a2 : (nt > 3 ? tok[3] : "");
    std::strncpy(out->key, k, 31);
    std::strncpy(out->val, v, 63);
  } else if (std::strcmp(a0, "cssvr") == 0 && (!a1[0] || std::strcmp(a1, "status") == 0))
    out->cmd = CssvrCmd::Status;
  else
    out->cmd = CssvrCmd::Help;
  return out->cmd != CssvrCmd::None;
}

} // namespace cssvr
