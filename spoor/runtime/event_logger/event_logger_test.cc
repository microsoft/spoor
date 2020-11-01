#include "spoor/runtime/event_logger/event_logger.h"

#include <chrono>
#include <cstddef>
#include <new>

#include "event_logger_notifier.h"
#include "gmock/gmock.h"
#include "gsl/gsl"
#include "gtest/gtest.h"
#include "spoor/runtime/buffer/circular_slice_buffer.h"
#include "spoor/runtime/buffer/reserved_buffer_slice_pool.h"
#include "spoor/runtime/event_logger/event_logger_notifier_mock.h"
#include "spoor/runtime/flush_queue/flush_queue_mock.h"
#include "spoor/runtime/trace/trace.h"
#include "util/time/clock_mock.h"

namespace {

using spoor::runtime::event_logger::EventLogger;
using spoor::runtime::event_logger::testing::EventLoggerNotifierMock;
using spoor::runtime::trace::Event;
using spoor::runtime::trace::TimestampNanoseconds;
using testing::Eq;
using testing::Invoke;
using testing::Return;
using util::time::testing::MakeTimePoint;
using util::time::testing::SteadyClockMock;
using util::time::testing::SystemClockMock;
using SizeType = spoor::runtime::event_logger::EventLogger::SizeType;
using Pool = spoor::runtime::buffer::ReservedBufferSlicePool<Event>;
using Buffer = spoor::runtime::buffer::CircularSliceBuffer<Event>;
using FlushQueueMock =
    spoor::runtime::flush_queue::testing::FlushQueueMock<Buffer>;

TEST(EventLogger, LogsEvents) {  // NOLINT
  TimestampNanoseconds time{0};
  SteadyClockMock steady_clock{};
  EXPECT_CALL(steady_clock, Now()).WillRepeatedly(Invoke([&time]() {
    const auto time_point = MakeTimePoint<std::chrono::steady_clock>(time);
    ++time;
    return time_point;
  }));
  EventLoggerNotifierMock event_logger_notifier{};
  EXPECT_CALL(event_logger_notifier, Subscribe);
  EXPECT_CALL(event_logger_notifier, Unsubscribe);
  constexpr SizeType capacity{4};
  Pool expected_buffer_pool{
      {.max_slice_capacity = capacity, .capacity = capacity}};
  Buffer expected_buffer{
      {.buffer_slice_pool = &expected_buffer_pool, .capacity = capacity}};
  expected_buffer.Push({Event::Type::kFunctionEntry, 3, 4});
  expected_buffer.Push({Event::Type::kFunctionExit, 3, 5});
  expected_buffer.Push({Event::Type::kFunctionExit, 2, 6});
  expected_buffer.Push({Event::Type::kFunctionExit, 1, 7});
  FlushQueueMock flush_queue{};
  EXPECT_CALL(flush_queue, Enqueue(Eq(std::cref(expected_buffer))));
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
  EventLogger event_logger{{.steady_clock = &steady_clock,
                            .event_logger_notifier = &event_logger_notifier,
                            .flush_queue = &flush_queue,
                            .preferred_capacity = capacity,
                            .flush_buffer_when_full = false}};
  event_logger.SetPool(&pool);
  event_logger.LogEvent(Event::Type::kFunctionEntry, 1);
  event_logger.LogEvent(Event::Type::kFunctionEntry, 2);
  event_logger.LogEvent(Event::Type::kFunctionEntry, 3);
  event_logger.LogEvent(Event::Type::kFunctionExit, 3);
  event_logger.LogEvent(Event::Type::kFunctionEntry, 3);
  event_logger.LogEvent(Event::Type::kFunctionExit, 3);
  event_logger.LogEvent(Event::Type::kFunctionExit, 2);
  event_logger.LogEvent(Event::Type::kFunctionExit, 1);
}

TEST(EventLogger, DoesNotLogEventsWithoutPool) {  // NOLINT
  SteadyClockMock steady_clock{};
  EXPECT_CALL(steady_clock, Now())
      .WillRepeatedly(Return(MakeTimePoint<std::chrono::steady_clock>(0)));
  EventLoggerNotifierMock event_logger_notifier{};
  EXPECT_CALL(event_logger_notifier, Subscribe);
  EXPECT_CALL(event_logger_notifier, Unsubscribe);
  FlushQueueMock flush_queue{};
  EXPECT_CALL(flush_queue, Enqueue).Times(0);
  EventLogger event_logger{{.steady_clock = &steady_clock,
                            .event_logger_notifier = &event_logger_notifier,
                            .flush_queue = &flush_queue,
                            .preferred_capacity = 3,
                            .flush_buffer_when_full = false}};
  ASSERT_TRUE(event_logger.Empty());
  event_logger.LogEvent(Event::Type::kFunctionEntry, 1);
  ASSERT_TRUE(event_logger.Empty());
  event_logger.LogEvent(Event::Type::kFunctionEntry, 2);
  ASSERT_TRUE(event_logger.Empty());
  event_logger.LogEvent(Event::Type::kFunctionEntry, 3);
  ASSERT_TRUE(event_logger.Empty());
  event_logger.Flush();
  ASSERT_TRUE(event_logger.Empty());
}

TEST(EventLogger, PushWithZeroCapacity) {  // NOLINT
  SteadyClockMock steady_clock{};
  EXPECT_CALL(steady_clock, Now())
      .WillRepeatedly(Return(MakeTimePoint<std::chrono::steady_clock>(0)));
  EventLoggerNotifierMock event_logger_notifier{};
  EXPECT_CALL(event_logger_notifier, Subscribe);
  EXPECT_CALL(event_logger_notifier, Unsubscribe);
  FlushQueueMock flush_queue{};
  EXPECT_CALL(flush_queue, Enqueue).Times(0);
  constexpr SizeType capacity{0};
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
  EventLogger event_logger{{.steady_clock = &steady_clock,
                            .event_logger_notifier = &event_logger_notifier,
                            .flush_queue = &flush_queue,
                            .preferred_capacity = 8,
                            .flush_buffer_when_full = false}};
  event_logger.SetPool(&pool);
  event_logger.LogEvent(Event::Type::kFunctionExit, 0);
  event_logger.LogEvent(Event::Type::kFunctionExit, 0);
  event_logger.LogEvent(Event::Type::kFunctionExit, 0);
  event_logger.LogEvent(Event::Type::kFunctionExit, 0);
  ASSERT_TRUE(event_logger.Empty());
}

TEST(EventLogger, RaiiSubscribeToAndUnsubscribeFromNotifier) {  // NOLINT
  SteadyClockMock steady_clock{};
  FlushQueueMock flush_queue{};
  constexpr SizeType capacity{0};
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
  EventLoggerNotifierMock event_logger_notifier{};
  EventLogger::Options event_logger_options{
      .steady_clock = &steady_clock,
      .event_logger_notifier = &event_logger_notifier,
      .flush_queue = &flush_queue,
      .preferred_capacity = capacity,
      .flush_buffer_when_full = false};
  std::array<std::byte, sizeof(EventLogger)> buffer{};
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  const auto* event_logger_ptr = reinterpret_cast<EventLogger*>(buffer.data());
  EXPECT_CALL(event_logger_notifier, Subscribe(Eq(event_logger_ptr)));
  const auto* event_logger = gsl::owner<EventLogger*>(
      new (buffer.data()) EventLogger{event_logger_options});
  EXPECT_CALL(event_logger_notifier, Unsubscribe(Eq(event_logger_ptr)));
  event_logger->~EventLogger();
}

TEST(EventLogger, FlushWhenFull) {  // NOLINT
  TimestampNanoseconds time{0};
  SteadyClockMock steady_clock{};
  EXPECT_CALL(steady_clock, Now()).WillRepeatedly(Invoke([&time]() {
    const auto time_point = MakeTimePoint<std::chrono::steady_clock>(time);
    ++time;
    return time_point;
  }));
  constexpr SizeType capacity{5};
  for (const SizeType multiplier : {1, 2, 5, 10}) {
    EventLoggerNotifierMock event_logger_notifier{};
    EXPECT_CALL(event_logger_notifier, Subscribe).RetiresOnSaturation();
    EXPECT_CALL(event_logger_notifier, Unsubscribe).RetiresOnSaturation();
    FlushQueueMock flush_queue{};
    EXPECT_CALL(flush_queue, Enqueue).Times(multiplier).RetiresOnSaturation();
    Pool pool{
        {.max_slice_capacity = capacity, .capacity = multiplier * capacity}};
    EventLogger event_logger{{.steady_clock = &steady_clock,
                              .event_logger_notifier = &event_logger_notifier,
                              .flush_queue = &flush_queue,
                              .preferred_capacity = capacity,
                              .flush_buffer_when_full = true}};
    event_logger.SetPool(&pool);
    for (SizeType i{0}; i < multiplier * capacity; ++i) {
      event_logger.LogEvent(Event::Type::kFunctionEntry, 0);
    }
  }
}

TEST(EventLogger, FlushesOnDestruction) {  // NOLINT
  SteadyClockMock steady_clock{};
  EXPECT_CALL(steady_clock, Now())
      .WillRepeatedly(Return(MakeTimePoint<std::chrono::steady_clock>(0)));
  EventLoggerNotifierMock event_logger_notifier{};
  EXPECT_CALL(event_logger_notifier, Subscribe);
  EXPECT_CALL(event_logger_notifier, Unsubscribe);
  FlushQueueMock flush_queue{};
  EXPECT_CALL(flush_queue, Enqueue);
  constexpr SizeType capacity{3};
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
  EventLogger event_logger{{.steady_clock = &steady_clock,
                            .event_logger_notifier = &event_logger_notifier,
                            .flush_queue = &flush_queue,
                            .preferred_capacity = capacity,
                            .flush_buffer_when_full = false}};
  event_logger.SetPool(&pool);
  event_logger.LogEvent(Event::Type::kFunctionEntry, 0);
  event_logger.LogEvent(Event::Type::kFunctionEntry, 0);
  event_logger.LogEvent(Event::Type::kFunctionEntry, 0);
}

TEST(EventLogger, DoesNotFlushWhenEmpty) {  // NOLINT
  SteadyClockMock steady_clock{};
  EXPECT_CALL(steady_clock, Now())
      .WillRepeatedly(Return(MakeTimePoint<std::chrono::steady_clock>(0)));
  EventLoggerNotifierMock event_logger_notifier{};
  EXPECT_CALL(event_logger_notifier, Subscribe);
  EXPECT_CALL(event_logger_notifier, Unsubscribe);
  FlushQueueMock flush_queue{};
  EXPECT_CALL(flush_queue, Enqueue).Times(0);
  constexpr SizeType capacity{0};
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
  EventLogger event_logger{{.steady_clock = &steady_clock,
                            .event_logger_notifier = &event_logger_notifier,
                            .flush_queue = &flush_queue,
                            .preferred_capacity = capacity,
                            .flush_buffer_when_full = false}};
  event_logger.SetPool(&pool);
  ASSERT_TRUE(event_logger.Empty());
  event_logger.Flush();
  event_logger.Flush();
  event_logger.Flush();
  ASSERT_TRUE(event_logger.Empty());
}

TEST(EventLogger, SizeEmptyCapacityFull) {  // NOLINT
  SteadyClockMock steady_clock{};
  EXPECT_CALL(steady_clock, Now())
      .WillRepeatedly(Return(MakeTimePoint<std::chrono::steady_clock>(0)));
  for (const SizeType capacity : {0, 1, 2, 5, 10}) {
    EventLoggerNotifierMock event_logger_notifier{};
    EXPECT_CALL(event_logger_notifier, Subscribe).RetiresOnSaturation();
    EXPECT_CALL(event_logger_notifier, Unsubscribe).RetiresOnSaturation();
    FlushQueueMock flush_queue{};
    if (0 < capacity) EXPECT_CALL(flush_queue, Enqueue).RetiresOnSaturation();
    Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
    EventLogger event_logger{{.steady_clock = &steady_clock,
                              .event_logger_notifier = &event_logger_notifier,
                              .flush_queue = &flush_queue,
                              .preferred_capacity = capacity,
                              .flush_buffer_when_full = false}};
    event_logger.SetPool(&pool);
    ASSERT_TRUE(event_logger.Empty());
    ASSERT_EQ(event_logger.Size(), 0);
    ASSERT_EQ(event_logger.Capacity(), capacity);
    ASSERT_EQ(event_logger.Full(), capacity == 0);
    for (SizeType i{0}; i < capacity; ++i) {
      ASSERT_FALSE(event_logger.Full());
      event_logger.LogEvent(Event::Type::kFunctionEntry, 0);
      ASSERT_FALSE(event_logger.Empty());
      ASSERT_EQ(event_logger.Size(), i + 1);
    }
    ASSERT_TRUE(event_logger.Full());
    for (SizeType i{0}; i < capacity; ++i) {
      ASSERT_FALSE(event_logger.Empty());
      ASSERT_TRUE(event_logger.Full());
      ASSERT_EQ(event_logger.Size(), capacity);
    }
    event_logger.Flush();
    ASSERT_TRUE(event_logger.Empty());
    ASSERT_EQ(event_logger.Capacity(), capacity);
  }
}

}  // namespace
