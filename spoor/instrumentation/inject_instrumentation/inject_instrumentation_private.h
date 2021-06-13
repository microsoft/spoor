// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "llvm/IR/Module.h"
#include "util/numeric.h"

namespace spoor::instrumentation::inject_instrumentation::internal {

auto ModuleHash(const llvm::Module& llvm_module) -> uint32;

}  // namespace spoor::instrumentation::inject_instrumentation::internal
