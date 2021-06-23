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
  std::optional<std::string> filters_file;
  bool force_binary_output;
  bool initialize_runtime;
  bool inject_instrumentation;
  std::optional<std::string> module_id;
  std::string output_file;
  std::string output_symbols_file;
  OutputLanguage output_language;
};

constexpr util::flat_map::FlatMap<std::string_view, OutputLanguage, 2>
    kOutputLanguages{{"bitcode", OutputLanguage::kBitcode},
                     {"ir", OutputLanguage::kIr}};

constexpr std::string_view kEnableRuntimeDoc{
    "Automatically enable Spoor's runtime."};
constexpr auto kEnableRuntimeDefaultValue{true};

constexpr std::string_view kFiltersFileDoc{"File path to the filters file."};
// This statically-constructed object is safe because its value is the type's
// default.
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
const std::optional<std::string> kFiltersFileDefaultValue{};

constexpr std::string_view kForceBinaryOutputDoc{
    "Force printing binary data to the console."};
constexpr auto kForceBinaryOutputDefaultValue{false};

constexpr std::string_view kInitializeRuntimeDoc{
    "Automatically initialize Spoor's runtime."};
constexpr auto kInitializeRuntimeDefaultValue{true};

constexpr std::string_view kInjectInstrumentationDoc{
    "Inject Spoor instrumentation."};
constexpr auto kInjectInstrumentationDefaultValue{true};

constexpr std::string_view kOutputSymbolsFileDoc{
    "Spoor instrumentation symbols output file."};
constexpr std::string_view kOutputSymbolsFileDefaultValue{};

constexpr std::string_view kModuleIdDoc{"Override the LLVM module's ID."};
// This statically-constructed object is safe because its value is the type's
// default.
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
const std::optional<std::string> kModuleIdDefaultValue{};

constexpr std::string_view kOutputFileDoc{"Output file."};
constexpr std::string_view kOutputFileDefaultValue{"-"};

constexpr std::string_view kOutputLanguageDoc{
    "Language in which to output the transformed code. Options: %s."};
constexpr auto kOutputLanguageDefaultValue{OutputLanguage::kBitcode};

auto BinaryOutput(OutputLanguage output_language) -> bool;

auto operator==(const Config& lhs, const Config& rhs) -> bool;

}  // namespace spoor::instrumentation::config
