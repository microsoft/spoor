// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <array>
#include <string_view>
#include <type_traits>
#include <vector>

#include "absl/base/internal/endian.h"
#include "gsl/gsl"
#include "util/compression/compressor.h"
#include "util/numeric.h"

namespace spoor::runtime::trace {

using CompressionStrategy = util::compression::Strategy;
using DurationNanoseconds = int64;
using EventCount = int32;
using EventType = uint32;
using FunctionId = uint64;
using MagicNumber = std::array<uint8, 10>;
using ProcessId = int64;  // pid_t is a signed integer
using SessionId = uint64;
using ThreadId = uint64;
using TimestampNanoseconds = int64;
using TraceFileVersion = uint32;

// IMPORTANT: Increment the version number if the Header or Event structure
// changes.
constexpr TraceFileVersion kTraceFileVersion{0};

constexpr std::string_view kTraceFileExtension{"spoor_trace"};

// Inspired by PNG's magic number.
constexpr MagicNumber kMagicNumber{
    {// This eight-bit signature was chosen because:
     // * By convention, setting the first byte's high bit signifies a binary
     //   file format (instead of a text file format).
     // * It is not an ASCII character.
     // * It is unique from the list of well-known binary file signatures.
     //   https://en.wikipedia.org/wiki/List_of_file_signatures
     0b1100'1000,
     // ASCII characters to easily identify the file format in a text editor.
     'S', 'p', 'o', 'o', 'r',
     // DOS-style line ending.
     '\r', '\n',
     // ASCII SUB control character that stops the file display under DOS when
     // the command type has been used as the end of file character.
     0x1a,
     // Unix-style line ending.
     '\n'}};

enum class Endian : uint8 {
  kLittle = 0,
  kBig = 1,
};

constexpr Endian kEndianness{
    absl::little_endian::IsLittleEndian() ? Endian::kLittle : Endian::kBig};

struct Header {
  MagicNumber magic_number{kMagicNumber};
  Endian endianness{kEndianness};
  CompressionStrategy compression_strategy;
  TraceFileVersion version{kTraceFileVersion};
  SessionId session_id;
  ProcessId process_id;
  ThreadId thread_id;
  TimestampNanoseconds system_clock_timestamp;
  TimestampNanoseconds steady_clock_timestamp;
  EventCount event_count;
  std::array<uint8, 4> padding{0};
};

static_assert(sizeof(Header) == 64);

constexpr auto operator==(const Header& lhs, const Header& rhs) -> bool;

// Aligning to 32 bytes (most efficient) results in an eight-byte space loss per
// event which is undesirable given the quantity of events generated.
struct alignas(8) Event {
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

struct TraceFile {
  Header header;
  std::vector<Event> events;
};

constexpr auto operator==(const TraceFile& lhs, const TraceFile& rhs) -> bool;

constexpr auto operator==(const Header& lhs, const Header& rhs) -> bool {
  return lhs.magic_number == rhs.magic_number &&
         lhs.endianness == rhs.endianness &&
         lhs.compression_strategy == rhs.compression_strategy &&
         lhs.version == rhs.version && lhs.session_id == rhs.session_id &&
         lhs.process_id == rhs.process_id && lhs.thread_id == rhs.thread_id &&
         lhs.system_clock_timestamp == rhs.system_clock_timestamp &&
         lhs.steady_clock_timestamp == rhs.steady_clock_timestamp &&
         lhs.event_count == rhs.event_count;
}

constexpr auto operator==(const Event& lhs, const Event& rhs) -> bool {
  return lhs.steady_clock_timestamp == rhs.steady_clock_timestamp &&
         lhs.payload_1 == rhs.payload_1 && lhs.type == rhs.type &&
         lhs.payload_2 == rhs.payload_2;
}

constexpr auto operator==(const TraceFile& lhs, const TraceFile& rhs) -> bool {
  return lhs.header == rhs.header && lhs.events == rhs.events;
}

}  // namespace spoor::runtime::trace
