#include "cssvrmod/banner.hpp"
#include "cssvrmod/toast.hpp"
#include "cssvrmod/xr_loader.hpp"
#include "cssvrmod/xr_session.hpp"
#include "test_framework.h"

using namespace cssvr;

TEST(xr_loader_searches_soname_then_pv_host) {
  int n = 0;
  const char* const* c = XrLoader_Candidates(&n);
  ASSERT_TRUE(n >= 4);
  ASSERT_TRUE(c != nullptr);
  ASSERT_STREQ(c[0], "libopenxr_loader.so.1");
  ASSERT_STREQ(c[1], "libopenxr_loader.so");
  ASSERT_TRUE(XrLoader_HasPvHostCandidate());
  bool pv = false, usr = false;
  for (int i = 0; i < n; ++i) {
    if (XrLoader_IsPvHostPath(c[i])) pv = true;
    if (c[i] && std::strstr(c[i], "/usr/lib") && std::strstr(c[i], "libopenxr_loader")) usr = true;
  }
  ASSERT_TRUE(pv);
  ASSERT_TRUE(usr);
  ASSERT_FALSE(XrLoader_IsPvHostPath(c[0]));
  ASSERT_FALSE(XrLoader_IsPvHostPath(nullptr));
}

TEST(xr_loader_preloads_jsoncpp_sibling) {
  ASSERT_STREQ(XrLoader_DepSoname(), "libjsoncpp.so.27");
  int n = 0;
  const char* const* d = XrLoader_DepCandidates(&n);
  ASSERT_TRUE(n >= 2);
  ASSERT_STREQ(d[0], "libjsoncpp.so.27");
  bool pv = false;
  for (int i = 0; i < n; ++i)
    if (d[i] && std::strstr(d[i], "/run/host/") && std::strstr(d[i], "jsoncpp")) pv = true;
  ASSERT_TRUE(pv);
  char sib[128];
  ASSERT_TRUE(XrLoader_SiblingDep("/run/host/usr/lib/libopenxr_loader.so.1", sib, 128));
  ASSERT_STREQ(sib, "/run/host/usr/lib/libjsoncpp.so.27");
  ASSERT_FALSE(XrLoader_SiblingDep("libopenxr_loader.so.1", sib, 128));
  ASSERT_FALSE(XrLoader_SiblingDep(nullptr, sib, 128));
  ASSERT_FALSE(XrLoader_SiblingDep("/run/host/usr/lib/libopenxr_loader.so.1", sib, 8));
}

TEST(xr_session_ok_only_when_running) {
  ASSERT_EQ((int)XrSession_FromState(kXrStateUnknown, false, false), (int)XrSessionPhase::none);
  ASSERT_STREQ(XrSession_Reason(XrSessionPhase::none), "no_session");
  ASSERT_FALSE(XrSession_IsOk(XrSessionPhase::none));

  const auto created = XrSession_FromState(kXrStateIdle, true, false);
  ASSERT_EQ((int)created, (int)XrSessionPhase::created);
  ASSERT_STREQ(XrSession_Reason(created), "session_created");
  ASSERT_FALSE(XrSession_IsOk(created));
  ASSERT_FALSE(XrSession_IsOkReason("session_created"));

  const auto ready = XrSession_FromState(kXrStateReady, true, false);
  ASSERT_EQ((int)ready, (int)XrSessionPhase::ready);
  ASSERT_STREQ(XrSession_Reason(ready), "session_ready");
  ASSERT_FALSE(XrSession_IsOk(ready));

  const auto run = XrSession_FromState(kXrStateFocused, true, true);
  ASSERT_EQ((int)run, (int)XrSessionPhase::running);
  ASSERT_STREQ(XrSession_Reason(run), "session_ok");
  ASSERT_TRUE(XrSession_IsOk(run));
  ASSERT_TRUE(XrSession_IsOkReason("session_ok"));
}

