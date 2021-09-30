// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "gmock/gmock.h"
#include "spoor/runtime/event_logger/event_logger_notifier.h"

namespace spoor::runtime::event_logger::testing {

class EventLoggerNotifierMock final : public EventLoggerNotifier {
 public:
  EventLoggerNotifierMock() = default;
  EventLoggerNotifierMock(const EventLoggerNotifierMock&) = delete;
  EventLoggerNotifierMock(EventLoggerNotifierMock&&) noexcept = delete;
  auto operator=(const EventLoggerNotifierMock&)
      -> EventLoggerNotifierMock& = delete;
  auto operator=(EventLoggerNotifierMock&&) noexcept
      -> EventLoggerNotifierMock& = delete;
  ~EventLoggerNotifierMock() override = default;

  MOCK_METHOD(  // NOLINT
      void, Subscribe, (gsl::not_null<EventLogger*> subscriber), (override));
  MOCK_METHOD(  // NOLINT
      void, Unsubscribe, (gsl::not_null<EventLogger*> subscriber), (override));
};

}  // namespace spoor::runtime::event_logger::testing
