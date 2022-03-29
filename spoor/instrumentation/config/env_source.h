// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "spoor/instrumentation/config/output_language.h"
#include "spoor/instrumentation/config/source.h"
#include "util/env/env.h"
#include "util/file_system/file_system.h"
#include "util/file_system/util.h"

namespace spoor::instrumentation::config {

constexpr std::string_view kEnableRuntimeEnvKey{
    "SPOOR_INSTRUMENTATION_ENABLE_RUNTIME"};
constexpr std::string_view kFiltersFileEnvKey{
    "SPOOR_INSTRUMENTATION_FILTERS_FILE"};
constexpr std::string_view kForceBinaryOutputEnvKey{
    "SPOOR_INSTRUMENTATION_FORCE_BINARY_OUTPUT"};
constexpr std::string_view kInitializeRuntimeEnvKey{
    "SPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME"};
constexpr std::string_view kInjectInstrumentationEnvKey{
    "SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION"};
constexpr std::string_view kModuleIdEnvKey{"SPOOR_INSTRUMENTATION_MODULE_ID"};
constexpr std::string_view kOutputFileEnvKey{
    "SPOOR_INSTRUMENTATION_OUTPUT_FILE"};
constexpr std::string_view kOutputLanguageEnvKey{
    "SPOOR_INSTRUMENTATION_OUTPUT_LANGUAGE"};
constexpr std::string_view kOutputSymbolsFileEnvKey{
    "SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE"};

constexpr std::array<std::string_view, 9> kEnvConfigKeys{{
    kEnableRuntimeEnvKey,
    kFiltersFileEnvKey,
    kForceBinaryOutputEnvKey,
    kInitializeRuntimeEnvKey,
    kInjectInstrumentationEnvKey,
    kModuleIdEnvKey,
    kOutputFileEnvKey,
    kOutputSymbolsFileEnvKey,
    kOutputLanguageEnvKey,
}};

class EnvSource final : public Source {
 public:
  struct Options {
    util::file_system::PathExpansionOptions path_expansion_options;
    std::unique_ptr<util::file_system::FileSystem> file_system;
    util::env::StdGetEnv get_env;
  };

  EnvSource() = delete;
  explicit EnvSource(Options options);
  EnvSource(const EnvSource&) = delete;
  EnvSource(EnvSource&&) noexcept = default;
  auto operator=(const EnvSource&) -> EnvSource& = delete;
  auto operator=(EnvSource&&) noexcept -> EnvSource& = default;
  ~EnvSource() override = default;

  [[nodiscard]] auto Read() -> std::vector<ReadError> override;
  [[nodiscard]] auto IsRead() const -> bool override;

  [[nodiscard]] auto GetEnableRuntime() const -> std::optional<bool> override;
  [[nodiscard]] auto GetFiltersFile() const
      -> std::optional<std::string> override;
  [[nodiscard]] auto GetForceBinaryOutput() const
      -> std::optional<bool> override;
  [[nodiscard]] auto GetInitializeRuntime() const
      -> std::optional<bool> override;
  [[nodiscard]] auto GetInjectInstrumentation() const
      -> std::optional<bool> override;
  [[nodiscard]] auto GetModuleId() const -> std::optional<std::string> override;
  [[nodiscard]] auto GetOutputFile() const
      -> std::optional<std::string> override;
  [[nodiscard]] auto GetOutputLanguage() const
      -> std::optional<OutputLanguage> override;
  [[nodiscard]] auto GetOutputSymbolsFile() const
      -> std::optional<std::string> override;

 private:
  Options options_;
  bool read_;

  std::optional<bool> enable_runtime_;
  std::optional<std::string> filters_file_;
  std::optional<bool> force_binary_output_;
  std::optional<bool> initialize_runtime_;
  std::optional<bool> inject_instrumentation_;
  std::optional<std::string> module_id_;
  std::optional<std::string> output_file_;
  std::optional<OutputLanguage> output_language_;
  std::optional<std::string> output_symbols_file_;
};

}  // namespace spoor::instrumentation::config
