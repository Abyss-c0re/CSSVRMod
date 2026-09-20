// CSS 64-bit defaults to shaderapivk / DXVK. GL swap never runs.
// Intercept vkQueuePresentKHR and dump the presented swapchain image.
#include "xr_host.hpp"
#include "cssvrmod/banner.hpp"
#include "cssvrmod/cssvr_ctl.hpp"
#include "cssvrmod/collision.hpp"
#include "cssvrmod/dual_paint.hpp"
#include "cssvrmod/hook_api.hpp"
#include "cssvrmod/launch.hpp"
#include "cssvrmod/input.hpp"
#include "cssvrmod/source_if.hpp"
#include "cssvrmod/tick.hpp"
#include "cssvrmod/usercmd.hpp"
#include "cssvrmod/view_hook.hpp"
#include "cssvrmod/vk_eye.hpp"
#include "cssvrmod/xr_session.hpp"
#include "cssvrmod/weapons.hpp"
#include <vulkan/vulkan.h>

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <fstream>
#include <sys/mman.h>
#include <unistd.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <pthread.h>
#include <string>
#include <sys/stat.h>
#include <unordered_map>
#include <vector>

static bool (*g_capture_eye_fn)(int) = nullptr;
static bool (*g_take_pair_fn)(cssvr::VkEyePair*) = nullptr;

namespace cssvr {
bool VkCaptureEye(int eye) { return g_capture_eye_fn ? g_capture_eye_fn(eye) : false; }
bool VkEye_TakePair(VkEyePair* out) { return g_take_pair_fn ? g_take_pair_fn(out) : false; }
} // namespace cssvr

