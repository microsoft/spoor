// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <functional>

#include "gmock/gmock.h"
#include "spoor/runtime/flush_queue/flush_queue.h"

namespace spoor::runtime::flush_queue::testing {

template <class T>
class FlushQueueMock : public FlushQueue<T> {
 public:
  FlushQueueMock() = default;
  FlushQueueMock(const FlushQueueMock&) = delete;
  FlushQueueMock(FlushQueueMock&&) noexcept = delete;
  auto operator=(const FlushQueueMock&) -> FlushQueueMock& = delete;
  auto operator=(FlushQueueMock&&) noexcept -> FlushQueueMock& = delete;
  ~FlushQueueMock() override = default;

  MOCK_METHOD(void, Run, (), (override));                         // NOLINT
  MOCK_METHOD(void, Enqueue, (T && item), (override));            // NOLINT
  MOCK_METHOD(void, DrainAndStop, (), (override));                // NOLINT
  MOCK_METHOD(void, Flush, (std::function<void()>), (override));  // NOLINT
  MOCK_METHOD(void, Clear, (), (override));                       // NOLINT
};

}  // namespace spoor::runtime::flush_queue::testing
