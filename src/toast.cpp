#include "cssvrmod/toast.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>

namespace cssvr {

bool Toast_FireDesktop(const char* copy) {
  if (!copy || !copy[0]) return false;
  const char* path = std::getenv("CSSVR_TOAST_PATH");
  if (!path || !path[0]) path = "/tmp/cssvrmod.toast";
  if (FILE* t = std::fopen(path, "w")) {
    std::fprintf(t, "%s\n", copy);
    std::fclose(t);
  }
  if (FILE* log = std::fopen("/tmp/cssvrmod.log", "a")) {
    std::fprintf(log, "cssvr toast: %s\n", copy);
    std::fclose(log);
  }
  const char* skip = std::getenv("CSSVR_TOAST");
  if (skip && skip[0] == '0' && skip[1] == 0) return true;

  pid_t pid = fork();
  if (pid == 0) {
    pid_t g = fork();
    if (g != 0) _exit(0);
    setsid();
    execlp("notify-send", "notify-send", "-u", "critical", "-t", "10000", "-a", "CSSVRMod",
           "CSSVRMod", copy, static_cast<char*>(nullptr));
    _exit(127);
  }
  if (pid > 0) waitpid(pid, nullptr, 0);
  return true;
}

} // namespace cssvr
