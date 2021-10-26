// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "spoor/runtime/buffer/circular_buffer.h"
#include "spoor/runtime/trace/trace.h"
#include "util/compression/compressor.h"
#include "util/numeric.h"

namespace spoor::runtime::config {

class Source {
 public:
  using SizeType = buffer::CircularBuffer<trace::Event>::SizeType;

  struct alignas(32) ReadError {
    enum class Type {
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

  [[nodiscard]] virtual auto TraceFilePath() const
      -> std::optional<std::string> = 0;
  [[nodiscard]] virtual auto CompressionStrategy() const
      -> std::optional<util::compression::Strategy> = 0;
  [[nodiscard]] virtual auto SessionId() const
      -> std::optional<trace::SessionId> = 0;
  [[nodiscard]] virtual auto ThreadEventBufferCapacity() const
      -> std::optional<SizeType> = 0;
  [[nodiscard]] virtual auto MaxReservedEventBufferSliceCapacity() const
      -> std::optional<SizeType> = 0;
  [[nodiscard]] virtual auto MaxDynamicEventBufferSliceCapacity() const
      -> std::optional<SizeType> = 0;
  [[nodiscard]] virtual auto ReservedEventPoolCapacity() const
      -> std::optional<SizeType> = 0;
  [[nodiscard]] virtual auto DynamicEventPoolCapacity() const
      -> std::optional<SizeType> = 0;
  [[nodiscard]] virtual auto DynamicEventSliceBorrowCasAttempts() const
      -> std::optional<SizeType> = 0;
  [[nodiscard]] virtual auto EventBufferRetentionDurationNanoseconds() const
      -> std::optional<trace::DurationNanoseconds> = 0;
  [[nodiscard]] virtual auto MaxFlushBufferToFileAttempts() const
      -> std::optional<int32> = 0;
  [[nodiscard]] virtual auto FlushAllEvents() const -> std::optional<bool> = 0;

 protected:
  constexpr Source() = default;
  constexpr Source(const Source&) = default;
  constexpr Source(Source&&) noexcept = default;
  constexpr auto operator=(const Source&) -> Source& = default;
  constexpr auto operator=(Source&&) noexcept -> Source& = default;
};

}  // namespace spoor::runtime::config
