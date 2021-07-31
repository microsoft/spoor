// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/config/command_line_config.h"

#include <string>
#include <utility>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include "spoor/instrumentation/config/config.h"
#include "spoor/instrumentation/config/env_config.h"
#include "util/env/env.h"
#include "util/file_system/util.h"
#include "util/flags/optional.h"
#include "util/numeric.h"

ABSL_FLAG(  // NOLINT
    bool, enable_runtime,
    spoor::instrumentation::config::kEnableRuntimeDefaultValue,
    spoor::instrumentation::config::kEnableRuntimeDoc);
ABSL_FLAG(  // NOLINT
    util::flags::Optional<std::string>, filters_file,
    util::flags::Optional<std::string>{
        spoor::instrumentation::config::kFiltersFileDefaultValue},
    spoor::instrumentation::config::kFiltersFileDoc);
ABSL_FLAG(  // NOLINT
    bool, force_binary_output,
    spoor::instrumentation::config::kForceBinaryOutputDefaultValue,
    spoor::instrumentation::config::kForceBinaryOutputDoc);
ABSL_FLAG(  // NOLINT
    bool, initialize_runtime,
    spoor::instrumentation::config::kInitializeRuntimeDefaultValue,
    spoor::instrumentation::config::kInitializeRuntimeDoc);
ABSL_FLAG(  // NOLINT
    bool, inject_instrumentation,
    spoor::instrumentation::config::kInjectInstrumentationDefaultValue,
    spoor::instrumentation::config::kInjectInstrumentationDoc);
ABSL_FLAG(  // NOLINT
    util::flags::Optional<std::string>, module_id,
    util::flags::Optional<std::string>{
        spoor::instrumentation::config::kModuleIdDefaultValue},
    spoor::instrumentation::config::kModuleIdDoc);
ABSL_FLAG(  // NOLINT
    std::string, output_file,
    std::string{spoor::instrumentation::config::kOutputFileDefaultValue},
    spoor::instrumentation::config::kOutputFileDoc);
ABSL_FLAG(  // NOLINT
    std::string, output_symbols_file,
    std::string{spoor::instrumentation::config::kOutputSymbolsFileDefaultValue},
    spoor::instrumentation::config::kOutputSymbolsFileDoc);
ABSL_FLAG(  // NOLINT
    spoor::instrumentation::config::OutputLanguage, output_language,
    spoor::instrumentation::config::kOutputLanguageDefaultValue,
    absl::StrFormat(
        spoor::instrumentation::config::kOutputLanguageDoc,
        absl::StrJoin(spoor::instrumentation::config::kOutputLanguages.Keys(),
                      ", ")));

namespace spoor::instrumentation::config {

using util::file_system::ExpandTilde;

auto ConfigFromCommandLineOrEnv(const int argc, char** argv,
                                const util::env::GetEnv& get_env)
    -> std::pair<Config, std::vector<char*>> {
  const auto env_config = ConfigFromEnv(get_env);
  absl::SetFlag(&FLAGS_enable_runtime, env_config.enable_runtime);
  absl::SetFlag(&FLAGS_filters_file, env_config.filters_file);
  absl::SetFlag(&FLAGS_force_binary_output, env_config.force_binary_output);
  absl::SetFlag(&FLAGS_initialize_runtime, env_config.initialize_runtime);
  absl::SetFlag(&FLAGS_inject_instrumentation,
                env_config.inject_instrumentation);
  absl::SetFlag(&FLAGS_module_id, env_config.module_id);
  absl::SetFlag(&FLAGS_output_file, env_config.output_file);
  absl::SetFlag(&FLAGS_output_symbols_file, env_config.output_symbols_file);
  absl::SetFlag(&FLAGS_output_language, env_config.output_language);
  auto positional_args = absl::ParseCommandLine(argc, argv);
  auto filters_file = absl::GetFlag(FLAGS_filters_file).StdOptional();
  if (filters_file.has_value()) {
    filters_file = ExpandTilde(filters_file.value(), get_env);
  }
  const auto output_file = absl::GetFlag(FLAGS_output_file);
  const auto output_symbols_file = absl::GetFlag(FLAGS_output_symbols_file);
  Config config{
      .enable_runtime = absl::GetFlag(FLAGS_enable_runtime),
      .filters_file = filters_file,
      .force_binary_output = absl::GetFlag(FLAGS_force_binary_output),
      .initialize_runtime = absl::GetFlag(FLAGS_initialize_runtime),
      .inject_instrumentation = absl::GetFlag(FLAGS_inject_instrumentation),
      .module_id = absl::GetFlag(FLAGS_module_id).StdOptional(),
      .output_file = ExpandTilde(output_file, get_env),
      .output_symbols_file = ExpandTilde(output_symbols_file, get_env),
      .output_language = absl::GetFlag(FLAGS_output_language)};
  return std::make_pair(std::move(config), std::move(positional_args));
}

auto AbslParseFlag(const absl::string_view user_key, OutputLanguage* language,
                   std::string* error) -> bool {
  std::string key{user_key};
  absl::AsciiStrToLower(&key);
  const auto option = kOutputLanguages.FirstValueForKey(key);
  if (option.has_value()) {
    *language = option.value();
    return true;
  }
  *error = absl::StrFormat("Unknown language '%s'. Options: %s.", user_key,
                           absl::StrJoin(kOutputLanguages.Keys(), ", "));
  return false;
}

auto AbslUnparseFlag(const OutputLanguage language) -> std::string {
  const auto fallback = absl::StrCat(language);
  const auto value =
      kOutputLanguages.FirstKeyForValue(language).value_or(fallback);
  return std::string{value};
}

}  // namespace spoor::instrumentation::config
