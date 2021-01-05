// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/flush_queue/black_hole_flush_queue.h"

#include <functional>

namespace spoor::runtime::flush_queue {

auto BlackHoleFlushQueue::Run() -> void {}

auto BlackHoleFlushQueue::Enqueue(Buffer&& /*unused*/) -> void {}

auto BlackHoleFlushQueue::DrainAndStop() -> void {}

auto BlackHoleFlushQueue::Flush(std::function<void()> completion) -> void {
  if (completion != nullptr) completion();
}

auto BlackHoleFlushQueue::Clear() -> void{};

// A member function is required to conform to `FlushQueue`'s interface.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
auto BlackHoleFlushQueue::Size() const -> SizeType { return 0; }

// A member function is required to conform to `FlushQueue`'s interface.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
auto BlackHoleFlushQueue::Empty() const -> bool { return true; }

}  // namespace spoor::runtime::flush_queue
