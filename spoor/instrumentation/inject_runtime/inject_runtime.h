#pragma once

#include <filesystem>
#include <ostream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "gsl/gsl"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassPlugin.h"
#include "spoor/instrumentation/instrumentation_map.pb.h"
#include "util/numeric.h"

namespace spoor::instrumentation::inject_runtime {

// TODO put these in the module up
constexpr std::string_view kPluginName{"inject-spoor-runtime"};
constexpr std::string_view kPluginVersion{"0.0.0"};

class InjectRuntime : public llvm::PassInfoMixin<InjectRuntime> {
 public:
  struct Options {
    std::filesystem::path instrumented_function_map_path;
    std::unordered_set<std::string> function_blocklist;
    std::unordered_set<std::string> function_allow_list;
    std::ostream* ostream;
    uint32 min_instruction_count_to_instrument;
    bool initialize_runtime;
    bool enable_runtime;
  };

  InjectRuntime() = delete;
  explicit InjectRuntime(Options options);
  InjectRuntime(const InjectRuntime&) = default;
  InjectRuntime(InjectRuntime&&) noexcept = default;
  auto operator=(const InjectRuntime&) -> InjectRuntime& = default;
  auto operator=(InjectRuntime&&) noexcept -> InjectRuntime& = default;
  ~InjectRuntime() = default;

  // NOLINTNEXTLINE(readability-identifier-naming)
  auto run(llvm::Module& module,
           llvm::ModuleAnalysisManager& module_analysis_manager)
      -> llvm::PreservedAnalyses;

 private:
  [[nodiscard]] auto InstrumentModule(gsl::not_null<llvm::Module*> module)
      -> std::pair<InstrumentedFunctionMap, bool>;

  Options options_;
};

}  // namespace spoor::instrumentation::inject_runtime
