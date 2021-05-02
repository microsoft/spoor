// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/runtime_manager/runtime_manager.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <map>
#include <system_error>
#include <thread>
#include <utility>
#include <vector>

#include "absl/strings/str_format.h"
#include "absl/synchronization/notification.h"
#include "absl/time/time.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "spoor/runtime/buffer/circular_slice_buffer.h"
#include "spoor/runtime/buffer/reserved_buffer_slice_pool.h"
#include "spoor/runtime/event_logger/event_logger.h"
#include "spoor/runtime/flush_queue/flush_queue_mock.h"
#include "spoor/runtime/trace/trace_reader_mock.h"
#include "util/file_system/directory_entry_mock.h"
#include "util/file_system/file_system_mock.h"
#include "util/time/clock_mock.h"

namespace {

using spoor::runtime::event_logger::EventLogger;
using spoor::runtime::runtime_manager::RuntimeManager;
using spoor::runtime::trace::CompressionStrategy;
using spoor::runtime::trace::Event;
using spoor::runtime::trace::EventType;
using spoor::runtime::trace::Header;
using spoor::runtime::trace::TimestampNanoseconds;
using spoor::runtime::trace::testing::TraceReaderMock;
using testing::_;
using testing::Eq;
using testing::Invoke;
using testing::MockFunction;
using testing::Return;
using util::file_system::testing::DirectoryEntryMock;
using util::file_system::testing::FileSystemMock;
using util::result::None;
using util::time::testing::MakeTimePoint;
using util::time::testing::SteadyClockMock;
using CircularSliceBuffer = spoor::runtime::buffer::CircularSliceBuffer<Event>;
using FlushQueueMock =
    spoor::runtime::flush_queue::testing::FlushQueueMock<CircularSliceBuffer>;
using Pool = spoor::runtime::buffer::ReservedBufferSlicePool<Event>;
using SizeType = EventLogger::SizeType;

constexpr absl::Duration kNotificationTimeout{absl::Milliseconds(1'000)};

ACTION_P(Notify, notification) {  // NOLINT
  notification->Notify();
}

TEST(RuntimeManager, Initialize) {  // NOLINT
  constexpr auto iterations{3};
  SteadyClockMock steady_clock{};
  FlushQueueMock flush_queue{};
  EXPECT_CALL(flush_queue, Run()).Times(iterations);
  EXPECT_CALL(flush_queue, Clear()).Times(iterations);
  EXPECT_CALL(flush_queue, DrainAndStop()).Times(iterations);
  RuntimeManager runtime_manager{{.steady_clock = &steady_clock,
                                  .flush_queue = &flush_queue,
                                  .thread_event_buffer_capacity = 0,
                                  .reserved_pool_capacity = 0,
                                  .reserved_pool_max_slice_capacity = 0,
                                  .dynamic_pool_capacity = 0,
                                  .dynamic_pool_max_slice_capacity = 0,
                                  .dynamic_pool_borrow_cas_attempts = 0,
                                  .max_buffer_flush_attempts = 0,
                                  .flush_all_events = false}};
  for (auto iteration{0}; iteration < iterations; ++iteration) {
    ASSERT_FALSE(runtime_manager.Enabled());
    ASSERT_FALSE(runtime_manager.Initialized());
    runtime_manager.Initialize();
    ASSERT_FALSE(runtime_manager.Enabled());
    ASSERT_TRUE(runtime_manager.Initialized());
    runtime_manager.Initialize();
    ASSERT_FALSE(runtime_manager.Enabled());
    ASSERT_TRUE(runtime_manager.Initialized());
    runtime_manager.Deinitialize();
    ASSERT_FALSE(runtime_manager.Enabled());
    ASSERT_FALSE(runtime_manager.Initialized());
    runtime_manager.Deinitialize();
    ASSERT_FALSE(runtime_manager.Enabled());
    ASSERT_FALSE(runtime_manager.Initialized());
  }
}

TEST(RuntimeManager, RaiiDeinitializeOnDestruction) {  // NOLINT
  SteadyClockMock steady_clock{};
  FlushQueueMock flush_queue{};
  EXPECT_CALL(flush_queue, Run());
  EXPECT_CALL(flush_queue, Clear());
  EXPECT_CALL(flush_queue, DrainAndStop());
  RuntimeManager runtime_manager{{.steady_clock = &steady_clock,
                                  .flush_queue = &flush_queue,
                                  .thread_event_buffer_capacity = 0,
                                  .reserved_pool_capacity = 0,
                                  .reserved_pool_max_slice_capacity = 0,
                                  .dynamic_pool_capacity = 0,
                                  .dynamic_pool_max_slice_capacity = 0,
                                  .dynamic_pool_borrow_cas_attempts = 0,
                                  .max_buffer_flush_attempts = 0,
                                  .flush_all_events = false}};
  ASSERT_FALSE(runtime_manager.Initialized());
  runtime_manager.Initialize();
  ASSERT_TRUE(runtime_manager.Initialized());
}

TEST(RuntimeManager, Enable) {  // NOLINT
  SteadyClockMock steady_clock{};
  FlushQueueMock flush_queue{};
  EXPECT_CALL(flush_queue, Run());
  EXPECT_CALL(flush_queue, Clear());
  EXPECT_CALL(flush_queue, DrainAndStop());
  RuntimeManager runtime_manager{{.steady_clock = &steady_clock,
                                  .flush_queue = &flush_queue,
                                  .thread_event_buffer_capacity = 0,
                                  .reserved_pool_capacity = 0,
                                  .reserved_pool_max_slice_capacity = 0,
                                  .dynamic_pool_capacity = 0,
                                  .dynamic_pool_max_slice_capacity = 0,
                                  .dynamic_pool_borrow_cas_attempts = 0,
                                  .max_buffer_flush_attempts = 0,
                                  .flush_all_events = false}};
  ASSERT_FALSE(runtime_manager.Enabled());
  runtime_manager.Initialize();
  ASSERT_FALSE(runtime_manager.Enabled());
  constexpr auto iterations{3};
  for (auto iteration{0}; iteration < iterations; ++iteration) {
    runtime_manager.Enable();
    ASSERT_TRUE(runtime_manager.Enabled());
    runtime_manager.Enable();
    ASSERT_TRUE(runtime_manager.Enabled());
    runtime_manager.Disable();
    ASSERT_FALSE(runtime_manager.Enabled());
    runtime_manager.Disable();
    ASSERT_FALSE(runtime_manager.Enabled());
  }
  runtime_manager.Enable();
  ASSERT_TRUE(runtime_manager.Enabled());
  runtime_manager.Deinitialize();
  ASSERT_FALSE(runtime_manager.Enabled());
}

TEST(RuntimeManager, Subscribe) {  // NOLINT
  SteadyClockMock steady_clock{};
  FlushQueueMock flush_queue{};
  EXPECT_CALL(flush_queue, Run());
  EXPECT_CALL(flush_queue, Enqueue(_));
  EXPECT_CALL(flush_queue, Clear());
  EXPECT_CALL(flush_queue, DrainAndStop());
  constexpr SizeType capacity{1};
  RuntimeManager runtime_manager{{.steady_clock = &steady_clock,
                                  .flush_queue = &flush_queue,
                                  .thread_event_buffer_capacity = capacity,
                                  .reserved_pool_capacity = capacity,
                                  .reserved_pool_max_slice_capacity = capacity,
                                  .dynamic_pool_capacity = 0,
                                  .dynamic_pool_max_slice_capacity = 0,
                                  .dynamic_pool_borrow_cas_attempts = 0,
                                  .max_buffer_flush_attempts = 1,
                                  .flush_all_events = false}};
  runtime_manager.Initialize();
  EventLogger event_logger{{.flush_queue = &flush_queue,
                            .preferred_capacity = capacity,
                            .flush_buffer_when_full = false},
                           &runtime_manager};
  event_logger.LogEvent(
      {.steady_clock_timestamp = 0,
       .payload_1 = 1,
       .type = static_cast<EventType>(Event::Type::kFunctionEntry),
       .payload_2 = 2});
  ASSERT_EQ(event_logger.Size(), capacity);
}

TEST(RuntimeManager, LogEvent) {  // NOLINT
  TimestampNanoseconds time{0};
  SteadyClockMock steady_clock{};
  EXPECT_CALL(steady_clock, Now()).WillRepeatedly(Invoke([&time]() {
    const auto time_point = MakeTimePoint<std::chrono::steady_clock>(time);
    ++time;
    return time_point;
  }));
  constexpr std::array<Event, 4> events{
      {{.steady_clock_timestamp = 0,
        .payload_1 = 1,
        .type = static_cast<EventType>(Event::Type::kFunctionEntry),
        .payload_2 = 0},
       {.steady_clock_timestamp = 1,
        .payload_1 = 2,
        .type = static_cast<EventType>(Event::Type::kFunctionExit),
        .payload_2 = 0},
       {.steady_clock_timestamp = 2,
        .payload_1 = 3,
        .type = static_cast<EventType>(Event::Type::kFunctionEntry),
        .payload_2 = 3},
       {.steady_clock_timestamp = 3,
        .payload_1 = 4,
        .type = static_cast<EventType>(Event::Type::kFunctionExit),
        .payload_2 = 4}}};
  Pool expected_buffer_pool{
      {.max_slice_capacity = std::size(events), .capacity = std::size(events)}};
  CircularSliceBuffer expected_buffer{
      {.buffer_slice_pool = &expected_buffer_pool,
       .capacity = std::size(events)}};
  std::for_each(std::cbegin(events), std::cend(events),
                [&](const auto event) { expected_buffer.Push(event); });
  FlushQueueMock flush_queue{};
  EXPECT_CALL(flush_queue, Run());
  EXPECT_CALL(flush_queue, Enqueue(Eq(std::cref(expected_buffer))));
  EXPECT_CALL(flush_queue, Clear());
  EXPECT_CALL(flush_queue, DrainAndStop());
  RuntimeManager runtime_manager{
      {.steady_clock = &steady_clock,
       .flush_queue = &flush_queue,
       .thread_event_buffer_capacity = std::size(events),
       .reserved_pool_capacity = std::size(events),
       .reserved_pool_max_slice_capacity = std::size(events),
       .dynamic_pool_capacity = 0,
       .dynamic_pool_max_slice_capacity = 0,
       .dynamic_pool_borrow_cas_attempts = 0,
       .max_buffer_flush_attempts = 1,
       .flush_all_events = false}};
  runtime_manager.Initialize();
  runtime_manager.Enable();
  ASSERT_EQ(events[0].type,
            static_cast<EventType>(Event::Type::kFunctionEntry));
  runtime_manager.LogFunctionEntry(events[0].payload_1);
  ASSERT_EQ(events[1].type, static_cast<EventType>(Event::Type::kFunctionExit));
  runtime_manager.LogFunctionExit(events[1].payload_1);
  runtime_manager.LogEvent(events[2].type, events[2].payload_1,
                           events[2].payload_2);
  runtime_manager.LogEvent(events[3]);
}

TEST(RuntimeManager, FlushedTraceFiles) {  // NOLINT
  std::map<std::filesystem::path, bool> path_to_matches{
      {"/path/to/dir", false},
      {"/path/to/0.spoor", true},
      {"/path/to/a.spoor", true},
      {"/path/to/z.spoor", false}};
  std::vector<std::filesystem::path> paths{};
  std::transform(std::cbegin(path_to_matches), std::cend(path_to_matches),
                 std::back_inserter(paths),
                 [](const auto& entry) { return entry.first; });
  std::vector<DirectoryEntryMock> directory_entries{};
  std::transform(std::cbegin(paths), std::cend(paths),
                 std::back_inserter(directory_entries),
                 [](const auto& path) { return DirectoryEntryMock{path}; });
  std::vector<std::filesystem::path> expected_trace_file_paths{};
  std::copy_if(std::cbegin(paths), std::cend(paths),
               std::back_inserter(expected_trace_file_paths),
               [&path_to_matches](const auto& path) {
                 return path_to_matches.at(path);
               });
  TraceReaderMock trace_reader{};
  for (const auto& [path, matches_trace_file_convention] : path_to_matches) {
    EXPECT_CALL(trace_reader, MatchesTraceFileConvention(path))
        .WillOnce(Return(matches_trace_file_convention));
  }

  MockFunction<void(std::vector<std::filesystem::path>)> callback{};
  absl::Notification done{};
  EXPECT_CALL(callback, Call(expected_trace_file_paths))
      .WillOnce(Notify(&done));
  RuntimeManager::FlushedTraceFiles(std::cbegin(directory_entries),
                                    std::cend(directory_entries), &trace_reader,
                                    callback.AsStdFunction());
  const auto success =
      done.WaitForNotificationWithTimeout(kNotificationTimeout);
  ASSERT_TRUE(success);
}

TEST(RuntimeManager, DeleteFlushedTraceFilesOlderThan) {  // NOLINT
  constexpr auto trace_files_size{5};
  const auto make_path = [](const SizeType n) {
    return std::filesystem::path{absl::StrFormat("%d.spoor", n)};
  };
  const auto make_timestamp = [](const int64 n) -> TimestampNanoseconds {
    return n * 1'000'000;
  };
  const auto make_header = [](TimestampNanoseconds system_clock_timestamp) {
    return Header{.compression_strategy = CompressionStrategy::kNone,
                  .session_id = 0,
                  .process_id = 0,
                  .thread_id = 0,
                  .system_clock_timestamp = system_clock_timestamp,
                  .steady_clock_timestamp = 0,
                  .event_count = 0};
  };
  const auto make_file_size = [](const SizeType n) { return 1ULL << n; };
  FileSystemMock file_system{};
  TraceReaderMock trace_reader{};
  std::vector<DirectoryEntryMock> directory_entries{};
  directory_entries.reserve(trace_files_size);
  for (SizeType i{0}; i < trace_files_size; ++i) {
    const auto path = make_path(i);
    const auto timestamp = make_timestamp(gsl::narrow_cast<int64>(i));
    const auto header = make_header(timestamp);
    const auto file_size = make_file_size(i);
    directory_entries.emplace_back(path);
    EXPECT_CALL(trace_reader, MatchesTraceFileConvention(path))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(trace_reader, ReadHeader(path)).WillRepeatedly(Return(header));
    EXPECT_CALL(file_system, FileSize(path)).WillRepeatedly(Return(file_size));
  }
  for (auto i{-1}; i < trace_files_size + 3; ++i) {
    for (auto j{0}; j < std::min(i, trace_files_size); ++j) {
      const auto path = make_path(j);
      EXPECT_CALL(file_system, Remove(path))
          .WillOnce(
              Return(util::result::Result<None, std::error_code>::Ok({})));
    }
    const auto timestamp = make_timestamp(i) - 1'000;

    RuntimeManager::DeletedFilesInfo expected_deleted_files_info{
        .deleted_bytes =
            [&] {
              // 2^0 + 2^1 + ... + 2^(n - 1) == 2^n - 1
              const auto shift{static_cast<uint64>(
                  std::min(std::max(i, 0), trace_files_size))};
              return gsl::narrow_cast<int64>((1ULL << shift) - 1);
            }(),
        .deleted_files = std::max(0, std::min(i, trace_files_size))};

    MockFunction<void(RuntimeManager::DeletedFilesInfo)> callback{};
    absl::Notification done{};
    EXPECT_CALL(callback, Call(expected_deleted_files_info))
        .WillOnce(Notify(&done));
    RuntimeManager::DeleteFlushedTraceFilesOlderThan(
        MakeTimePoint<std::chrono::system_clock>(timestamp),
        std::cbegin(directory_entries), std::cend(directory_entries),
        &file_system, &trace_reader, callback.AsStdFunction());
    const auto success =
        done.WaitForNotificationWithTimeout(kNotificationTimeout);
    ASSERT_TRUE(success);
  }
}

TEST(RuntimeManagerDeletedFilesInfo, Equality) {  // NOLINT
  RuntimeManager::DeletedFilesInfo info_a{.deleted_bytes = 2,
                                          .deleted_files = 1};
  RuntimeManager::DeletedFilesInfo info_b{.deleted_bytes = 2,
                                          .deleted_files = 1};
  RuntimeManager::DeletedFilesInfo info_c{.deleted_bytes = 1,
                                          .deleted_files = 2};
  ASSERT_EQ(info_a, info_b);
  ASSERT_NE(info_b, info_c);
}

}  // namespace
