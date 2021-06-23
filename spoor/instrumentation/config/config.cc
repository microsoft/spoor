// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/config/config.h"

namespace spoor::instrumentation::config {

auto operator==(const Config& lhs, const Config& rhs) -> bool {
  return lhs.enable_runtime == rhs.enable_runtime &&
         lhs.filters_file == rhs.filters_file &&
         lhs.force_binary_output == rhs.force_binary_output &&
         lhs.initialize_runtime == rhs.initialize_runtime &&
         lhs.inject_instrumentation == rhs.inject_instrumentation &&
         lhs.module_id == rhs.module_id && lhs.output_file == rhs.output_file &&
         lhs.output_symbols_file == rhs.output_symbols_file &&
         lhs.output_language == rhs.output_language;
}

auto BinaryOutput(const OutputLanguage output_language) -> bool {
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
