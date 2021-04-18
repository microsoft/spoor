// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <cstdlib>

#include "spoor/instrumentation/config/config.h"
#include "util/env/env.h"

namespace spoor::instrumentation::config {

constexpr std::string_view kEnableRuntimeKey{
    "SPOOR_INSTRUMENTATION_RUNTIME_ENABLE_RUNTIME"};
constexpr std::string_view kForceBinaryOutputKey{
    "SPOOR_INSTRUMENTATION_FORCE_BINARY_OUTPUT"};
constexpr std::string_view kFunctionAllowListFileKey{
    "SPOOR_INSTRUMENTATION_FUNCTION_ALLOW_LIST_FILE"};
constexpr std::string_view kFunctionBlocklistFileKey{
    "SPOOR_INSTRUMENTATION_FUNCTION_BLOCKLIST_FILE"};
constexpr std::string_view kInitializeRuntimeKey{
    "SPOOR_INSTRUMENTATION_RUNTIME_INITIALIZE_RUNTIME"};
constexpr std::string_view kInjectInstrumentationKey{
    "SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION"};
constexpr std::string_view kInstrumentedFunctionMapOutputPathKey{
    "SPOOR_INSTRUMENTATION_INSTRUMENTED_FUNCTION_MAP_OUTPUT_PATH"};
constexpr std::string_view kMinInstructionThresholdKey{
    "SPOOR_INSTRUMENTATION_MIN_INSTRUCTION_THRESHOLD"};
constexpr std::string_view kModuleIdKey{"SPOOR_INSTRUMENTATION_MODULE_ID"};
constexpr std::string_view kOutputFileKey{"SPOOR_INSTRUMENTATION_OUTPUT_FILE"};
constexpr std::string_view kOutputLanguageKey{
    "SPOOR_INSTRUMENTATION_OUTPUT_LANGUAGE"};

auto ConfigFromEnv(const util::env::GetEnv& get_env = std::getenv) -> Config;

}  // namespace spoor::instrumentation::config