TEST(xr_session_stop_and_loss_are_not_ok) {
  const auto stop = XrSession_FromState(kXrStateStopping, true, false);
  ASSERT_EQ((int)stop, (int)XrSessionPhase::stopping);
  ASSERT_STREQ(XrSession_Reason(stop), "session_stopping");
  ASSERT_FALSE(XrSession_IsOk(stop));

  const auto lost = XrSession_FromState(kXrStateLossPending, true, true);
  ASSERT_EQ((int)lost, (int)XrSessionPhase::lost);
  ASSERT_STREQ(XrSession_Reason(lost), "no_hmd");
  ASSERT_FALSE(XrSession_IsOk(lost));

  const auto exit = XrSession_FromState(kXrStateExiting, true, false);
  ASSERT_EQ((int)exit, (int)XrSessionPhase::lost);
  ASSERT_STREQ(XrSession_Reason(exit), "no_hmd");
  ASSERT_TRUE(XrSession_ShouldDestroy(true, true));
  ASSERT_FALSE(XrSession_ShouldDestroy(true, false)); // already dropped
  ASSERT_FALSE(XrSession_ShouldDestroy(false, true)); // STOPPING is EndSession
  ASSERT_TRUE(XrSession_AllowInit(false, 0));
  ASSERT_TRUE(XrSession_AllowInit(false, 3));
  ASSERT_FALSE(XrSession_AllowInit(false, 4)); // hammer-guard
  ASSERT_FALSE(XrSession_AllowInit(true, 0));
  ASSERT_EQ(XrSession_InitFailsAfter(true, false, 3), 0); // live session
  ASSERT_EQ(XrSession_InitFailsAfter(false, true, 4), 0); // LOSS dropped
  ASSERT_EQ(XrSession_InitFailsAfter(false, false, 3), 4);
  ASSERT_FALSE(XrSession_CountInitFail("no_hmd"));
  ASSERT_TRUE(XrSession_CountInitFail("create_session"));
  ASSERT_TRUE(XrSession_CountInitFail("create_instance"));
  ASSERT_EQ(XrSession_InitFailsAfter(false, false, 3, false), 3); // no_hmd keeps trying
  ASSERT_TRUE(XrSession_AllowInit(false, XrSession_InitFailsAfter(false, false, 3, false)));
}

TEST(xr_submit_fail_not_counted_on_warmup_or_skip) {
  ASSERT_FALSE(XrSubmit_CountFail("session_created", false, false));
  ASSERT_FALSE(XrSubmit_CountFail("session_ready", false, false));
  ASSERT_FALSE(XrSubmit_CountFail("session_not_running", false, false));
  ASSERT_FALSE(XrSubmit_CountFail("session_stopping", false, false));
  ASSERT_FALSE(XrSubmit_CountFail("no_hmd", false, false));
  ASSERT_FALSE(XrSubmit_CountFail("session_ok", true, false));
  ASSERT_FALSE(XrSubmit_CountFail("session_ok", false, true));
  ASSERT_TRUE(XrSubmit_CountFail("session_ok", false, false));

  ASSERT_EQ((int)XrSession_BeginKind(false, false, false), (int)XrBeginKind::miss);
  ASSERT_EQ((int)XrSession_BeginKind(true, true, false), (int)XrBeginKind::skip);
  ASSERT_EQ((int)XrSession_BeginKind(true, true, true), (int)XrBeginKind::ready);
  ASSERT_EQ((int)XrSession_BeginKind(true, false, true), (int)XrBeginKind::miss);
}

