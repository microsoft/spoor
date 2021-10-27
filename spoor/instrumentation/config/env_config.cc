// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/config/env_config.h"

#include <string>

#include "spoor/instrumentation/config/config.h"
#include "util/env/env.h"
#include "util/file_system/util.h"

namespace spoor::instrumentation::config {

using util::env::GetEnv;
using util::file_system::ExpandTilde;

auto ConfigFromEnv(const util::env::StdGetEnv& get_env) -> Config {
  constexpr auto empty_string_is_nullopt{true};
  auto filters_file = GetEnv(kFiltersFileKey, empty_string_is_nullopt, get_env);
  if (filters_file.has_value()) {
    filters_file = ExpandTilde(filters_file.value(), get_env);
  }
  const auto output_file =
      GetEnv(kOutputFileKey, empty_string_is_nullopt, get_env)
          .value_or(std::string{kOutputFileDefaultValue});
  const auto output_symbols_file =
      GetEnv(kOutputSymbolsFileKey, empty_string_is_nullopt, get_env)
          .value_or(std::string{kOutputSymbolsFileDefaultValue});
  return {
      .enable_runtime = GetEnv<bool>(kEnableRuntimeKey, get_env)
                            .value_or(kEnableRuntimeDefaultValue),
      .filters_file = filters_file,
      .force_binary_output = GetEnv<bool>(kForceBinaryOutputKey, get_env)
                                 .value_or(kForceBinaryOutputDefaultValue),
      .initialize_runtime = GetEnv<bool>(kInitializeRuntimeKey, get_env)
                                .value_or(kInitializeRuntimeDefaultValue),
      .inject_instrumentation =
          GetEnv<bool>(kInjectInstrumentationKey, get_env)
              .value_or(kInjectInstrumentationDefaultValue),
      .module_id = GetEnv(kModuleIdKey, empty_string_is_nullopt, get_env),
      .output_file = ExpandTilde(output_file, get_env),
      .output_symbols_file = ExpandTilde(output_symbols_file, get_env),
      .output_language =
          GetEnv(kOutputLanguageKey, kOutputLanguages, true, get_env)
              .value_or(kOutputLanguageDefaultValue),
  };
}

}  // namespace spoor::instrumentation::config
