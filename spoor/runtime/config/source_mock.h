// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "spoor/runtime/config/source.h"
#include "spoor/runtime/trace/trace.h"
#include "util/compression/compressor.h"
#include "util/numeric.h"

namespace spoor::runtime::config::testing {

class SourceMock final : public Source {
 public:
  SourceMock() = default;
  SourceMock(const SourceMock&) = delete;
  SourceMock(SourceMock&&) noexcept = delete;
  auto operator=(const SourceMock&) -> SourceMock& = delete;
  auto operator=(SourceMock&&) noexcept -> SourceMock& = delete;
  ~SourceMock() override = default;

  MOCK_METHOD((std::vector<ReadError>), Read, (), (override));  // NOLINT
  MOCK_METHOD(bool, IsRead, (), (const, override));             // NOLINT

  MOCK_METHOD(  // NOLINT
      (std::optional<std::string>), TraceFilePath, (), (const, override));
  MOCK_METHOD(  // NOLINT
      std::optional<util::compression::Strategy>, CompressionStrategy, (),
      (const, override));
  MOCK_METHOD(  // NOLINT
      (std::optional<trace::SessionId>), SessionId, (), (const, override));
  MOCK_METHOD(  // NOLINT
      (std::optional<SizeType>), ThreadEventBufferCapacity, (),
      (const, override));
  MOCK_METHOD(  // NOLINT
      (std::optional<SizeType>), MaxReservedEventBufferSliceCapacity, (),
      (const, override));
  MOCK_METHOD(  // NOLINT
      (std::optional<SizeType>), MaxDynamicEventBufferSliceCapacity, (),
      (const, override));
  MOCK_METHOD(  // NOLINT
      (std::optional<SizeType>), ReservedEventPoolCapacity, (),
      (const, override));
  MOCK_METHOD(  // NOLINT
      (std::optional<SizeType>), DynamicEventPoolCapacity, (),
      (const, override));
  MOCK_METHOD(  // NOLINT
      (std::optional<SizeType>), DynamicEventSliceBorrowCasAttempts, (),
      (const, override));
  MOCK_METHOD(  // NOLINT
      (std::optional<trace::DurationNanoseconds>),
      EventBufferRetentionDurationNanoseconds, (), (const, override));
  MOCK_METHOD(  // NOLINT
      (std::optional<int32>), MaxFlushBufferToFileAttempts, (),
      (const, override));
  MOCK_METHOD(  // NOLINT
      (std::optional<bool>), FlushAllEvents, (), (const, override));
};

}  // namespace spoor::runtime::config::testing
