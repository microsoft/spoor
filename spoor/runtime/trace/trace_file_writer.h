// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <filesystem>

#include "spoor/runtime/trace/trace.h"
#include "spoor/runtime/trace/trace_writer.h"

namespace spoor::runtime::trace {

class TraceFileWriter final : public TraceWriter {
 public:
  auto Write(const std::filesystem::path& file, Header header,
             Events* events) const -> Result override;
};

}  // namespace spoor::runtime::trace
