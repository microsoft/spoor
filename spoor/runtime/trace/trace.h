#pragma once

#include <chrono>
#include <vector>

#include "util/numeric.h"

namespace spoor::runtime::trace {

#if __WORDSIZE != 64
#error "Trace types are designed for 64-bit architectures."
#endif

using Crc = uint32;
using DurationNanoseconds = uint64;
using EventCount = uint32;
using EventTimestamp = std::chrono::time_point<std::chrono::steady_clock>;
using FunctionId = uint64;
using ProcessId = int64;  // pid_t is a signed integer
using SessionId = uint64;
using SystemTimestamp = std::chrono::time_point<std::chrono::system_clock>;
using ThreadId = uint64;
using TimestampNanoseconds = uint64;
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
};

static_assert(sizeof(Header) == 56);

class Event {
 public:
  enum class Type : bool {
    kFunctionExit = false,
    kFunctionEntry = true,
  };

  Event() = delete;
  Event(Type type, FunctionId function_id, TimestampNanoseconds timestamp);

  [[nodiscard]] auto GetType() const -> Type;
  [[nodiscard]] auto GetFunctionId() const -> FunctionId;
  [[nodiscard]] auto GetTimestamp() const -> TimestampNanoseconds;

 private:
  FunctionId function_id_;
  // Use the most significant bit for the event and the least significant 63
  // bits for the timestamp (in nanoseconds). The STL represents times as
  // signed integers but we only expect nonnegative values. Therefore, there is
  // no risk of data loss with this space-saving optimization.
  uint64 type_and_timestamp_;

  static auto MakeTypeAndTimestamp(Type type, TimestampNanoseconds timestamp)
      -> uint64;
};

static_assert(sizeof(Event) == 16);

struct Footer {
  const Crc checksum;
};

static_assert(sizeof(Footer) == 4);

struct File {
  Header header;
  std::vector<Event> events;
  Footer footer;
};

}  // namespace spoor::runtime::trace
