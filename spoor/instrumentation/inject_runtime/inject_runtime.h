// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Instrument an IR module by injecting calls to Spoor's runtime library and
// write the generated instrumented function map to the supplied output stream.

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

  // NOLINTNEXTLINE(google-runtime-references, readability-identifier-naming)
  auto run(llvm::Module& llvm_module,
           // NOLINTNEXTLINE(google-runtime-references)
           llvm::ModuleAnalysisManager& module_analysis_manager)
      -> llvm::PreservedAnalyses;

 private:
  [[nodiscard]] auto InstrumentModule(gsl::not_null<llvm::Module*> llvm_module)
      const -> std::pair<InstrumentedFunctionMap, bool>;

  Options options_;
};

}  // namespace spoor::instrumentation::inject_runtime
