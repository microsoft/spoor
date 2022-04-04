// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "spoor/instrumentation/config/output_language.h"
#include "spoor/instrumentation/config/source.h"

namespace spoor::instrumentation::config {

// Prefer alphabetization readability over optimal struct ordering for this
// single-use config. NOLINTNEXTLINE(clang-analyzer-optin.performance.Padding)
struct Config {
  // Alphabetized to match the order printed in --help.
  bool enable_runtime;
  std::optional<std::string> filters_file;
  bool force_binary_output;
  bool initialize_runtime;
  bool inject_instrumentation;
  std::optional<std::string> module_id;
  std::string output_file;
  OutputLanguage output_language;
  std::string output_symbols_file;

  static auto Default() -> Config;
  static auto FromSourcesOrDefault(
      std::vector<std::unique_ptr<Source>>&& sources,
      const Config& default_config) -> Config;
};

auto operator==(const Config& lhs, const Config& rhs) -> bool;

}  // namespace spoor::instrumentation::config
