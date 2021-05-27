// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/trace/trace_file_reader.h"

#include <algorithm>
#include <limits>
#include <string>
#include <system_error>

#include "absl/base/internal/endian.h"
#include "absl/strings/str_cat.h"
#include "gmock/gmock.h"
#include "gsl/gsl"
#include "gtest/gtest.h"
#include "util/compression/compressor_factory.h"
#include "util/file_system/file_reader_mock.h"
#include "util/file_system/file_system_mock.h"
#include "util/flat_map/flat_map.h"
#include "util/result.h"

namespace {

using spoor::runtime::trace::CompressionStrategy;
using spoor::runtime::trace::Endian;
using spoor::runtime::trace::Event;
using spoor::runtime::trace::EventCount;
using spoor::runtime::trace::Header;
using spoor::runtime::trace::kEndianness;
using spoor::runtime::trace::kMagicNumber;
using spoor::runtime::trace::kTraceFileVersion;
using spoor::runtime::trace::TraceFile;
using spoor::runtime::trace::TraceFileReader;
using spoor::runtime::trace::TraceFileVersion;
using testing::_;
using testing::Return;
using util::file_system::testing::FileReaderMock;
using util::file_system::testing::FileSystemMock;
using Error = spoor::runtime::trace::TraceFileReader::Error;

TEST(TraceFileReader, MatchesTraceFileConvention) {  // NOLINT
  const util::flat_map::FlatMap<std::filesystem::path, bool, 3>
      matching_paths_is_regular_file{{
          {"/path/to/"
           "ffffffffffffffff-ffffffffffffffff-ffffffffffffffff.spoor_trace",
           true},
          {"ffffffffffffffff-ffffffffffffffff-ffffffffffffffff.spoor_trace",
           true},
          {"a.spoor_trace", true},
      }};
  const util::flat_map::FlatMap<std::filesystem::path,
                                util::result::Result<bool, std::error_code>, 7>
      not_matching_paths_is_regular_file{{
          {"b.spoor", true},
          {"c.trace", true},
          {"d.spoor-trace", true},
          {"e.spoor_trace", false},
          {"f.spoor_trace", std::error_code{}},
          {"/path/spoor_trace/foo.bar", true},
          {"/path/trace.spoor_trace/foo.bar", true},
      }};
  FileSystemMock file_system{};
  EXPECT_CALL(file_system, IsRegularFile(_))
      .WillRepeatedly(
          [&](const auto& path) -> util::result::Result<bool, std::error_code> {
            const auto is_regular_file =
                matching_paths_is_regular_file.FirstValueForKey(path);
            if (is_regular_file.has_value()) return is_regular_file.value();
            return not_matching_paths_is_regular_file.FirstValueForKey(path)
                .value();
          });
  FileReaderMock file_reader{};
  TraceFileReader trace_file_reader{{
      .file_system = &file_system,
      .file_reader = &file_reader,
  }};
  std::for_each(
      std::cbegin(matching_paths_is_regular_file),
      std::cend(matching_paths_is_regular_file),
      [&](const auto& path_is_regular_file) {
        const auto& [path, _] = path_is_regular_file;
        ASSERT_TRUE(trace_file_reader.MatchesTraceFileConvention(path));
      });
  std::for_each(
      std::cbegin(not_matching_paths_is_regular_file),
      std::cend(not_matching_paths_is_regular_file),
      [&](const auto& path_is_regular_file) {
        const auto& [path, _] = path_is_regular_file;
        ASSERT_FALSE(trace_file_reader.MatchesTraceFileConvention(path));
      });
}

TEST(TraceFileReader, ReadHeaderParsesGoodData) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/trace.spoor_trace"};
  const Header header{
      .magic_number = kMagicNumber,
      .endianness = kEndianness,
      .compression_strategy = CompressionStrategy::kNone,
      .version = kTraceFileVersion,
      .session_id = 1,
      .process_id = 2,
      .thread_id = 3,
      .system_clock_timestamp = 4,
      .steady_clock_timestamp = 5,
      .event_count = 0,
      .padding = {},
  };
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  const std::string buffer{reinterpret_cast<const char*>(&header),
                           sizeof(header)};
  FileSystemMock file_system{};
  FileReaderMock file_reader{};
  EXPECT_CALL(file_reader, Open(file_path, std::ios::binary));
  EXPECT_CALL(file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(file_reader, Read(sizeof(Header))).WillOnce(Return(buffer));
  EXPECT_CALL(file_reader, Close());
  TraceFileReader trace_file_reader{{
      .file_system = &file_system,
      .file_reader = &file_reader,
  }};
  const auto result = trace_file_reader.ReadHeader(file_path);
  ASSERT_TRUE(result.IsOk());
  const auto& retrieved_header = result.Ok();
  ASSERT_EQ(retrieved_header, header);
}

TEST(TraceFileReader, ReadHeaderHandlesFailedToOpenFile) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/trace.spoor_trace"};
  FileSystemMock file_system{};
  FileReaderMock file_reader{};
  EXPECT_CALL(file_reader, Open(file_path, std::ios::binary));
  EXPECT_CALL(file_reader, IsOpen()).WillOnce(Return(false));
  TraceFileReader trace_file_reader{{
      .file_system = &file_system,
      .file_reader = &file_reader,
  }};
  const auto result = trace_file_reader.ReadHeader(file_path);
  ASSERT_TRUE(result.IsErr());
  const auto& error = result.Err();
  ASSERT_EQ(error, Error::kFailedToOpenFile);
}

