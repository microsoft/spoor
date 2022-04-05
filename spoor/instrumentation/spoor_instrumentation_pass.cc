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
#include "spoor/instrumentation/config/config.h"
#include "spoor/instrumentation/config/env_source.h"
#include "spoor/instrumentation/config/file_source.h"
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

namespace spoor::instrumentation {

using filters::FiltersFileReader;
using spoor::instrumentation::config::Config;
using spoor::instrumentation::config::EnvSource;
using spoor::instrumentation::config::FileSource;
using symbols::SymbolsFileWriter;
using util::file_system::LocalFileReader;
using util::file_system::LocalFileSystem;
using util::file_system::LocalFileWriter;
using util::file_system::PathExpansionOptions;
using ConfigSource = spoor::instrumentation::config::Source;

auto PluginInfo() -> llvm::PassPluginLibraryInfo {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  const auto callback = [](llvm::PassBuilder& pass_builder) {
    const auto pipeline_parsing_callback =
        [](llvm::StringRef pass_name, llvm::ModulePassManager& pass_manager,
           llvm::ArrayRef<llvm::PassBuilder::PipelineElement> /*unused*/) {
          if (pass_name != kPluginName.data()) return false;

          const PathExpansionOptions path_expansion_options{
              .get_env = std::getenv,
              .expand_tilde = true,
              .expand_environment_variables = true,
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
          const auto config_file_path =
              FileSource::FindConfigFile(current_path.Ok());
          if (config_file_path.IsOk() && config_file_path.Ok().has_value()) {
            FileSource::Options file_source_options{
                .file_reader{std::make_unique<LocalFileReader>()},
                .path_expansion_options{path_expansion_options},
                .file_path{config_file_path.Ok().value()},
            };
            sources.emplace_back(
                std::make_unique<FileSource>(std::move(file_source_options)));
          }
          const auto config = Config::FromSourcesOrDefault(std::move(sources),
                                                           Config::Default());

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
