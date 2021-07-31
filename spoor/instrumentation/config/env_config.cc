// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/config/env_config.h"

#include <string>

#include "spoor/instrumentation/config/config.h"
#include "util/env/env.h"
#include "util/file_system/util.h"

namespace spoor::instrumentation::config {

using util::env::GetEnvOrDefault;
using util::file_system::ExpandTilde;

auto ConfigFromEnv(const util::env::GetEnv& get_env) -> Config {
  auto filters_file = GetEnvOrDefault(kFiltersFileKey.data(),
                                      kFiltersFileDefaultValue, true, get_env);
  if (filters_file.has_value()) {
    filters_file = ExpandTilde(filters_file.value(), get_env);
  }
  const auto output_file = GetEnvOrDefault(
      kOutputFileKey.data(), std::string{kOutputFileDefaultValue}, get_env);
  const auto output_symbols_file =
      GetEnvOrDefault(kOutputSymbolsFileKey.data(),
                      std::string{kOutputSymbolsFileDefaultValue}, get_env);
  return {.enable_runtime = GetEnvOrDefault(
              kEnableRuntimeKey.data(), kEnableRuntimeDefaultValue, get_env),
          .filters_file = filters_file,
          .force_binary_output =
              GetEnvOrDefault(kForceBinaryOutputKey.data(),
                              kForceBinaryOutputDefaultValue, get_env),
          .initialize_runtime =
              GetEnvOrDefault(kInitializeRuntimeKey.data(),
                              kInitializeRuntimeDefaultValue, get_env),
          .inject_instrumentation =
              GetEnvOrDefault(kInjectInstrumentationKey.data(),
                              kInjectInstrumentationDefaultValue, get_env),
          .module_id = GetEnvOrDefault(kModuleIdKey.data(),
                                       kModuleIdDefaultValue, true, get_env),
          .output_file = ExpandTilde(output_file, get_env),
          .output_symbols_file = ExpandTilde(output_symbols_file, get_env),
          .output_language = GetEnvOrDefault(kOutputLanguageKey.data(),
                                             kOutputLanguageDefaultValue,
                                             kOutputLanguages, true, get_env)};
}

}  // namespace spoor::instrumentation::config