TEST(TraceFileReader, ReadHeaderHandlesFileTooSmall) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/trace.spoor_trace"};
  const Header header{
      .magic_number = kMagicNumber,
      .endianness = kEndianness,
      .compression_strategy = CompressionStrategy::kNone,
      .version = kTraceFileVersion,
      .session_id = 1,
      .process_id = 2,
      .thread_id = 3,
      .system_clock_timestamp = 4,
      .steady_clock_timestamp = 5,
      .event_count = 0,
      .padding = {},
  };
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  const std::string buffer{reinterpret_cast<const char*>(&header),
                           sizeof(header) - 1};
  FileSystemMock file_system{};
  FileReaderMock file_reader{};
  EXPECT_CALL(file_reader, Open(file_path, std::ios::binary));
  EXPECT_CALL(file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(file_reader, Read(sizeof(Header))).WillOnce(Return(buffer));
  EXPECT_CALL(file_reader, Close());
  TraceFileReader trace_file_reader{{
      .file_system = &file_system,
      .file_reader = &file_reader,
  }};
  const auto result = trace_file_reader.ReadHeader(file_path);
  ASSERT_TRUE(result.IsErr());
  const auto& error = result.Err();
  ASSERT_EQ(error, Error::kCorruptData);
}

TEST(TraceFileReader, ReadHeaderHandlesBadMagicNumber) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/trace.spoor_trace"};
  const Header header{
      .magic_number = {},
      .endianness = kEndianness,
      .compression_strategy = CompressionStrategy::kNone,
      .version = kTraceFileVersion,
      .session_id = 1,
      .process_id = 2,
      .thread_id = 3,
      .system_clock_timestamp = 4,
      .steady_clock_timestamp = 5,
      .event_count = 0,
      .padding = {},
  };
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  const std::string buffer{reinterpret_cast<const char*>(&header),
                           sizeof(header)};
  FileSystemMock file_system{};
  FileReaderMock file_reader{};
  EXPECT_CALL(file_reader, Open(file_path, std::ios::binary));
  EXPECT_CALL(file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(file_reader, Read(sizeof(Header))).WillOnce(Return(buffer));
  EXPECT_CALL(file_reader, Close());
  TraceFileReader trace_file_reader{{
      .file_system = &file_system,
      .file_reader = &file_reader,
  }};
  const auto result = trace_file_reader.ReadHeader(file_path);
  ASSERT_TRUE(result.IsErr());
  const auto& error = result.Err();
  ASSERT_EQ(error, Error::kCorruptData);
}

