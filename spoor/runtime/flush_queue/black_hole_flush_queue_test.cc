// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/flush_queue/black_hole_flush_queue.h"

#include <future>

#include "gtest/gtest.h"
#include "spoor/runtime/buffer/circular_slice_buffer.h"
#include "spoor/runtime/buffer/reserved_buffer_slice_pool.h"
#include "spoor/runtime/trace/trace.h"

namespace {

using spoor::runtime::flush_queue::BlackHoleFlushQueue;
using spoor::runtime::trace::Event;
using Buffer = spoor::runtime::buffer::CircularSliceBuffer<Event>;
using Pool = spoor::runtime::buffer::ReservedBufferSlicePool<Event>;

TEST(BlackHoleFlushQueue, Run) {  // NOLINT
  BlackHoleFlushQueue flush_queue{};
  flush_queue.Run();
}

TEST(BlackHoleFlushQueue, Enqueue) {  // NOLINT
  constexpr auto capacity{0};
  Pool pool{{.max_slice_capacity = capacity, .capacity = capacity}};
  Buffer buffer{{.buffer_slice_pool = &pool, .capacity = capacity}};
  BlackHoleFlushQueue flush_queue{};
  flush_queue.Enqueue(std::move(buffer));
}

TEST(BlackHoleFlushQueue, DrainAndStop) {  // NOLINT
  BlackHoleFlushQueue flush_queue{};
  flush_queue.DrainAndStop();
}

TEST(BlackHoleFlushQueue, Flush) {  // NOLINT
  BlackHoleFlushQueue flush_queue{};
  std::promise<void> promise{};
  flush_queue.Flush([&promise] { promise.set_value(); });
  auto future = promise.get_future();
  future.wait();
}

TEST(BlackHoleFlushQueue, FlushWithNullptr) {  // NOLINT
  BlackHoleFlushQueue flush_queue{};
  flush_queue.Flush({});
}

TEST(BlackHoleFlushQueue, Clear) {  // NOLINT
  BlackHoleFlushQueue flush_queue{};
  flush_queue.Clear();
}

TEST(BlackHoleFlushQueue, Size) {  // NOLINT
  BlackHoleFlushQueue flush_queue{};
  ASSERT_EQ(flush_queue.Size(), 0);
}

TEST(BlackHoleFlushQueue, Empty) {  // NOLINT
  BlackHoleFlushQueue flush_queue{};
  ASSERT_TRUE(flush_queue.Empty());
}

}  // namespace
