#pragma once

#include "llvm/Passes/PassPlugin.h"

extern "C" {
// NOLINTNEXTLINE(readability-identifier-naming)
LLVM_ATTRIBUTE_WEAK auto llvmGetPassPluginInfo() -> llvm::PassPluginLibraryInfo;
}
