#pragma once

#include "gsl/gsl"

namespace spoor::runtime::event_logger {

class EventLogger;

class EventLoggerNotifier {
 public:
  virtual ~EventLoggerNotifier() = default;

  virtual auto Subscribe(gsl::not_null<EventLogger*> subscriber) -> void = 0;
  virtual auto Unsubscribe(gsl::not_null<EventLogger*> subscriber) -> void = 0;

 protected:
  EventLoggerNotifier() = default;
  EventLoggerNotifier(const EventLoggerNotifier&) = default;
  EventLoggerNotifier(EventLoggerNotifier&&) noexcept = default;
  auto operator=(const EventLoggerNotifier&) -> EventLoggerNotifier& = default;
  auto operator=(EventLoggerNotifier&&) noexcept
      -> EventLoggerNotifier& = default;
};

}  // namespace spoor::runtime::event_logger
