// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <functional>

#include "spoor/runtime/buffer/circular_slice_buffer.h"
#include "spoor/runtime/flush_queue/flush_queue.h"
#include "spoor/runtime/trace/trace.h"

namespace spoor::runtime::flush_queue {

class BlackHoleFlushQueue final
    : public FlushQueue<buffer::CircularSliceBuffer<trace::Event>> {
 public:
  using Buffer = buffer::CircularSliceBuffer<trace::Event>;
  using SizeType = Buffer::SizeType;

  auto Run() -> void override;
  auto Enqueue(Buffer&& /*unused*/) -> void override;
  auto DrainAndStop() -> void override;
  auto Flush(std::function<void()> completion) -> void override;
  auto Clear() -> void override;

  [[nodiscard]] auto Size() const -> SizeType;
  [[nodiscard]] auto Empty() const -> bool;
};

}  // namespace spoor::runtime::flush_queue