namespace {
using namespace cssvr;

void Log(const char* fmt, ...) {
  FILE* f = std::fopen("/tmp/cssvrmod.log", "a");
  if (!f) return;
  std::fputs("vk: ", f);
  va_list ap;
  va_start(ap, fmt);
  std::vfprintf(f, fmt, ap);
  va_end(ap);
  std::fputc('\n', f);
  std::fclose(f);
}

std::string DumpDir() {
  if (const char* e = std::getenv("CSSVR_DUMP_DIR")) return e;
  return "/home/voldemar/Dev/GMod/gVRMod/.scratch/cssvrmod";
}

void WritePpm(const std::string& path, int w, int h, const unsigned char* rgba) {
  FILE* f = std::fopen(path.c_str(), "wb");
  if (!f) return;
  std::fprintf(f, "P6\n%d %d\n255\n", w, h);
  std::vector<unsigned char> row((size_t)w * 3);
  for (int y = 0; y < h; ++y) {
    const unsigned char* src = rgba + (size_t)y * (size_t)w * 4;
    for (int x = 0; x < w; ++x) {
      row[(size_t)x * 3 + 0] = src[(size_t)x * 4 + 0];
      row[(size_t)x * 3 + 1] = src[(size_t)x * 4 + 1];
      row[(size_t)x * 3 + 2] = src[(size_t)x * 4 + 2];
    }
    std::fwrite(row.data(), 1, row.size(), f);
  }
  std::fclose(f);
}

struct DevFns {
  PFN_vkGetDeviceProcAddr gdpa = nullptr;
  PFN_vkGetDeviceQueue getQueue = nullptr;
  PFN_vkCreateSwapchainKHR createSc = nullptr;
  PFN_vkGetSwapchainImagesKHR getScImgs = nullptr;
  PFN_vkQueuePresentKHR present = nullptr;
  PFN_vkCreateCommandPool createPool = nullptr;
  PFN_vkDestroyCommandPool destroyPool = nullptr;
  PFN_vkAllocateCommandBuffers allocCmd = nullptr;
  PFN_vkBeginCommandBuffer beginCmd = nullptr;
  PFN_vkEndCommandBuffer endCmd = nullptr;
  PFN_vkCmdPipelineBarrier barrier = nullptr;
  PFN_vkCmdCopyImageToBuffer copy = nullptr;
  PFN_vkQueueSubmit submit = nullptr;
  PFN_vkQueueWaitIdle waitIdle = nullptr;
  PFN_vkCreateBuffer createBuf = nullptr;
  PFN_vkDestroyBuffer destroyBuf = nullptr;
  PFN_vkGetBufferMemoryRequirements bufReq = nullptr;
  PFN_vkAllocateMemory allocMem = nullptr;
  PFN_vkFreeMemory freeMem = nullptr;
  PFN_vkBindBufferMemory bindBuf = nullptr;
  PFN_vkMapMemory map = nullptr;
  PFN_vkUnmapMemory unmap = nullptr;
  PFN_vkCreateFence createFence = nullptr;
  PFN_vkDestroyFence destroyFence = nullptr;
  PFN_vkWaitForFences waitFences = nullptr;
  PFN_vkResetFences resetFences = nullptr;
  PFN_vkGetPhysicalDeviceMemoryProperties memProps = nullptr;
  PFN_vkCreateImage createImg = nullptr;
  PFN_vkCreateImageView createView = nullptr;
  PFN_vkCreateFramebuffer createFb = nullptr;
  PFN_vkCmdBeginRenderPass beginRp = nullptr;
  PFN_vkAcquireNextImageKHR acquire = nullptr;
};

struct SwapState {
  VkDevice device = VK_NULL_HANDLE;
  VkPhysicalDevice phys = VK_NULL_HANDLE;
  uint32_t w = 0, h = 0;
  VkFormat format = VK_FORMAT_UNDEFINED;
  std::vector<VkImage> images;
};

struct DeviceState {
  VkPhysicalDevice phys = VK_NULL_HANDLE;
  uint32_t gfx_family = 0;
  VkQueue queue = VK_NULL_HANDLE;
  DevFns fn;
  VkBuffer stage = VK_NULL_HANDLE;
  VkDeviceMemory stage_mem = VK_NULL_HANDLE;
  VkDeviceSize stage_bytes = 0;
  VkCommandPool pool = VK_NULL_HANDLE;
  VkCommandBuffer cmd = VK_NULL_HANDLE;
  VkFence fence = VK_NULL_HANDLE;
  bool copy_inflight = false;
  bool copy_for_xr = false;
  int copy_epoch = 0;
  uint32_t copy_w = 0, copy_h = 0;
  VkFormat copy_fmt = VK_FORMAT_UNDEFINED;
  VkBuffer eye_stage = VK_NULL_HANDLE;
  VkDeviceMemory eye_mem = VK_NULL_HANDLE;
  VkDeviceSize eye_bytes = 0;
  VkCommandBuffer eye_cmd = VK_NULL_HANDLE;
  VkFence eye_fence = VK_NULL_HANDLE;
};

std::mutex g_mu;
std::unordered_map<VkDevice, DeviceState> g_devs;
std::unordered_map<VkSwapchainKHR, SwapState> g_scs;
std::unordered_map<VkImage, std::pair<uint32_t, uint32_t>> g_img_sz;
std::unordered_map<VkImage, VkFormat> g_img_fmt;
std::unordered_map<VkImageView, VkImage> g_views;
struct FbColor {
  VkImage img = VK_NULL_HANDLE;
  uint32_t w = 0, h = 0;
  VkFormat fmt = VK_FORMAT_UNDEFINED;
  VkDevice dev = VK_NULL_HANDLE;
  VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
};
std::unordered_map<VkFramebuffer, FbColor> g_fbs;
std::unordered_map<VkImage, VkDevice> g_img_dev;
FbColor g_last_rt{};
VkSwapchainKHR g_cur_sc = VK_NULL_HANDLE;
uint32_t g_cur_idx = 0;
VkEyePair g_vk_eyes{};
int g_presents = 0;
int g_dumps = 0;
int g_xr_ok = 0;
int g_xr_fail = 0;
EngineIf g_eng{};
UserCmdOverlay g_prev_cmd{};

bool XrWanted() { return CssvrWantXr(); }

struct XrMailbox {
  std::mutex mu;
  std::condition_variable cv;
  std::vector<unsigned char> rgba;
  std::vector<unsigned char> rgba_r;
  int w = 0, h = 0;
  int epoch = 0;
  bool bgra = false;
  bool dual = false;
  bool have = false;
  bool stop = false;
};
XrMailbox g_mb;
std::atomic<bool> g_xr_thread{false};

void PushXrFrame(const unsigned char* rgba, int w, int h, bool bgra) {
  if (!rgba || w < 2 || h < 2) return;
  std::lock_guard<std::mutex> lk(g_mb.mu);
  // Latest-wins. Never queue — a backlog is what made the game hitch.
  g_mb.rgba.assign(rgba, rgba + (size_t)w * (size_t)h * 4);
  g_mb.rgba_r.clear();
  g_mb.w = w;
  g_mb.h = h;
  g_mb.bgra = bgra;
  g_mb.dual = false;
  g_mb.epoch = XrHostEpoch();
  g_mb.have = true;
  g_mb.cv.notify_one();
}

void PushXrDual(const VkEyePair& p) {
  if (!VkEye_Ready(p)) return;
  std::lock_guard<std::mutex> lk(g_mb.mu);
  g_mb.rgba = p.eye[0].px;
  g_mb.rgba_r = p.eye[1].px;
  g_mb.w = p.eye[0].w;
  g_mb.h = p.eye[0].h;
  g_mb.bgra = p.eye[0].bgra;
  g_mb.dual = true;
  g_mb.epoch = XrHostEpoch();
  g_mb.have = true;
  g_mb.cv.notify_one();
}

bool MailboxFull() {
  std::lock_guard<std::mutex> lk(g_mb.mu);
  return g_mb.have;
}

void DropVkEyes() {
  std::lock_guard<std::mutex> lk(g_mu);
  VkEye_Clear(&g_vk_eyes);
}

void DropMailbox() {
  std::lock_guard<std::mutex> lk(g_mb.mu);
  g_mb.have = false;
  g_mb.dual = false;
  g_mb.epoch = 0;
  g_mb.rgba.clear();
  g_mb.rgba_r.clear();
}

void* XrWorker(void*) {
  Log("xr worker start");
  TurnState turn;
  WallState leftWall, rightWall;
  HandVelState leftVel, rightVel;
  float nextMelee = 0.f;
  float now = 0.f;
  while (true) {
    std::vector<unsigned char> frame, frame_r;
    int w = 0, h = 0;
    bool bgra = false;
    bool dual = false;
    bool have = false;
    int epoch = 0;
    {
      std::unique_lock<std::mutex> lk(g_mb.mu);
      g_mb.cv.wait_for(lk, std::chrono::milliseconds(50),
                       [] { return g_mb.have || g_mb.stop; });
      if (g_mb.stop) break;
      have = g_mb.have;
      if (have) {
        frame.swap(g_mb.rgba);
        frame_r.swap(g_mb.rgba_r);
        w = g_mb.w;
        h = g_mb.h;
        bgra = g_mb.bgra;
        dual = g_mb.dual;
        epoch = g_mb.epoch;
        g_mb.have = false;
        g_mb.dual = false;
      }
    }
    // Skip leftover submit on STOPPING/LOSS, but still pump or READY is never seen.
    if (XrWorker_ShouldPump(XrWanted())) XrHostPumpEvents();
    const bool session_ok = XrSession_IsOkReason(XrHostStatus().reason);
    // Yaw / last-free / hand vel used to survive STOPPING while XR stayed wanted.
    Tick_DropState(Tick_KeepState(XrWanted(), session_ok), &turn, &leftWall, &rightWall,
                   &leftVel, &rightVel, &nextMelee, &now);
    const bool keep_mb = XrMailbox_Keep(XrWanted(), session_ok);
    if (!keep_mb) DropMailbox();
    if (!XrMailbox_Accept(keep_mb, epoch, XrHostEpoch())) {
      have = false;
      dual = false;
    }
    XrMailbox_Drop(keep_mb, &have, &dual);
    if (!have) continue;
    if (!XrWorker_ShouldSubmit(XrWanted(), session_ok)) continue;
    const bool ok = (dual && !frame_r.empty())
                        ? XrHostSubmitEyePixels(frame.data(), frame_r.data(), w, h, bgra, true)
                        : XrHostSubmitPixels(frame.data(), w, h, bgra);
    if (ok) {
      g_xr_ok++;
      if (g_xr_ok == 1 || (g_xr_ok % 300) == 0)
        Log("xr submit #%d %ux%u %s", g_xr_ok, w, h, XrHostStatus().reason);
      XrSample xr{};
      if (XrHostPollInput(&xr)) {
        TickIn tin;
        tin.xr = xr;
        tin.wep = nullptr; // no live weapon query yet — fist only on melee_intent
        tin.now = now;
        tin.dt = 0.011f;
        tin.input.left_handed = Input_LeftHandedLive();
        tin.current_view = xr.hmd.ang;
        tin.trace = EngineMakeTraceFn(g_eng);
        tin.hand_vel_left = &leftVel;
        tin.hand_vel_right = &rightVel;
        tin.turn = &turn;
        TickOut tout = Tick(tin, leftWall, rightWall, &nextMelee);
        now += 0.011f;
        UserCmdOverlay cmd = tout.cmd;
        UserCmd_NoteOverlay(cmd);
        Turn_NoteYawOff(turn.yaw_off);
        if (g_eng.screen_ok || ProbeLiveEngine(g_eng))
          ClientCmd_ApplyEdges(g_eng, cmd, g_prev_cmd, UserCmd_HookLive());
        g_prev_cmd = cmd;
      }
    } else if (XrSubmit_CountFail(XrHostStatus().reason, false, XrHostLastFrameSkipped())) {
      g_xr_fail++;
      if (g_xr_fail == 1 || (g_xr_fail % 120) == 0)
        Log("xr submit fail #%d %s", g_xr_fail, XrHostStatus().reason);
    }
  }
  Log("xr worker stop");
  return nullptr;
}

void EnsureXrWorker() {
  bool expected = false;
  if (!g_xr_thread.compare_exchange_strong(expected, true)) return;
  pthread_t th;
  if (pthread_create(&th, nullptr, XrWorker, nullptr) != 0) {
    g_xr_thread = false;
    Log("xr worker create failed");
    return;
  }
  pthread_detach(th);
}

PFN_vkGetInstanceProcAddr g_gipa = nullptr;
PFN_vkGetDeviceProcAddr g_gdpa = nullptr;
PFN_vkCreateDevice g_createDevice = nullptr;
PFN_vkQueuePresentKHR g_present = nullptr;
PFN_vkCreateSwapchainKHR g_createSc = nullptr;

void* VulkanSym(const char* name) {
  static void* lib = nullptr;
  if (!lib) {
    int n = 0;
    const char* const* c = cssvr::Vulkan_LoaderCandidates(&n);
    for (int i = 0; i < n && !lib; ++i)
      lib = dlopen(c[i], RTLD_NOW | RTLD_LOCAL);
  }
  void* p = lib ? dlsym(lib, name) : nullptr;
  if (!p) p = dlsym(RTLD_NEXT, name);
  return p;
}

uint32_t FindHostMem(VkPhysicalDevice phys, PFN_vkGetPhysicalDeviceMemoryProperties getProps,
                     uint32_t typeBits) {
  VkPhysicalDeviceMemoryProperties p{};
  getProps(phys, &p);
  for (uint32_t i = 0; i < p.memoryTypeCount; ++i) {
    if ((typeBits & (1u << i)) &&
        (p.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
        (p.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
      return i;
  }
  return UINT32_MAX;
}

void FillDevFns(VkDevice dev, DeviceState& ds) {
  auto gdpa = ds.fn.gdpa ? ds.fn.gdpa : g_gdpa;
  if (!gdpa) return;
#define L(name, field) ds.fn.field = (PFN_##name)gdpa(dev, #name)
  L(vkGetDeviceQueue, getQueue);
  L(vkCreateSwapchainKHR, createSc);
  L(vkGetSwapchainImagesKHR, getScImgs);
  L(vkQueuePresentKHR, present);
  L(vkCreateCommandPool, createPool);
  L(vkDestroyCommandPool, destroyPool);
  L(vkAllocateCommandBuffers, allocCmd);
  L(vkBeginCommandBuffer, beginCmd);
  L(vkEndCommandBuffer, endCmd);
  L(vkCmdPipelineBarrier, barrier);
  L(vkCmdCopyImageToBuffer, copy);
  L(vkQueueSubmit, submit);
  L(vkQueueWaitIdle, waitIdle);
  L(vkCreateBuffer, createBuf);
  L(vkDestroyBuffer, destroyBuf);
  L(vkGetBufferMemoryRequirements, bufReq);
  L(vkAllocateMemory, allocMem);
  L(vkFreeMemory, freeMem);
  L(vkBindBufferMemory, bindBuf);
  L(vkMapMemory, map);
  L(vkUnmapMemory, unmap);
  L(vkCreateFence, createFence);
  L(vkDestroyFence, destroyFence);
  L(vkWaitForFences, waitFences);
  L(vkResetFences, resetFences);
  L(vkCreateImage, createImg);
  L(vkCreateImageView, createView);
  L(vkCreateFramebuffer, createFb);
  L(vkCmdBeginRenderPass, beginRp);
  L(vkAcquireNextImageKHR, acquire);
#undef L
  if (!ds.fn.memProps && g_gipa) {
    // instance-level; try device first then instance
    ds.fn.memProps =
        (PFN_vkGetPhysicalDeviceMemoryProperties)g_gipa(VK_NULL_HANDLE,
                                                        "vkGetPhysicalDeviceMemoryProperties");
  }
}

bool EnsureStage(DeviceState* ds, VkDevice dev, VkDeviceSize bytes) {
  if (ds->stage && ds->stage_bytes >= bytes && ds->cmd && ds->fence) return true;
  if (!ds->fn.createFence) return false;
  if (ds->stage) { ds->fn.destroyBuf(dev, ds->stage, nullptr); ds->stage = VK_NULL_HANDLE; }
  if (ds->stage_mem) { ds->fn.freeMem(dev, ds->stage_mem, nullptr); ds->stage_mem = VK_NULL_HANDLE; }
  VkBufferCreateInfo bi{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  bi.size = bytes;
  bi.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  if (ds->fn.createBuf(dev, &bi, nullptr, &ds->stage) != VK_SUCCESS) return false;
  VkMemoryRequirements req{};
  ds->fn.bufReq(dev, ds->stage, &req);
  uint32_t memIdx = FindHostMem(ds->phys, ds->fn.memProps, req.memoryTypeBits);
  if (memIdx == UINT32_MAX) return false;
  VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  ai.allocationSize = req.size;
  ai.memoryTypeIndex = memIdx;
  if (ds->fn.allocMem(dev, &ai, nullptr, &ds->stage_mem) != VK_SUCCESS) return false;
  ds->fn.bindBuf(dev, ds->stage, ds->stage_mem, 0);
  ds->stage_bytes = bytes;
  if (!ds->pool) {
    VkCommandPoolCreateInfo pci{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pci.queueFamilyIndex = ds->gfx_family;
    pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (ds->fn.createPool(dev, &pci, nullptr, &ds->pool) != VK_SUCCESS) return false;
    VkCommandBufferAllocateInfo cai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    cai.commandPool = ds->pool;
    cai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cai.commandBufferCount = 1;
    ds->fn.allocCmd(dev, &cai, &ds->cmd);
  }
  if (!ds->fence) {
    VkFenceCreateInfo fi{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    ds->fn.createFence(dev, &fi, nullptr, &ds->fence);
  }
  return ds->stage && ds->cmd && ds->fence;
}

bool FormatIsBgra(VkFormat fmt) {
  return fmt == VK_FORMAT_B8G8R8A8_UNORM || fmt == VK_FORMAT_B8G8R8A8_SRGB;
}

void HarvestCopy(DeviceState* ds, VkDevice dev, bool want_ppm, bool painted_dual) {
  if (!ds->copy_inflight || !ds->fn.waitFences) return;
  if (ds->fn.waitFences(dev, 1, &ds->fence, VK_TRUE, 0) != VK_SUCCESS) return;
  ds->copy_inflight = false;
  ds->fn.resetFences(dev, 1, &ds->fence);
  const uint32_t w = ds->copy_w, h = ds->copy_h;
  if (w < 2 || h < 2 || !ds->stage_mem) return;
  const VkDeviceSize bytes = (VkDeviceSize)w * h * 4;
  const bool take_xr =
      XrCopy_Take(XrMailbox_Keep(XrWanted(), XrSession_IsOkReason(XrHostStatus().reason)),
                  ds->copy_for_xr, ds->copy_epoch, XrHostEpoch()) &&
      !MailboxFull();
  if (!take_xr && !want_ppm) return;
  void* mapped = nullptr;
  if (ds->fn.map(dev, ds->stage_mem, 0, bytes, 0, &mapped) != VK_SUCCESS || !mapped) return;
  const auto* srcp = static_cast<const unsigned char*>(mapped);
  const char* reason = XrHostStatus().reason;
  const bool bgra = FormatIsBgra(ds->copy_fmt);
  const BannerPlan plan = Banner_Decide(reason, painted_dual);
  std::vector<unsigned char> stamped;
  const unsigned char* xr_px = srcp;
  const unsigned char* desk_px = srcp;
  if (plan.should_stamp) {
    stamped.assign(srcp, srcp + (size_t)bytes);
    if (Banner_Stamp(stamped.data(), (int)w, (int)h, bgra, false, reason, painted_dual)) {
      desk_px = stamped.data();
      Chrome_NoteStatus(plan.text);
      if (plan.stamp_xr) xr_px = stamped.data();
    }
  } else if (painted_dual) {
    Chrome_NoteStatus("CSS");
  }
  if (take_xr) {
    EnsureXrWorker();
    PushXrFrame(xr_px, (int)w, (int)h, bgra);
  }
  if (want_ppm && g_dumps < 2) {
    std::vector<unsigned char> rgba((size_t)bytes);
    const unsigned char* src = desk_px;
    if (bgra) {
      for (size_t i = 0; i < (size_t)w * h; ++i) {
        rgba[i * 4 + 0] = src[i * 4 + 2];
        rgba[i * 4 + 1] = src[i * 4 + 1];
        rgba[i * 4 + 2] = src[i * 4 + 0];
        rgba[i * 4 + 3] = src[i * 4 + 3];
      }
    } else {
      std::memcpy(rgba.data(), src, (size_t)bytes);
    }
    mkdir(DumpDir().c_str(), 0755);
    char path[512];
    std::snprintf(path, sizeof(path), "%s/vk_%03d_%ux%u_scene.ppm", DumpDir().c_str(), g_dumps, w, h);
    WritePpm(path, (int)w, (int)h, rgba.data());
    g_dumps++;
    Log("dump %s", path);
  }
  ds->fn.unmap(dev, ds->stage_mem);
}

bool EnsureEyeStage(DeviceState* ds, VkDevice dev, VkDeviceSize bytes) {
  if (ds->eye_stage && ds->eye_bytes >= bytes && ds->eye_cmd && ds->eye_fence) return true;
  if (!ds->fn.createFence || !ds->fn.createBuf) return false;
  if (ds->eye_stage) {
    ds->fn.destroyBuf(dev, ds->eye_stage, nullptr);
    ds->eye_stage = VK_NULL_HANDLE;
  }
  if (ds->eye_mem) {
    ds->fn.freeMem(dev, ds->eye_mem, nullptr);
    ds->eye_mem = VK_NULL_HANDLE;
  }
  VkBufferCreateInfo bi{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  bi.size = bytes;
  bi.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  if (ds->fn.createBuf(dev, &bi, nullptr, &ds->eye_stage) != VK_SUCCESS) return false;
  VkMemoryRequirements req{};
  ds->fn.bufReq(dev, ds->eye_stage, &req);
  uint32_t memIdx = FindHostMem(ds->phys, ds->fn.memProps, req.memoryTypeBits);
  if (memIdx == UINT32_MAX) return false;
  VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  ai.allocationSize = req.size;
  ai.memoryTypeIndex = memIdx;
  if (ds->fn.allocMem(dev, &ai, nullptr, &ds->eye_mem) != VK_SUCCESS) return false;
  ds->fn.bindBuf(dev, ds->eye_stage, ds->eye_mem, 0);
  ds->eye_bytes = bytes;
  if (!ds->pool) {
    VkCommandPoolCreateInfo pci{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pci.queueFamilyIndex = ds->gfx_family;
    pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (ds->fn.createPool(dev, &pci, nullptr, &ds->pool) != VK_SUCCESS) return false;
  }
  if (!ds->eye_cmd) {
    VkCommandBufferAllocateInfo cai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    cai.commandPool = ds->pool;
    cai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cai.commandBufferCount = 1;
    ds->fn.allocCmd(dev, &cai, &ds->eye_cmd);
  }
  if (!ds->eye_fence) {
    VkFenceCreateInfo fi{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    ds->fn.createFence(dev, &fi, nullptr, &ds->eye_fence);
  }
  return ds->eye_stage && ds->eye_cmd && ds->eye_fence;
}

bool CopyImageToEyeLayout(int eye, VkDevice dev, DeviceState* ds, VkImage img, uint32_t w,
                          uint32_t h, VkFormat fmt, VkImageLayout src_layout) {
  if (!ds || !ds->queue || !img || w < 8 || h < 8) return false;
  if (src_layout == VK_IMAGE_LAYOUT_UNDEFINED || src_layout == VK_IMAGE_LAYOUT_PREINITIALIZED)
    src_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  const VkDeviceSize bytes = (VkDeviceSize)w * h * 4;
  if (!EnsureEyeStage(ds, dev, bytes)) return false;
  VkCommandBufferBeginInfo cbi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  cbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  if (ds->fn.beginCmd(ds->eye_cmd, &cbi) != VK_SUCCESS) return false;
  VkImageMemoryBarrier bar{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
  bar.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
  bar.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  bar.oldLayout = src_layout;
  bar.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  bar.image = img;
  bar.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  ds->fn.barrier(ds->eye_cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                 VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &bar);
  VkBufferImageCopy copy{};
  copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
  copy.imageExtent = {w, h, 1};
  ds->fn.copy(ds->eye_cmd, img, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, ds->eye_stage, 1, &copy);
  bar.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  bar.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
  bar.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  bar.newLayout = src_layout;
  ds->fn.barrier(ds->eye_cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1,
                 &bar);
  if (ds->fn.endCmd(ds->eye_cmd) != VK_SUCCESS) return false;
  ds->fn.resetFences(dev, 1, &ds->eye_fence);
  VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
  si.commandBufferCount = 1;
  si.pCommandBuffers = &ds->eye_cmd;
  if (ds->fn.submit(ds->queue, 1, &si, ds->eye_fence) != VK_SUCCESS) return false;
  // Render thread between eyes — must finish before the next paint overwrites the RT.
  // Not the present thread. Timeout keeps a miss honest (painted_dual stays false).
  if (ds->fn.waitFences(dev, 1, &ds->eye_fence, VK_TRUE, 8ull * 1000ull * 1000ull) != VK_SUCCESS)
    return false;
  void* mapped = nullptr;
  if (ds->fn.map(dev, ds->eye_mem, 0, bytes, 0, &mapped) != VK_SUCCESS || !mapped) return false;
  const bool ok =
      VkEye_Store(&g_vk_eyes, eye, static_cast<const unsigned char*>(mapped), (int)w, (int)h,
                  FormatIsBgra(fmt));
  ds->fn.unmap(dev, ds->eye_mem);
  return ok;
}

bool CopyImageToEye(int eye, VkDevice dev, DeviceState* ds, VkImage img, uint32_t w, uint32_t h,
                    VkFormat fmt, VkImageLayout src_layout) {
  // One submit. A GENERAL retry after an 8 ms wait timeout reuses an in-flight
  // command buffer and can hang the GPU.
  return CopyImageToEyeLayout(eye, dev, ds, img, w, h, fmt, src_layout);
}

} // namespace

namespace cssvr {
bool VkEye_DeviceLive() {
  std::lock_guard<std::mutex> lk(g_mu);
  return !g_devs.empty();
}
} // namespace cssvr

namespace {

bool CaptureEyeImpl(int eye) {
  FbColor rt{};
  VkDevice dev = VK_NULL_HANDLE;
  DeviceState* ds = nullptr;
  bool have_dev = false, have_queue = false;
  {
    std::lock_guard<std::mutex> lk(g_mu);
    rt = g_last_rt;
    // Do not fall back to the acquired swap image: that is PRESENT_SRC, not
    // COLOR_ATTACHMENT. A wrong layout here can hang the GPU.
    if (rt.dev) {
      auto dit = g_devs.find(rt.dev);
      if (dit != g_devs.end()) {
        have_dev = true;
        ds = &dit->second;
        dev = rt.dev;
        have_queue = ds->queue != VK_NULL_HANDLE;
      }
    }
  }
  const bool copied = (rt.img && have_dev && have_queue)
                          ? CopyImageToEye(eye, dev, ds, rt.img, rt.w, rt.h, rt.fmt, rt.layout)
                          : false;
  VkEyeMissIn miss;
  miss.have_img = rt.img != VK_NULL_HANDLE;
  miss.w = rt.w;
  miss.h = rt.h;
  miss.have_dev = have_dev;
  miss.have_queue = have_queue;
  miss.copied = copied;
  const char* why = VkEye_MissDecide(miss);
  static int n = 0;
  if (!copied && n++ < 6)
    Log("vk capture eye=%d miss %s img=%p %ux%u lay=%d", eye, why, (void*)rt.img, rt.w, rt.h,
        (int)rt.layout);
  return copied;
}

bool TakePairImpl(VkEyePair* out) {
  if (!out) return false;
  std::lock_guard<std::mutex> lk(g_mu);
  if (!VkEye_Ready(g_vk_eyes)) return false;
  *out = g_vk_eyes;
  return true;
}

struct CaptureReg {
  CaptureReg() {
    g_capture_eye_fn = CaptureEyeImpl;
    g_take_pair_fn = TakePairImpl;
  }
};
CaptureReg g_cap_reg;

void DumpSwapchain(VkQueue queue, VkSwapchainKHR sc, uint32_t idx, bool want_ppm,
                   bool painted_dual) {
  DeviceState* ds = nullptr;
  SwapState* ss = nullptr;
  {
    std::lock_guard<std::mutex> lk(g_mu);
    auto sit = g_scs.find(sc);
    if (sit == g_scs.end()) return;
    ss = &sit->second;
    auto dit = g_devs.find(ss->device);
    if (dit == g_devs.end()) return;
    ds = &dit->second;
  }
  if (!ds->fn.copy || idx >= ss->images.size() || ss->w == 0 || ss->h == 0 || !ds->fn.memProps) return;
  VkDevice dev = ss->device;
  HarvestCopy(ds, dev, want_ppm, painted_dual);
  const bool want_xr =
      XrMailbox_Keep(XrWanted(), XrSession_IsOkReason(XrHostStatus().reason)) && !MailboxFull();
  if (!want_xr && !want_ppm) return;
  if (ds->copy_inflight) return; // previous GPU copy still running — never stall present
  const VkDeviceSize bytes = (VkDeviceSize)ss->w * ss->h * 4;
  if (!EnsureStage(ds, dev, bytes)) return;
  VkCommandBufferBeginInfo cbi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  cbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  ds->fn.beginCmd(ds->cmd, &cbi);
  VkImageMemoryBarrier bar{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
  bar.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  bar.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  bar.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  bar.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  bar.image = ss->images[idx];
  bar.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  ds->fn.barrier(ds->cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                 nullptr, 0, nullptr, 1, &bar);
  VkBufferImageCopy copy{};
  copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
  copy.imageExtent = {ss->w, ss->h, 1};
  ds->fn.copy(ds->cmd, ss->images[idx], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, ds->stage, 1, &copy);
  bar.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  bar.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  bar.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  bar.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  ds->fn.barrier(ds->cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0,
                 nullptr, 0, nullptr, 1, &bar);
  ds->fn.endCmd(ds->cmd);
  VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
  si.commandBufferCount = 1;
  si.pCommandBuffers = &ds->cmd;
  if (ds->fn.submit(queue, 1, &si, ds->fence) != VK_SUCCESS) return;
  ds->copy_inflight = true;
  ds->copy_for_xr = want_xr;
  ds->copy_epoch = XrHostEpoch();
  ds->copy_w = ss->w;
  ds->copy_h = ss->h;
  ds->copy_fmt = ss->format;
}

VKAPI_ATTR VkResult VKAPI_CALL WrapPresent(VkQueue queue, const VkPresentInfoKHR* info) {
  PFN_vkQueuePresentKHR real = g_present;
  {
    std::lock_guard<std::mutex> lk(g_mu);
    g_presents++;
    if (!real && info && info->swapchainCount) {
      auto it = g_scs.find(info->pSwapchains[0]);
      if (it != g_scs.end()) {
        auto dit = g_devs.find(it->second.device);
        if (dit != g_devs.end()) real = dit->second.fn.present;
      }
    }
  }
  if (!real) return VK_ERROR_UNKNOWN;
  // Cmd wrap must not wait on RenderView locate — Steam CSSVR_XR=0 starts XR via cssvr_start.
  // Splash wrap used to freeze ICvar: CCvar has no echo until engine register.
  if (!EngineProbe_PresentDone(EngineCmd_WrapReady(), ICvar_CmdsReady()))
    ProbeLiveEngine(g_eng);
  ViewHookTryInstall();
  UserCmd_HookLive();
  const bool run = DualPaint_ShouldRun(XrWanted(), XrSession_IsOkReason(XrHostStatus().reason));
  // STOPPING/LOSS used to keep last stick/buttons walking (Peek gated WantXr only).
  if (!run) ClientCmd_ReleaseHeld(g_eng, &g_prev_cmd, UserCmd_HookLive());
  UserCmd_NoteSessionOk(run);
  const VkResult pr = real(queue, info);
  // After present: never wait. Harvest a finished GPU copy, kick the next if XR is hungry.
  VkEyePair dual{};
  // STOPPING/LOSS without cssvr_stop used to keep g_vk_eyes, so the next session_ok
  // submitted last-session rasters before two new world paints.
  // STOPPING/LOSS used to leave mailbox dual sitting until READY submitted it.
  if (!run) {
    DropVkEyes();
    DropMailbox();
  }
  const bool have_dual = DualPaint_Latch(TakePairImpl(&dual) && VkEye_WorldsDiffer(dual), run);
  if (have_dual && !MailboxFull()) {
    EnsureXrWorker();
    PushXrDual(dual);
  }
  if (info && info->swapchainCount) {
    const bool want_ppm = g_dumps < 2;
    if (!have_dual)
      DumpSwapchain(queue, info->pSwapchains[0], info->pImageIndices[0], want_ppm, false);
    else if (want_ppm)
      DumpSwapchain(queue, info->pSwapchains[0], info->pImageIndices[0], true, true);
  }
  if ((g_presents % 300) == 0)
    Log("present=%d xr_ok=%d xr_fail=%d dumps=%d", g_presents, g_xr_ok, g_xr_fail, g_dumps);
  ViewHookOnSwap(); // next CViewRender may dual-paint again
  return pr;
}

VKAPI_ATTR VkResult VKAPI_CALL WrapCreateSwapchain(VkDevice device,
                                                   const VkSwapchainCreateInfoKHR* ci,
                                                   const VkAllocationCallbacks* a,
                                                   VkSwapchainKHR* out) {
  PFN_vkCreateSwapchainKHR real = g_createSc;
  DeviceState* ds = nullptr;
  {
    std::lock_guard<std::mutex> lk(g_mu);
    auto it = g_devs.find(device);
    if (it != g_devs.end()) {
      ds = &it->second;
      if (ds->fn.createSc) real = ds->fn.createSc;
    }
  }
  if (!real) return VK_ERROR_UNKNOWN;
  VkResult rc = real(device, ci, a, out);
  if (rc != VK_SUCCESS || !out || !ci) return rc;
  SwapState ss;
  ss.device = device;
  ss.w = ci->imageExtent.width;
  ss.h = ci->imageExtent.height;
  ss.format = ci->imageFormat;
  {
    std::lock_guard<std::mutex> lk(g_mu);
    auto it = g_devs.find(device);
    if (it != g_devs.end()) {
      ss.phys = it->second.phys;
      if (it->second.fn.getScImgs) {
        uint32_t n = 0;
        it->second.fn.getScImgs(device, *out, &n, nullptr);
        ss.images.resize(n);
        if (n) it->second.fn.getScImgs(device, *out, &n, ss.images.data());
      }
    }
    g_scs[*out] = std::move(ss);
  }
  Log("swapchain %ux%u fmt=%d imgs=%zu", ci->imageExtent.width, ci->imageExtent.height,
      (int)ci->imageFormat, g_scs[*out].images.size());
  (void)ds;
  return rc;
}

VKAPI_ATTR VkResult VKAPI_CALL WrapCreateDevice(VkPhysicalDevice phys,
                                                const VkDeviceCreateInfo* ci,
                                                const VkAllocationCallbacks* a, VkDevice* out) {
  if (!g_createDevice) g_createDevice = (PFN_vkCreateDevice)VulkanSym("vkCreateDevice");
  if (!g_createDevice) return VK_ERROR_UNKNOWN;
  VkResult rc = g_createDevice(phys, ci, a, out);
  if (rc != VK_SUCCESS || !out) return rc;
  DeviceState ds;
  ds.phys = phys;
  ds.fn.gdpa = g_gdpa ? g_gdpa : (PFN_vkGetDeviceProcAddr)VulkanSym("vkGetDeviceProcAddr");
  if (ci && ci->queueCreateInfoCount)
    ds.gfx_family = ci->pQueueCreateInfos[0].queueFamilyIndex;
  FillDevFns(*out, ds);
  if (ds.fn.getQueue) ds.fn.getQueue(*out, ds.gfx_family, 0, &ds.queue);
  if (!ds.fn.memProps) {
    auto instFn = (PFN_vkGetPhysicalDeviceMemoryProperties)VulkanSym(
        "vkGetPhysicalDeviceMemoryProperties");
    ds.fn.memProps = instFn;
  }
  {
    std::lock_guard<std::mutex> lk(g_mu);
    g_devs[*out] = ds;
    if (ds.fn.present) g_present = ds.fn.present;
    if (ds.fn.createSc) g_createSc = ds.fn.createSc;
  }
  Log("device ok family=%u present=%p", ds.gfx_family, (void*)ds.fn.present);
  return rc;
}

VKAPI_ATTR VkResult VKAPI_CALL WrapCreateImage(VkDevice device, const VkImageCreateInfo* ci,
                                               const VkAllocationCallbacks* a, VkImage* out) {
  PFN_vkCreateImage real = nullptr;
  {
    std::lock_guard<std::mutex> lk(g_mu);
    auto it = g_devs.find(device);
    if (it != g_devs.end()) real = it->second.fn.createImg;
  }
  if (!real) real = (PFN_vkCreateImage)VulkanSym("vkCreateImage");
  if (!real) return VK_ERROR_UNKNOWN;
  VkImageCreateInfo patched{};
  const VkImageCreateInfo* use = ci;
  if (ci && ci->imageType == VK_IMAGE_TYPE_2D && (ci->usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
    patched = *ci;
    patched.usage = VkEye_AugmentUsage(ci->usage);
    use = &patched;
  }
  const VkResult rc = real(device, use, a, out);
  if (rc == VK_SUCCESS && out && ci && ci->imageType == VK_IMAGE_TYPE_2D &&
      (ci->usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
    std::lock_guard<std::mutex> lk(g_mu);
    g_img_sz[*out] = {ci->extent.width, ci->extent.height};
    g_img_fmt[*out] = ci->format;
    g_img_dev[*out] = device;
  }
  return rc;
}

VKAPI_ATTR VkResult VKAPI_CALL WrapCreateImageView(VkDevice device, const VkImageViewCreateInfo* ci,
                                                   const VkAllocationCallbacks* a, VkImageView* out) {
  PFN_vkCreateImageView real = nullptr;
  {
    std::lock_guard<std::mutex> lk(g_mu);
    auto it = g_devs.find(device);
    if (it != g_devs.end()) real = it->second.fn.createView;
  }
  if (!real) real = (PFN_vkCreateImageView)VulkanSym("vkCreateImageView");
  if (!real) return VK_ERROR_UNKNOWN;
  const VkResult rc = real(device, ci, a, out);
  if (rc == VK_SUCCESS && out && ci) {
    std::lock_guard<std::mutex> lk(g_mu);
    g_views[*out] = ci->image;
  }
  return rc;
}

VKAPI_ATTR VkResult VKAPI_CALL WrapCreateFramebuffer(VkDevice device,
                                                     const VkFramebufferCreateInfo* ci,
                                                     const VkAllocationCallbacks* a,
                                                     VkFramebuffer* out) {
  PFN_vkCreateFramebuffer real = nullptr;
  {
    std::lock_guard<std::mutex> lk(g_mu);
    auto it = g_devs.find(device);
    if (it != g_devs.end()) real = it->second.fn.createFb;
  }
  if (!real) real = (PFN_vkCreateFramebuffer)VulkanSym("vkCreateFramebuffer");
  if (!real) return VK_ERROR_UNKNOWN;
  const VkResult rc = real(device, ci, a, out);
  if (rc == VK_SUCCESS && out && ci && ci->attachmentCount && ci->pAttachments) {
    std::lock_guard<std::mutex> lk(g_mu);
    FbColor fb;
    fb.dev = device;
    fb.w = ci->width;
    fb.h = ci->height;
    auto vit = g_views.find(ci->pAttachments[0]);
    if (vit != g_views.end()) {
      fb.img = vit->second;
      auto fit = g_img_fmt.find(fb.img);
      if (fit != g_img_fmt.end()) fb.fmt = fit->second;
    }
    g_fbs[*out] = fb;
  }
  return rc;
}

void NoteLastRtFromView(VkImageView view, uint32_t hint_w, uint32_t hint_h, VkDevice dev,
                        VkImageLayout layout) {
  if (!view) return;
  auto vit = g_views.find(view);
  if (vit == g_views.end()) return;
  FbColor fb;
  fb.img = vit->second;
  fb.dev = dev;
  fb.layout = layout;
  auto sit = g_img_sz.find(fb.img);
  if (sit != g_img_sz.end()) {
    fb.w = sit->second.first;
    fb.h = sit->second.second;
  } else {
    fb.w = hint_w;
    fb.h = hint_h;
  }
  auto fit = g_img_fmt.find(fb.img);
  if (fit != g_img_fmt.end()) fb.fmt = fit->second;
  if (!fb.dev) {
    auto dit = g_img_dev.find(fb.img);
    if (dit != g_img_dev.end()) fb.dev = dit->second;
  }
  if (fb.img && fb.w >= 640 && fb.h >= 400) g_last_rt = fb;
}

VKAPI_ATTR void VKAPI_CALL WrapBeginRenderPass(VkCommandBuffer cmd,
                                               const VkRenderPassBeginInfo* info,
                                               VkSubpassContents contents) {
  PFN_vkCmdBeginRenderPass real = nullptr;
  {
    std::lock_guard<std::mutex> lk(g_mu);
    if (info) {
      auto it = g_fbs.find(info->framebuffer);
      if (it != g_fbs.end() && it->second.w >= 640 && it->second.h >= 400 && it->second.img)
        g_last_rt = it->second;
    }
    if (!g_devs.empty()) real = g_devs.begin()->second.fn.beginRp;
  }
  if (!real) real = (PFN_vkCmdBeginRenderPass)VulkanSym("vkCmdBeginRenderPass");
  if (real) real(cmd, info, contents);
}

VKAPI_ATTR void VKAPI_CALL WrapBeginRendering(VkCommandBuffer cmd, const VkRenderingInfo* info) {
  PFN_vkCmdBeginRendering real = (PFN_vkCmdBeginRendering)VulkanSym("vkCmdBeginRendering");
  if (!real) real = (PFN_vkCmdBeginRendering)VulkanSym("vkCmdBeginRenderingKHR");
  {
    std::lock_guard<std::mutex> lk(g_mu);
    if (info && info->colorAttachmentCount && info->pColorAttachments) {
      const VkRenderingAttachmentInfo& att = info->pColorAttachments[0];
      NoteLastRtFromView(att.imageView, info->renderArea.extent.width, info->renderArea.extent.height,
                         VK_NULL_HANDLE, att.imageLayout);
    }
  }
  if (real) real(cmd, info);
}

VKAPI_ATTR VkResult VKAPI_CALL WrapAcquireNextImage(VkDevice device, VkSwapchainKHR sc,
                                                    uint64_t timeout, VkSemaphore sem, VkFence fence,
                                                    uint32_t* idx) {
  PFN_vkAcquireNextImageKHR real = nullptr;
  {
    std::lock_guard<std::mutex> lk(g_mu);
    auto it = g_devs.find(device);
    if (it != g_devs.end()) real = it->second.fn.acquire;
  }
  if (!real) real = (PFN_vkAcquireNextImageKHR)VulkanSym("vkAcquireNextImageKHR");
  if (!real) return VK_ERROR_UNKNOWN;
  const VkResult rc = real(device, sc, timeout, sem, fence, idx);
  if ((rc == VK_SUCCESS || rc == VK_SUBOPTIMAL_KHR) && idx) {
    std::lock_guard<std::mutex> lk(g_mu);
    g_cur_sc = sc;
    g_cur_idx = *idx;
  }
  return rc;
}

PFN_vkVoidFunction WrapName(const char* name, PFN_vkVoidFunction real) {
  if (!name) return real;
  if (std::strcmp(name, "vkQueuePresentKHR") == 0) {
    if (real) g_present = (PFN_vkQueuePresentKHR)real;
    return (PFN_vkVoidFunction)WrapPresent;
  }
  if (std::strcmp(name, "vkCreateSwapchainKHR") == 0) {
    if (real) g_createSc = (PFN_vkCreateSwapchainKHR)real;
    return (PFN_vkVoidFunction)WrapCreateSwapchain;
  }
  if (std::strcmp(name, "vkCreateDevice") == 0) {
    if (real) g_createDevice = (PFN_vkCreateDevice)real;
    return (PFN_vkVoidFunction)WrapCreateDevice;
  }
  if (std::strcmp(name, "vkCreateImage") == 0) return (PFN_vkVoidFunction)WrapCreateImage;
  if (std::strcmp(name, "vkCreateImageView") == 0) return (PFN_vkVoidFunction)WrapCreateImageView;
  if (std::strcmp(name, "vkCreateFramebuffer") == 0)
    return (PFN_vkVoidFunction)WrapCreateFramebuffer;
  if (std::strcmp(name, "vkCmdBeginRenderPass") == 0)
    return (PFN_vkVoidFunction)WrapBeginRenderPass;
  if (std::strcmp(name, "vkCmdBeginRendering") == 0 ||
      std::strcmp(name, "vkCmdBeginRenderingKHR") == 0)
    return (PFN_vkVoidFunction)WrapBeginRendering;
  if (std::strcmp(name, "vkAcquireNextImageKHR") == 0)
    return (PFN_vkVoidFunction)WrapAcquireNextImage;
  return real;
}

} // namespace

extern "C" {

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance inst, const char* name) {
  if (!g_gipa) g_gipa = (PFN_vkGetInstanceProcAddr)VulkanSym("vkGetInstanceProcAddr");
  PFN_vkVoidFunction real = g_gipa ? g_gipa(inst, name) : nullptr;
  return WrapName(name, real);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice dev, const char* name) {
  if (!g_gdpa) g_gdpa = (PFN_vkGetDeviceProcAddr)VulkanSym("vkGetDeviceProcAddr");
  PFN_vkVoidFunction real = g_gdpa ? g_gdpa(dev, name) : nullptr;
  return WrapName(name, real);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDevice(VkPhysicalDevice phys, const VkDeviceCreateInfo* ci,
                                              const VkAllocationCallbacks* a, VkDevice* out) {
  return WrapCreateDevice(phys, ci, a, out);
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* info) {
  return WrapPresent(queue, info);
}

} // extern "C"

__attribute__((constructor)) static void vk_ctor() {
  g_gipa = (PFN_vkGetInstanceProcAddr)VulkanSym("vkGetInstanceProcAddr");
  g_gdpa = (PFN_vkGetDeviceProcAddr)VulkanSym("vkGetDeviceProcAddr");
  g_createDevice = (PFN_vkCreateDevice)VulkanSym("vkCreateDevice");
  Log("vulkan intercept ready gipa=%p", (void*)g_gipa);
}

namespace cssvr {
void VkLateAttachPresent() {
  void* vk = dlopen("libvulkan.so.1", RTLD_NOLOAD);
  if (!vk) vk = dlopen("libvulkan.so.1", RTLD_NOW);
  void* real = vk ? dlsym(vk, "vkQueuePresentKHR") : nullptr;
  void* wrap = (void*)&vkQueuePresentKHR;
  if (!real || real == wrap) {
    Log("late attach: present already ours or missing");
    return;
  }
  std::ifstream maps("/proc/self/maps");
  std::string line;
  int hits = 0;
  long page = sysconf(_SC_PAGESIZE);
  if (page < 4096) page = 4096;
  while (std::getline(maps, line)) {
    if (line.find("shaderapivk") == std::string::npos && line.find("libvulkan") == std::string::npos)
      continue;
    if (line.find("w") == std::string::npos) continue;
    unsigned long a = 0, b = 0;
    if (std::sscanf(line.c_str(), "%lx-%lx", &a, &b) != 2 || b <= a) continue;
    auto* p = reinterpret_cast<unsigned char*>(a);
    const size_t n = (size_t)(b - a);
    uintptr_t pg = (uintptr_t)p & ~((uintptr_t)page - 1);
    if (mprotect((void*)pg, (size_t)((uintptr_t)p + n - pg), PROT_READ | PROT_WRITE | PROT_EXEC) != 0)
      continue;
    for (size_t i = 0; i + sizeof(void*) <= n; i += sizeof(void*)) {
      void** slot = reinterpret_cast<void**>(p + i);
      if (*slot == real) {
        *slot = wrap;
        hits++;
      }
    }
  }
  Log("late attach present patches=%d", hits);
}
} // namespace cssvr
