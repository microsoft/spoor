// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <istream>
#include <memory>
#include <unordered_set>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/flags/usage_config.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "gsl/gsl"
#include "spoor/instrumentation/instrumentation.h"
#include "spoor/instrumentation/symbols/symbols_file_reader.h"
#include "spoor/instrumentation/symbols/symbols_utility.h"
#include "spoor/runtime/trace/trace.h"
#include "spoor/runtime/trace/trace_file_reader.h"
#include "spoor/runtime/trace/trace_reader.h"
#include "spoor/tools/config/command_line_config.h"
#include "spoor/tools/config/config.h"
#include "spoor/tools/serialization/serialize.h"
#include "spoor/tools/tools.h"
#include "util/file_system/local_file_reader.h"
#include "util/file_system/local_file_system.h"
#include "util/file_system/local_file_writer.h"

namespace {

using spoor::instrumentation::kSymbolsFileExtension;
using spoor::instrumentation::symbols::ReduceSymbols;
using spoor::instrumentation::symbols::Symbols;
using spoor::instrumentation::symbols::SymbolsFileReader;
using spoor::instrumentation::symbols::SymbolsReader;
using spoor::runtime::trace::kTraceFileExtension;
using spoor::runtime::trace::TraceFile;
using spoor::runtime::trace::TraceFileReader;
using spoor::runtime::trace::TraceReader;
using spoor::tools::config::ConfigFromCommandLine;
using spoor::tools::serialization::OutputFormat;
using spoor::tools::serialization::OutputFormatFromConfig;
using spoor::tools::serialization::OutputFormatFromConfigError;
using spoor::tools::serialization::SerializeToOstream;
using util::file_system::LocalFileReader;
using util::file_system::LocalFileSystem;
using util::file_system::LocalFileWriter;

constexpr std::string_view kVersion{"%s %s\n"};
constexpr std::string_view kUsage{
    "Parse and symbolize Spoor traces.\n\n"
    "USAGE: %1$s [options...] <search_paths...>"};

}  // namespace

auto main(const int argc, char** argv) -> int {
  const auto short_program_invocation_name = [argc, argv] {
    const gsl::span<char*> args{argv,
                                static_cast<gsl::span<char*>::size_type>(argc)};
    const auto program_invocation_name = std::filesystem::path{args.front()};
    return program_invocation_name.filename();
  }();
  const auto show_help = [](const absl::string_view source) {
    std::string string{source};
    absl::AsciiStrToLower(&string);
    return absl::StrContains(string, "spoor") ||
           absl::StrContains(string, "usage");
  };
  absl::SetFlagsUsageConfig({
      .contains_helpshort_flags = show_help,
      .contains_help_flags = show_help,
      .version_string =
          [&] {
            return absl::StrFormat(kVersion, short_program_invocation_name,
                                   spoor::tools::kVersion);
          },
  });
  absl::SetProgramUsageMessage(
      absl::StrFormat(kUsage, short_program_invocation_name));

  const auto [config, positional_args] = ConfigFromCommandLine(argc, argv);

  if (positional_args.size() < 2) {
    std::cerr << "error: Expected at least one input source.\n\n"
              << absl::ProgramUsageMessage() << "\n\n"
              << "Try --help to list available flags.\n";
    return EXIT_FAILURE;
  }
  const gsl::span<char* const> input_sources{
      &(*std::next(std::cbegin(positional_args))),
      &(*std::cend(positional_args))};

  LocalFileWriter output_file_writer{};
  output_file_writer.Open(config.output_file);
  if (!output_file_writer.IsOpen()) {
    std::cerr << "error: Failed to open the output file \""
              << config.output_file << "\".\n";
    return EXIT_FAILURE;
  }

  const auto output_format_result = OutputFormatFromConfig(config);
  if (output_format_result.IsErr()) {
    switch (output_format_result.Err()) {
      case OutputFormatFromConfigError::kUnknownFileExtension: {
        std::cerr << "error: Cannot infer the output format from the output "
                     "file's extension.\n";
        return EXIT_FAILURE;
      }
    }
  }
  const auto output_format = output_format_result.Ok();

  LocalFileSystem file_system{};
  std::vector<std::filesystem::path> trace_file_paths{};
  std::vector<std::filesystem::path> symbols_file_paths{};
  {
    std::unordered_set<std::string> visited{};
    for (const auto& path : input_sources) {
      auto visit_file = [&](const std::filesystem::path& path) {
        auto [_, inserted] = visited.emplace(path.string());
        if (!inserted || !file_system.IsRegularFile(path).OkOr(false)) return;
        if (output_format == OutputFormat::kPerfettoProto &&
            path.extension() == absl::StrCat(".", kTraceFileExtension)) {
          trace_file_paths.emplace_back(path);
        }
        if (path.extension() == absl::StrCat(".", kSymbolsFileExtension)) {
          symbols_file_paths.emplace_back(path);
        }
      };
      if (file_system.IsDirectory(path).OkOr(false)) {
        std::filesystem::recursive_directory_iterator iterator{path};
        for (const auto& entry : iterator) {
          visit_file(entry.path());
        }
      } else {
        visit_file(path);
      }
    }
  }

  std::vector<TraceFile> trace_files{};
  trace_files.reserve(trace_file_paths.size());
  auto trace_file_reader = TraceFileReader{{
      .file_system{std::make_unique<LocalFileSystem>()},
      .file_reader{std::make_unique<LocalFileReader>()},
  }};
  for (const auto& trace_file_path : trace_file_paths) {
    auto result = trace_file_reader.Read(trace_file_path);
    if (result.IsErr()) {
      switch (result.Err()) {
        case TraceReader::Error::kFailedToOpenFile: {
          std::cerr << "error:: Failed to open the trace file "
                    << trace_file_path << ".\n";
        }
        case TraceReader::Error::kUnknownVersion: {
          std::cerr << "error: Failed to parse the trace file "
                    << trace_file_path << ". Unknown version.\n";
        }
        case TraceReader::Error::kCorruptData: {
          std::cerr << "error: Failed to parse the trace file "
                    << trace_file_path << ". The file is corrupt.\n";
        }
      }
      continue;
    }
    trace_files.emplace_back(std::move(result.Ok()));
  }
  std::sort(std::begin(trace_files), std::end(trace_files),
            [](const auto& lhs, const auto& rhs) {
              return lhs.header.steady_clock_timestamp <
                     rhs.header.steady_clock_timestamp;
            });

  Symbols symbols{};
  SymbolsFileReader symbols_file_reader{{
      .file_reader{std::make_unique<util::file_system::LocalFileReader>()},
  }};
  for (const auto& symbols_file_path : symbols_file_paths) {
    auto result = symbols_file_reader.Read(symbols_file_path);
    if (result.IsErr()) {
      switch (result.Err()) {
        case SymbolsReader::Error::kFailedToOpenFile: {
          std::cerr << "error: Failed to open the trace symbols file "
                    << symbols_file_path << ".\n";
        }
        case SymbolsReader::Error::kCorruptData: {
          std::cerr << "error: Failed to parse the symbols file "
                    << symbols_file_path << ". The file is corrupt.\n";
        }
      }
      continue;
    }
    auto& file_symbols = result.Ok();
    ReduceSymbols(&file_symbols, &symbols);
  }

  const auto result = SerializeToOstream(trace_files, symbols, output_format,
                                         &output_file_writer.Ostream());
  return result.IsOk() ? EXIT_SUCCESS : EXIT_FAILURE;
}
