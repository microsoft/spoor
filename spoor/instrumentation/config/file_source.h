// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <array>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "gsl/gsl"
#include "spoor/instrumentation/config/output_language.h"
#include "spoor/instrumentation/config/source.h"
#include "util/env/env.h"
#include "util/file_system/file_reader.h"
#include "util/file_system/util.h"

namespace spoor::instrumentation::config {

constexpr std::string_view kConfigFileName{"spoor_config.toml"};

constexpr std::string_view kEnableRuntimeFileKey{"enable_runtime"};
constexpr std::string_view kFiltersFileFileKey{"filters_file"};
constexpr std::string_view kForceBinaryOutputFileKey{"force_binary_output"};
constexpr std::string_view kInitializeRuntimeFileKey{"initialize_runtime"};
constexpr std::string_view kInjectInstrumentationFileKey{
    "inject_instrumentation"};
constexpr std::string_view kModuleIdFileKey{"module_id"};
constexpr std::string_view kOutputFileFileKey{"output_file"};
constexpr std::string_view kOutputLanguageFileKey{"output_language"};
constexpr std::string_view kOutputSymbolsFileFileKey{"output_symbols_file"};

constexpr std::array<std::string_view, 9> kFileConfigKeys{{
    kEnableRuntimeFileKey,
    kFiltersFileFileKey,
    kForceBinaryOutputFileKey,
    kInitializeRuntimeFileKey,
    kInjectInstrumentationFileKey,
    kModuleIdFileKey,
    kOutputFileFileKey,
    kOutputSymbolsFileFileKey,
    kOutputLanguageFileKey,
}};

class FileSource final : public Source {
 public:
  struct Options {
    std::unique_ptr<util::file_system::FileReader> file_reader;
    util::file_system::PathExpansionOptions path_expansion_options;
    std::filesystem::path file_path;
  };

  FileSource() = delete;
  explicit FileSource(Options options);
  FileSource(const FileSource&) = delete;
  FileSource(FileSource&&) noexcept = default;
  auto operator=(const FileSource&) -> FileSource& = delete;
  auto operator=(FileSource&&) noexcept -> FileSource& = default;
  ~FileSource() override = default;

  [[nodiscard]] static auto FindConfigFile(
      const std::filesystem::path& deepest_search_path)
      -> std::optional<std::filesystem::path>;

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
