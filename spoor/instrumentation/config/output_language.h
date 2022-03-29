// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "util/flat_map/flat_map.h"

namespace spoor::instrumentation::config {

enum class OutputLanguage {
  kBitcode,
  kIr,
};

constexpr util::flat_map::FlatMap<std::string_view, OutputLanguage, 2>
    kOutputLanguages{
        {"bitcode", OutputLanguage::kBitcode},
        {"ir", OutputLanguage::kIr},
    };

constexpr auto BinaryOutput(OutputLanguage output_language) -> bool {
  switch (output_language) {
    case OutputLanguage::kBitcode: {
      return true;
    }
    case OutputLanguage::kIr: {
      return false;
    }
  }
}

}  // namespace spoor::instrumentation::config
