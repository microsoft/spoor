// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/config/config.h"

#include "util/env/env.h"

namespace spoor::instrumentation::config {

using util::env::GetEnvOrDefault;

auto Config::FromEnv(const util::env::GetEnv& get_env) -> Config {
  return {
      .instrumentation_map_output_path =
          GetEnvOrDefault(kInstrumentationMapOutputPathKey.data(),
                          kInstrumentationMapOutputPathDefaultValue, get_env),
      .initialize_runtime =
          GetEnvOrDefault(kInitializeRuntimeKey.data(),
                          kInitializeRuntimeDefaultValue, get_env),
      .enable_runtime = GetEnvOrDefault(kEnableRuntimeKey.data(),
                                        kEnableRuntimeDefaultValue, get_env),
      .min_instruction_threshold =
          GetEnvOrDefault(kMinInstructionThresholdKey.data(),
                          kMinInstructionThresholdDefaultValue, get_env),
      .function_allow_list_file =
          GetEnvOrDefault(kFunctionAllowListFileKey.data(),
                          kFunctionAllowListFileDefaultValue, true, get_env),
      .function_blocklist_file =
          GetEnvOrDefault(kFunctionBlocklistFileKey.data(),
                          kFunctionBlocklistFileDefaultValue, true, get_env)};
}

auto operator==(const Config& lhs, const Config& rhs) -> bool {
  return lhs.instrumentation_map_output_path ==
             rhs.instrumentation_map_output_path &&
         lhs.initialize_runtime == rhs.initialize_runtime &&
         lhs.enable_runtime == rhs.enable_runtime &&
         lhs.min_instruction_threshold == rhs.min_instruction_threshold &&
         lhs.function_allow_list_file == rhs.function_allow_list_file &&
         lhs.function_blocklist_file == rhs.function_blocklist_file;
}

}  // namespace spoor::instrumentation::config
