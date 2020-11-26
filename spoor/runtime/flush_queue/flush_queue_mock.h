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
  MOCK_METHOD(void, Run, (), (override));
  MOCK_METHOD(void, Enqueue, (T && item), (override));
  MOCK_METHOD(void, DrainAndStop, (), (override));
  MOCK_METHOD(void, Flush, (std::function<void()>), (override));
  MOCK_METHOD(void, Clear, (), (override));
};

}  // namespace spoor::runtime::flush_queue::testing
