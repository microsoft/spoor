// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_set>
#include <utility>

#include "absl/flags/flag.h"
#include "absl/flags/usage.h"
#include "absl/flags/usage_config.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "gsl/gsl"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/raw_ostream.h"
#include "spoor/instrumentation/config/command_line.h"
#include "spoor/instrumentation/config/config.h"
#include "spoor/instrumentation/config/env_source.h"
#include "spoor/instrumentation/config/file_source.h"
#include "spoor/instrumentation/config/output_language.h"
#include "spoor/instrumentation/config/source.h"
#include "spoor/instrumentation/filters/filters_file_reader.h"
#include "spoor/instrumentation/inject_instrumentation/inject_instrumentation.h"
#include "spoor/instrumentation/instrumentation.h"
#include "spoor/instrumentation/symbols/symbols_file_writer.h"
#include "util/file_system/local_file_reader.h"
#include "util/file_system/local_file_system.h"
#include "util/file_system/local_file_writer.h"
#include "util/file_system/util.h"
#include "util/time/clock.h"

namespace {

using spoor::instrumentation::config::Config;
using spoor::instrumentation::config::ConfigFromCommandLineOrDefault;
using spoor::instrumentation::config::EnvSource;
using spoor::instrumentation::config::FileSource;
using spoor::instrumentation::config::OutputLanguage;
using spoor::instrumentation::filters::FiltersFileReader;
using spoor::instrumentation::symbols::SymbolsFileWriter;
using util::file_system::LocalFileReader;
using util::file_system::LocalFileSystem;
using util::file_system::LocalFileWriter;
using util::file_system::PathExpansionOptions;
using ConfigSource = spoor::instrumentation::config::Source;

constexpr auto kExpandTilde{true};
constexpr auto kExpandEnvironmentVariables{true};
constexpr std::string_view kStdinFileName{"-"};
constexpr std::string_view kUsage{
    "Transform LLVM Bitcode/IR by injecting Spoor instrumentation.\n\n"
    "USAGE: %1$s [options...] [input_file]\n\n"
    "EXAMPLES\n"
    "$ %1$s source.bc --output_file=instrumented_source.bc\n"
    "$ clang++ source.cc -c -emit-llvm -o - | %1$s | "
    "clang++ -x ir - -lspoor_runtime\n\n"
    "Reads from stdin if an input file is not provided.\n"
    "Prints to stdout if an output file is not provided."};
constexpr std::string_view kVersion{"%s %s\nBased on LLVM %d.%d.%d\n"};

}  // namespace

