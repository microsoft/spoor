// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/config/env_source.h"

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "absl/strings/str_format.h"
#include "gsl/gsl"
#include "spoor/instrumentation/config/output_language.h"
#include "spoor/instrumentation/config/source.h"
#include "util/env/env.h"
#include "util/file_system/util.h"
#include "util/result.h"

namespace spoor::instrumentation::config {

using util::env::GetEnv;
using util::file_system::PathExpansionOptions;

EnvSource::EnvSource(Options options)
    : options_{std::move(options)}, read_{false} {}

template <class T>
auto ReadConfig(
    const std::string_view key,
    gsl::not_null<std::vector<Source::ReadError>*> errors,
    std::function<std::optional<util::result::Result<T, util::result::None>>(
        std::string_view)>
        get_env) -> std::optional<T> {
  const auto value = get_env(key);
  if (!value.has_value()) return {};
  if (value.value().IsErr()) {
    errors->push_back({
        .type = Source::ReadError::Type::kUnknownValue,
        .message = absl::StrFormat("Cannot parse value for key \"%s\".", key),
    });
    return {};
  }
  return value.value().Ok();
}

auto ReadFilePathConfig(const std::string_view key,
                        const std::filesystem::path& parent_path,
                        const util::env::StdGetEnv& get_env,
                        const PathExpansionOptions& path_expansion_options,
                        gsl::not_null<std::vector<Source::ReadError>*> errors)
    -> std::optional<std::filesystem::path> {
  constexpr auto empty_string_is_nullopt{true};
  const auto user_value =
      ReadConfig<std::string>(key, errors, [&](const auto key) {
        return GetEnv(key, empty_string_is_nullopt, get_env);
      });
  if (!user_value.has_value()) return {};
  const auto expanded = ExpandPath(user_value.value(), path_expansion_options);
  const auto file_path = std::filesystem::path{expanded};
  if (file_path.is_absolute()) return file_path;
  return parent_path / file_path;
}

auto EnvSource::Read() -> std::vector<ReadError> {
  if (read_) return {};

  constexpr auto empty_string_is_nullopt{true};
  constexpr auto normalize{true};
  auto finally = gsl::finally([this] { read_ = true; });

  std::vector<ReadError> errors{};

  const auto parent_path = [&]() -> std::filesystem::path {
    const auto path = options_.file_system->CurrentPath();
    if (path.IsErr()) {
      errors.push_back({
          .type = Source::ReadError::Type::kFailedToGetHomeDirectory,
          .message = path.Err().message(),
      });
      return {};
    }
    return path.Ok();
  }();

  enable_runtime_ = ReadConfig<decltype(enable_runtime_)::value_type>(
      kEnableRuntimeEnvKey, &errors, [&](const auto key) {
        return GetEnv<decltype(enable_runtime_)::value_type>(key,
                                                             options_.get_env);
      });
  filters_file_ =
      ReadFilePathConfig(kFiltersFileEnvKey, parent_path, options_.get_env,
                         options_.path_expansion_options, &errors);
  force_binary_output_ = ReadConfig<decltype(force_binary_output_)::value_type>(
      kForceBinaryOutputEnvKey, &errors, [&](const auto key) {
        return GetEnv<decltype(force_binary_output_)::value_type>(
            key, options_.get_env);
      });
  initialize_runtime_ = ReadConfig<decltype(initialize_runtime_)::value_type>(
      kInitializeRuntimeEnvKey, &errors, [&](const auto key) {
        return GetEnv<decltype(initialize_runtime_)::value_type>(
            key, options_.get_env);
      });
  inject_instrumentation_ =
      ReadConfig<decltype(inject_instrumentation_)::value_type>(
          kInjectInstrumentationEnvKey, &errors, [&](const auto key) {
            return GetEnv<decltype(inject_instrumentation_)::value_type>(
                key, options_.get_env);
          });
  module_id_ = ReadConfig<decltype(module_id_)::value_type>(
      kModuleIdEnvKey, &errors, [&](const auto key) {
        return GetEnv(key, empty_string_is_nullopt, options_.get_env);
      });
  output_file_ =
      ReadFilePathConfig(kOutputFileEnvKey, parent_path, options_.get_env,
                         options_.path_expansion_options, &errors);
  output_language_ = ReadConfig<decltype(output_language_)::value_type>(
      kOutputLanguageEnvKey, &errors, [&](const auto key) {
        return GetEnv(key, kOutputLanguages, normalize, options_.get_env);
      });
  output_symbols_file_ = ReadFilePathConfig(
      kOutputSymbolsFileEnvKey, parent_path, options_.get_env,
      options_.path_expansion_options, &errors);

  return errors;
}

auto EnvSource::IsRead() const -> bool { return read_; }

auto EnvSource::GetEnableRuntime() const -> std::optional<bool> {
  return enable_runtime_;
}

auto EnvSource::GetFiltersFile() const -> std::optional<std::string> {
  return filters_file_;
}

auto EnvSource::GetForceBinaryOutput() const -> std::optional<bool> {
  return force_binary_output_;
}

auto EnvSource::GetInitializeRuntime() const -> std::optional<bool> {
  return initialize_runtime_;
}

auto EnvSource::GetInjectInstrumentation() const -> std::optional<bool> {
  return inject_instrumentation_;
}

auto EnvSource::GetModuleId() const -> std::optional<std::string> {
  return module_id_;
}

auto EnvSource::GetOutputFile() const -> std::optional<std::string> {
  return output_file_;
}

auto EnvSource::GetOutputLanguage() const -> std::optional<OutputLanguage> {
  return output_language_;
}

auto EnvSource::GetOutputSymbolsFile() const -> std::optional<std::string> {
  return output_symbols_file_;
}

}  // namespace spoor::instrumentation::config
