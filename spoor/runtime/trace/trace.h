// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <array>

#include "absl/base/internal/endian.h"
#include "gsl/gsl"
#include "util/numeric.h"

namespace spoor::runtime::trace {

using DurationNanoseconds = int64;
using EventCount = int32;
using FunctionId = uint64;
using ProcessId = int64;  // pid_t is a signed integer
using SessionId = uint64;
using ThreadId = uint64;
using TimestampNanoseconds = int64;
using TraceFileVersion = int64;

// IMPORTANT: Increment the version number if the header, event, or footer
// structure changes.
constexpr TraceFileVersion kTraceFileVersion{0};

struct alignas(8) Header {
  TraceFileVersion version;  // IMPORTANT: Keep `version` as the first property.
  SessionId session_id;
  ProcessId process_id;
  ThreadId thread_id;
  TimestampNanoseconds system_clock_timestamp;
  TimestampNanoseconds steady_clock_timestamp;
  EventCount event_count;
  std::array<std::byte, 4> padding;
};

static_assert(sizeof(Header) == 56);

class alignas(8) Event {
 public:
  enum class Type : bool {
    kFunctionExit = false,
    kFunctionEntry = true,
  };

  constexpr Event() = default;
  constexpr Event(Type type, FunctionId function_id,
                  TimestampNanoseconds timestamp);

  [[nodiscard]] constexpr auto GetType() const -> Type;
  [[nodiscard]] constexpr auto GetFunctionId() const -> FunctionId;
  [[nodiscard]] constexpr auto GetTimestamp() const -> TimestampNanoseconds;

 private:
  friend constexpr auto operator==(const Event& lhs, const Event& rhs) -> bool;
  friend inline auto Serialize(Event event) -> std::array<char, 16>;
  friend inline auto Deserialize(std::array<char, 16> serialized) -> Event;

  FunctionId function_id_;
  // Use the most significant bit for the event and the least significant 63
  // bits for the timestamp (in nanoseconds). The STL represents times as
  // signed integers but we only expect nonnegative values. Therefore, there is
  // no risk of data loss with this space-saving optimization.
  uint64 type_and_timestamp_;

  static constexpr auto MakeTypeAndTimestamp(Type type,
                                             TimestampNanoseconds timestamp)
      -> uint64;
};

static_assert(sizeof(Event) == 16);

struct alignas(1) Footer {
 private:
  friend constexpr auto operator==(const Footer& lhs, const Footer& rhs)
      -> bool;
};

static_assert(sizeof(Footer) == 1);

constexpr Event::Event(const Type type, const FunctionId function_id,
                       const TimestampNanoseconds timestamp)
    : function_id_{function_id},
      type_and_timestamp_{MakeTypeAndTimestamp(type, timestamp)} {}

constexpr auto Event::GetType() const -> Type {
  return static_cast<Type>(type_and_timestamp_ >> uint64{63});
}

constexpr auto Event::GetFunctionId() const -> FunctionId {
  return function_id_;
}

constexpr auto Event::GetTimestamp() const -> TimestampNanoseconds {
  return type_and_timestamp_ & ~(uint64{1} << uint64{63});
}

constexpr auto Event::MakeTypeAndTimestamp(const Type type,
                                           const TimestampNanoseconds timestamp)
    -> uint64 {
  auto type_and_timestamp = static_cast<uint64>(type) << uint64{63};
  type_and_timestamp |=
      gsl::narrow_cast<uint64>(timestamp) & uint64{0x7fffffffffffffff};
  return type_and_timestamp;
}

constexpr auto operator==(const Header& lhs, const Header& rhs) -> bool {
  return lhs.version == rhs.version && lhs.session_id == rhs.session_id &&
         lhs.process_id == rhs.process_id && lhs.thread_id == rhs.thread_id &&
         lhs.system_clock_timestamp == rhs.system_clock_timestamp &&
         lhs.steady_clock_timestamp == rhs.steady_clock_timestamp &&
         lhs.event_count == rhs.event_count;
}

constexpr auto operator==(const Event& lhs, const Event& rhs) -> bool {
  return lhs.function_id_ == rhs.function_id_ &&
         lhs.type_and_timestamp_ == rhs.type_and_timestamp_;
}

constexpr auto operator==(const Footer& /*unused*/, const Footer& /*unused*/)
    -> bool {
  return true;
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
  Header header{};
  header.version = absl::gntohll(serialized_header->version);
  header.session_id = absl::gntohll(serialized_header->session_id);
  header.process_id = absl::gntohll(serialized_header->process_id);
  header.thread_id = absl::gntohll(serialized_header->thread_id);
  header.system_clock_timestamp =
      absl::gntohll(serialized_header->system_clock_timestamp);
  header.steady_clock_timestamp =
      absl::gntohll(serialized_header->steady_clock_timestamp);
  header.event_count = absl::gntohl(serialized_header->event_count);
  return header;
}

inline auto Serialize(const Event event) -> std::array<char, sizeof(Event)> {
  std::array<char, sizeof(Event)> serialized{};
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* serialized_event = reinterpret_cast<Event*>(serialized.data());
  serialized_event->function_id_ = absl::ghtonll(event.function_id_);
  serialized_event->type_and_timestamp_ =
      absl::ghtonll(event.type_and_timestamp_);
  return serialized;
}

inline auto Deserialize(std::array<char, sizeof(Event)> serialized) -> Event {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* serialized_event = reinterpret_cast<Event*>(serialized.data());
  Event event{};
  event.function_id_ = absl::gntohll(serialized_event->function_id_);
  event.type_and_timestamp_ =
      absl::gntohll(serialized_event->type_and_timestamp_);
  return event;
}

inline auto Serialize(const Footer /*unused*/)
    -> std::array<char, sizeof(Footer)> {
  return {};
}

inline auto Deserialize(std::array<char, sizeof(Footer)> /*unused*/) -> Footer {
  return {};
}

}  // namespace spoor::runtime::trace
