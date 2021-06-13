// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/inject_instrumentation/inject_instrumentation_private.h"

#include "city_hash/city.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/Module.h"
#include "util/numeric.h"

namespace spoor::instrumentation::inject_instrumentation::internal {

auto ModuleHash(const llvm::Module& llvm_module) -> uint32 {
  llvm::SmallVector<char, 0> buffer{};
  llvm::BitcodeWriter bitcode_writer{buffer};
  constexpr auto preserve_user_list_order{false};
  constexpr llvm::ModuleSummaryIndex* index{nullptr};
  constexpr auto generate_hash{true};
  llvm::ModuleHash module_hash{};
  bitcode_writer.writeModule(llvm_module, preserve_user_list_order, index,
                             generate_hash, &module_hash);
  // Fulfills `assert(WroteStrtab)` in BitcodeWriter's destructor.
  bitcode_writer.writeStrtab();
  return CityHash32(
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      reinterpret_cast<const char*>(module_hash.data()),
      sizeof(llvm::ModuleHash::value_type) * module_hash.size());
}

}  // namespace spoor::instrumentation::inject_instrumentation::internal