TEST(TraceFileReader, ReadHeaderHandlesBadVersion) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/trace.spoor_trace"};
  const Header header{
      .magic_number = kMagicNumber,
      .endianness = kEndianness,
      .compression_strategy = CompressionStrategy::kNone,
      .version = std::numeric_limits<TraceFileVersion>::max(),
      .session_id = 1,
      .process_id = 2,
      .thread_id = 3,
      .system_clock_timestamp = 4,
      .steady_clock_timestamp = 5,
      .event_count = 0,
      .padding = {},
  };
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  const std::string buffer{reinterpret_cast<const char*>(&header),
                           sizeof(header)};
  FileSystemMock file_system{};
  FileReaderMock file_reader{};
  EXPECT_CALL(file_reader, Open(file_path, std::ios::binary));
  EXPECT_CALL(file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(file_reader, Read(sizeof(Header))).WillOnce(Return(buffer));
  EXPECT_CALL(file_reader, Close());
  TraceFileReader trace_file_reader{{
      .file_system = &file_system,
      .file_reader = &file_reader,
  }};
  const auto result = trace_file_reader.ReadHeader(file_path);
  ASSERT_TRUE(result.IsErr());
  const auto& error = result.Err();
  ASSERT_EQ(error, Error::kUnknownVersion);
}

TEST(TraceFileReader, ReadParsesGoodDataUncompressed) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/trace.spoor_trace"};
  const TraceFile trace_file{
      .header =
          {
              .magic_number = kMagicNumber,
              .endianness = kEndianness,
              .compression_strategy = CompressionStrategy::kNone,
              .version = kTraceFileVersion,
              .session_id = 1,
              .process_id = 2,
              .thread_id = 3,
              .system_clock_timestamp = 4,
              .steady_clock_timestamp = 5,
              .event_count = 3,
              .padding = {},
          },
      .events =
          {
              {.steady_clock_timestamp = 0,
               .payload_1 = 0,
               .type = 0,
               .payload_2 = 0},
              {.steady_clock_timestamp = 1,
               .payload_1 = 1,
               .type = 1,
               .payload_2 = 1},
              {.steady_clock_timestamp = 2,
               .payload_1 = 2,
               .type = 2,
               .payload_2 = 2},
          },
  };
  ASSERT_EQ(trace_file.header.event_count, trace_file.events.size());
  const std::string buffer = [&] {
    const std::string header_buffer{
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<const char*>(&trace_file.header), sizeof(Header)};
    const std::string events_buffer{
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<const char*>(trace_file.events.data()),
        trace_file.header.event_count * sizeof(Event)};
    return absl::StrCat(header_buffer, events_buffer);
  }();
  ASSERT_EQ(buffer.size(),
            sizeof(Header) + trace_file.events.size() * sizeof(Event));
  FileSystemMock file_system{};
  FileReaderMock file_reader{};
  EXPECT_CALL(file_reader, Open(file_path, std::ios::binary));
  EXPECT_CALL(file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(file_reader, Read()).WillOnce(Return(buffer));
  EXPECT_CALL(file_reader, Close());
  TraceFileReader trace_file_reader{{
      .file_system = &file_system,
      .file_reader = &file_reader,
  }};
  const auto result = trace_file_reader.Read(file_path);
  ASSERT_TRUE(result.IsOk());
  const auto& retrieved_trace_file = result.Ok();
  ASSERT_EQ(retrieved_trace_file, trace_file);
}

