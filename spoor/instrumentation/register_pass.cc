// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Register `InjectRuntime` with LLVM's pass manager.

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "spoor/instrumentation/inject_runtime/inject_runtime.h"
#include "spoor/instrumentation/instrumentation.h"

namespace spoor::instrumentation {

auto PluginInfo() -> llvm::PassPluginLibraryInfo {
  const auto callback = [](llvm::PassBuilder& pass_builder) {
    const auto pipeline_parsing_callback =
        [](llvm::StringRef pass_name, llvm::ModulePassManager& pass_manager,
           llvm::ArrayRef<llvm::PassBuilder::PipelineElement> /*unused*/) {
          if (pass_name != kPluginName.data()) return false;
          inject_runtime::InjectRuntime::Options options{};
          pass_manager.addPass(inject_runtime::InjectRuntime(options));
          return true;
        };
    pass_builder.registerPipelineParsingCallback(pipeline_parsing_callback);
  };
  return {.APIVersion = LLVM_PLUGIN_API_VERSION,
          .PluginName = kPluginName.data(),
          .PluginVersion = kPluginVersion.data(),
          .RegisterPassBuilderCallbacks = callback};
}

}  // namespace spoor::instrumentation

auto llvmGetPassPluginInfo() -> llvm::PassPluginLibraryInfo {
  return spoor::instrumentation::PluginInfo();
}
