// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/flush_queue/disk_flush_queue.h"

#include <array>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <iterator>

#include "absl/strings/str_format.h"
#include "absl/synchronization/notification.h"
#include "absl/time/time.h"
#include "gmock/gmock.h"
#include "gsl/gsl"
#include "gtest/gtest.h"
#include "spoor/runtime/buffer/circular_slice_buffer.h"
#include "spoor/runtime/buffer/reserved_buffer_slice_pool.h"
#include "spoor/runtime/trace/trace.h"
#include "spoor/runtime/trace/trace_writer_mock.h"
#include "util/compression/compressor.h"
#include "util/numeric.h"
#include "util/time/clock_mock.h"

namespace {

using spoor::runtime::flush_queue::DiskFlushQueue;
using spoor::runtime::flush_queue::kTraceFileExtension;
using spoor::runtime::trace::CompressionStrategy;
using spoor::runtime::trace::Event;
using spoor::runtime::trace::EventType;
using spoor::runtime::trace::Header;
using spoor::runtime::trace::kEndianness;
using spoor::runtime::trace::kMagicNumber;
using spoor::runtime::trace::kTraceFileVersion;
using spoor::runtime::trace::TimestampNanoseconds;
using spoor::runtime::trace::TraceWriter;
using spoor::runtime::trace::testing::TraceWriterMock;
using std::literals::chrono_literals::operator""ns;
using testing::_;
using testing::InSequence;
using testing::Invoke;
using testing::MatchesRegex;
using testing::MockFunction;
using testing::Return;
using testing::Truly;
using util::time::testing::MakeTimePoint;
using util::time::testing::SteadyClockMock;
using util::time::testing::SystemClockMock;
using Buffer = spoor::runtime::buffer::CircularSliceBuffer<Event>;
using SizeType = typename Buffer::SizeType;
using Pool = spoor::runtime::buffer::ReservedBufferSlicePool<Event>;

constexpr absl::Duration kNotificationTimeout{absl::Milliseconds(1'000)};
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
const std::filesystem::path kTraceFilePath{"trace/file/path"};
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
const std::string kTraceFilePattern = absl::StrFormat(
    R"(trace\/file\/path\/[0-9a-f]{16}-[0-9a-f]{16}-[0-9a-f]{16}\.%s)",
    kTraceFileExtension);
constexpr spoor::runtime::trace::SessionId kSessionId{42};
constexpr spoor::runtime::trace::ProcessId kProcessId{1729};

ACTION_P(Notify, notification) {  // NOLINT
  notification->Notify();
}

TEST(DiskFlushQueue, Enqueue) {  // NOLINT
  constexpr SizeType capacity{0};
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
  Buffer buffer_a{{.buffer_slice_pool = &pool, .capacity = capacity}};
  Buffer buffer_b{{.buffer_slice_pool = &pool, .capacity = capacity}};

  SystemClockMock system_clock{};
  EXPECT_CALL(system_clock, Now())
      .Times(2)
      .WillRepeatedly(Return(MakeTimePoint<std::chrono::system_clock>(0)));
  SteadyClockMock steady_clock{};
  std::atomic<TimestampNanoseconds> time{1};
  EXPECT_CALL(steady_clock, Now()).WillRepeatedly(Invoke([&time]() {
    return MakeTimePoint<std::chrono::steady_clock>(time);
  }));

  TraceWriterMock trace_writer{};
  EXPECT_CALL(trace_writer, Write(MatchesRegex(kTraceFilePattern), _, _))
      .Times(2)
      .WillRepeatedly(Return(TraceWriter::Result::Ok({})));

  DiskFlushQueue flush_queue{{.trace_file_path = kTraceFilePath,
                              .buffer_retention_duration = 2ns,
                              .system_clock = &system_clock,
                              .steady_clock = &steady_clock,
                              .trace_writer = &trace_writer,
                              .session_id = kSessionId,
                              .process_id = kProcessId,
                              .max_buffer_flush_attempts = 1,
                              .flush_all_events = false}};
  flush_queue.Run();
  ASSERT_TRUE(flush_queue.Empty());
  flush_queue.Enqueue(std::move(buffer_a));
  ASSERT_EQ(flush_queue.Size(), 1);
  flush_queue.Enqueue(std::move(buffer_b));
  ASSERT_EQ(flush_queue.Size(), 2);
  ++time;
  flush_queue.Flush({});
  flush_queue.DrainAndStop();
  ASSERT_TRUE(flush_queue.Empty());
}

TEST(DiskFlushQueue, WritesEvents) {  // NOLINT
  const std::array<Event, 8> events{
      {{.steady_clock_timestamp = 0,
        .payload_1 = 1,
        .type = static_cast<EventType>(Event::Type::kFunctionEntry),
        .payload_2 = 0},
       {.steady_clock_timestamp = 1,
        .payload_1 = 2,
        .type = static_cast<EventType>(Event::Type::kFunctionEntry),
        .payload_2 = 0},
       {.steady_clock_timestamp = 2,
        .payload_1 = 3,
        .type = static_cast<EventType>(Event::Type::kFunctionEntry),
        .payload_2 = 0},
       {.steady_clock_timestamp = 3,
        .payload_1 = 3,
        .type = static_cast<EventType>(Event::Type::kFunctionExit),
        .payload_2 = 0},
       {.steady_clock_timestamp = 4,
        .payload_1 = 3,
        .type = static_cast<EventType>(Event::Type::kFunctionEntry),
        .payload_2 = 0},
       {.steady_clock_timestamp = 5,
        .payload_1 = 3,
        .type = static_cast<EventType>(Event::Type::kFunctionExit),
        .payload_2 = 0},
       {.steady_clock_timestamp = 6,
        .payload_1 = 2,
        .type = static_cast<EventType>(Event::Type::kFunctionExit),
        .payload_2 = 0},
       {.steady_clock_timestamp = 7,
        .payload_1 = 1,
        .type = static_cast<EventType>(Event::Type::kFunctionExit),
        .payload_2 = 0}}};
  const SizeType capacity{events.size()};
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
  Buffer buffer{{.buffer_slice_pool = &pool, .capacity = capacity}};
  for (const auto event : events) {
    buffer.Push(event);
  }

  SystemClockMock system_clock{};
  constexpr TimestampNanoseconds system_clock_timestamp{100'000};
  EXPECT_CALL(system_clock, Now())
      .WillOnce(Return(
          MakeTimePoint<std::chrono::system_clock>(system_clock_timestamp)));
  SteadyClockMock steady_clock{};
  constexpr TimestampNanoseconds steady_clock_timestamp{42};
  EXPECT_CALL(steady_clock, Now())
      .WillRepeatedly(Return(
          MakeTimePoint<std::chrono::steady_clock>(steady_clock_timestamp)));

  const std::string trace_file_pattern =
      absl::StrFormat(R"(trace\/file\/path\/%016x-[0-9a-f]{16}-%016x\.%s)",
                      kSessionId, steady_clock_timestamp, kTraceFileExtension);
  const auto matches_header = [&](const Header& header) {
    // Ignore the `thread_id` because it reflects the hash of the true value
    // which cannot be determined.
    return header.magic_number == kMagicNumber &&
           header.endianness == kEndianness &&
           header.compression_strategy == CompressionStrategy::kNone &&
           header.version == kTraceFileVersion &&
           header.session_id == kSessionId && header.process_id == kProcessId &&
           header.system_clock_timestamp == system_clock_timestamp &&
           header.steady_clock_timestamp == steady_clock_timestamp &&
           gsl::narrow_cast<SizeType>(header.event_count) == events.size();
  };
  const auto matches_events = [expected_events{&events}](Buffer* buffer) {
    const auto chunks = buffer->ContiguousMemoryChunks();
    if (chunks.size() != 1) return false;
    const auto events = chunks.front();
    if (events.size() != expected_events->size()) return false;
    return std::equal(std::cbegin(events), std::cend(events),
                      std::cbegin(*expected_events));
  };
  TraceWriterMock trace_writer{};
  EXPECT_CALL(trace_writer, Write(MatchesRegex(trace_file_pattern),
                                  Truly(matches_header), Truly(matches_events)))
      .WillOnce(Return(TraceWriter::Result::Ok({})));

  DiskFlushQueue flush_queue{{.trace_file_path = kTraceFilePath,
                              .buffer_retention_duration = 0ns,
                              .system_clock = &system_clock,
                              .steady_clock = &steady_clock,
                              .trace_writer = &trace_writer,
                              .session_id = kSessionId,
                              .process_id = kProcessId,
                              .max_buffer_flush_attempts = 1,
                              .flush_all_events = true}};
  flush_queue.Run();
  ASSERT_TRUE(flush_queue.Empty());
  flush_queue.Enqueue(std::move(buffer));
  flush_queue.DrainAndStop();
  ASSERT_TRUE(flush_queue.Empty());
}

TEST(DiskFlushQueue, Flush) {  // NOLINT
  constexpr SizeType capacity{0};
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
  Buffer buffer{{.buffer_slice_pool = &pool, .capacity = capacity}};

  SystemClockMock system_clock{};
  EXPECT_CALL(system_clock, Now())
      .WillOnce(Return(MakeTimePoint<std::chrono::system_clock>(0)));
  SteadyClockMock steady_clock{};
  std::atomic<TimestampNanoseconds> time{1};
  EXPECT_CALL(steady_clock, Now()).WillRepeatedly(Invoke([&time]() {
    return MakeTimePoint<std::chrono::steady_clock>(time);
  }));

  TraceWriterMock trace_writer{};
  EXPECT_CALL(trace_writer, Write(MatchesRegex(kTraceFilePattern), _, _))
      .WillOnce(Return(TraceWriter::Result::Ok({})));

  DiskFlushQueue flush_queue{{.trace_file_path = kTraceFilePath,
                              .buffer_retention_duration = 4ns,
                              .system_clock = &system_clock,
                              .steady_clock = &steady_clock,
                              .trace_writer = &trace_writer,
                              .session_id = kSessionId,
                              .process_id = kProcessId,
                              .max_buffer_flush_attempts = 1,
                              .flush_all_events = false}};
  flush_queue.Run();
  ASSERT_TRUE(flush_queue.Empty());
  flush_queue.Enqueue(std::move(buffer));
  ASSERT_EQ(flush_queue.Size(), 1);
  ++time;
  flush_queue.Flush({});
  ++time;
  flush_queue.DrainAndStop();
  ASSERT_TRUE(flush_queue.Empty());
}

TEST(DiskFlushQueue, FlushCallback) {  // NOLINT
  constexpr SizeType capacity{0};
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
  Buffer buffer_a{{.buffer_slice_pool = &pool, .capacity = capacity}};
  Buffer buffer_b{{.buffer_slice_pool = &pool, .capacity = capacity}};

  SystemClockMock system_clock{};
  EXPECT_CALL(system_clock, Now())
      .WillOnce(Return(MakeTimePoint<std::chrono::system_clock>(0)));
  SteadyClockMock steady_clock{};
  std::atomic<TimestampNanoseconds> time{1};
  EXPECT_CALL(steady_clock, Now()).WillRepeatedly(Invoke([&time]() {
    return MakeTimePoint<std::chrono::steady_clock>(time);
  }));

  TraceWriterMock trace_writer{};
  EXPECT_CALL(trace_writer, Write(MatchesRegex(kTraceFilePattern), _, _))
      .WillOnce(Return(TraceWriter::Result::Ok({})));

  DiskFlushQueue flush_queue{{.trace_file_path = kTraceFilePath,
                              .buffer_retention_duration = 3ns,
                              .system_clock = &system_clock,
                              .steady_clock = &steady_clock,
                              .trace_writer = &trace_writer,
                              .session_id = kSessionId,
                              .process_id = kProcessId,
                              .max_buffer_flush_attempts = 1,
                              .flush_all_events = false}};
  flush_queue.Run();
  ASSERT_TRUE(flush_queue.Empty());
  flush_queue.Enqueue(std::move(buffer_a));
  ASSERT_EQ(flush_queue.Size(), 1);
  ++time;

  MockFunction<void()> callback{};
  absl::Notification done{};
  EXPECT_CALL(callback, Call()).WillOnce(Notify(&done));
  flush_queue.Flush(callback.AsStdFunction());
  const auto success =
      done.WaitForNotificationWithTimeout(kNotificationTimeout);
  ASSERT_TRUE(success);
  ASSERT_TRUE(flush_queue.Empty());

  ++time;
  flush_queue.Enqueue(std::move(buffer_b));
  ASSERT_EQ(flush_queue.Size(), 1);
  flush_queue.Clear();
  ASSERT_TRUE(flush_queue.Empty());
  flush_queue.DrainAndStop();
}

TEST(DiskFlushQueue, Clear) {  // NOLINT
  constexpr SizeType capacity{0};
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
  Buffer buffer{{.buffer_slice_pool = &pool, .capacity = capacity}};

  SystemClockMock system_clock{};
  EXPECT_CALL(system_clock, Now()).Times(0);
  SteadyClockMock steady_clock{};
  std::atomic<TimestampNanoseconds> time{1};
  EXPECT_CALL(steady_clock, Now()).WillRepeatedly(Invoke([&time]() {
    return MakeTimePoint<std::chrono::steady_clock>(time);
  }));

  TraceWriterMock trace_writer{};
  EXPECT_CALL(trace_writer, Write(_, _, _)).Times(0);

  DiskFlushQueue flush_queue{{.trace_file_path = kTraceFilePath,
                              .buffer_retention_duration = 1'000ns,
                              .system_clock = &system_clock,
                              .steady_clock = &steady_clock,
                              .trace_writer = &trace_writer,
                              .session_id = kSessionId,
                              .process_id = kProcessId,
                              .max_buffer_flush_attempts = 1,
                              .flush_all_events = false}};
  flush_queue.Run();
  ASSERT_TRUE(flush_queue.Empty());
  flush_queue.Enqueue(std::move(buffer));
  ASSERT_EQ(flush_queue.Size(), 1);
  ++time;
  flush_queue.Clear();
  ASSERT_TRUE(flush_queue.Empty());
  ++time;
  ASSERT_TRUE(flush_queue.Empty());
  flush_queue.DrainAndStop();
  ASSERT_TRUE(flush_queue.Empty());
}

TEST(DiskFlushQueue, RetainsEventsUntilTimePoint) {  // NOLINT
  constexpr SizeType capacity{0};
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
  Buffer buffer{{.buffer_slice_pool = &pool, .capacity = capacity}};

  SystemClockMock system_clock{};
  EXPECT_CALL(system_clock, Now()).Times(0);
  SteadyClockMock steady_clock{};
  std::atomic<TimestampNanoseconds> time{1};
  EXPECT_CALL(steady_clock, Now()).WillRepeatedly(Invoke([&time]() {
    return MakeTimePoint<std::chrono::steady_clock>(time);
  }));

  TraceWriterMock trace_writer{};
  EXPECT_CALL(trace_writer, Write(_, _, _)).Times(0);

  constexpr auto retention_duration = 3ns;
  DiskFlushQueue flush_queue{{.trace_file_path = kTraceFilePath,
                              .buffer_retention_duration = retention_duration,
                              .system_clock = &system_clock,
                              .steady_clock = &steady_clock,
                              .trace_writer = &trace_writer,
                              .session_id = kSessionId,
                              .process_id = kProcessId,
                              .max_buffer_flush_attempts = 1,
                              .flush_all_events = false}};
  flush_queue.Run();
  ASSERT_TRUE(flush_queue.Empty());
  flush_queue.Enqueue(std::move(buffer));
  ASSERT_EQ(flush_queue.Size(), 1);
  time += retention_duration.count();
  flush_queue.DrainAndStop();
  ASSERT_TRUE(flush_queue.Empty());
}

TEST(DiskFlushQueue, DropsEventsWhenNotRunning) {  // NOLINT
  constexpr SizeType capacity{0};
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
  Buffer buffer{{.buffer_slice_pool = &pool, .capacity = capacity}};

  SystemClockMock system_clock{};
  EXPECT_CALL(system_clock, Now()).Times(0);
  SteadyClockMock steady_clock{};
  EXPECT_CALL(steady_clock, Now())
      .WillOnce(Return(MakeTimePoint<std::chrono::steady_clock>(0)));

  TraceWriterMock trace_writer{};
  EXPECT_CALL(trace_writer, Write(_, _, _)).Times(0);

  DiskFlushQueue flush_queue{{.trace_file_path = kTraceFilePath,
                              .buffer_retention_duration = 1'000ns,
                              .system_clock = &system_clock,
                              .steady_clock = &steady_clock,
                              .trace_writer = &trace_writer,
                              .session_id = kSessionId,
                              .process_id = kProcessId,
                              .max_buffer_flush_attempts = 1,
                              .flush_all_events = false}};
  ASSERT_TRUE(flush_queue.Empty());
  flush_queue.Enqueue(std::move(buffer));
  ASSERT_TRUE(flush_queue.Empty());
}

TEST(DiskFlushQueue, FlushesOnLastAttempt) {  // NOLINT
  constexpr SizeType capacity{0};
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};

  SystemClockMock system_clock{};
  EXPECT_CALL(system_clock, Now())
      .WillRepeatedly(Return(MakeTimePoint<std::chrono::system_clock>(0)));
  SteadyClockMock steady_clock{};
  EXPECT_CALL(steady_clock, Now())
      .WillRepeatedly(Return(MakeTimePoint<std::chrono::steady_clock>(0)));

  for (const auto max_attempts : {0, 1, 3, 5, 10}) {
    Buffer buffer{{.buffer_slice_pool = &pool, .capacity = capacity}};

    TraceWriterMock trace_writer{};
    InSequence in_sequence{};
    if (1 < max_attempts) {
      EXPECT_CALL(trace_writer, Write(MatchesRegex(kTraceFilePattern), _, _))
          .Times(max_attempts - 1)
          .WillRepeatedly(Return(TraceWriter::Error::kFailedToOpenFile))
          .RetiresOnSaturation();
    }
    EXPECT_CALL(trace_writer, Write(MatchesRegex(kTraceFilePattern), _, _))
        .WillOnce(Return(TraceWriter::Result::Ok({})))
        .RetiresOnSaturation();

    DiskFlushQueue flush_queue{{.trace_file_path = kTraceFilePath,
                                .buffer_retention_duration = 0ns,
                                .system_clock = &system_clock,
                                .steady_clock = &steady_clock,
                                .trace_writer = &trace_writer,
                                .session_id = kSessionId,
                                .process_id = kProcessId,
                                .max_buffer_flush_attempts = max_attempts,
                                .flush_all_events = true}};
    flush_queue.Run();
    flush_queue.Enqueue(std::move(buffer));
    flush_queue.DrainAndStop();
  }
}

TEST(DiskFlushQueue, DropsEventsAfterMaxFlushAttempts) {  // NOLINT
  constexpr SizeType capacity{0};
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};

  SystemClockMock system_clock{};
  EXPECT_CALL(system_clock, Now())
      .WillRepeatedly(Return(MakeTimePoint<std::chrono::system_clock>(0)));
  SteadyClockMock steady_clock{};
  EXPECT_CALL(steady_clock, Now())
      .WillRepeatedly(Return(MakeTimePoint<std::chrono::steady_clock>(0)));

  for (const auto max_attempts : {0, 1, 3, 5, 10}) {
    Buffer buffer{{.buffer_slice_pool = &pool, .capacity = capacity}};

    TraceWriterMock trace_writer{};
    InSequence in_sequence{};
    EXPECT_CALL(trace_writer, Write(MatchesRegex(kTraceFilePattern), _, _))
        .Times(std::max(1, max_attempts))
        .WillRepeatedly(Return(TraceWriter::Error::kFailedToOpenFile))
        .RetiresOnSaturation();

    DiskFlushQueue flush_queue{{.trace_file_path = kTraceFilePath,
                                .buffer_retention_duration = 0ns,
                                .system_clock = &system_clock,
                                .steady_clock = &steady_clock,
                                .trace_writer = &trace_writer,
                                .session_id = kSessionId,
                                .process_id = kProcessId,
                                .max_buffer_flush_attempts = max_attempts,
                                .flush_all_events = true}};
    flush_queue.Run();
    flush_queue.Enqueue(std::move(buffer));
    flush_queue.DrainAndStop();
  }
}

TEST(DiskFlushQueue, HandlesConsecutiveCallsToRunAndDrainAndStop) {  // NOLINT
  constexpr SizeType capacity{0};
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
  Buffer buffer{{.buffer_slice_pool = &pool, .capacity = capacity}};

  SystemClockMock system_clock{};
  EXPECT_CALL(system_clock, Now()).Times(0);
  SteadyClockMock steady_clock{};
  EXPECT_CALL(steady_clock, Now()).Times(0);

  TraceWriterMock trace_writer{};
  EXPECT_CALL(trace_writer, Write(MatchesRegex(kTraceFilePattern), _, _))
      .Times(0);

  DiskFlushQueue flush_queue{{.trace_file_path = kTraceFilePath,
                              .buffer_retention_duration = 0ns,
                              .system_clock = &system_clock,
                              .steady_clock = &steady_clock,
                              .trace_writer = &trace_writer,
                              .session_id = kSessionId,
                              .process_id = kProcessId,
                              .max_buffer_flush_attempts = 1,
                              .flush_all_events = true}};
  flush_queue.Run();
  flush_queue.Run();
  flush_queue.Run();
  flush_queue.Run();
  flush_queue.Run();
  flush_queue.DrainAndStop();
  flush_queue.DrainAndStop();
  flush_queue.DrainAndStop();
  flush_queue.DrainAndStop();
  flush_queue.DrainAndStop();
  flush_queue.DrainAndStop();
}

}  // namespace
