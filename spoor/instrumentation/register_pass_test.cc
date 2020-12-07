// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "gtest/gtest.h"
#include "llvm/Passes/PassPlugin.h"
#include "spoor/instrumentation/instrumentation.h"

namespace {

TEST(RegisterPass, PassPluginInfo) {  // NOLINT
  const auto result = llvmGetPassPluginInfo();
  ASSERT_EQ(result.APIVersion, LLVM_PLUGIN_API_VERSION);
  ASSERT_EQ(result.PluginName, spoor::instrumentation::kPluginName);
  ASSERT_EQ(result.PluginVersion, spoor::instrumentation::kPluginVersion);
  ASSERT_NE(result.RegisterPassBuilderCallbacks, nullptr);
}

}  // namespace
