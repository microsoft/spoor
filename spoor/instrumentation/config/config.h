// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "util/flat_map/flat_map.h"
#include "util/numeric.h"

namespace spoor::instrumentation::config {

enum class OutputLanguage {
  kBitcode,
  kIr,
};

struct Config {
  // Alphabetized to match the order printed in --help.
  bool enable_runtime;
  bool force_binary_output;
  std::optional<std::string> function_allow_list_file;
  std::optional<std::string> function_blocklist_file;
  bool initialize_runtime;
  bool inject_instrumentation;
  std::string instrumented_function_map_output_path;
  uint32 min_instruction_threshold;
  std::optional<std::string> module_id;
  std::string output_file;
  OutputLanguage output_language;
};

constexpr util::flat_map::FlatMap<std::string_view, OutputLanguage, 2>
    kOutputLanguages{{"bitcode", OutputLanguage::kBitcode},
                     {"ir", OutputLanguage::kIr}};

constexpr std::string_view kEnableRuntimeDoc{
    "Automatically enable Spoor's runtime."};
constexpr bool kEnableRuntimeDefaultValue{true};

constexpr std::string_view kForceBinaryOutputDoc{
    "Force printing binary data to the console."};
constexpr bool kForceBinaryOutputDefaultValue{false};

constexpr std::string_view kFunctionAllowListFileDoc{
    "File path to the function allow list."};
// This statically-constructed object is safe because its value is the type's
// default.
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
const std::optional<std::string> kFunctionAllowListFileDefaultValue{};

constexpr std::string_view kFunctionBlocklistFileDoc{
    "File path to the function blocklist."};
// This statically-constructed object is safe because its value is the type's
// default.
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
const std::optional<std::string> kFunctionBlocklistFileDefaultValue{};

constexpr std::string_view kInitializeRuntimeDoc{
    "Automatically initialize Spoor's runtime."};
constexpr bool kInitializeRuntimeDefaultValue{true};

constexpr std::string_view kInjectInstrumentationDoc{
    "Inject Spoor instrumentation."};
constexpr auto kInjectInstrumentationDefaultValue{true};

constexpr std::string_view kInstrumentedFunctionMapOutputPathDoc{
    "Spoor function map output file."};
constexpr std::string_view kInstrumentedFunctionMapOutputPathDefaultValue{"."};

constexpr std::string_view kMinInstructionThresholdDoc{
    "Minimum number of LLVM IR instructions required to instrument a "
    "function."};
constexpr uint32 kMinInstructionThresholdDefaultValue{0};

constexpr std::string_view kModuleIdDoc{"Override the LLVM module's ID."};
// This statically-constructed object is safe because its value is the type's
// default.
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
const std::optional<std::string> kModuleIdDefaultValue{};

constexpr std::string_view kOutputFileDoc{"Output file."};
// This statically-constructed object is safe because its value is the type's
// default.
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
const std::string_view kOutputFileDefaultValue{"-"};

constexpr std::string_view kOutputLanguageDoc{
    "Language in which to output the transformed code. Options: %s."};
constexpr auto kOutputLanguageDefaultValue{OutputLanguage::kBitcode};

auto BinaryOutput(OutputLanguage output_language) -> bool;

auto operator==(const Config& lhs, const Config& rhs) -> bool;

}  // namespace spoor::instrumentation::config
