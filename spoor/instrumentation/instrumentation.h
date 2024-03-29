// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <string_view>

#include "util/numeric.h"

namespace spoor::instrumentation {

using FunctionId = uint64;

constexpr std::string_view kPluginName{"inject-spoor-instrumentation"};
constexpr std::string_view kVersion{"0.0.0"};
constexpr std::string_view kSymbolsFileExtension{"spoor_symbols"};

}  // namespace spoor::instrumentation
