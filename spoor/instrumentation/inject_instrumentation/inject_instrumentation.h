// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Instrument an IR module by injecting calls to Spoor's runtime library and
// write the generated symbols to a file.

#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "gsl/gsl"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassPlugin.h"
#include "spoor/instrumentation/filters/filters.h"
#include "spoor/instrumentation/filters/filters_reader.h"
#include "spoor/instrumentation/symbols/symbols.pb.h"
#include "spoor/instrumentation/symbols/symbols_writer.h"
#include "util/numeric.h"
#include "util/time/clock.h"

namespace spoor::instrumentation::inject_instrumentation {

class InjectInstrumentation
    : public llvm::PassInfoMixin<InjectInstrumentation> {
 public:
  struct alignas(128) Options {
    bool inject_instrumentation;
    std::unique_ptr<filters::FiltersReader> filters_reader;
    std::unique_ptr<symbols::SymbolsWriter> symbols_writer;
    std::unique_ptr<util::time::SystemClock> system_clock;
    std::optional<std::filesystem::path> filters_file_path;
    std::filesystem::path symbols_file_path;
    std::optional<std::string> module_id;
    bool initialize_runtime;
    bool enable_runtime;
  };

  InjectInstrumentation() = delete;
  explicit InjectInstrumentation(Options&& options);
  InjectInstrumentation(const InjectInstrumentation&) = delete;
  InjectInstrumentation(InjectInstrumentation&&) noexcept = default;
  auto operator=(const InjectInstrumentation&)
      -> InjectInstrumentation& = delete;
  auto operator=(InjectInstrumentation&&) noexcept
      -> InjectInstrumentation& = default;
  ~InjectInstrumentation() = default;

  // LLVM's pass interface requires deviating from Spoor's naming convention.
  // NOLINTNEXTLINE(google-runtime-references, readability-identifier-naming)
  auto run(llvm::Module& llvm_module,
           // NOLINTNEXTLINE(google-runtime-references)
           llvm::ModuleAnalysisManager& module_analysis_manager)
      -> llvm::PreservedAnalyses;

 private:
  struct alignas(128) InstrumentModuleResult {
    symbols::Symbols symbols;
    bool modified;
  };

  enum class ReadFileLinesToSetError {
    kFailedToOpenFile,
    kReadError,
  };

  [[nodiscard]] auto InstrumentModule(gsl::not_null<llvm::Module*> llvm_module,
                                      const filters::Filters& filters) const
      -> InstrumentModuleResult;
  [[nodiscard]] auto ReadFileLinesToSet(const std::filesystem::path& file_path)
      const -> util::result::Result<std::unordered_set<std::string>,
                                    ReadFileLinesToSetError>;

  Options options_;
};

}  // namespace spoor::instrumentation::inject_instrumentation
