#pragma once

#include "gmock/gmock.h"
#include "spoor/runtime/event_logger/event_logger_notifier.h"

namespace spoor::runtime::event_logger::testing {

class EventLoggerNotifierMock final : public EventLoggerNotifier {
 public:
  MOCK_METHOD(  // NOLINT
      void, Subscribe, (gsl::not_null<EventLogger*> subscriber), (override));
  MOCK_METHOD(  // NOLINT
      void, Unsubscribe, (gsl::not_null<EventLogger*> subscriber), (override));
};

}  // namespace spoor::runtime::event_logger::testing
