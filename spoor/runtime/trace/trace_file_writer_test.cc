// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/trace/trace_file_writer.h"

#include "gmock/gmock.h"
#include "gsl/gsl"
#include "gtest/gtest.h"
#include "spoor/runtime/buffer/circular_slice_buffer.h"
#include "spoor/runtime/buffer/reserved_buffer_slice_pool.h"
#include "trace.h"
#include "util/compression/compressor.h"
#include "util/compression/compressor_factory.h"
#include "util/file_system/file_writer_mock.h"

namespace {

using spoor::runtime::trace::CompressionStrategy;
using spoor::runtime::trace::Event;
using spoor::runtime::trace::Header;
using spoor::runtime::trace::kEndianness;
using spoor::runtime::trace::kMagicNumber;
using spoor::runtime::trace::kTraceFileVersion;
using spoor::runtime::trace::TraceFileWriter;
using testing::_;
using testing::Return;
using util::compression::MakeCompressor;
using util::file_system::testing::FileWriterMock;
using CircularSliceBuffer = spoor::runtime::buffer::CircularSliceBuffer<Event>;
using Pool = spoor::runtime::buffer::ReservedBufferSlicePool<Event>;

MATCHER_P(MatchesHeader, expected_header, "") {  // NOLINT
  const gsl::span<const char> expected{
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      reinterpret_cast<const char*>(&expected_header), sizeof(expected_header)};
  return arg == expected;
}

MATCHER_P2(MatchesEvents, expected_events, compressor, "") {  // NOLINT
  const gsl::span<const char> expected{
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      reinterpret_cast<const char*>(expected_events.data()),
      expected_events.size() *
          sizeof(typename decltype(expected_events)::value_type)};
  const auto result = compressor->Uncompress(arg);
  if (result.IsErr()) return false;
  const auto uncompressed = result.Ok();
  return uncompressed == expected;
}

TEST(TraceFileWriter, Write) {  // NOLINT
std::cerr << __FILE__ << ':' << __LINE__ << '\n';
  const std::filesystem::path path{"/path/to/file.spoor"};
std::cerr << __FILE__ << ':' << __LINE__ << '\n';
  constexpr std::array<Event, 4> raw_events{{
      {.steady_clock_timestamp = 0, .payload_1 = 1, .type = 1, .payload_2 = 2},
      {.steady_clock_timestamp = 1, .payload_1 = 3, .type = 2, .payload_2 = 4},
      {.steady_clock_timestamp = 2, .payload_1 = 5, .type = 2, .payload_2 = 6},
      {.steady_clock_timestamp = 3, .payload_1 = 9, .type = 1, .payload_2 = 8},
  }};
std::cerr << __FILE__ << ':' << __LINE__ << '\n';
  Pool pool{
      {.max_slice_capacity = raw_events.size(), .capacity = raw_events.size()}};
std::cerr << __FILE__ << ':' << __LINE__ << '\n';
  CircularSliceBuffer events{
      {.buffer_slice_pool = &pool, .capacity = raw_events.size()}};
std::cerr << __FILE__ << ':' << __LINE__ << '\n';
  std::for_each(std::cbegin(raw_events), std ::cend(raw_events),
                [&events](const auto event) { events.Push(event); });
std::cerr << __FILE__ << ':' << __LINE__ << '\n';
  for (const auto compression_strategy : util::compression::kStrategies) {
std::cerr << __FILE__ << ':' << __LINE__ << '\n';
    const Header header{.magic_number = kMagicNumber,
                        .endianness = kEndianness,
                        .compression_strategy = compression_strategy,
                        .version = kTraceFileVersion,
                        .session_id = 1,
                        .process_id = 2,
                        .thread_id = 3,
                        .system_clock_timestamp = 4,
                        .steady_clock_timestamp = 5,
                        .event_count = raw_events.size()};
std::cerr << __FILE__ << ':' << __LINE__ << '\n';
    auto compressor =
        MakeCompressor(compression_strategy, raw_events.size() * sizeof(Event));
std::cerr << __FILE__ << ':' << __LINE__ << '\n';
    FileWriterMock file_writer{};
std::cerr << __FILE__ << ':' << __LINE__ << '\n';
    EXPECT_CALL(file_writer, Open(path, std::ios::trunc | std::ios::binary));
std::cerr << __FILE__ << ':' << __LINE__ << '\n';
    EXPECT_CALL(file_writer, IsOpen()).WillOnce(Return(true));
std::cerr << __FILE__ << ':' << __LINE__ << '\n';
    EXPECT_CALL(file_writer, Write(MatchesHeader(header)));
std::cerr << __FILE__ << ':' << __LINE__ << '\n';
    EXPECT_CALL(file_writer,
                Write(MatchesEvents(raw_events, compressor.get())));
std::cerr << __FILE__ << ':' << __LINE__ << '\n';
    EXPECT_CALL(file_writer, Close());
std::cerr << __FILE__ << ':' << __LINE__ << '\n';
    TraceFileWriter trace_file_writer{
        {.file_writer = &file_writer,
         .compression_strategy = compression_strategy,
         .initial_buffer_capacity = raw_events.size()}};
std::cerr << __FILE__ << ':' << __LINE__ << '\n';
    const auto result = trace_file_writer.Write(path, header, &events);
std::cerr << __FILE__ << ':' << __LINE__ << '\n';
    ASSERT_TRUE(result.IsOk());
std::cerr << __FILE__ << ':' << __LINE__ << '\n';
  }
std::cerr << __FILE__ << ':' << __LINE__ << '\n';
}

TEST(TraceFileWriter, SetsHeaderCompressionStrategy) {  // NOLINT
  const std::filesystem::path path{"/path/to/file.spoor"};
  constexpr std::array<Event, 1> raw_events{{{.steady_clock_timestamp = 0,
                                              .payload_1 = 1,
                                              .type = 2,
                                              .payload_2 = 3}}};
  constexpr Header input_header{
      .magic_number = kMagicNumber,
      .endianness = kEndianness,
      .compression_strategy = CompressionStrategy::kNone,
      .version = kTraceFileVersion,
      .session_id = 1,
      .process_id = 2,
      .thread_id = 3,
      .system_clock_timestamp = 4,
      .steady_clock_timestamp = 5,
      .event_count = raw_events.size()};
  constexpr auto compression_strategy{CompressionStrategy::kSnappy};
  constexpr Header expected_header{
      .magic_number = input_header.magic_number,
      .endianness = input_header.endianness,
      .compression_strategy = compression_strategy,
      .version = input_header.version,
      .session_id = input_header.session_id,
      .process_id = input_header.process_id,
      .thread_id = input_header.thread_id,
      .system_clock_timestamp = input_header.system_clock_timestamp,
      .steady_clock_timestamp = input_header.steady_clock_timestamp,
      .event_count = input_header.event_count};
  ASSERT_NE(input_header.compression_strategy,
            expected_header.compression_strategy);
  Pool pool{
      {.max_slice_capacity = raw_events.size(), .capacity = raw_events.size()}};
  CircularSliceBuffer events{
      {.buffer_slice_pool = &pool, .capacity = raw_events.size()}};
  std::for_each(std::cbegin(raw_events), std ::cend(raw_events),
                [&events](const auto event) { events.Push(event); });
  auto compressor =
      MakeCompressor(compression_strategy, raw_events.size() * sizeof(Event));
  FileWriterMock file_writer{};
  EXPECT_CALL(file_writer, Open(path, std::ios::trunc | std::ios::binary));
  EXPECT_CALL(file_writer, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(file_writer, Write(MatchesHeader(expected_header)));
  EXPECT_CALL(file_writer, Write(MatchesEvents(raw_events, compressor.get())));
  EXPECT_CALL(file_writer, Close());
  TraceFileWriter trace_file_writer{
      {.file_writer = &file_writer,
       .compression_strategy = CompressionStrategy::kSnappy,
       .initial_buffer_capacity = raw_events.size()}};
  const auto result = trace_file_writer.Write(path, input_header, &events);
  ASSERT_TRUE(result.IsOk());
}

TEST(TraceFileWriter, DoesNotWriteZeroEvents) {  // NOLINT
  const std::filesystem::path path{"/path/to/file.spoor"};
  constexpr auto compression_strategy{CompressionStrategy::kNone};
  constexpr std::array<Event, 0> raw_events{};
  constexpr Header header{.magic_number = kMagicNumber,
                          .endianness = kEndianness,
                          .compression_strategy = compression_strategy,
                          .version = kTraceFileVersion,
                          .session_id = 1,
                          .process_id = 2,
                          .thread_id = 3,
                          .system_clock_timestamp = 4,
                          .steady_clock_timestamp = 5,
                          .event_count = raw_events.size()};
  Pool pool{{.max_slice_capacity = 0, .capacity = 0}};
  CircularSliceBuffer events{{.buffer_slice_pool = &pool, .capacity = 0}};
  FileWriterMock file_writer{};
  EXPECT_CALL(file_writer, Open(_, _)).Times(0);
  EXPECT_CALL(file_writer, IsOpen()).Times(0);
  EXPECT_CALL(file_writer, Write(_)).Times(0);
  EXPECT_CALL(file_writer, Close()).Times(0);
  TraceFileWriter trace_file_writer{
      {.file_writer = &file_writer,
       .compression_strategy = compression_strategy,
       .initial_buffer_capacity = raw_events.size()}};
  const auto result = trace_file_writer.Write(path, header, &events);
  ASSERT_TRUE(result.IsOk());
}

TEST(TraceFileWriter, WritesUncompressedIfCompressedIsLarger) {  // NOLINT
  const std::filesystem::path path{"/path/to/file.spoor"};
  constexpr std::array<Event, 1> raw_events{
      {{.steady_clock_timestamp = 0x0123456789abcdef,
        .payload_1 = 0xfedcba9876543210,
        .type = 0xabab,
        .payload_2 = 0x1a2b3c4d}}};
  constexpr Header header{.magic_number = kMagicNumber,
                          .endianness = kEndianness,
                          .compression_strategy = CompressionStrategy::kNone,
                          .version = kTraceFileVersion,
                          .session_id = 1,
                          .process_id = 2,
                          .thread_id = 3,
                          .system_clock_timestamp = 4,
                          .steady_clock_timestamp = 5,
                          .event_count = raw_events.size()};
  Pool pool{
      {.max_slice_capacity = raw_events.size(), .capacity = raw_events.size()}};
  CircularSliceBuffer events{
      {.buffer_slice_pool = &pool, .capacity = raw_events.size()}};
  std::for_each(std::cbegin(raw_events), std ::cend(raw_events),
                [&events](const auto event) { events.Push(event); });
  auto compressor = MakeCompressor(CompressionStrategy::kNone,
                                   raw_events.size() * sizeof(Event));
  ASSERT_EQ(header.compression_strategy, compressor->Strategy());
  FileWriterMock file_writer{};
  EXPECT_CALL(file_writer, Open(path, std::ios::trunc | std::ios::binary));
  EXPECT_CALL(file_writer, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(file_writer, Write(MatchesHeader(header)));
  EXPECT_CALL(file_writer, Write(MatchesEvents(raw_events, compressor.get())));
  EXPECT_CALL(file_writer, Close());
  TraceFileWriter trace_file_writer{
      {.file_writer = &file_writer,
       .compression_strategy = CompressionStrategy::kSnappy,
       .initial_buffer_capacity = raw_events.size()}};
  const auto result = trace_file_writer.Write(path, header, &events);
  ASSERT_TRUE(result.IsOk());
}

TEST(TraceFileWriter, FailsIfFileCannotBeOpened) {  // NOLINT
  const std::filesystem::path path{"/path/to/bad/file.spoor"};
  constexpr auto compression_strategy{CompressionStrategy::kNone};
  constexpr std::array<Event, 1> raw_events{{{.steady_clock_timestamp = 0,
                                              .payload_1 = 1,
                                              .type = 2,
                                              .payload_2 = 3}}};
  constexpr Header header{.magic_number = kMagicNumber,
                          .endianness = kEndianness,
                          .compression_strategy = compression_strategy,
                          .version = kTraceFileVersion,
                          .session_id = 1,
                          .process_id = 2,
                          .thread_id = 3,
                          .system_clock_timestamp = 4,
                          .steady_clock_timestamp = 5,
                          .event_count = raw_events.size()};
  Pool pool{
      {.max_slice_capacity = raw_events.size(), .capacity = raw_events.size()}};
  CircularSliceBuffer events{
      {.buffer_slice_pool = &pool, .capacity = raw_events.size()}};
  std::for_each(std::cbegin(raw_events), std ::cend(raw_events),
                [&events](const auto event) { events.Push(event); });
  FileWriterMock file_writer{};
  EXPECT_CALL(file_writer, Open(path, std::ios::trunc | std::ios::binary));
  EXPECT_CALL(file_writer, IsOpen()).WillOnce(Return(false));
  EXPECT_CALL(file_writer, Write(_)).Times(0);
  EXPECT_CALL(file_writer, Close()).Times(0);
  TraceFileWriter trace_file_writer{
      {.file_writer = &file_writer,
       .compression_strategy = compression_strategy,
       .initial_buffer_capacity = raw_events.size()}};
  const auto result = trace_file_writer.Write(path, header, &events);
  ASSERT_TRUE(result.IsErr());
  ASSERT_EQ(result.Err(), TraceFileWriter::Error::kFailedToOpenFile);
}

}  // namespace
