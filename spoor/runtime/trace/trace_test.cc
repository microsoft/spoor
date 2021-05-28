// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/trace/trace.h"

#include "gtest/gtest.h"

namespace {

using spoor::runtime::trace::CompressionStrategy;
using spoor::runtime::trace::Endian;
using spoor::runtime::trace::Event;
using spoor::runtime::trace::EventType;
using spoor::runtime::trace::Header;
using spoor::runtime::trace::kMagicNumber;
using spoor::runtime::trace::TraceFile;
using Type = spoor::runtime::trace::Event::Type;

TEST(Header, Equality) {  // NOLINT
  constexpr Header header_a{
      .magic_number = kMagicNumber,
      .endianness = Endian::kLittle,
      .compression_strategy = CompressionStrategy::kNone,
      .version = 0,
      .session_id = 1,
      .process_id = 2,
      .thread_id = 3,
      .system_clock_timestamp = 4,
      .steady_clock_timestamp = 5,
      .event_count = 6,
  };
  constexpr Header header_b{
      .magic_number = kMagicNumber,
      .endianness = Endian::kLittle,
      .compression_strategy = CompressionStrategy::kNone,
      .version = 0,
      .session_id = 1,
      .process_id = 2,
      .thread_id = 3,
      .system_clock_timestamp = 4,
      .steady_clock_timestamp = 5,
      .event_count = 6,
  };
  constexpr Header header_c{
      .magic_number = kMagicNumber,
      .endianness = Endian::kBig,
      .compression_strategy = CompressionStrategy::kSnappy,
      .version = 10,
      .session_id = 11,
      .process_id = 12,
      .thread_id = 13,
      .system_clock_timestamp = 14,
      .steady_clock_timestamp = 15,
      .event_count = 16,
  };
  ASSERT_EQ(header_a, header_b);
  ASSERT_NE(header_a, header_c);
}

TEST(Event, Equality) {  // NOLINT
  constexpr Event event_a{
      .steady_clock_timestamp = 0, .payload_1 = 1, .type = 2, .payload_2 = 3};
  constexpr Event event_b{
      .steady_clock_timestamp = 0, .payload_1 = 1, .type = 2, .payload_2 = 3};
  constexpr Event event_c{.steady_clock_timestamp = 10,
                          .payload_1 = 11,
                          .type = 12,
                          .payload_2 = 13};
  ASSERT_EQ(event_a, event_b);
  ASSERT_NE(event_a, event_c);
}

TEST(TraceFile, Equality) {  // NOLINT
  const TraceFile trace_file_a{
      .header =
          {
              .magic_number = kMagicNumber,
              .endianness = Endian::kLittle,
              .compression_strategy = CompressionStrategy::kNone,
              .version = 0,
              .session_id = 1,
              .process_id = 2,
              .thread_id = 3,
              .system_clock_timestamp = 4,
              .steady_clock_timestamp = 5,
              .event_count = 1,
          },
      .events = {{
          .steady_clock_timestamp = 0,
          .payload_1 = 0,
          .type = 0,
          .payload_2 = 0,
      }},
  };
  const TraceFile trace_file_b{
      .header =
          {
              .magic_number = kMagicNumber,
              .endianness = Endian::kLittle,
              .compression_strategy = CompressionStrategy::kNone,
              .version = 0,
              .session_id = 1,
              .process_id = 2,
              .thread_id = 3,
              .system_clock_timestamp = 4,
              .steady_clock_timestamp = 5,
              .event_count = 1,
          },
      .events = {{
          .steady_clock_timestamp = 0,
          .payload_1 = 0,
          .type = 0,
          .payload_2 = 0,
      }},
  };
  const TraceFile trace_file_c{
      .header =
          {
              .magic_number = kMagicNumber,
              .endianness = Endian::kBig,
              .compression_strategy = CompressionStrategy::kSnappy,
              .version = 10,
              .session_id = 11,
              .process_id = 12,
              .thread_id = 13,
              .system_clock_timestamp = 14,
              .steady_clock_timestamp = 15,
              .event_count = 1,
          },
      .events = {{
          .steady_clock_timestamp = 1,
          .payload_1 = 1,
          .type = 1,
          .payload_2 = 1,
      }},
  };
  ASSERT_EQ(trace_file_a, trace_file_b);
  ASSERT_NE(trace_file_a, trace_file_c);
}

}  // namespace
