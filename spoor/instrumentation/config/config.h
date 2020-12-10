// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <cstdlib>
#include <limits>
#include <optional>
#include <string>
#include <string_view>

#include "util/env/env.h"
#include "util/numeric.h"

namespace spoor::instrumentation::config {

constexpr std::string_view kInstrumentationMapOutputPathKey{
    "SPOOR_INSTRUMENTATION_MAP_OUTPUT_PATH"};
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
const std::string kInstrumentationMapOutputPathDefaultValue{"."};
constexpr std::string_view kInitializeRuntimeKey{
    "SPOOR_INSTRUMENTATION_RUNTIME_INITIALIZE_RUNTIME"};
constexpr bool kInitializeRuntimeDefaultValue{true};
constexpr std::string_view kEnableRuntimeKey{
    "SPOOR_INSTRUMENTATION_RUNTIME_ENABLE_RUNTIME"};
constexpr bool kEnableRuntimeDefaultValue{true};
constexpr std::string_view kMinInstructionThresholdKey{
    "SPOOR_INSTRUMENTATION_MIN_INSTRUCTION_THRESHOLD"};
constexpr uint32 kMinInstructionThresholdDefaultValue{0};
constexpr std::string_view kFunctionAllowListFileKey{
    "SPOOR_INSTRUMENTATION_FUNCTION_ALLOW_LIST_FILE"};
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
const std::optional<std::string> kFunctionAllowListFileDefaultValue{};
constexpr std::string_view kFunctionBlocklistFileKey{
    "SPOOR_INSTRUMENTATION_FUNCTION_BLOCKLIST_FILE"};
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
const std::optional<std::string> kFunctionBlocklistFileDefaultValue{};

struct Config {
  static auto FromEnv(const util::env::GetEnv& get_env = [](const char* key) {
    return std::getenv(key);
  }) -> Config;

  std::string instrumentation_map_output_path;
  bool initialize_runtime;
  bool enable_runtime;
  uint32 min_instruction_threshold;
  std::optional<std::string> function_allow_list_file;
  std::optional<std::string> function_blocklist_file;
};

auto operator==(const Config& lhs, const Config& rhs) -> bool;

}  // namespace spoor::instrumentation::config