TEST(TraceFileReader, ReadParsesGoodDataCompressed) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/trace.spoor_trace"};
  constexpr auto compression_strategy = CompressionStrategy::kSnappy;
  const TraceFile trace_file{
      .header =
          {
              .magic_number = kMagicNumber,
              .endianness = kEndianness,
              .compression_strategy = compression_strategy,
              .version = kTraceFileVersion,
              .session_id = 1,
              .process_id = 2,
              .thread_id = 3,
              .system_clock_timestamp = 4,
              .steady_clock_timestamp = 5,
              .event_count = 3,
              .padding = {},
          },
      .events =
          {
              {.steady_clock_timestamp = 0,
               .payload_1 = 0,
               .type = 0,
               .payload_2 = 0},
              {.steady_clock_timestamp = 1,
               .payload_1 = 1,
               .type = 1,
               .payload_2 = 1},
              {.steady_clock_timestamp = 2,
               .payload_1 = 2,
               .type = 2,
               .payload_2 = 2},
          },
  };
  ASSERT_EQ(trace_file.header.event_count, trace_file.events.size());
  const std::string buffer = [&] {
    const std::string header_buffer{
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<const char*>(&trace_file.header), sizeof(Header)};
    auto compressor = util::compression::MakeCompressor(
        compression_strategy, trace_file.events.size() * sizeof(Event));
    const auto events_buffer_view =
        compressor
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            ->Compress({reinterpret_cast<const char*>(trace_file.events.data()),
                        trace_file.events.size() * sizeof(Event)})
            .Ok();
    const std::string events_buffer{events_buffer_view.data(),
                                    events_buffer_view.size()};
    return absl::StrCat(header_buffer, events_buffer);
  }();
  ASSERT_LE(buffer.size(),
            sizeof(Header) + trace_file.events.size() * sizeof(Event));
  FileSystemMock file_system{};
  FileReaderMock file_reader{};
  EXPECT_CALL(file_reader, Open(file_path, std::ios::binary));
  EXPECT_CALL(file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(file_reader, Read()).WillOnce(Return(buffer));
  EXPECT_CALL(file_reader, Close());
  TraceFileReader trace_file_reader{{
      .file_system = &file_system,
      .file_reader = &file_reader,
  }};
  const auto result = trace_file_reader.Read(file_path);
  ASSERT_TRUE(result.IsOk());
  const auto& retrieved_trace_file = result.Ok();
  ASSERT_EQ(retrieved_trace_file, trace_file);
}

TEST(TraceFileReader, ReadParsesGoodDataDifferentEndian) {  // NOLINT
  constexpr auto other_endianness{
      kEndianness == Endian::kLittle ? Endian::kBig : Endian::kLittle};
  const std::filesystem::path file_path{"/path/to/trace.spoor_trace"};
  const TraceFile expected_trace_file{
      .header =
          {
              .magic_number = kMagicNumber,
              .endianness = other_endianness,
              .compression_strategy = CompressionStrategy::kNone,
              .version = kTraceFileVersion,
              .session_id = 0x0011223344556677,
              .process_id = 0x0011223344556677,
              .thread_id = 0x0011223344556677,
              .system_clock_timestamp = 0x0011223344556677,
              .steady_clock_timestamp = 0x0011223344556677,
              .event_count = gsl::narrow_cast<EventCount>(1),
              .padding = {},
          },
      .events = {{
          .steady_clock_timestamp = 0x0011223344556677,
          .payload_1 = 0x0011223344556677,
          .type = 0x00112233,
          .payload_2 = 0x00112233,
      }},
  };
  ASSERT_EQ(expected_trace_file.header.event_count,
            expected_trace_file.events.size());
  const TraceFile trace_file_other_endian{
      .header =
          {
              .magic_number = kMagicNumber,
              .endianness = other_endianness,
              .compression_strategy = CompressionStrategy::kNone,
              .version = absl::gbswap_32(kTraceFileVersion),
              .session_id = 0x7766554433221100,
              .process_id = 0x7766554433221100,
              .thread_id = 0x7766554433221100,
              .system_clock_timestamp = 0x7766554433221100,
              .steady_clock_timestamp = 0x7766554433221100,
              .event_count = static_cast<EventCount>(absl::gbswap_32(1)),
              .padding = {},
          },
      .events = {{
          .steady_clock_timestamp = 0x7766554433221100,
          .payload_1 = 0x7766554433221100,
          .type = 0x33221100,
          .payload_2 = 0x33221100,
      }},
  };
  ASSERT_EQ(expected_trace_file.events.size(),
            trace_file_other_endian.events.size());
  const std::string buffer = [&] {
    const std::string header_buffer{
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<const char*>(&trace_file_other_endian.header),
        sizeof(Header)};
    const std::string events_buffer{
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<const char*>(trace_file_other_endian.events.data()),
        trace_file_other_endian.events.size() * sizeof(Event)};
    return absl::StrCat(header_buffer, events_buffer);
  }();
  ASSERT_EQ(
      buffer.size(),
      sizeof(Header) + trace_file_other_endian.events.size() * sizeof(Event));
  FileSystemMock file_system{};
  FileReaderMock file_reader{};
  EXPECT_CALL(file_reader, Open(file_path, std::ios::binary));
  EXPECT_CALL(file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(file_reader, Read()).WillOnce(Return(buffer));
  EXPECT_CALL(file_reader, Close());
  TraceFileReader trace_file_reader{{
      .file_system = &file_system,
      .file_reader = &file_reader,
  }};
  const auto result = trace_file_reader.Read(file_path);
  ASSERT_TRUE(result.IsOk());
  const auto& retrieved_trace_file = result.Ok();
  ASSERT_EQ(retrieved_trace_file.header, expected_trace_file.header);
}

TEST(TraceFileReader, ReadHandlesFailedToOpenFile) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/trace.spoor_trace"};
  FileSystemMock file_system{};
  FileReaderMock file_reader{};
  EXPECT_CALL(file_reader, Open(file_path, std::ios::binary));
  EXPECT_CALL(file_reader, IsOpen()).WillOnce(Return(false));
  TraceFileReader trace_file_reader{{
      .file_system = &file_system,
      .file_reader = &file_reader,
  }};
  const auto result = trace_file_reader.Read(file_path);
  ASSERT_TRUE(result.IsErr());
  const auto& error = result.Err();
  ASSERT_EQ(error, Error::kFailedToOpenFile);
}