TEST(xr_worker_pumps_while_skipping_leftover_submit) {
  ASSERT_TRUE(XrWorker_ShouldPump(true));
  ASSERT_FALSE(XrWorker_ShouldPump(false));
  ASSERT_TRUE(XrWorker_ShouldSubmit(true, true));
  ASSERT_FALSE(XrWorker_ShouldSubmit(true, false));
  ASSERT_FALSE(XrWorker_ShouldSubmit(false, true));
  ASSERT_FALSE(XrWorker_ShouldSubmit(false, false));
  // STOPPING must still pump: otherwise READY after EndSession is never seen.
  ASSERT_TRUE(XrWorker_ShouldPump(true) && !XrWorker_ShouldSubmit(true, false));
  ASSERT_FALSE(XrSession_IsOkReason("session_stopping"));
  ASSERT_TRUE(XrSession_IsOkReason("session_ok"));
  // cssvr_stop: still pump while running so RequestExit can reach STOPPING.
  ASSERT_TRUE(XrWorker_ShouldPump(false, true));
  ASSERT_FALSE(XrWorker_ShouldPump(false, false));
  // After LeaveRunning, session_ok is false. Drain until EndSession or the
  // session sticks STOPPING (KeepWaited skipped EndSession on Begin miss).
  ASSERT_TRUE(XrWorker_ShouldPump(false, false, true));
  ASSERT_FALSE(XrWorker_ShouldPump(false, false, false));
  ASSERT_TRUE(XrSession_RequestExit(false, true));
  ASSERT_FALSE(XrSession_RequestExit(true, true));
  ASSERT_FALSE(XrSession_RequestExit(false, false));
  ASSERT_FALSE(XrWorker_ShouldSubmit(false, true));
  ASSERT_FALSE(XrSession_LatchExitReq(false, false)); // fail must retry
  ASSERT_TRUE(XrSession_LatchExitReq(false, true));
  ASSERT_TRUE(XrSession_LatchExitReq(true, false));
  ASSERT_TRUE(XrSession_LatchExitReq(true, true));
}

TEST(xr_mailbox_drops_on_session_loss) {
  bool have = true, dual = true;
  XrMailbox_Drop(XrMailbox_Keep(true, true), &have, &dual);
  ASSERT_TRUE(have);
  ASSERT_TRUE(dual);
  // STOPPING/LOSS: leftover dual must not sit until READY submits it.
  XrMailbox_Drop(XrMailbox_Keep(true, false), &have, &dual);
  ASSERT_FALSE(have);
  ASSERT_FALSE(dual);
  ASSERT_FALSE(XrMailbox_Keep(true, true) && have);
  have = true;
  dual = true;
  XrMailbox_Drop(XrMailbox_Keep(false, true), &have, &dual);
  ASSERT_FALSE(have);
  ASSERT_FALSE(dual);
  XrMailbox_Drop(false, nullptr, nullptr);

  ASSERT_EQ(XrSession_BumpEpoch(true, false, 0), 1);
  ASSERT_EQ(XrSession_BumpEpoch(false, true, 1), 1); // READY does not bump
  ASSERT_EQ(XrSession_BumpEpoch(true, true, 1), 1);
  ASSERT_TRUE(XrMailbox_Accept(true, 1, 1));
  ASSERT_FALSE(XrMailbox_Accept(true, 0, 1)); // leftover from previous epoch
  ASSERT_FALSE(XrMailbox_Accept(false, 1, 1));

  ASSERT_TRUE(XrCopy_Take(true, true, 1, 1));
  ASSERT_FALSE(XrCopy_Take(true, true, 0, 1)); // last-session inflight
  ASSERT_FALSE(XrCopy_Take(true, false, 1, 1)); // ppm-only during STOPPING
  ASSERT_FALSE(XrCopy_Take(false, true, 1, 1));
}

TEST(xr_begun_frame_ends_on_abort) {
  ASSERT_FALSE(XrFrame_EndOnAbort(false, false));
  ASSERT_FALSE(XrFrame_EndOnAbort(false, true));
  ASSERT_FALSE(XrFrame_EndOnAbort(true, true)); // success path EndFrames itself
  ASSERT_TRUE(XrFrame_EndOnAbort(true, false)); // swapchain/blit miss
  // skip: Wait+Begin ran, shouldRender=false. Caller must still EndFrame.
  ASSERT_EQ((int)XrSession_BeginKind(true, true, false), (int)XrBeginKind::skip);
  ASSERT_TRUE(XrFrame_EndOnAbort(true, false));
  // STOPPING/LOSS: EndFrame before EndSession. Next READY Wait+Begin is illegal if begun.
  ASSERT_TRUE(XrFrame_EndOnAbort(true, XrWorker_ShouldSubmit(true, false)));
}