auto main(int argc, char** argv) -> int {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

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
                                   spoor::instrumentation::kVersion,
                                   LLVM_VERSION_MAJOR, LLVM_VERSION_MINOR,
                                   LLVM_VERSION_PATCH);
          },
  });
  absl::SetProgramUsageMessage(
      absl::StrFormat(kUsage, short_program_invocation_name));

  const PathExpansionOptions path_expansion_options{
      .get_env = std::getenv,
      .expand_tilde = kExpandTilde,
      .expand_environment_variables = kExpandEnvironmentVariables,
  };

  std::vector<std::unique_ptr<ConfigSource>> sources{};
  EnvSource::Options env_source_options{
      .path_expansion_options{path_expansion_options},
      .file_system{std::make_unique<LocalFileSystem>()},
      .get_env{std::getenv},
  };
  sources.emplace_back(
      std::make_unique<EnvSource>(std::move(env_source_options)));
  auto file_system = LocalFileSystem();
  auto current_path = file_system.CurrentPath();
  if (current_path.IsErr()) {
    llvm::WithColor::error();
    llvm::errs() << current_path.Err().message() << "\n\n"
                 << absl::ProgramUsageMessage() << "\n\n"
                 << "Try --help to list available flags.\n";
    return EXIT_FAILURE;
  }
  const auto config_file_result = FileSource::FindConfigFile(current_path.Ok());
  if (config_file_result.IsErr()) {
    llvm::WithColor::error();
    llvm::errs() << config_file_result.Err().message() << "\n\n"
                 << absl::ProgramUsageMessage() << "\n\n"
                 << "Try --help to list available flags.\n";
    return EXIT_FAILURE;
  }
  const auto& config_file_path = config_file_result.Ok();
  if (config_file_path.has_value()) {
    FileSource::Options file_source_options{
        .file_reader{std::make_unique<util::file_system::LocalFileReader>()},
        .path_expansion_options{path_expansion_options},
        .file_path{config_file_path.value()},
    };
    sources.emplace_back(
        std::make_unique<FileSource>(std::move(file_source_options)));
  }

  const auto [config, positional_args] = ConfigFromCommandLineOrDefault(
      argc, argv,
      Config::FromSourcesOrDefault(std::move(sources), Config::Default()),
      path_expansion_options);
  if (2 < positional_args.size()) {
    llvm::WithColor::error();
    llvm::errs() << "Expected at most one positional argument.\n\n"
                 << absl::ProgramUsageMessage() << "\n\n"
                 << "Try --help to list available flags.\n";
    return EXIT_FAILURE;
  }
  const auto* const input_file = [positional_args = positional_args] {
    if (positional_args.size() == 1) return kStdinFileName.data();
    return static_cast<const char*>(positional_args.back());
  }();

  llvm::SMDiagnostic parse_ir_file_diagnostic{};
  llvm::LLVMContext context{};
  const auto llvm_module =
      llvm::parseIRFile(input_file, parse_ir_file_diagnostic, context);
  if (llvm_module == nullptr) {
    parse_ir_file_diagnostic.print(short_program_invocation_name.c_str(),
                                   llvm::errs());
    return EXIT_FAILURE;
  }

  auto file_reader = std::make_unique<LocalFileReader>();
  auto filters_reader = std::make_unique<FiltersFileReader>(
      FiltersFileReader::Options{.file_reader = std::move(file_reader)});
  auto file_writer = std::make_unique<LocalFileWriter>();
  auto symbols_writer = std::make_unique<SymbolsFileWriter>(
      SymbolsFileWriter::Options{.file_writer = std::move(file_writer)});
  auto system_clock = std::make_unique<util::time::SystemClock>();

  spoor::instrumentation::inject_instrumentation::InjectInstrumentation
      inject_instrumentation{{
          .inject_instrumentation = config.inject_instrumentation,
          .filters_reader = std::move(filters_reader),
          .symbols_writer = std::move(symbols_writer),
          .system_clock = std::move(system_clock),
          .filters_file_path = config.filters_file,
          .symbols_file_path = config.output_symbols_file,
          .module_id = config.module_id,
          .initialize_runtime = config.initialize_runtime,
          .enable_runtime = config.enable_runtime,
      }};
  llvm::ModuleAnalysisManager module_analysis_manager{};
  inject_instrumentation.run(*llvm_module, module_analysis_manager);

  std::error_code create_ostream_error{};
  llvm::raw_fd_ostream ostream{config.output_file, create_ostream_error};
  if (create_ostream_error) {
    llvm::WithColor::error();
    llvm::errs() << create_ostream_error.message();
    return EXIT_FAILURE;
  }
  const auto binary_output = BinaryOutput(config.output_language);
  if (ostream.is_displayed() && binary_output && !config.force_binary_output) {
    const auto force_binary_output_flag = absl::StrFormat(
        "--%s",
        absl::GetFlagReflectionHandle(FLAGS_force_binary_output).Name());
    const auto output_language_flag_suggestion = absl::StrFormat(
        "--%s=%s", absl::GetFlagReflectionHandle(FLAGS_output_language).Name(),
        AbslUnparseFlag(OutputLanguage::kIr));
    llvm::WithColor::warning();
    llvm::errs() << "Attempting to print binary data to the console. This "
                    "might cause display problems. Pass "
                 << force_binary_output_flag
                 << " to suppress this warning or try "
                 << output_language_flag_suggestion
                 << " for a textual output.\n";
    return EXIT_FAILURE;
  }
  switch (config.output_language) {
    case OutputLanguage::kBitcode: {
      llvm::WriteBitcodeToFile(*llvm_module, ostream);
      break;
    }
    case OutputLanguage::kIr: {
      llvm::PrintModulePass print_module_pass{ostream};
      print_module_pass.run(*llvm_module, module_analysis_manager);
      break;
    }
  }

  return EXIT_SUCCESS;
}
