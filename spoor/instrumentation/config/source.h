// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "spoor/instrumentation/config/output_language.h"

namespace spoor::instrumentation::config {

class Source {
 public:
  struct ReadError {
    enum class Type {
      kFailedToGetHomeDirectory,
      kFailedToOpenFile,
      kMalformedFile,
      kUnknownKey,
      kUnknownValue,
    };

    Type type;
    std::string message;
  };

  virtual ~Source() = default;

  // Synchronously read the configuration from the data source. Makes a "best
  // effort" to read the configuration, even if there are errors.
  [[nodiscard]] virtual auto Read() -> std::vector<ReadError> = 0;
  [[nodiscard]] virtual auto IsRead() const -> bool = 0;

  [[nodiscard]] virtual auto GetEnableRuntime() const
      -> std::optional<bool> = 0;
  [[nodiscard]] virtual auto GetFiltersFile() const
      -> std::optional<std::string> = 0;
  [[nodiscard]] virtual auto GetForceBinaryOutput() const
      -> std::optional<bool> = 0;
  [[nodiscard]] virtual auto GetInitializeRuntime() const
      -> std::optional<bool> = 0;
  [[nodiscard]] virtual auto GetInjectInstrumentation() const
      -> std::optional<bool> = 0;
  [[nodiscard]] virtual auto GetModuleId() const
      -> std::optional<std::string> = 0;
  [[nodiscard]] virtual auto GetOutputFile() const
      -> std::optional<std::string> = 0;
  [[nodiscard]] virtual auto GetOutputLanguage() const
      -> std::optional<OutputLanguage> = 0;
  [[nodiscard]] virtual auto GetOutputSymbolsFile() const
      -> std::optional<std::string> = 0;

 protected:
  constexpr Source() = default;
  constexpr Source(const Source&) = default;
  constexpr Source(Source&&) noexcept = default;
  constexpr auto operator=(const Source&) -> Source& = default;
  constexpr auto operator=(Source&&) noexcept -> Source& = default;
};

}  // namespace spoor::instrumentation::config
