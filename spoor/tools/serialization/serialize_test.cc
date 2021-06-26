// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/tools/serialization/serialize.h"

#include <ios>
#include <sstream>
#include <utility>
#include <vector>

#include "google/protobuf/util/time_util.h"
#include "gtest/gtest.h"
#include "protos/perfetto/trace/trace.pb.h"
#include "spoor/instrumentation/symbols/symbols.pb.h"
#include "spoor/runtime/trace/trace.h"
#include "spoor/tools/config/config.h"
#include "spoor/tools/serialization/csv/csv.h"

namespace {

using google::protobuf::util::TimeUtil;
using perfetto::protos::Trace;
using spoor::instrumentation::symbols::Symbols;
using spoor::runtime::trace::CompressionStrategy;
using spoor::runtime::trace::Event;
using spoor::runtime::trace::EventCount;
using spoor::runtime::trace::EventType;
using spoor::runtime::trace::FunctionId;
using spoor::runtime::trace::kEndianness;
using spoor::runtime::trace::kMagicNumber;
using spoor::runtime::trace::kTraceFileVersion;
using spoor::runtime::trace::TraceFile;
using spoor::tools::config::Config;
using spoor::tools::serialization::kOutputFormats;
using spoor::tools::serialization::OutputFormat;
using spoor::tools::serialization::OutputFormatFromConfig;
using spoor::tools::serialization::OutputFormatFromConfigError;
using spoor::tools::serialization::SerializeToOstream;
using spoor::tools::serialization::csv::kCsvDelimiter;
using ConfigOutputFormat = spoor::tools::config::OutputFormat;
using SerializationOutputFormat = spoor::tools::serialization::OutputFormat;

TEST(OutputFormatFromConfig, ParsesOutputFormat) {  // NOLINT
  {
    const Config config{
        .output_file = "/path/to/file.perfetto",
        .output_format = ConfigOutputFormat::kAutomatic,
    };
    const auto output_format_result = OutputFormatFromConfig(config);
    ASSERT_TRUE(output_format_result.IsOk());
    const auto output_format = output_format_result.Ok();
    ASSERT_EQ(output_format, SerializationOutputFormat::kPerfettoProto);
  }
  {
    const Config config{
        .output_file = "/path/to/file.spoor_symbols",
        .output_format = ConfigOutputFormat::kAutomatic,
    };
    const auto output_format_result = OutputFormatFromConfig(config);
    ASSERT_TRUE(output_format_result.IsOk());
    const auto output_format = output_format_result.Ok();
    ASSERT_EQ(output_format, SerializationOutputFormat::kSpoorSymbolsProto);
  }
  {
    const Config config{
        .output_file = "/path/to/file.csv",
        .output_format = ConfigOutputFormat::kAutomatic,
    };
    const auto output_format_result = OutputFormatFromConfig(config);
    ASSERT_TRUE(output_format_result.IsOk());
    const auto output_format = output_format_result.Ok();
    ASSERT_EQ(output_format, SerializationOutputFormat::kSpoorSymbolsCsv);
  }
  {
    const Config config{
        .output_file = "/path/to/file.unknown_extension",
        .output_format = ConfigOutputFormat::kAutomatic,
    };
    const auto output_format_result = OutputFormatFromConfig(config);
    ASSERT_TRUE(output_format_result.IsErr());
    const auto error = output_format_result.Err();
    ASSERT_EQ(error, OutputFormatFromConfigError::kUnknownFileExtension);
  }
  {
    const Config config{
        .output_file = "/path/to/file.spoor_symbols",
        .output_format = ConfigOutputFormat::kPerfettoProto,
    };
    const auto output_format_result = OutputFormatFromConfig(config);
    ASSERT_TRUE(output_format_result.IsOk());
    const auto output_format = output_format_result.Ok();
    ASSERT_EQ(output_format, SerializationOutputFormat::kPerfettoProto);
  }
  {
    const Config config{
        .output_file = "/path/to/file.perfetto",
        .output_format = ConfigOutputFormat::kSpoorSymbolsProto,
    };
    const auto output_format_result = OutputFormatFromConfig(config);
    ASSERT_TRUE(output_format_result.IsOk());
    const auto output_format = output_format_result.Ok();
    ASSERT_EQ(output_format, SerializationOutputFormat::kSpoorSymbolsProto);
  }
}

TEST(SerializeToOstream, SerializesToOstream) {  // NOLINT
  constexpr FunctionId function_id{0};
  const auto trace_files = [&] {
    std::vector<Event> events{
        {
            .steady_clock_timestamp = 0,
            .payload_1 = function_id,
            .type = static_cast<EventType>(Event::Type::kFunctionEntry),
            .payload_2 = 0,
        },
        {
            .steady_clock_timestamp = 1,
            .payload_1 = function_id,
            .type = static_cast<EventType>(Event::Type::kFunctionExit),
            .payload_2 = 0,
        },
    };
    const auto event_count = events.size();
    return std::vector<TraceFile>{{
        .header =
            {
                .magic_number = kMagicNumber,
                .endianness = kEndianness,
                .compression_strategy = CompressionStrategy::kNone,
                .version = kTraceFileVersion,
                .session_id = 1,
                .process_id = 2,
                .thread_id = 3,
                .system_clock_timestamp = 0,
                .steady_clock_timestamp = 0,
                .event_count = gsl::narrow_cast<EventCount>(event_count),
                .padding = {},
            },
        .events = std::move(events),
    }};
  }();
  const auto symbols = [&] {
    Symbols symbols{};
    auto& function_symbols_table = *symbols.mutable_function_symbols_table();
    auto& repeated_function_infos = function_symbols_table[function_id];
    auto* function_info = repeated_function_infos.add_function_infos();
    function_info->set_module_id("module_id");
    function_info->set_linkage_name("function_a");
    function_info->set_demangled_name("FunctionA()");
    function_info->set_file_name("file_a.source");
    function_info->set_directory("/path/to/a/");
    function_info->set_line(1);
    function_info->set_instrumented(true);
    *function_info->mutable_created_at() = TimeUtil::NanosecondsToTimestamp(0);
    return symbols;
  }();
  {
    std::stringstream buffer{};
    const auto result = SerializeToOstream(
        trace_files, symbols, OutputFormat::kPerfettoProto, &buffer);
    ASSERT_TRUE(result.IsOk());
    Trace parsed_trace{};
    const auto trace_success = parsed_trace.ParseFromIstream(&buffer);
    ASSERT_TRUE(trace_success);
  }
  {
    std::stringstream buffer{};
    const auto result = SerializeToOstream(
        trace_files, symbols, OutputFormat::kSpoorSymbolsProto, &buffer);
    ASSERT_TRUE(result.IsOk());
    Symbols parsed_symbols{};
    const auto symbols_success = parsed_symbols.ParseFromIstream(&buffer);
    ASSERT_TRUE(symbols_success);
  }
  {
    std::stringstream buffer{};
    const auto result = SerializeToOstream(
        trace_files, symbols, OutputFormat::kSpoorSymbolsCsv, &buffer);
    ASSERT_TRUE(result.IsOk());
    ASSERT_NE(buffer.str().find(kCsvDelimiter), std::string::npos);
  }
  for (const auto output_format : kOutputFormats) {
    std::stringstream buffer{};
    buffer.setstate(std::ios::failbit);
    const auto result =
        SerializeToOstream(trace_files, symbols, output_format, &buffer);
    ASSERT_TRUE(result.IsErr());
  }
}

}  // namespace
