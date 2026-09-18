#include "cssvrmod/banner.hpp"
#include "cssvrmod/toast.hpp"
#include "cssvrmod/xr_session.hpp"
#include "test_framework.h"

using namespace cssvr;

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
