// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/config/config.h"

#include "spoor/common/config/util.h"
#include "spoor/instrumentation/config/config_private.h"
#include "spoor/instrumentation/config/output_language.h"

namespace spoor::instrumentation::config {

using spoor::common::util::ValueFromSourceOrDefault;

auto Config::Default() -> Config {
  return Config{
      .enable_runtime = kEnableRuntimeDefaultValue,
      .filters_file{kFiltersFileDefaultValue},
      .force_binary_output = kForceBinaryOutputDefaultValue,
      .initialize_runtime = kInitializeRuntimeDefaultValue,
      .inject_instrumentation = kInjectInstrumentationDefaultValue,
      .module_id = kModuleIdDefaultValue,
      .output_file{kOutputFileDefaultValue},
      .output_language = kOutputLanguageDefaultValue,
      .output_symbols_file{kOutputSymbolsFileDefaultValue},
  };
}

auto Config::FromSourcesOrDefault(
    std::vector<std::unique_ptr<Source>>&& sources,
    const Config& default_config) -> Config {
  return {
      .enable_runtime = ValueFromSourceOrDefault(
          sources, &Source::GetEnableRuntime, default_config.enable_runtime),
      .filters_file = ValueFromSourceOrDefault(sources, &Source::GetFiltersFile,
                                               default_config.filters_file),
      .force_binary_output =
          ValueFromSourceOrDefault(sources, &Source::GetForceBinaryOutput,
                                   default_config.force_binary_output),
      .initialize_runtime =
          ValueFromSourceOrDefault(sources, &Source::GetInitializeRuntime,
                                   default_config.initialize_runtime),
      .inject_instrumentation =
          ValueFromSourceOrDefault(sources, &Source::GetInjectInstrumentation,
                                   default_config.inject_instrumentation),
      .module_id = ValueFromSourceOrDefault(sources, &Source::GetModuleId,
                                            default_config.module_id),
      .output_file = ValueFromSourceOrDefault(sources, &Source::GetOutputFile,
                                              default_config.output_file),
      .output_language = ValueFromSourceOrDefault(
          sources, &Source::GetOutputLanguage, default_config.output_language),
      .output_symbols_file =
          ValueFromSourceOrDefault(sources, &Source::GetOutputSymbolsFile,
                                   default_config.output_symbols_file),
  };
}

auto operator==(const Config& lhs, const Config& rhs) -> bool {
  return lhs.enable_runtime == rhs.enable_runtime &&
         lhs.filters_file == rhs.filters_file &&
         lhs.force_binary_output == rhs.force_binary_output &&
         lhs.initialize_runtime == rhs.initialize_runtime &&
         lhs.inject_instrumentation == rhs.inject_instrumentation &&
         lhs.module_id == rhs.module_id && lhs.output_file == rhs.output_file &&
         lhs.output_language == rhs.output_language &&
         lhs.output_symbols_file == rhs.output_symbols_file;
}

}  // namespace spoor::instrumentation::config
