// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/config/config.h"

namespace spoor::instrumentation::config {

auto operator==(const Config& lhs, const Config& rhs) -> bool {
  return lhs.function_allow_list_file == rhs.function_allow_list_file &&
         lhs.function_blocklist_file == rhs.function_blocklist_file &&
         lhs.enable_runtime == rhs.enable_runtime &&
         lhs.initialize_runtime == rhs.initialize_runtime &&
         lhs.inject_instrumentation == rhs.inject_instrumentation &&
         lhs.min_instruction_threshold == rhs.min_instruction_threshold &&
         lhs.module_id == rhs.module_id && lhs.output_file == rhs.output_file &&
         lhs.output_function_map_file == rhs.output_function_map_file &&
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