TEST(xr_wait_needs_begin) {
  ASSERT_FALSE(XrFrame_SkipWait(false));
  ASSERT_TRUE(XrFrame_SkipWait(true));
  ASSERT_FALSE(XrFrame_BeginBeforeLeave(false, false));
  ASSERT_FALSE(XrFrame_BeginBeforeLeave(false, true));
  ASSERT_FALSE(XrFrame_BeginBeforeLeave(true, true)); // already begun
  ASSERT_TRUE(XrFrame_BeginBeforeLeave(true, false)); // Wait ok, Begin miss
  ASSERT_TRUE(XrFrame_CanEndSession(false, false));
  ASSERT_FALSE(XrFrame_CanEndSession(true, false));
  ASSERT_FALSE(XrFrame_CanEndSession(false, true));
  ASSERT_FALSE(XrFrame_CanEndSession(true, true));
  ASSERT_FALSE(XrFrame_KeepWaited(false, false));
  ASSERT_FALSE(XrFrame_KeepWaited(false, true));
  ASSERT_FALSE(XrFrame_KeepWaited(true, true)); // Begin closed Wait
  ASSERT_TRUE(XrFrame_KeepWaited(true, false)); // Begin miss — do not lie
  // LeaveRunning used to clear waited on Begin fail; EndSession then leaked.
  ASSERT_FALSE(XrFrame_CanEndSession(XrFrame_KeepWaited(true, false), false));
  ASSERT_TRUE(XrFrame_CanEndSession(XrFrame_KeepWaited(true, true), false));
  ASSERT_TRUE(XrFrame_RetryLeave(true, true, false)); // Begin miss — pump again
  ASSERT_TRUE(XrFrame_RetryLeave(true, false, true)); // begun still open
  ASSERT_FALSE(XrFrame_RetryLeave(true, false, false)); // pair closed
  ASSERT_FALSE(XrFrame_RetryLeave(false, true, false));
  ASSERT_TRUE(XrFrame_ShouldEndSession(true, true, false));
  ASSERT_FALSE(XrFrame_ShouldEndSession(true, true, true)); // already ended
  ASSERT_FALSE(XrFrame_ShouldEndSession(true, false, false)); // Wait still open
  ASSERT_FALSE(XrFrame_ShouldEndSession(false, true, false));
}

TEST(xr_begin_miss_keeps_stopping_and_loss) {
  ASSERT_STREQ(XrSession_BeginMiss(false, false, "no_loader"), "no_loader");
  ASSERT_STREQ(XrSession_BeginMiss(false, false, nullptr), "no_session");
  ASSERT_STREQ(XrSession_BeginMiss(true, true, "session_ok"), "session_ok");
  ASSERT_STREQ(XrSession_BeginMiss(true, false, "session_ok"), "session_not_running");
  ASSERT_STREQ(XrSession_BeginMiss(true, false, "session_created"), "session_not_running");
  ASSERT_STREQ(XrSession_BeginMiss(true, false, "session_stopping"), "session_stopping");
  ASSERT_STREQ(XrSession_BeginMiss(true, false, "no_hmd"), "no_hmd");
  ASSERT_FALSE(XrSession_ResetToastOnShutdown());
}

TEST(xr_session_mono_banner_before_running) {
  ASSERT_TRUE(XrSession_ShowsMono("session_created"));
  ASSERT_TRUE(XrSession_ShowsMono("session_ready"));
  ASSERT_TRUE(XrSession_ShowsMono("session_ok"));
  ASSERT_FALSE(XrSession_ShowsMono("session_stopping"));
  ASSERT_FALSE(XrSession_ShowsMono("no_hmd"));
  const auto warm = Banner_Decide("session_created", false);
  ASSERT_TRUE(warm.should_stamp);
  ASSERT_FALSE(warm.stamp_xr);
  ASSERT_STREQ(warm.text, "MONO");
  ASSERT_FALSE(Toast_ShouldShow("session_created", false));
  ASSERT_FALSE(Toast_ShouldShow("session_not_running", false));
  ASSERT_FALSE(Toast_ShouldShow("session_stopping", false));
  ASSERT_TRUE(Toast_ShouldShow("no_hmd", false));
}
