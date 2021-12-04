// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/config/env_config.h"

#include <string>

#include "spoor/instrumentation/config/config.h"
#include "util/env/env.h"
#include "util/file_system/util.h"
#include "util/result.h"

namespace spoor::instrumentation::config {

using util::env::GetEnv;
using util::file_system::ExpandPath;
using util::file_system::PathExpansionOptions;
using util::result::None;

constexpr PathExpansionOptions kPathExpansionOptions{
    .expand_tilde = true,
    .expand_environment_variables = true,
};

auto ConfigFromEnv(const util::env::StdGetEnv& get_env) -> Config {
  constexpr auto empty_string_is_nullopt{true};
  // Temporarily returning an `Err` is okay because it immediately gets
  // converted to the default value -- the desired behavior if the key doesn't
  // exist.
  const auto enable_runtime =
      GetEnv<bool>(kEnableRuntimeKey, get_env)
          .value_or(util::result::Result<bool, None>::Err({}))
          .OkOr(kEnableRuntimeDefaultValue);
  const auto filters_file = [&]() -> std::optional<std::string> {
    const auto filters_file =
        GetEnv(kFiltersFileKey, empty_string_is_nullopt, get_env);
    if (!filters_file.has_value() || filters_file.value().IsErr()) return {};
    return ExpandPath(filters_file.value().Ok(), kPathExpansionOptions,
                      get_env);
  }();
  const auto force_binary_output =
      GetEnv<bool>(kForceBinaryOutputKey, get_env)
          .value_or(util::result::Result<bool, None>::Err({}))
          .OkOr(kForceBinaryOutputDefaultValue);
  const auto initialize_runtime =
      GetEnv<bool>(kInitializeRuntimeKey, get_env)
          .value_or(util::result::Result<bool, None>::Err({}))
          .OkOr(kInitializeRuntimeDefaultValue);
  const auto inject_instrumentation =
      GetEnv<bool>(kInjectInstrumentationKey, get_env)
          .value_or(util::result::Result<bool, None>::Err({}))
          .OkOr(kInjectInstrumentationDefaultValue);
  const auto module_id = [&]() -> std::optional<std::string> {
    const auto module_id =
        GetEnv(kModuleIdKey, empty_string_is_nullopt, get_env);
    if (!module_id.has_value() || module_id.value().IsErr()) return {};
    return module_id.value().Ok();
  }();
  const auto output_file =
      GetEnv(kOutputFileKey, empty_string_is_nullopt, get_env)
          .value_or(util::result::Result<std::string, None>::Err({}))
          .OkOr(std::string{kOutputFileDefaultValue});
  const auto output_symbols_file =
      GetEnv(kOutputSymbolsFileKey, empty_string_is_nullopt, get_env)
          .value_or(util::result::Result<std::string, None>::Err({}))
          .OkOr(std::string{kOutputSymbolsFileDefaultValue});
  const auto output_language =
      GetEnv(kOutputLanguageKey, kOutputLanguages, true, get_env)
          .value_or(util::result::Result<OutputLanguage, None>::Err({}))
          .OkOr(kOutputLanguageDefaultValue);
  return {
      .enable_runtime = enable_runtime,
      .filters_file = filters_file,
      .force_binary_output = force_binary_output,
      .initialize_runtime = initialize_runtime,
      .inject_instrumentation = inject_instrumentation,
      .module_id = module_id,
      .output_file = ExpandPath(output_file, kPathExpansionOptions, get_env),
      .output_symbols_file =
          ExpandPath(output_symbols_file, kPathExpansionOptions, get_env),
      .output_language = output_language,
  };
}

}  // namespace spoor::instrumentation::config
