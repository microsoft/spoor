// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Register `InjectRuntime` with LLVM's pass manager.

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <system_error>
#include <unordered_set>

#include "llvm/ADT/StringRef.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/raw_ostream.h"
#include "spoor/instrumentation/config/config.h"
#include "spoor/instrumentation/inject_runtime/inject_runtime.h"
#include "spoor/instrumentation/instrumentation.h"
#include "util/time/clock.h"

namespace spoor::instrumentation {

auto PluginInfo() -> llvm::PassPluginLibraryInfo {
  const auto callback = [](llvm::PassBuilder& pass_builder) {
    const auto pipeline_parsing_callback =
        [](llvm::StringRef pass_name, llvm::ModulePassManager& pass_manager,
           llvm::ArrayRef<llvm::PassBuilder::PipelineElement> /*unused*/) {
          if (pass_name != kPluginName.data()) return false;
          const auto config = config::Config::FromEnv();

          std::unordered_set<std::string> function_allow_list{};
          if (config.function_allow_list_file.has_value()) {
            const auto& file_path = config.function_allow_list_file.value();
            std::ifstream file_stream{file_path};
            if (!file_stream.is_open()) {
              llvm::WithColor::error();
              llvm::errs() << "Failed to parse the function allow list file '"
                           << file_path << "'.\n";
              exit(EXIT_FAILURE);
            }
            std::copy(std::istream_iterator<std::string>(file_stream),
                      std::istream_iterator<std::string>(),
                      std::inserter(function_allow_list,
                                    std::begin(function_allow_list)));
          }
          std::unordered_set<std::string> function_blocklist{};
          if (config.function_blocklist_file.has_value()) {
            const auto& file_path = config.function_allow_list_file.value();
            std::ifstream file_stream{file_path};
            if (!file_stream.is_open()) {
              llvm::WithColor::error();
              llvm::errs() << "Failed to open the function blocklist file '"
                           << file_path << "'.\n";
              exit(EXIT_FAILURE);
            }
            std::copy(std::istream_iterator<std::string>(file_stream),
                      std::istream_iterator<std::string>(),
                      std::inserter(function_blocklist,
                                    std::begin(function_blocklist)));
          }

          auto instrumented_function_map_output_stream =
              [](const llvm::StringRef file_path,
                 gsl::not_null<std::error_code*> error) {
                return std::make_unique<llvm::raw_fd_ostream>(file_path,
                                                              *error);
              };
          util::time::SystemClock system_clock{};
          pass_manager.addPass(inject_runtime::InjectRuntime({
              .instrumented_function_map_output_path =
                  config.instrumentation_map_output_path,
              .instrumented_function_map_output_stream =
                  std::move(instrumented_function_map_output_stream),
              .system_clock = &system_clock,
              .function_allow_list = function_allow_list,
              .function_blocklist = function_blocklist,
              .min_instruction_count_to_instrument =
                  config.min_instruction_threshold,
              .initialize_runtime = config.initialize_runtime,
              .enable_runtime = config.enable_runtime,
          }));
          return true;
        };
    pass_builder.registerPipelineParsingCallback(pipeline_parsing_callback);
  };
  return {.APIVersion = LLVM_PLUGIN_API_VERSION,
          .PluginName = kPluginName.data(),
          .PluginVersion = kPluginVersion.data(),
          .RegisterPassBuilderCallbacks = callback};
}

}  // namespace spoor::instrumentation

auto llvmGetPassPluginInfo() -> llvm::PassPluginLibraryInfo {
  return spoor::instrumentation::PluginInfo();
}
