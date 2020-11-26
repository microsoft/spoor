// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/runtime_manager/runtime_manager.h"

#include <algorithm>
#include <chrono>
#include <future>
#include <map>
#include <system_error>
#include <thread>
#include <utility>

#include "absl/strings/str_format.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "spoor/runtime/buffer/circular_slice_buffer.h"
#include "spoor/runtime/config/config.h"
#include "spoor/runtime/event_logger/event_logger.h"
#include "spoor/runtime/flush_queue/flush_queue_mock.h"
#include "spoor/runtime/trace/trace_reader_mock.h"
#include "util/file_system/directory_entry_mock.h"
#include "util/file_system/file_system_mock.h"
#include "util/time/clock_mock.h"

namespace {

using spoor::runtime::buffer::CircularSliceBuffer;
using spoor::runtime::event_logger::EventLogger;
using spoor::runtime::runtime_manager::RuntimeManager;
using spoor::runtime::trace::Event;
using spoor::runtime::trace::Header;
using spoor::runtime::trace::kTraceFileVersion;
using spoor::runtime::trace::TimestampNanoseconds;
using spoor::runtime::trace::testing::TraceReaderMock;
using testing::_;
using testing::ElementsAreArray;
using testing::Return;
using util::file_system::testing::DirectoryEntryMock;
using util::file_system::testing::FileSystemMock;
using util::result::None;
using util::time::testing::MakeTimePoint;
using util::time::testing::SteadyClockMock;
using FlushQueueMock = spoor::runtime::flush_queue::testing::FlushQueueMock<
    CircularSliceBuffer<Event>>;
using SizeType = EventLogger::SizeType;

TEST(RuntimeManager, Initialize) {  // NOLINT
  constexpr auto iterations{3};
  SteadyClockMock steady_clock{};
  FlushQueueMock flush_queue{};
  EXPECT_CALL(flush_queue, Run()).Times(iterations);
  EXPECT_CALL(flush_queue, Clear()).Times(iterations);
  EXPECT_CALL(flush_queue, DrainAndStop()).Times(iterations);
  const RuntimeManager::Options options{.steady_clock = &steady_clock,
                                        .flush_queue = &flush_queue,
                                        .thread_event_buffer_capacity = 0,
                                        .reserved_pool_capacity = 0,
                                        .reserved_pool_max_slice_capacity = 0,
                                        .dynamic_pool_capacity = 0,
                                        .dynamic_pool_max_slice_capacity = 0,
                                        .dynamic_pool_borrow_cas_attempts = 0,
                                        .max_buffer_flush_attempts = 0,
                                        .flush_all_events = false};
  RuntimeManager runtime_manager{options};
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
  const RuntimeManager::Options options{.steady_clock = &steady_clock,
                                        .flush_queue = &flush_queue,
                                        .thread_event_buffer_capacity = 0,
                                        .reserved_pool_capacity = 0,
                                        .reserved_pool_max_slice_capacity = 0,
                                        .dynamic_pool_capacity = 0,
                                        .dynamic_pool_max_slice_capacity = 0,
                                        .dynamic_pool_borrow_cas_attempts = 0,
                                        .max_buffer_flush_attempts = 0,
                                        .flush_all_events = false};
  RuntimeManager runtime_manager{options};
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
  const RuntimeManager::Options options{.steady_clock = &steady_clock,
                                        .flush_queue = &flush_queue,
                                        .thread_event_buffer_capacity = 0,
                                        .reserved_pool_capacity = 0,
                                        .reserved_pool_max_slice_capacity = 0,
                                        .dynamic_pool_capacity = 0,
                                        .dynamic_pool_max_slice_capacity = 0,
                                        .dynamic_pool_borrow_cas_attempts = 0,
                                        .max_buffer_flush_attempts = 0,
                                        .flush_all_events = false};
  RuntimeManager runtime_manager{options};
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
  EXPECT_CALL(steady_clock, Now())
      .WillOnce(Return(MakeTimePoint<std::chrono::steady_clock>(0)));
  FlushQueueMock flush_queue{};
  EXPECT_CALL(flush_queue, Run());
  EXPECT_CALL(flush_queue, Enqueue(_));
  EXPECT_CALL(flush_queue, Clear());
  EXPECT_CALL(flush_queue, DrainAndStop());
  constexpr SizeType capacity{1};
  const RuntimeManager::Options runtime_manager_options{
      .steady_clock = &steady_clock,
      .flush_queue = &flush_queue,
      .thread_event_buffer_capacity = capacity,
      .reserved_pool_capacity = capacity,
      .reserved_pool_max_slice_capacity = capacity,
      .dynamic_pool_capacity = 0,
      .dynamic_pool_max_slice_capacity = 0,
      .dynamic_pool_borrow_cas_attempts = 0,
      .max_buffer_flush_attempts = 1,
      .flush_all_events = false};
  RuntimeManager runtime_manager{runtime_manager_options};
  runtime_manager.Initialize();
  const EventLogger::Options event_logger_options{
      .steady_clock = &steady_clock,
      .event_logger_notifier = &runtime_manager,
      .flush_queue = &flush_queue,
      .preferred_capacity = capacity,
      .flush_buffer_when_full = false};
  EventLogger event_logger{event_logger_options};
  event_logger.LogEvent(Event::Type::kFunctionEntry, 42);
  ASSERT_EQ(event_logger.Size(), capacity);
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
  std::promise<std::vector<std::filesystem::path>> promise{};
  RuntimeManager::FlushedTraceFiles(
      std::cbegin(directory_entries), std::cend(directory_entries),
      &trace_reader, [&](auto trace_file_paths) {
        promise.set_value(std::move(trace_file_paths));
      });
  auto future = promise.get_future();
  const auto result = future.get();
  ASSERT_THAT(result, ElementsAreArray(expected_trace_file_paths));
}

TEST(RuntimeManager, DeleteFlushedTraceFilesOlderThan) {  // NOLINT
  constexpr auto trace_files_size{5};
  const auto make_path = [](const SizeType n) {
    return std::filesystem::path{absl::StrFormat("%d.spoor", n)};
  };
  const auto make_timestamp = [](const int64 n) { return n * 1'000'000; };
  const auto make_header = [](TimestampNanoseconds system_clock_timestamp) {
    return Header{.version = kTraceFileVersion,
                  .session_id = 0,
                  .process_id = 0,
                  .thread_id = 0,
                  .system_clock_timestamp = system_clock_timestamp,
                  .steady_clock_timestamp = 0,
                  .event_count = 0,
                  .padding{}};
  };
  const auto make_file_size = [](const SizeType n) { return 1ULL << n; };
  FileSystemMock file_system{};
  TraceReaderMock trace_reader{};
  std::vector<DirectoryEntryMock> directory_entries{};
  directory_entries.reserve(trace_files_size);
  for (SizeType i{0}; i < trace_files_size; ++i) {
    const auto path = make_path(i);
    const auto timestamp = make_timestamp(i);
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
    std::promise<RuntimeManager::DeletedFilesInfo> promise{};
    RuntimeManager::DeleteFlushedTraceFilesOlderThan(
        MakeTimePoint<std::chrono::system_clock>(timestamp),
        std::cbegin(directory_entries), std::cend(directory_entries),
        &file_system, &trace_reader, [&](auto deleted_files_info) {
          promise.set_value(std::move(deleted_files_info));
        });
    auto future = promise.get_future();
    const auto result = future.get();
    const auto expected_deleted_files =
        std::max(0, std::min(i, trace_files_size));
    const auto expected_deleted_bytes = [&] {
      // 2^0 + 2^1 + ... + 2^(n - 1) == 2^n - 1
      const auto shift{
          static_cast<uint64>(std::min(std::max(i, 0), trace_files_size))};
      return (1ULL << shift) - 1;
    }();
    ASSERT_EQ(result.deleted_files, expected_deleted_files);
    ASSERT_EQ(result.deleted_bytes, expected_deleted_bytes);
  }
}

}  // namespace
