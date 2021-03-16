// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/event_logger/event_logger.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <new>
#include <vector>

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
using spoor::runtime::trace::EventType;
using spoor::runtime::trace::TimestampNanoseconds;
using testing::Eq;
using util::time::testing::SteadyClockMock;
using util::time::testing::SystemClockMock;
using SizeType = spoor::runtime::event_logger::EventLogger::SizeType;
using Pool = spoor::runtime::buffer::ReservedBufferSlicePool<Event>;
using Buffer = spoor::runtime::buffer::CircularSliceBuffer<Event>;
using FlushQueueMock =
    spoor::runtime::flush_queue::testing::FlushQueueMock<Buffer>;

TEST(EventLogger, LogsEvents) {  // NOLINT
  EventLoggerNotifierMock event_logger_notifier{};
  EXPECT_CALL(event_logger_notifier, Subscribe);
  EXPECT_CALL(event_logger_notifier, Unsubscribe);
  constexpr SizeType capacity{4};
  Pool expected_buffer_pool{
      {.max_slice_capacity = capacity, .capacity = capacity}};
  Buffer expected_buffer{
      {.buffer_slice_pool = &expected_buffer_pool, .capacity = capacity}};
  constexpr std::array<Event, 8> events{
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
  std::for_each(std::prev(std::cend(events), capacity), std::cend(events),
                [&](const auto event) { expected_buffer.Push(event); });
  FlushQueueMock flush_queue{};
  EXPECT_CALL(flush_queue, Enqueue(Eq(std::cref(expected_buffer))));
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
  EventLogger event_logger{{.flush_queue = &flush_queue,
                            .preferred_capacity = capacity,
                            .flush_buffer_when_full = false},
                           &event_logger_notifier};
  event_logger.SetPool(&pool);
  std::for_each(std::cbegin(events), std::cend(events),
                [&](const auto event) { event_logger.LogEvent(event); });
}

TEST(EventLogger, DoesNotLogEventsWithoutPool) {  // NOLINT
  EventLoggerNotifierMock event_logger_notifier{};
  EXPECT_CALL(event_logger_notifier, Subscribe);
  EXPECT_CALL(event_logger_notifier, Unsubscribe);
  FlushQueueMock flush_queue{};
  EXPECT_CALL(flush_queue, Enqueue).Times(0);
  EventLogger event_logger{{.flush_queue = &flush_queue,
                            .preferred_capacity = 3,
                            .flush_buffer_when_full = false},
                           &event_logger_notifier};
  ASSERT_TRUE(event_logger.Empty());
  event_logger.LogEvent({});
  ASSERT_TRUE(event_logger.Empty());
  event_logger.LogEvent({});
  ASSERT_TRUE(event_logger.Empty());
  event_logger.LogEvent({});
  ASSERT_TRUE(event_logger.Empty());
  event_logger.Flush();
  ASSERT_TRUE(event_logger.Empty());
}

TEST(EventLogger, PushWithZeroCapacity) {  // NOLINT
  EventLoggerNotifierMock event_logger_notifier{};
  EXPECT_CALL(event_logger_notifier, Subscribe);
  EXPECT_CALL(event_logger_notifier, Unsubscribe);
  FlushQueueMock flush_queue{};
  EXPECT_CALL(flush_queue, Enqueue).Times(0);
  constexpr SizeType capacity{0};
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
  EventLogger event_logger{{.flush_queue = &flush_queue,
                            .preferred_capacity = 8,
                            .flush_buffer_when_full = false},
                           &event_logger_notifier};
  event_logger.SetPool(&pool);
  event_logger.LogEvent({});
  event_logger.LogEvent({});
  event_logger.LogEvent({});
  event_logger.LogEvent({});
  ASSERT_TRUE(event_logger.Empty());
}

TEST(EventLogger, RaiiSubscribeToAndUnsubscribeFromNotifier) {  // NOLINT
  FlushQueueMock flush_queue{};
  constexpr SizeType capacity{0};
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
  EventLoggerNotifierMock event_logger_notifier{};
  std::array<std::byte, sizeof(EventLogger)> buffer{};
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  const auto* event_logger_ptr = reinterpret_cast<EventLogger*>(buffer.data());
  EXPECT_CALL(event_logger_notifier, Subscribe(Eq(event_logger_ptr)));
  const auto* event_logger = gsl::owner<EventLogger*>(
      new (buffer.data()) EventLogger{{.flush_queue = &flush_queue,
                                       .preferred_capacity = capacity,
                                       .flush_buffer_when_full = false},
                                      &event_logger_notifier});
  EXPECT_CALL(event_logger_notifier, Unsubscribe(Eq(event_logger_ptr)));
  event_logger->~EventLogger();
}

TEST(EventLogger, FlushWhenFull) {  // NOLINT
  constexpr SizeType capacity{5};
  for (const SizeType multiplier : {1, 2, 5, 10}) {
    EventLoggerNotifierMock event_logger_notifier{};
    EXPECT_CALL(event_logger_notifier, Subscribe).RetiresOnSaturation();
    EXPECT_CALL(event_logger_notifier, Unsubscribe).RetiresOnSaturation();
    FlushQueueMock flush_queue{};
    EXPECT_CALL(flush_queue, Enqueue).Times(multiplier).RetiresOnSaturation();
    Pool pool{
        {.max_slice_capacity = capacity, .capacity = multiplier * capacity}};
    EventLogger event_logger{{.flush_queue = &flush_queue,
                              .preferred_capacity = capacity,
                              .flush_buffer_when_full = true},
                             &event_logger_notifier};
    event_logger.SetPool(&pool);
    for (SizeType i{0}; i < multiplier * capacity; ++i) {
      event_logger.LogEvent({});
      ASSERT_EQ(event_logger.Empty(), (i + 1) % capacity == 0);
    }
  }
}

TEST(EventLogger, FlushesOnDestruction) {  // NOLINT
  EventLoggerNotifierMock event_logger_notifier{};
  EXPECT_CALL(event_logger_notifier, Subscribe);
  EXPECT_CALL(event_logger_notifier, Unsubscribe);
  FlushQueueMock flush_queue{};
  EXPECT_CALL(flush_queue, Enqueue);
  constexpr SizeType capacity{3};
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
  EventLogger event_logger{{.flush_queue = &flush_queue,
                            .preferred_capacity = capacity,
                            .flush_buffer_when_full = false},
                           &event_logger_notifier};
  event_logger.SetPool(&pool);
  event_logger.LogEvent({});
  event_logger.LogEvent({});
  event_logger.LogEvent({});
}

TEST(EventLogger, DoesNotFlushWhenEmpty) {  // NOLINT
  EventLoggerNotifierMock event_logger_notifier{};
  EXPECT_CALL(event_logger_notifier, Subscribe);
  EXPECT_CALL(event_logger_notifier, Unsubscribe);
  FlushQueueMock flush_queue{};
  EXPECT_CALL(flush_queue, Enqueue).Times(0);
  constexpr SizeType capacity{0};
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
  EventLogger event_logger{{.flush_queue = &flush_queue,
                            .preferred_capacity = capacity,
                            .flush_buffer_when_full = false},
                           &event_logger_notifier};
  event_logger.SetPool(&pool);
  ASSERT_TRUE(event_logger.Empty());
  event_logger.Flush();
  ASSERT_TRUE(event_logger.Empty());
  event_logger.Flush();
  event_logger.Flush();
  event_logger.Flush();
  ASSERT_TRUE(event_logger.Empty());
}

TEST(EventLogger, SizeEmptyCapacityFull) {  // NOLINT
  for (const SizeType capacity : {0, 1, 2, 5, 10}) {
    EventLoggerNotifierMock event_logger_notifier{};
    EXPECT_CALL(event_logger_notifier, Subscribe).RetiresOnSaturation();
    EXPECT_CALL(event_logger_notifier, Unsubscribe).RetiresOnSaturation();
    FlushQueueMock flush_queue{};
    if (0 < capacity) EXPECT_CALL(flush_queue, Enqueue).RetiresOnSaturation();
    Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
    EventLogger event_logger{{.flush_queue = &flush_queue,
                              .preferred_capacity = capacity,
                              .flush_buffer_when_full = false},
                             &event_logger_notifier};
    event_logger.SetPool(&pool);
    ASSERT_TRUE(event_logger.Empty());
    ASSERT_EQ(event_logger.Size(), 0);
    ASSERT_EQ(event_logger.Capacity(), capacity);
    ASSERT_EQ(event_logger.Full(), capacity == 0);
    for (SizeType i{0}; i < capacity; ++i) {
      ASSERT_FALSE(event_logger.Full());
      event_logger.LogEvent({});
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
