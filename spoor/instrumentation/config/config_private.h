// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "spoor/instrumentation/config/output_language.h"

namespace spoor::instrumentation::config {

constexpr auto kEnableRuntimeDefaultValue{true};
// This statically-constructed object is safe because its value is the type's
// default. NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
const std::optional<std::string> kFiltersFileDefaultValue{};
constexpr auto kForceBinaryOutputDefaultValue{false};
constexpr auto kInitializeRuntimeDefaultValue{true};
constexpr auto kInjectInstrumentationDefaultValue{true};
// This statically-constructed object is safe because its value is the type's
// default. NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
const std::optional<std::string> kModuleIdDefaultValue{};
constexpr std::string_view kOutputFileDefaultValue{"-"};
constexpr auto kOutputLanguageDefaultValue{OutputLanguage::kBitcode};
constexpr std::string_view kOutputSymbolsFileDefaultValue{};

}  // namespace spoor::instrumentation::config