TEST(TraceFileReader, ReadHandlesFileTooSmall) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/trace.spoor_trace"};
  const Header header{
      .magic_number = kMagicNumber,
      .endianness = kEndianness,
      .compression_strategy = CompressionStrategy::kNone,
      .version = kTraceFileVersion,
      .session_id = 1,
      .process_id = 2,
      .thread_id = 3,
      .system_clock_timestamp = 4,
      .steady_clock_timestamp = 5,
      .event_count = 0,
      .padding = {},
  };
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  const std::string buffer{reinterpret_cast<const char*>(&header),
                           sizeof(header) - 1};
  FileSystemMock file_system{};
  FileReaderMock file_reader{};
  EXPECT_CALL(file_reader, Open(file_path, std::ios::binary));
  EXPECT_CALL(file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(file_reader, Read()).WillOnce(Return(buffer));
  EXPECT_CALL(file_reader, Close());
  TraceFileReader trace_file_reader{{
      .file_system = &file_system,
      .file_reader = &file_reader,
  }};
  const auto result = trace_file_reader.Read(file_path);
  ASSERT_TRUE(result.IsErr());
  const auto& error = result.Err();
  ASSERT_EQ(error, Error::kCorruptData);
}

TEST(TraceFileReader, ReadHandlesBadMagicNumber) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/trace.spoor_trace"};
  const Header header{
      .magic_number = {},
      .endianness = kEndianness,
      .compression_strategy = CompressionStrategy::kNone,
      .version = kTraceFileVersion,
      .session_id = 1,
      .process_id = 2,
      .thread_id = 3,
      .system_clock_timestamp = 4,
      .steady_clock_timestamp = 5,
      .event_count = 0,
      .padding = {},
  };
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  const std::string buffer{reinterpret_cast<const char*>(&header),
                           sizeof(header)};
  FileSystemMock file_system{};
  FileReaderMock file_reader{};
  EXPECT_CALL(file_reader, Open(file_path, std::ios::binary));
  EXPECT_CALL(file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(file_reader, Read()).WillOnce(Return(buffer));
  EXPECT_CALL(file_reader, Close());
  TraceFileReader trace_file_reader{{
      .file_system = &file_system,
      .file_reader = &file_reader,
  }};
  const auto result = trace_file_reader.Read(file_path);
  ASSERT_TRUE(result.IsErr());
  const auto& error = result.Err();
  ASSERT_EQ(error, Error::kCorruptData);
}

TEST(TraceFileReader, ReadHandlesBadVersion) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/trace.spoor_trace"};
  const Header header{
      .magic_number = kMagicNumber,
      .endianness = kEndianness,
      .compression_strategy = CompressionStrategy::kNone,
      .version = std::numeric_limits<TraceFileVersion>::max(),
      .session_id = 1,
      .process_id = 2,
      .thread_id = 3,
      .system_clock_timestamp = 4,
      .steady_clock_timestamp = 5,
      .event_count = 0,
      .padding = {},
  };
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  const std::string buffer{reinterpret_cast<const char*>(&header),
                           sizeof(header)};
  FileSystemMock file_system{};
  FileReaderMock file_reader{};
  EXPECT_CALL(file_reader, Open(file_path, std::ios::binary));
  EXPECT_CALL(file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(file_reader, Read()).WillOnce(Return(buffer));
  EXPECT_CALL(file_reader, Close());
  TraceFileReader trace_file_reader{{
      .file_system = &file_system,
      .file_reader = &file_reader,
  }};
  const auto result = trace_file_reader.Read(file_path);
  ASSERT_TRUE(result.IsErr());
  const auto& error = result.Err();
  ASSERT_EQ(error, Error::kUnknownVersion);
}

