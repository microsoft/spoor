// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/config/file_source.h"

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include "absl/strings/ascii.h"
#include "absl/strings/str_format.h"
#include "gsl/gsl"
#include "spoor/instrumentation/config/output_language.h"
#include "spoor/instrumentation/config/source.h"
#include "tomlplusplus/toml.h"
#include "util/env/env.h"
#include "util/file_system/util.h"
#include "util/result.h"

namespace spoor::instrumentation::config {

using util::file_system::PathExpansionOptions;
using util::result::Result;

FileSource::FileSource(Options options)
    : options_{std::move(options)}, read_{false} {}

template <class T>
auto ReadConfig(const toml::table& table, const std::string_view key,
                gsl::not_null<std::vector<Source::ReadError>*> errors)
    -> std::optional<T> {
  const auto* table_value = table.get(key);
  if (table_value == nullptr) return {};
  auto value = table_value->value<T>();
  if (!value.has_value()) {
    errors->push_back({
        .type = Source::ReadError::Type::kUnknownValue,
        .message = absl::StrFormat("Cannot parse value for key \"%s\".", key),
    });
  }
  return value;
}

auto ReadFilePathConfig(const toml::table& table, const std::string_view key,
                        const std::filesystem::path& parent_path,
                        const PathExpansionOptions& path_expansion_options,
                        gsl::not_null<std::vector<Source::ReadError>*> errors)
    -> std::optional<std::filesystem::path> {
  const auto user_value = ReadConfig<std::string>(table, key, errors);
  if (!user_value.has_value()) return {};
  const auto expanded = ExpandPath(user_value.value(), path_expansion_options);
  const auto file_path = std::filesystem::path{expanded};
  if (file_path.is_absolute()) return file_path;
  return parent_path / file_path;
}

auto FileSource::FindConfigFile(
    const std::filesystem::path& deepest_search_path)
    -> Result<std::optional<std::filesystem::path>, std::error_code> {
  using Result = Result<std::optional<std::filesystem::path>, std::error_code>;
  std::error_code error{};
  for (auto path = deepest_search_path; path != path.root_path();
       path = path.parent_path()) {
    const auto iterator = std::filesystem::directory_iterator{path, error};
    if (error) return error;
    const auto config_file = std::find_if(
        iterator, std::filesystem::end(iterator), [](const auto& entry) {
          return entry.path().filename() == kConfigFileName;
        });
    if (config_file != std::filesystem::end(iterator)) {
      return Result::Ok(*config_file);
    }
  }
  return Result::Ok({});
}

auto FileSource::Read() -> std::vector<ReadError> {
  if (read_) return {};

  auto finally_set_read = gsl::finally([this] { read_ = true; });

  const auto& config_file_path = options_.file_path;
  auto& file = *options_.file_reader;
  file.Open(config_file_path);
  if (!file.IsOpen()) {
    return {{
        .type = ReadError::Type::kFailedToOpenFile,
        .message = absl::StrFormat(
            "Failed to open the file \"%s\" for reading.", config_file_path),
    }};
  }
  auto finally_close_file = gsl::finally([&file] { file.Close(); });
  const auto parent_path = config_file_path.parent_path();

  auto toml_result = toml::parse(file.Istream());
  if (!toml_result) {
    return {{
        .type = ReadError::Type::kMalformedFile,
        .message{toml_result.error().description()},
    }};
  }
  const auto table = std::move(toml_result).table();

  std::vector<ReadError> errors{};

  for (const auto& [key, node] : table) {
    if (std::find(std::cbegin(kFileConfigKeys), std::cend(kFileConfigKeys),
                  key) == std::cend(kFileConfigKeys)) {
      errors.push_back({
          .type = ReadError::Type::kUnknownKey,
          .message = absl::StrFormat("Unknown key \"%s\".", key),
      });
    }
  }

  enable_runtime_ = ReadConfig<decltype(enable_runtime_)::value_type>(
      table, kEnableRuntimeFileKey, &errors);
  filters_file_ = ReadFilePathConfig(table, kFiltersFileFileKey, parent_path,
                                     options_.path_expansion_options, &errors);
  force_binary_output_ = ReadConfig<decltype(force_binary_output_)::value_type>(
      table, kForceBinaryOutputFileKey, &errors);
  initialize_runtime_ = ReadConfig<decltype(initialize_runtime_)::value_type>(
      table, kInitializeRuntimeFileKey, &errors);
  inject_instrumentation_ =
      ReadConfig<decltype(inject_instrumentation_)::value_type>(
          table, kInjectInstrumentationFileKey, &errors);
  module_id_ = ReadConfig<decltype(module_id_)::value_type>(
      table, kModuleIdFileKey, &errors);
  output_file_ = ReadFilePathConfig(table, kOutputFileFileKey, parent_path,
                                    options_.path_expansion_options, &errors);
  output_language_ = [&]() -> decltype(output_language_) {
    const auto output_language_user_value =
        ReadConfig<std::string>(table, kOutputLanguageFileKey, &errors);
    if (!output_language_user_value.has_value()) return {};
    auto normalized_value = output_language_user_value.value();
    absl::StripAsciiWhitespace(&normalized_value);
    absl::AsciiStrToLower(&normalized_value);
    const auto output_language =
        kOutputLanguages.FirstValueForKey(normalized_value);
    if (!output_language.has_value()) {
      errors.push_back({
          .type = ReadError::Type::kUnknownValue,
          .message = absl::StrFormat("Unknown output_language \"%s\".",
                                     output_language_user_value.value()),
      });
    }
    return output_language;
  }();
  output_symbols_file_ =
      ReadFilePathConfig(table, kOutputSymbolsFileFileKey, parent_path,
                         options_.path_expansion_options, &errors);

  return errors;
}

auto FileSource::IsRead() const -> bool { return read_; }

auto FileSource::GetEnableRuntime() const -> std::optional<bool> {
  return enable_runtime_;
}

auto FileSource::GetFiltersFile() const -> std::optional<std::string> {
  return filters_file_;
}

auto FileSource::GetForceBinaryOutput() const -> std::optional<bool> {
  return force_binary_output_;
}

auto FileSource::GetInitializeRuntime() const -> std::optional<bool> {
  return initialize_runtime_;
}

auto FileSource::GetInjectInstrumentation() const -> std::optional<bool> {
  return inject_instrumentation_;
}

auto FileSource::GetModuleId() const -> std::optional<std::string> {
  return module_id_;
}

auto FileSource::GetOutputFile() const -> std::optional<std::string> {
  return output_file_;
}

auto FileSource::GetOutputLanguage() const -> std::optional<OutputLanguage> {
  return output_language_;
}

auto FileSource::GetOutputSymbolsFile() const -> std::optional<std::string> {
  return output_symbols_file_;
}

}  // namespace spoor::instrumentation::config
