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
#include "google/protobuf/stubs/common.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/WithColor.h"
#include "spoor/instrumentation/config/env_config.h"
#include "spoor/instrumentation/filters/filters_file_reader.h"
#include "spoor/instrumentation/inject_instrumentation/inject_instrumentation.h"
#include "spoor/instrumentation/instrumentation.h"
#include "spoor/instrumentation/symbols/symbols_file_writer.h"
#include "util/file_system/local_file_reader.h"
#include "util/file_system/local_file_writer.h"
#include "util/time/clock.h"

namespace spoor::instrumentation {

using filters::FiltersFileReader;
using symbols::SymbolsFileWriter;
using util::file_system::LocalFileReader;
using util::file_system::LocalFileWriter;

auto PluginInfo() -> llvm::PassPluginLibraryInfo {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  const auto callback = [](llvm::PassBuilder& pass_builder) {
    const auto pipeline_parsing_callback =
        [](llvm::StringRef pass_name, llvm::ModulePassManager& pass_manager,
           llvm::ArrayRef<llvm::PassBuilder::PipelineElement> /*unused*/) {
          if (pass_name != kPluginName.data()) return false;
          const auto config = config::ConfigFromEnv();

          auto file_reader = std::make_unique<LocalFileReader>();
          auto filters_reader =
              std::make_unique<FiltersFileReader>(FiltersFileReader::Options{
                  .file_reader = std::move(file_reader)});
          auto file_writer = std::make_unique<LocalFileWriter>();
          auto symbols_writer =
              std::make_unique<SymbolsFileWriter>(SymbolsFileWriter::Options{
                  .file_writer = std::move(file_writer)});
          auto system_clock = std::make_unique<util::time::SystemClock>();
          pass_manager.addPass(inject_instrumentation::InjectInstrumentation{{
              .inject_instrumentation = config.inject_instrumentation,
              .filters_reader = std::move(filters_reader),
              .symbols_writer = std::move(symbols_writer),
              .system_clock = std::move(system_clock),
              .filters_file_path = config.filters_file,
              .symbols_file_path = config.output_symbols_file,
              .module_id = config.module_id,
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
