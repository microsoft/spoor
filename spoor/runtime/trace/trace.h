// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <array>
#include <type_traits>

#include "absl/base/internal/endian.h"
#include "gsl/gsl"
#include "util/numeric.h"

namespace spoor::runtime::trace {

using DurationNanoseconds = int64;
using EventCount = int32;
using EventType = uint32;
using FunctionId = uint64;
using ProcessId = int64;  // pid_t is a signed integer
using SessionId = uint64;
using ThreadId = uint64;
using TimestampNanoseconds = int64;
using TraceFileVersion = uint64;

// IMPORTANT: Increment the version number if the Header or Event structure
// changes.
constexpr TraceFileVersion kTraceFileVersion{0};

struct alignas(8) Header {
  TraceFileVersion version;  // IMPORTANT: Keep `version` as the first property.
  SessionId session_id;
  ProcessId process_id;
  ThreadId thread_id;
  TimestampNanoseconds system_clock_timestamp;
  TimestampNanoseconds steady_clock_timestamp;
  EventCount event_count;
};

static_assert(sizeof(Header) == 56);

constexpr auto operator==(const Header& lhs, const Header& rhs) -> bool;
inline auto Serialize(Header header) -> std::array<char, sizeof(Header)>;
inline auto Deserialize(std::array<char, sizeof(Header)> serialized) -> Header;

class alignas(8) Event {
 public:
  enum class Type : EventType {
    kFunctionEntry = 1,
    kFunctionExit = 2,
  };

  TimestampNanoseconds steady_clock_timestamp;
  uint64 payload_1;
  EventType type;
  uint32 payload_2;
};

static_assert(sizeof(Event) == 24);

constexpr auto operator==(const Event& lhs, const Event& rhs) -> bool;
inline auto Serialize(Event event) -> std::array<char, sizeof(Event)>;
inline auto Deserialize(std::array<char, sizeof(Event)> serialized) -> Event;

constexpr auto operator==(const Header& lhs, const Header& rhs) -> bool {
  return lhs.version == rhs.version && lhs.session_id == rhs.session_id &&
         lhs.process_id == rhs.process_id && lhs.thread_id == rhs.thread_id &&
         lhs.system_clock_timestamp == rhs.system_clock_timestamp &&
         lhs.steady_clock_timestamp == rhs.steady_clock_timestamp &&
         lhs.event_count == rhs.event_count;
}

inline auto Serialize(const Header header) -> std::array<char, sizeof(Header)> {
  std::array<char, sizeof(Header)> serialized{};
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* serialized_header = reinterpret_cast<Header*>(serialized.data());
  serialized_header->version = absl::ghtonll(header.version);
  serialized_header->session_id = absl::ghtonll(header.session_id);
  serialized_header->process_id = absl::ghtonll(header.process_id);
  serialized_header->thread_id = absl::ghtonll(header.thread_id);
  serialized_header->system_clock_timestamp =
      absl::ghtonll(header.system_clock_timestamp);
  serialized_header->steady_clock_timestamp =
      absl::ghtonll(header.steady_clock_timestamp);
  serialized_header->event_count = absl::ghtonl(header.event_count);
  return serialized;
}

inline auto Deserialize(std::array<char, sizeof(Header)> serialized) -> Header {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* serialized_header = reinterpret_cast<Header*>(serialized.data());
  return Header{
      .version = absl::gntohll(serialized_header->version),
      .session_id = absl::gntohll(serialized_header->session_id),
      .process_id = gsl::narrow_cast<ProcessId>(
          absl::gntohll(serialized_header->process_id)),
      .thread_id = absl::gntohll(serialized_header->thread_id),
      .system_clock_timestamp = gsl::narrow_cast<TimestampNanoseconds>(
          absl::gntohll(serialized_header->system_clock_timestamp)),
      .steady_clock_timestamp = gsl::narrow_cast<TimestampNanoseconds>(
          absl::gntohll(serialized_header->steady_clock_timestamp)),
      .event_count = gsl::narrow_cast<EventCount>(
          absl::gntohl(serialized_header->event_count))};
}

constexpr auto operator==(const Event& lhs, const Event& rhs) -> bool {
  return lhs.steady_clock_timestamp == rhs.steady_clock_timestamp &&
         lhs.payload_1 == rhs.payload_1 && lhs.type == rhs.type &&
         lhs.payload_2 == rhs.payload_2;
}

inline auto Serialize(const Event event) -> std::array<char, sizeof(Event)> {
  std::array<char, sizeof(Event)> serialized{};
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* serialized_event = reinterpret_cast<Event*>(serialized.data());
  serialized_event->steady_clock_timestamp =
      absl::ghtonll(event.steady_clock_timestamp);
  serialized_event->payload_1 = absl::ghtonll(event.payload_1);
  const auto type =
      static_cast<std::underlying_type_t<Event::Type>>(event.type);
  serialized_event->type = absl::ghtonl(type);
  serialized_event->payload_2 = absl::ghtonl(event.payload_2);
  return serialized;
}

inline auto Deserialize(std::array<char, sizeof(Event)> serialized) -> Event {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* serialized_event = reinterpret_cast<Event*>(serialized.data());
  const auto type =
      static_cast<std::underlying_type_t<Event::Type>>(serialized_event->type);
  return Event{.steady_clock_timestamp = gsl::narrow_cast<TimestampNanoseconds>(
                   absl::gntohll(serialized_event->steady_clock_timestamp)),
               .payload_1 = absl::gntohll(serialized_event->payload_1),
               .type = absl::gntohl(type),
               .payload_2 = absl::gntohl(serialized_event->payload_2)};
}

}  // namespace spoor::runtime::trace
