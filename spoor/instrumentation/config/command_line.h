// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Command line is not a `Source` because the Abseil Flags library requires a
// default value.

#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "absl/flags/declare.h"
#include "spoor/instrumentation/config/config.h"
#include "spoor/instrumentation/config/config_private.h"
#include "spoor/instrumentation/config/output_language.h"
#include "util/env/env.h"
#include "util/file_system/util.h"
#include "util/flags/optional.h"

ABSL_DECLARE_FLAG(bool, enable_runtime);                              // NOLINT
ABSL_DECLARE_FLAG(util::flags::Optional<std::string>, filters_file);  // NOLINT
ABSL_DECLARE_FLAG(bool, force_binary_output);                         // NOLINT
ABSL_DECLARE_FLAG(bool, initialize_runtime);                          // NOLINT
ABSL_DECLARE_FLAG(bool, inject_instrumentation);                      // NOLINT
ABSL_DECLARE_FLAG(util::flags::Optional<std::string>, module_id);     // NOLINT
ABSL_DECLARE_FLAG(std::string, output_file);                          // NOLINT
ABSL_DECLARE_FLAG(                                                    // NOLINT
    spoor::instrumentation::config::OutputLanguage, output_language);
ABSL_DECLARE_FLAG(std::string, output_symbols_file);  // NOLINT

namespace spoor::instrumentation::config {

constexpr std::string_view kEnableRuntimeDoc{
    "Automatically enable Spoor's runtime."};
constexpr std::string_view kFiltersFileDoc{"File path to the filters file."};
constexpr std::string_view kForceBinaryOutputDoc{
    "Force printing binary data to the console."};
constexpr std::string_view kInitializeRuntimeDoc{
    "Automatically initialize Spoor's runtime."};
constexpr std::string_view kInjectInstrumentationDoc{
    "Inject Spoor instrumentation."};
constexpr std::string_view kModuleIdDoc{"Override the LLVM module's ID."};
constexpr std::string_view kOutputFileDoc{"Output file."};
constexpr std::string_view kOutputLanguageDoc{
    "Language in which to output the transformed code. Options: %s."};
constexpr std::string_view kOutputSymbolsFileDoc{
    "Spoor instrumentation symbols output file."};

auto ConfigFromCommandLineOrDefault(
    int argc, char** argv, const Config& default_config,
    const util::file_system::PathExpansionOptions& path_expansion_options)
    -> std::pair<Config, std::vector<char*>>;

auto AbslParseFlag(absl::string_view user_key, OutputLanguage* language,
                   std::string* error) -> bool;
auto AbslUnparseFlag(OutputLanguage language) -> std::string;

}  // namespace spoor::instrumentation::config
