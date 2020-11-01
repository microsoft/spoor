#include "spoor/instrumentation/instrumentation.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "spoor/instrumentation/config/config.h"
#include "spoor/instrumentation/inject_runtime/inject_runtime.h"

namespace spoor::instrumentation {

auto PluginInfo() -> llvm::PassPluginLibraryInfo {
  const auto callback = [](llvm::PassBuilder& pass_builder) {
    const auto pipeline_parsing_callback =
        [](llvm::StringRef pass_name, llvm::ModulePassManager& pass_manager,
           llvm::ArrayRef<llvm::PassBuilder::PipelineElement> /*unused*/) {
          if (pass_name != inject_runtime::kPluginName.data()) return false;
          // const auto config = config::Config::FromEnv();  // TODO
          inject_runtime::InjectRuntime::Options options{};
          pass_manager.addPass(inject_runtime::InjectRuntime(options));
          return true;
        };
    pass_builder.registerPipelineParsingCallback(pipeline_parsing_callback);
  };
  return {LLVM_PLUGIN_API_VERSION, inject_runtime::kPluginName.data(),
          inject_runtime::kPluginVersion.data(), callback};
}

}  // namespace spoor::instrumentation

auto llvmGetPassPluginInfo() -> llvm::PassPluginLibraryInfo {
  return spoor::instrumentation::PluginInfo();
}
