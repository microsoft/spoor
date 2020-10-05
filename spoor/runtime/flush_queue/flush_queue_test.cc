#include "spoor/runtime/flush_queue/flush_queue.h"

#include <chrono>
#include <filesystem>
#include <iterator>
#include <vector>

#include "absl/strings/str_format.h"
#include "gmock/gmock.h"
#include "gsl/gsl"
#include "gtest/gtest.h"
#include "spoor/runtime/buffer/circular_slice_buffer.h"
#include "spoor/runtime/buffer/reserved_buffer_slice_pool.h"
#include "spoor/runtime/trace/trace.h"
#include "spoor/runtime/trace/trace_writer.h"
#include "spoor/runtime/trace/trace_writer_mock.h"
#include "util/numeric.h"
#include "util/time/clock_mock.h"

namespace {

using spoor::runtime::flush_queue::FlushQueue;
using spoor::runtime::trace::Event;
using spoor::runtime::trace::Footer;
using spoor::runtime::trace::Header;
using spoor::runtime::trace::TimestampNanoseconds;
using std::literals::chrono_literals::operator""ns;
using testing::_;
using testing::MatchesRegex;
using testing::Return;
using testing::Truly;
using Buffer = spoor::runtime::buffer::CircularSliceBuffer<Event>;
using SizeType = typename Buffer::SizeType;
using Pool = spoor::runtime::buffer::ReservedBufferSlicePool<Event>;
using Result = util::result::Result<util::result::None, util::result::None>;

const std::filesystem::path kTraceFilePath{"trace/file/path"};
const std::string kTraceFilePattern{
    R"(trace\/file\/path\/spoor-[0-9a-f]{16}-[0-9a-f]{16}-[0-9a-f]{16}\.trace)"};
constexpr spoor::runtime::trace::SessionId kSessionId{42};
constexpr spoor::runtime::trace::ProcessId kProcessId{1729};
constexpr Footer kExpectedFooter{};

// Run
// DrainAndStop
// Enqueue
// Flush
// Clear
// FlushWriter receives events
//

template <class ChronoClock>
auto MakeTimePoint(const TimestampNanoseconds timestamp_nanoseconds)
    -> std::chrono::time_point<ChronoClock> {
  const std::chrono::nanoseconds duration_nanoseconds{timestamp_nanoseconds};
  const auto duration =
      std::chrono::duration_cast<typename ChronoClock::duration>(
          duration_nanoseconds);
  // Check for a loss of precision when casting to `ChronoClock::duration`.
  assert(std::chrono::duration_cast<std::chrono::nanoseconds>(duration) ==
         duration_nanoseconds);
  return std::chrono::time_point<ChronoClock>{duration};
}

TEST(FlushQueue, Enqueue) {  // NOLINT
  constexpr SizeType capacity{0};
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
  Buffer buffer{{.buffer_slice_pool = &pool, .capacity = capacity}};

  util::time::SystemClockMock system_clock{};
  EXPECT_CALL(system_clock, Now())
      .WillOnce(Return(MakeTimePoint<std::chrono::system_clock>(0)));
  util::time::SteadyClockMock steady_clock{};
  constexpr TimestampNanoseconds enqueue_timestamp{10};
  EXPECT_CALL(steady_clock, Now())
      .WillOnce(Return(MakeTimePoint<std::chrono::steady_clock>(
          enqueue_timestamp - 1)))
      .WillRepeatedly(Return(
          MakeTimePoint<std::chrono::steady_clock>(enqueue_timestamp)));

  spoor::runtime::trace::TraceWriterMock trace_writer{};
  EXPECT_CALL(trace_writer, Write(MatchesRegex(kTraceFilePattern), _, _, _))
      .WillOnce(Return(Result::Ok({})));

  FlushQueue flush_queue{{.trace_file_path = kTraceFilePath,
                          .buffer_retention_duration = 1ns,
                          .system_clock = &system_clock,
                          .steady_clock = &steady_clock,
                          .trace_writer = &trace_writer,
                          .session_id = kSessionId,
                          .process_id = kProcessId,
                          .max_buffer_flush_attempts = 1}};
  flush_queue.Run();
  ASSERT_TRUE(flush_queue.Empty());
  flush_queue.Enqueue(std::move(buffer));
  ASSERT_EQ(flush_queue.Size(), 1);
  EXPECT_CALL(steady_clock, Now())
      .WillRepeatedly(Return(MakeTimePoint<std::chrono::steady_clock>(
          enqueue_timestamp + 1)));
  flush_queue.DrainAndStop();
  ASSERT_TRUE(flush_queue.Empty());
}

TEST(FlushQueue, WritesEvents) {  // NOLINT
  const std::vector events{Event(Event::Type::kFunctionEntry, 1, 0),
                           Event(Event::Type::kFunctionEntry, 2, 1),
                           Event(Event::Type::kFunctionEntry, 3, 2),
                           Event(Event::Type::kFunctionExit, 3, 3),
                           Event(Event::Type::kFunctionEntry, 3, 4),
                           Event(Event::Type::kFunctionExit, 3, 5),
                           Event(Event::Type::kFunctionExit, 2, 6),
                           Event(Event::Type::kFunctionExit, 1, 7)};
  const SizeType capacity{events.size()};
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
  Buffer buffer{{.buffer_slice_pool = &pool, .capacity = capacity}};
  for (const auto event : events) {
    buffer.Push(event);
  }

  util::time::SystemClockMock system_clock{};
  constexpr TimestampNanoseconds system_clock_timestamp{100'000};
  EXPECT_CALL(system_clock, Now())
      .WillOnce(Return(
          MakeTimePoint<std::chrono::system_clock>(system_clock_timestamp)));
  util::time::SteadyClockMock steady_clock{};
  constexpr TimestampNanoseconds enqueue_timestamp{10};
  EXPECT_CALL(steady_clock, Now())
      .WillOnce(Return(
          MakeTimePoint<std::chrono::steady_clock>(enqueue_timestamp)))
      .WillRepeatedly(Return(
          MakeTimePoint<std::chrono::steady_clock>(enqueue_timestamp)));

  const std::string trace_file_pattern = absl::StrFormat(
      R"(trace\/file\/path\/spoor-%016x-[0-9a-f]{16}-%016x\.trace)", kSessionId,
      enqueue_timestamp);
  const auto matches_header = [&](const Header& header) {
    // Ignore the `thread_id` because it reflects the hash of the true value
    // which cannot be determined.
    return header.version == spoor::runtime::trace::kTraceFileVersion &&
           header.session_id == kSessionId && header.process_id == kProcessId &&
           header.system_clock_timestamp == system_clock_timestamp &&
           header.steady_clock_timestamp == enqueue_timestamp + 1 &&
           gsl::narrow_cast<SizeType>(header.event_count) == events.size();
  };
  const auto matches_events = [expected_events = &events](Buffer& buffer) {
    const auto chunks = buffer.ContiguousMemoryChunks();
    if (chunks.size() != 1) return false;
    const auto events = chunks.front();
    if (events.size() != expected_events->size()) return false;
    return std::equal(std::cbegin(events), std::cend(events),
                      std::cbegin(*expected_events));
  };
  spoor::runtime::trace::TraceWriterMock trace_writer{};
  EXPECT_CALL(trace_writer,
              Write(MatchesRegex(trace_file_pattern), Truly(matches_header),
                    Truly(matches_events), kExpectedFooter))
      .WillOnce(Return(Result::Ok({})));

  FlushQueue flush_queue{{.trace_file_path = kTraceFilePath,
                          .buffer_retention_duration = 1ns,
                          .system_clock = &system_clock,
                          .steady_clock = &steady_clock,
                          .trace_writer = &trace_writer,
                          .session_id = kSessionId,
                          .process_id = kProcessId,
                          .max_buffer_flush_attempts = 1}};
  flush_queue.Run();
  ASSERT_TRUE(flush_queue.Empty());
  flush_queue.Enqueue(std::move(buffer));
  ASSERT_EQ(flush_queue.Size(), 1);
  EXPECT_CALL(steady_clock, Now())
      .WillRepeatedly(Return(MakeTimePoint<std::chrono::steady_clock>(
          enqueue_timestamp + 1)));
  flush_queue.DrainAndStop();
  ASSERT_TRUE(flush_queue.Empty());
}

TEST(FlushQueue, Flush) {  // NOLINT
  constexpr SizeType capacity{0};
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
  Buffer buffer{{.buffer_slice_pool = &pool, .capacity = capacity}};

  util::time::SystemClockMock system_clock{};
  EXPECT_CALL(system_clock, Now())
      .WillOnce(Return(MakeTimePoint<std::chrono::system_clock>(0)));
  util::time::SteadyClockMock steady_clock{};
  constexpr TimestampNanoseconds enqueue_timestamp{10};
  EXPECT_CALL(steady_clock, Now())
      .WillOnce(Return(MakeTimePoint<std::chrono::steady_clock>(
          enqueue_timestamp - 1)))
      .WillRepeatedly(Return(
          MakeTimePoint<std::chrono::steady_clock>(enqueue_timestamp)));

  spoor::runtime::trace::TraceWriterMock trace_writer{};
  EXPECT_CALL(trace_writer,
              Write(MatchesRegex(kTraceFilePattern), _, _, kExpectedFooter))
      .WillOnce(Return(Result::Ok({})));

  FlushQueue flush_queue{{.trace_file_path = kTraceFilePath,
                          .buffer_retention_duration = 1'000ns,
                          .system_clock = &system_clock,
                          .steady_clock = &steady_clock,
                          .trace_writer = &trace_writer,
                          .session_id = kSessionId,
                          .process_id = kProcessId,
                          .max_buffer_flush_attempts = 1}};
  flush_queue.Run();
  ASSERT_TRUE(flush_queue.Empty());
  flush_queue.Enqueue(std::move(buffer));
  EXPECT_CALL(steady_clock, Now())
      .WillRepeatedly(Return(MakeTimePoint<std::chrono::steady_clock>(
          enqueue_timestamp + 1)));
  flush_queue.Flush();
  EXPECT_CALL(steady_clock, Now())
      .WillRepeatedly(Return(MakeTimePoint<std::chrono::steady_clock>(
          enqueue_timestamp + 2)));
  flush_queue.DrainAndStop();
  ASSERT_TRUE(flush_queue.Empty());
}

TEST(FlushQueue, Clear) {  // NOLINT
}

TEST(FlushQueue, RetainsEventUntilTimePoint) {  // NOLINT
}

TEST(FlushQueue, DropsEventsWhenNotRunning) {  // NOLINT
}

TEST(FlushQueue, DropsEventsWhenDraining) {  // NOLINT
}

TEST(FlushQueue, FlushAttempts) {  // NOLINT
}

TEST(FlushQueue, DropsEventsAfterMaxFlushAttempts) {  // NOLINT
}

}  // namespace
