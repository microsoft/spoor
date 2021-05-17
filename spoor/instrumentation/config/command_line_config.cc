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
#include "util/flags/optional.h"
#include "util/numeric.h"

ABSL_FLAG(  // NOLINT
    bool, enable_runtime,
    spoor::instrumentation::config::kEnableRuntimeDefaultValue,
    spoor::instrumentation::config::kEnableRuntimeDoc);
ABSL_FLAG(  // NOLINT
    bool, force_binary_output,
    spoor::instrumentation::config::kForceBinaryOutputDefaultValue,
    spoor::instrumentation::config::kForceBinaryOutputDoc);
ABSL_FLAG(  // NOLINT
    util::flags::Optional<std::string>, function_allow_list_file,
    util::flags::Optional<std::string>{
        spoor::instrumentation::config::kFunctionAllowListFileDefaultValue},
    spoor::instrumentation::config::kFunctionAllowListFileDoc);
ABSL_FLAG(  // NOLINT
    util::flags::Optional<std::string>, function_blocklist_file,
    util::flags::Optional<std::string>{
        spoor::instrumentation::config::kFunctionBlocklistFileDefaultValue},
    spoor::instrumentation::config::kFunctionBlocklistFileDoc);
ABSL_FLAG(  // NOLINT
    bool, initialize_runtime,
    spoor::instrumentation::config::kInitializeRuntimeDefaultValue,
    spoor::instrumentation::config::kInitializeRuntimeDoc);
ABSL_FLAG(  // NOLINT
    bool, inject_instrumentation,
    spoor::instrumentation::config::kInjectInstrumentationDefaultValue,
    spoor::instrumentation::config::kInjectInstrumentationDoc);
ABSL_FLAG(  // NOLINT
    uint32, min_instruction_threshold,
    spoor::instrumentation::config::kMinInstructionThresholdDefaultValue,
    spoor::instrumentation::config::kMinInstructionThresholdDoc);
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
    std::string, output_function_map_file,
    std::string{
        spoor::instrumentation::config::kOutputFunctionMapFileDefaultValue},
    spoor::instrumentation::config::kOutputFunctionMapFileDoc);
ABSL_FLAG(  // NOLINT
    spoor::instrumentation::config::OutputLanguage, output_language,
    spoor::instrumentation::config::kOutputLanguageDefaultValue,
    absl::StrFormat(
        spoor::instrumentation::config::kOutputLanguageDoc,
        absl::StrJoin(spoor::instrumentation::config::kOutputLanguages.Keys(),
                      ", ")));

namespace spoor::instrumentation::config {

auto ConfigFromCommandLineOrEnv(const int argc, char** argv,
                                const util::env::GetEnv& get_env)
    -> std::pair<Config, std::vector<char*>> {
  const auto env_config = ConfigFromEnv(get_env);
  absl::SetFlag(&FLAGS_function_allow_list_file,
                env_config.function_allow_list_file);
  absl::SetFlag(&FLAGS_force_binary_output, env_config.force_binary_output);
  absl::SetFlag(&FLAGS_function_blocklist_file,
                env_config.function_blocklist_file);
  absl::SetFlag(&FLAGS_enable_runtime, env_config.enable_runtime);
  absl::SetFlag(&FLAGS_initialize_runtime, env_config.initialize_runtime);
  absl::SetFlag(&FLAGS_inject_instrumentation,
                env_config.inject_instrumentation);
  absl::SetFlag(&FLAGS_min_instruction_threshold,
                env_config.min_instruction_threshold);
  absl::SetFlag(&FLAGS_module_id, env_config.module_id);
  absl::SetFlag(&FLAGS_output_file, env_config.output_file);
  absl::SetFlag(&FLAGS_output_function_map_file,
                env_config.output_function_map_file);
  absl::SetFlag(&FLAGS_output_language, env_config.output_language);
  auto positional_args = absl::ParseCommandLine(argc, argv);
  Config config{
      .enable_runtime = absl::GetFlag(FLAGS_enable_runtime),
      .force_binary_output = absl::GetFlag(FLAGS_force_binary_output),
      .function_allow_list_file =
          absl::GetFlag(FLAGS_function_allow_list_file).StdOptional(),
      .function_blocklist_file =
          absl::GetFlag(FLAGS_function_blocklist_file).StdOptional(),
      .initialize_runtime = absl::GetFlag(FLAGS_initialize_runtime),
      .inject_instrumentation = absl::GetFlag(FLAGS_inject_instrumentation),
      .min_instruction_threshold =
          absl::GetFlag(FLAGS_min_instruction_threshold),
      .module_id = absl::GetFlag(FLAGS_module_id).StdOptional(),
      .output_file = absl::GetFlag(FLAGS_output_file),
      .output_function_map_file = absl::GetFlag(FLAGS_output_function_map_file),
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
