// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/config/env_config.h"

#include <string>

#include "spoor/instrumentation/config/config.h"
#include "util/env/env.h"

namespace spoor::instrumentation::config {

using util::env::GetEnvOrDefault;

auto ConfigFromEnv(const util::env::GetEnv& get_env) -> Config {
  return {
      .enable_runtime = GetEnvOrDefault(kEnableRuntimeKey.data(),
                                        kEnableRuntimeDefaultValue, get_env),
      .force_binary_output =
          GetEnvOrDefault(kForceBinaryOutputKey.data(),
                          kForceBinaryOutputDefaultValue, get_env),
      .function_allow_list_file =
          GetEnvOrDefault(kFunctionAllowListFileKey.data(),
                          kFunctionAllowListFileDefaultValue, true, get_env),
      .function_blocklist_file =
          GetEnvOrDefault(kFunctionBlocklistFileKey.data(),
                          kFunctionBlocklistFileDefaultValue, true, get_env),
      .initialize_runtime =
          GetEnvOrDefault(kInitializeRuntimeKey.data(),
                          kInitializeRuntimeDefaultValue, get_env),
      .inject_instrumentation =
          GetEnvOrDefault(kInjectInstrumentationKey.data(),
                          kInjectInstrumentationDefaultValue, get_env),
      .instrumented_function_map_output_path = GetEnvOrDefault(
          kInstrumentedFunctionMapOutputPathKey.data(),
          std::string{kInstrumentedFunctionMapOutputPathDefaultValue}, get_env),
      .min_instruction_threshold =
          GetEnvOrDefault(kMinInstructionThresholdKey.data(),
                          kMinInstructionThresholdDefaultValue, get_env),
      .module_id = GetEnvOrDefault(kModuleIdKey.data(), kModuleIdDefaultValue,
                                   true, get_env),
      .output_file = GetEnvOrDefault(
          kOutputFileKey.data(), std::string{kOutputFileDefaultValue}, get_env),
      .output_language = GetEnvOrDefault(kOutputLanguageKey.data(),
                                         kOutputLanguageDefaultValue,
                                         kOutputLanguages, true, get_env)};
}

}  // namespace spoor::instrumentation::config
