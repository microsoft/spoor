// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <array>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_set>

#include "absl/flags/flag.h"
#include "absl/flags/usage.h"
#include "absl/flags/usage_config.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "gsl/gsl"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/raw_ostream.h"
#include "spoor/instrumentation/config/command_line_config.h"
#include "spoor/instrumentation/config/config.h"
#include "spoor/instrumentation/inject_instrumentation/inject_instrumentation.h"
#include "spoor/instrumentation/instrumentation.h"
#include "spoor/instrumentation/support/support.h"
#include "util/time/clock.h"

namespace {

using spoor::instrumentation::config::ConfigFromCommandLineOrEnv;
using spoor::instrumentation::config::OutputLanguage;
using spoor::instrumentation::support::ReadLinesToSet;

constexpr std::string_view kStdinFileName{"-"};
constexpr std::string_view kUsage{
    "Transform LLVM Bitcode/IR by injecting Spoor instrumentation.\n\n"
    "USAGE: %1$s [options] [input_file]\n\n"
    "EXAMPLES\n"
    "$ %1$s source.bc --output_file=instrumented_source.bc\n"
    "$ clang++ source.cc -c -emit-llvm -o - | %1$s | "
    "clang++ -x ir - -lspoor_runtime\n\n"
    "Reads from stdin if an input file is not provided.\n"
    "Prints to stdout if an output file is not provided."};

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
          [] {
            return absl::StrFormat("v%s\n", spoor::instrumentation::kVersion);
          },
  });
  absl::SetProgramUsageMessage(
      absl::StrFormat(kUsage, short_program_invocation_name));

  const auto [config, positional_args] = ConfigFromCommandLineOrEnv(argc, argv);
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

  auto system_clock = std::make_unique<util::time::SystemClock>();

  auto output_function_map_stream =
      std::make_unique<std::ofstream>(config.output_function_map_file);
  if (!output_function_map_stream->is_open()) {
    llvm::WithColor::error();
    llvm::errs() << "Failed to create the function map file '"
                 << config.output_function_map_file << "'.\n";
    return EXIT_FAILURE;
  }

  std::unordered_set<std::string> function_allow_list{};
  if (config.function_allow_list_file.has_value()) {
    std::ifstream file{config.function_allow_list_file.value()};
    if (!file.is_open()) {
      llvm::WithColor::error();
      llvm::errs() << "Failed to read the function allow list file '"
                   << config.function_allow_list_file.value() << "'.\n";
      return EXIT_FAILURE;
    }
    function_allow_list = ReadLinesToSet(&file);
  }

  std::unordered_set<std::string> function_blocklist{};
  if (config.function_blocklist_file.has_value()) {
    std::ifstream file{config.function_blocklist_file.value()};
    if (!file.is_open()) {
      llvm::WithColor::error();
      llvm::errs() << "Failed to read the function allow list file '"
                   << config.function_blocklist_file.value() << "'.\n";
      return EXIT_FAILURE;
    }
    function_blocklist = ReadLinesToSet(&file);
  }

  spoor::instrumentation::inject_instrumentation::InjectInstrumentation
      inject_instrumentation{{
          .inject_instrumentation = config.inject_instrumentation,
          .output_function_map_stream = std::move(output_function_map_stream),
          .system_clock = std::move(system_clock),
          .function_allow_list = std::move(function_allow_list),
          .function_blocklist = std::move(function_blocklist),
          .module_id = config.module_id,
          .min_instruction_count_to_instrument =
              config.min_instruction_threshold,
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
