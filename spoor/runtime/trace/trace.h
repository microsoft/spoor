#pragma once

#include <type_traits>
#include <vector>

#include "gsl/gsl"
#include "util/numeric.h"

namespace spoor::runtime::trace {

#if __WORDSIZE != 64
#error "Trace types are designed for 64-bit architectures."
#endif

using DurationNanoseconds = int64;
using EventCount = int32;
using FunctionId = uint64;
using ProcessId = int64;  // pid_t is a signed integer
using SessionId = uint64;
using ThreadId = uint64;
using TimestampNanoseconds = int64;
using TraceFileVersion = uint64;

// IMPORTANT: Update the version number if the header, event, or footer
// structure changes.
const TraceFileVersion kTraceFileVersion{0};

struct Header {
  // IMPORTANT: Keep `version` as the first property.
  TraceFileVersion version;
  SessionId session_id;
  ProcessId process_id;
  ThreadId thread_id;
  TimestampNanoseconds system_clock_timestamp;
  TimestampNanoseconds steady_clock_timestamp;
  EventCount event_count;

 private:
  friend constexpr auto operator==(const Header& lhs, const Header& rhs)
      -> bool;
};

static_assert(sizeof(Header) == 56);

class Event {
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

struct Footer {
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

auto constexpr operator==(const Header& lhs, const Header& rhs) -> bool {
  return lhs.version == rhs.version && lhs.session_id == rhs.session_id &&
         lhs.process_id == rhs.process_id && lhs.thread_id == rhs.thread_id &&
         lhs.system_clock_timestamp == rhs.system_clock_timestamp &&
         lhs.steady_clock_timestamp == rhs.steady_clock_timestamp &&
         lhs.event_count == rhs.event_count;
}

auto constexpr operator==(const Event& lhs, const Event& rhs) -> bool {
  return lhs.function_id_ == rhs.function_id_ &&
         lhs.type_and_timestamp_ == rhs.type_and_timestamp_;
}

auto constexpr operator==(const Footer&, const Footer&) -> bool { return true; }

}  // namespace spoor::runtime::trace
