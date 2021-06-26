// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/inject_instrumentation/inject_instrumentation.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <ios>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>

#include "absl/strings/str_format.h"
#include "google/protobuf/timestamp.pb.h"
#include "google/protobuf/util/time_util.h"
#include "gsl/gsl"
#include "llvm/ADT/APInt.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/ErrorHandling.h"
#include "spoor/instrumentation/filters/filters.h"
#include "spoor/instrumentation/filters/filters_file_reader.h"
#include "spoor/instrumentation/inject_instrumentation/inject_instrumentation_private.h"
#include "spoor/instrumentation/instrumentation.h"
#include "spoor/instrumentation/symbols/symbols.pb.h"
#include "spoor/instrumentation/symbols/symbols_file_writer.h"
#include "swift/Demangling/Demangle.h"
#include "swift/Demangling/Demangler.h"
#include "util/numeric.h"
#include "util/time/clock.h"

namespace spoor::instrumentation::inject_instrumentation {

using filters::Filters;
using google::protobuf::util::TimeUtil;
using symbols::Symbols;
using SymbolsWriterError =
    spoor::instrumentation::symbols::SymbolsWriter::Error;

constexpr std::string_view kMainFunctionName{"main"};
constexpr std::string_view kInitializeRuntimeFunctionName{
    "_spoor_runtime_Initialize"};
constexpr std::string_view kDeinitializeRuntimeFunctionName{
    "_spoor_runtime_Deinitialize"};
constexpr std::string_view kEnableRuntimeFunctionName{"_spoor_runtime_Enable"};
constexpr std::string_view kLogFunctionEntryFunctionName{
    "_spoor_runtime_LogFunctionEntry"};
constexpr std::string_view kLogFunctionExitFunctionName{
    "_spoor_runtime_LogFunctionExit"};

InjectInstrumentation::InjectInstrumentation(Options&& options)
    : options_{std::move(options)} {}

auto InjectInstrumentation::run(llvm::Module& llvm_module,
                                llvm::ModuleAnalysisManager&
                                /*unused*/) -> llvm::PreservedAnalyses {
  if (!options_.inject_instrumentation) return llvm::PreservedAnalyses::all();

  Filters filters{{}};
  if (options_.filters_file_path.has_value()) {
    auto filters_result =
        options_.filters_reader->Read(options_.filters_file_path.value());
    if (filters_result.IsErr()) {
      llvm::report_fatal_error(filters_result.Err().message, false);
    }
    filters = std::move(filters_result).Ok();
  }

  auto [symbols, modified] = InstrumentModule(&llvm_module, filters);

  auto preserved_analyses = [modified = modified] {
    if (modified) return llvm::PreservedAnalyses::none();
    return llvm::PreservedAnalyses::all();
  }();

  const auto symbols_write_result =
      options_.symbols_writer->Write(options_.symbols_file_path, symbols);
  if (symbols_write_result.IsErr()) {
    const auto message = [error = symbols_write_result.Err(),
                          file_path = options_.symbols_file_path] {
      switch (error) {
        case SymbolsWriterError::kFailedToOpenFile: {
          return absl::StrFormat("Failed to open the symbols file '%s'.",
                                 file_path);
        }
        case SymbolsWriterError::kSerializationError: {
          return absl::StrFormat(
              "Failed to write the instrumentation symbols to '%s'.",
              file_path);
        }
      }
    }();
    llvm::report_fatal_error(message, false);
  }

  return preserved_analyses;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
auto InjectInstrumentation::InstrumentModule(
    gsl::not_null<llvm::Module*> llvm_module, const Filters& filters) const
    -> InjectInstrumentation::InstrumentModuleResult {
  const auto& module_id = llvm_module->getModuleIdentifier();
  const auto module_hash = internal::ModuleHash(*llvm_module);

  Symbols symbols{};
  auto& function_symbols_table = *symbols.mutable_function_symbols_table();

  auto& context = llvm_module->getContext();
  auto* void_return_type = llvm::Type::getVoidTy(context);
  constexpr auto variadic{true};

  auto* initialization_function_type =
      llvm::FunctionType::get(void_return_type, {}, !variadic);
  const auto initialize_runtime = llvm_module->getOrInsertFunction(
      kInitializeRuntimeFunctionName.data(), initialization_function_type);
  const auto deinitialize_runtime = llvm_module->getOrInsertFunction(
      kDeinitializeRuntimeFunctionName.data(), initialization_function_type);

  auto* enable_function_type =
      llvm::FunctionType::get(void_return_type, {}, !variadic);
  const auto enable_runtime = llvm_module->getOrInsertFunction(
      kEnableRuntimeFunctionName.data(), enable_function_type);

  auto* log_function_type = llvm::FunctionType::get(
      void_return_type, {llvm::Type::getInt64Ty(context)}, !variadic);
  const auto log_function_entry = llvm_module->getOrInsertFunction(
      kLogFunctionEntryFunctionName.data(), log_function_type);
  const auto log_function_exit = llvm_module->getOrInsertFunction(
      kLogFunctionExitFunctionName.data(), log_function_type);

  const auto make_function_id = [module_hash](const uint64 counter) {
    constexpr FunctionId partition{32};
    static_assert(sizeof(module_hash) * 8 == partition);
    return (static_cast<FunctionId>(module_hash) << partition) | counter;
  };

  uint64 counter{0};
  auto modified{false};
  for (auto& function : llvm_module->functions()) {
    if (function.isDeclaration()) continue;
    const auto function_id = make_function_id(counter);
    const auto finally = gsl::finally([&counter] { ++counter; });

    auto& function_infos = function_symbols_table[function_id];
    auto& function_info = *function_infos.add_function_infos();
    function_info.set_module_id(module_id);

    const auto function_name = function.getName().str();
    function_info.set_linkage_name(function_name);

    const auto demangled_name = [&function_name] {
      if (swift::Demangle::isSwiftSymbol(function_name)) {
        return swift::Demangle::demangleSymbolAsString(
            function_name.c_str(), function_name.size(), {});
      }
      std::string sanitized_function_name{};
      std::copy_if(
          std::cbegin(function_name), std::cend(function_name),
          std::back_inserter(sanitized_function_name),
          [](const auto character) { return !std::iscntrl(character); });
      return llvm::demangle(sanitized_function_name);
    }();
    function_info.set_demangled_name(demangled_name);

    const auto ir_instruction_count = function.getInstructionCount();
    function_info.set_ir_instruction_count(ir_instruction_count);

    const auto* subprogram = function.getSubprogram();
    const auto directory =
        [subprogram]() -> std::optional<std::filesystem::path> {
      if (subprogram == nullptr) return {};
      auto directory = subprogram->getDirectory().str();
      if (directory.empty()) return {};
      return directory;
    }();
    const auto file_name =
        [subprogram, &directory]() -> std::optional<std::filesystem::path> {
      if (subprogram == nullptr) return {};
      auto file_name = subprogram->getFilename().str();
      if (file_name.empty()) return {};
      if (!directory.has_value()) return file_name;
      // Sometimes the debug info's file name is an absolute path and other
      // times it is relative to the directory.
      const auto directory_string = directory->string();
      if (!file_name.starts_with(directory_string)) return file_name;
      const std::string separator{std::filesystem::path::preferred_separator};
      return file_name.substr(directory_string.size() + separator.size());
    }();
    const auto line_number = [subprogram]() -> std::optional<int32> {
      if (subprogram == nullptr) return {};
      const auto line_number = subprogram->getLine();
      if (line_number < 1) return {};
      return gsl::narrow_cast<int32>(line_number);
    }();

    if (file_name.has_value()) function_info.set_file_name(file_name->string());
    if (directory.has_value()) function_info.set_directory(directory->string());
    if (line_number.has_value()) function_info.set_line(line_number.value());

    const auto filter_source_file_path = [&directory,
                                          &file_name]() -> std::string {
      if (directory.has_value() && file_name.has_value()) {
        return directory.value() / file_name.value();
      }
      if (directory.has_value()) return directory.value();
      if (file_name.has_value()) return file_name.value();
      return {};
    }();

    *function_info.mutable_created_at() = [&] {
      const auto now = options_.system_clock->Now().time_since_epoch();
      const auto nanoseconds =
          std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
      return TimeUtil::NanosecondsToTimestamp(nanoseconds);
    }();

    const auto is_main = function_name == kMainFunctionName;

    const auto [instrument_function,
                active_filter_rule_name] = filters.InstrumentFunction({
        .source_file_path = filter_source_file_path,
        .demangled_name = demangled_name,
        .linkage_name = function_name,
        .ir_instruction_count = gsl::narrow_cast<int32>(ir_instruction_count),
    });
    function_info.set_instrumented(instrument_function);
    if (active_filter_rule_name.has_value()) {
      function_info.set_instrumented_reason(active_filter_rule_name.value());
    }

    modified |= instrument_function;
    if (!instrument_function) continue;

    llvm::IRBuilder builder{context};
    builder.SetInsertPoint(&*function.getEntryBlock().getFirstInsertionPt());
    if (is_main && options_.initialize_runtime) {
      builder.CreateCall(initialize_runtime);
      if (options_.enable_runtime) builder.CreateCall(enable_runtime);
    }
    builder.CreateCall(log_function_entry, {builder.getInt64(function_id)});
    for (auto& basic_block : function) {
      for (auto& instruction : basic_block) {
        const auto is_return_instruction =
            llvm::ReturnInst::classof(&instruction);
        if (is_return_instruction) {
          builder.SetInsertPoint(&instruction);
          builder.CreateCall(log_function_exit,
                             {builder.getInt64(function_id)});
          if (is_main) builder.CreateCall(deinitialize_runtime);
        }
      }
    }
  }
  return {.symbols = symbols, .modified = modified};
}

}  // namespace spoor::instrumentation::inject_instrumentation
