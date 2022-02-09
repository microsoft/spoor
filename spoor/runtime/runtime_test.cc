// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/runtime.h"

#include <filesystem>
#include <functional>
#include <limits>
#include <vector>

#include "absl/synchronization/notification.h"
#include "absl/time/time.h"
#include "gmock/gmock.h"
#include "gsl/gsl"
#include "gtest/gtest.h"

namespace {

using testing::MockFunction;
using testing::Test;

constexpr absl::Duration kNotificationTimeout{absl::Milliseconds(1'000)};

class RuntimeAutoInitialize : public Test {
 protected:
  auto SetUp() -> void override { spoor::runtime::Initialize(); }
  auto TearDown() -> void override { spoor::runtime::Deinitialize(); }
};

ACTION_P(Notify, notification) {  // NOLINT
  notification->Notify();
}

TEST(Runtime, Initialize) {  // NOLINT
  for (auto iteration{0}; iteration < 3; ++iteration) {
    ASSERT_FALSE(spoor::runtime::Enabled());
    ASSERT_FALSE(spoor::runtime::Initialized());
    spoor::runtime::Initialize();
    ASSERT_FALSE(spoor::runtime::Enabled());
    ASSERT_TRUE(spoor::runtime::Initialized());
    spoor::runtime::Initialize();
    ASSERT_FALSE(spoor::runtime::Enabled());
    ASSERT_TRUE(spoor::runtime::Initialized());
    spoor::runtime::Deinitialize();
    ASSERT_FALSE(spoor::runtime::Enabled());
    ASSERT_FALSE(spoor::runtime::Initialized());
    spoor::runtime::Deinitialize();
    ASSERT_FALSE(spoor::runtime::Enabled());
    ASSERT_FALSE(spoor::runtime::Initialized());
  }

  _spoor_runtime_Initialize();
  _spoor_runtime_Deinitialize();
}

TEST(Runtime, Enable) {  // NOLINT
  ASSERT_FALSE(spoor::runtime::Enabled());
  spoor::runtime::Initialize();
  ASSERT_FALSE(spoor::runtime::Enabled());
  for (auto iteration{0}; iteration < 3; ++iteration) {
    spoor::runtime::Enable();
    ASSERT_TRUE(spoor::runtime::Enabled());
    spoor::runtime::Enable();
    ASSERT_TRUE(spoor::runtime::Enabled());
    spoor::runtime::Disable();
    ASSERT_FALSE(spoor::runtime::Enabled());
    spoor::runtime::Disable();
    ASSERT_FALSE(spoor::runtime::Enabled());
  }
  spoor::runtime::Enable();
  ASSERT_TRUE(spoor::runtime::Enabled());
  spoor::runtime::Deinitialize();
  ASSERT_FALSE(spoor::runtime::Enabled());

  _spoor_runtime_Enable();
  _spoor_runtime_Initialize();
  _spoor_runtime_Enable();
  _spoor_runtime_Disable();
  _spoor_runtime_Deinitialize();
}

TEST_F(RuntimeAutoInitialize, FlushTraceEvents) {  // NOLINT
  spoor::runtime::FlushTraceEvents({});

  MockFunction<void()> callback{};
  absl::Notification done{};
  EXPECT_CALL(callback, Call()).WillOnce(Notify(&done));
  spoor::runtime::FlushTraceEvents(callback.AsStdFunction());
  const auto success =
      done.WaitForNotificationWithTimeout(kNotificationTimeout);
  ASSERT_TRUE(success);
}

TEST(Runtime, FlushTraceEventsNotRunning) {  // NOLINT
  spoor::runtime::FlushTraceEvents({});

  MockFunction<void()> callback{};
  absl::Notification done{};
  EXPECT_CALL(callback, Call()).WillOnce(Notify(&done));
  spoor::runtime::FlushTraceEvents(callback.AsStdFunction());
  const auto success =
      done.WaitForNotificationWithTimeout(kNotificationTimeout);
  ASSERT_TRUE(success);
}

TEST(Runtime, ClearTraceEvents) {  // NOLINT
  spoor::runtime::Deinitialize();
  spoor::runtime::ClearTraceEvents();
  spoor::runtime::Initialize();
  spoor::runtime::ClearTraceEvents();
  spoor::runtime::Deinitialize();
}

TEST(Runtime, FlushedTraceFiles) {  // NOLINT
  const std::vector<std::filesystem::path> expected_trace_files{};
  MockFunction<void(std::vector<std::filesystem::path>)> callback{};
  absl::Notification done{};
  EXPECT_CALL(callback, Call(expected_trace_files)).WillOnce(Notify(&done));
  spoor::runtime::FlushedTraceFiles(callback.AsStdFunction());
  const auto success =
      done.WaitForNotificationWithTimeout(kNotificationTimeout);
  ASSERT_TRUE(success);
}

TEST(Runtime, DeleteFlushedTraceFilesOlderThan) {  // NOLINT
  constexpr spoor::runtime::DeletedFilesInfo expected_deleted_files_info{
      .deleted_files = 0, .deleted_bytes = 0};
  MockFunction<void(spoor::runtime::DeletedFilesInfo)> callback{};
  absl::Notification done{};
  EXPECT_CALL(callback, Call(expected_deleted_files_info))
      .WillOnce(Notify(&done));
  spoor::runtime::DeleteFlushedTraceFilesOlderThan(
      std::numeric_limits<spoor::runtime::SystemTimestampSeconds>::max(),
      callback.AsStdFunction());
  const auto success =
      done.WaitForNotificationWithTimeout(kNotificationTimeout);
  ASSERT_TRUE(success);
}

TEST(Runtime, GetConfig) {  // NOLINT
  const spoor::runtime::Config expected_config{};
  const auto config = spoor::runtime::GetConfig();
  ASSERT_NE(config, expected_config);
}

TEST(Runtime, StubImplementation) {  // NOLINT
  ASSERT_FALSE(spoor::runtime::StubImplementation());
}

}  // namespace