TEST(TraceFileReader, ReadHandlesBadCompression) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/trace.spoor_trace"};
  constexpr Header header{
      .magic_number = kMagicNumber,
      .endianness = kEndianness,
      .compression_strategy = CompressionStrategy::kSnappy,
      .version = kTraceFileVersion,
      .session_id = 1,
      .process_id = 2,
      .thread_id = 3,
      .system_clock_timestamp = 4,
      .steady_clock_timestamp = 5,
      .event_count = 6,
      .padding = {},
  };
  const std::string buffer = [&] {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const std::string header_buffer{reinterpret_cast<const char*>(&header),
                                    sizeof(Header)};
    return absl::StrCat(header_buffer, "bad compression");
  }();
  FileSystemMock file_system{};
  FileReaderMock file_reader{};
  EXPECT_CALL(file_reader, Open(file_path, std::ios::binary));
  EXPECT_CALL(file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(file_reader, Read()).WillOnce(Return(buffer));
  EXPECT_CALL(file_reader, Close());
  TraceFileReader trace_file_reader{{
      .file_system = &file_system,
      .file_reader = &file_reader,
  }};
  const auto result = trace_file_reader.Read(file_path);
  ASSERT_TRUE(result.IsErr());
  const auto& error = result.Err();
  ASSERT_EQ(error, Error::kCorruptData);
}

TEST(TraceFileReader, ReadHandlesWrongNumberOfEvents) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/trace.spoor_trace"};
  std::vector<Event> events{
      {.steady_clock_timestamp = 0, .payload_1 = 0, .type = 0, .payload_2 = 0},
      {.steady_clock_timestamp = 1, .payload_1 = 1, .type = 1, .payload_2 = 1},
      {.steady_clock_timestamp = 2, .payload_1 = 2, .type = 2, .payload_2 = 2},
  };
  for (const auto event_count : {events.size() - 1, events.size() + 1}) {
    const TraceFile trace_file{
        .header =
            {
                .magic_number = kMagicNumber,
                .endianness = kEndianness,
                .compression_strategy = CompressionStrategy::kNone,
                .version = kTraceFileVersion,
                .session_id = 1,
                .process_id = 2,
                .thread_id = 3,
                .system_clock_timestamp = 4,
                .steady_clock_timestamp = 5,
                .event_count = gsl::narrow_cast<EventCount>(event_count),
                .padding = {},
            },
        .events = events,
    };
    const std::string buffer = [&] {
      const std::string header_buffer{
          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
          reinterpret_cast<const char*>(&trace_file.header), sizeof(Header)};
      const std::string events_buffer{
          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
          reinterpret_cast<const char*>(trace_file.events.data()),
          trace_file.events.size() * sizeof(Event)};
      return absl::StrCat(header_buffer, events_buffer);
    }();
    ASSERT_EQ(buffer.size(),
              sizeof(Header) + trace_file.events.size() * sizeof(Event));
    FileSystemMock file_system{};
    FileReaderMock file_reader{};
    EXPECT_CALL(file_reader, Open(file_path, std::ios::binary));
    EXPECT_CALL(file_reader, IsOpen()).WillOnce(Return(true));
    EXPECT_CALL(file_reader, Read()).WillOnce(Return(buffer));
    EXPECT_CALL(file_reader, Close());
    TraceFileReader trace_file_reader{{
        .file_system = &file_system,
        .file_reader = &file_reader,
    }};
    const auto result = trace_file_reader.Read(file_path);
    ASSERT_TRUE(result.IsErr());
    const auto& error = result.Err();
    ASSERT_EQ(error, Error::kCorruptData);
  }
}

}  // namespace
