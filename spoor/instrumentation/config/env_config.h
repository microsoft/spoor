// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <cstdlib>

#include "spoor/instrumentation/config/config.h"
#include "util/env/env.h"

namespace spoor::instrumentation::config {

constexpr std::string_view kEnableRuntimeKey{
    "SPOOR_INSTRUMENTATION_ENABLE_RUNTIME"};
constexpr std::string_view kFiltersFileKey{
    "SPOOR_INSTRUMENTATION_FILTERS_FILE"};
constexpr std::string_view kForceBinaryOutputKey{
    "SPOOR_INSTRUMENTATION_FORCE_BINARY_OUTPUT"};
constexpr std::string_view kInitializeRuntimeKey{
    "SPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME"};
constexpr std::string_view kInjectInstrumentationKey{
    "SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION"};
constexpr std::string_view kModuleIdKey{"SPOOR_INSTRUMENTATION_MODULE_ID"};
constexpr std::string_view kOutputFileKey{"SPOOR_INSTRUMENTATION_OUTPUT_FILE"};
constexpr std::string_view kOutputSymbolsFileKey{
    "SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE"};
constexpr std::string_view kOutputLanguageKey{
    "SPOOR_INSTRUMENTATION_OUTPUT_LANGUAGE"};

auto ConfigFromEnv(const util::env::StdGetEnv& get_env = std::getenv) -> Config;

}  // namespace spoor::instrumentation::config
