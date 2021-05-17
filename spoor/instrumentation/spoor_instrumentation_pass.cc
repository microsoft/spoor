// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Register `InjectInstrumentation` with LLVM's pass manager.

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <system_error>
#include <unordered_set>

#include "absl/strings/str_format.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/WithColor.h"
#include "spoor/instrumentation/config/env_config.h"
#include "spoor/instrumentation/inject_instrumentation/inject_instrumentation.h"
#include "spoor/instrumentation/instrumentation.h"
#include "spoor/instrumentation/support/support.h"
#include "util/time/clock.h"

namespace spoor::instrumentation {

auto PluginInfo() -> llvm::PassPluginLibraryInfo {
  const auto callback = [](llvm::PassBuilder& pass_builder) {
    const auto pipeline_parsing_callback =
        [](llvm::StringRef pass_name, llvm::ModulePassManager& pass_manager,
           llvm::ArrayRef<llvm::PassBuilder::PipelineElement> /*unused*/) {
          if (pass_name != kPluginName.data()) return false;
          const auto config = config::ConfigFromEnv();

          auto output_function_map_stream =
              std::make_unique<std::ofstream>(config.output_function_map_file);
          if (!output_function_map_stream->is_open()) {
            const auto message =
                absl::StrFormat("Failed to create the function map file '%s'.",
                                config.output_function_map_file);
            llvm::report_fatal_error(message, false);
          }

          std::unordered_set<std::string> function_allow_list{};
          if (config.function_allow_list_file.has_value()) {
            std::ifstream file{config.function_allow_list_file.value()};
            if (!file.is_open()) {
              const auto message = absl::StrFormat(
                  "Failed to read the function allow list file '%s'.",
                  config.function_allow_list_file.value());
              llvm::report_fatal_error(message, false);
            }
            function_allow_list = support::ReadLinesToSet(&file);
          }

          std::unordered_set<std::string> function_blocklist{};
          if (config.function_blocklist_file.has_value()) {
            std::ifstream file{config.function_blocklist_file.value()};
            if (!file.is_open()) {
              const auto message = absl::StrFormat(
                  "Failed to read the function blocklist file '%s'.",
                  config.function_blocklist_file.value());
              llvm::report_fatal_error(message, false);
            }
            function_blocklist = support::ReadLinesToSet(&file);
          }

          auto system_clock = std::make_unique<util::time::SystemClock>();
          pass_manager.addPass(inject_instrumentation::InjectInstrumentation{{
              .inject_instrumentation = config.inject_instrumentation,
              .output_function_map_stream =
                  std::move(output_function_map_stream),
              .system_clock = std::move(system_clock),
              .function_allow_list = std::move(function_allow_list),
              .function_blocklist = std::move(function_blocklist),
              .module_id = config.module_id,
              .min_instruction_count_to_instrument =
                  config.min_instruction_threshold,
              .initialize_runtime = config.initialize_runtime,
              .enable_runtime = config.enable_runtime,
          }});
          return true;
        };
    pass_builder.registerPipelineParsingCallback(pipeline_parsing_callback);
  };
  return {.APIVersion = LLVM_PLUGIN_API_VERSION,
          .PluginName = kPluginName.data(),
          .PluginVersion = kVersion.data(),
          .RegisterPassBuilderCallbacks = callback};
}

}  // namespace spoor::instrumentation

auto llvmGetPassPluginInfo() -> llvm::PassPluginLibraryInfo {
  return spoor::instrumentation::PluginInfo();
}
