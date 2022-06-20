// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <utility>

#include "gtest/gtest.h"
#include "util/memory/owned_ptr.h"
#include "util/memory/ptr_owner.h"
#include "util/numeric.h"

namespace {

class Value;

using PtrOwner = util::memory::PtrOwner<Value>;
using OwnedPtr = util::memory::OwnedPtr<Value>;

class Value {
 public:
  constexpr explicit Value(int32 value);
  [[nodiscard]] constexpr auto Get() const -> int32;

 private:
  int32 value_;
};

class Owner : public PtrOwner {
 public:
  constexpr explicit Owner(int32 value);
  constexpr auto Borrow() -> OwnedPtr;
  auto Return(OwnedPtr&& owned_ptr) -> PtrOwner::Result override;
  [[nodiscard]] constexpr auto BorrowedSize() const -> uint32;

 protected:
  auto Return(Value* value) -> PtrOwner::ReturnRawPtrResult override;

 private:
  uint32 borrowed_size_{0};
  Value value_;
};

constexpr Value::Value(const int32 value) : value_{value} {}

constexpr auto Value::Get() const -> int32 { return value_; };

constexpr Owner::Owner(const int32 value) : value_{value} {}

constexpr auto Owner::Borrow() -> OwnedPtr {
  ++borrowed_size_;
  return {&value_, this};
}

auto Owner::Return(OwnedPtr&& owned_ptr) -> PtrOwner::Result {
  if (owned_ptr.Owner() != this || owned_ptr.Ptr() != &value_) {
    return std::move(owned_ptr);
  }
  --borrowed_size_;
  return PtrOwner::Result::Ok({});
}

constexpr auto Owner::BorrowedSize() const -> uint32 { return borrowed_size_; }

auto Owner::Return(Value* value) -> PtrOwner::ReturnRawPtrResult {
  if (value != &value_) return PtrOwner::ReturnRawPtrResult::Err({});
  --borrowed_size_;
  return PtrOwner::ReturnRawPtrResult::Ok({});
}

TEST(OwnedPtr, MoveConstructor) {  // NOLINT
  Owner owner{42};
  auto owned_ptr = owner.Borrow();
  const auto* value = owned_ptr.Ptr();
  ASSERT_NE(value, nullptr);
  ASSERT_EQ(owned_ptr.Owner(), &owner);
  OwnedPtr new_owned_ptr{std::move(owned_ptr)};
  ASSERT_EQ(new_owned_ptr.Ptr(), value);
  ASSERT_EQ(new_owned_ptr.Owner(), &owner);
  // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move)
  ASSERT_EQ(owned_ptr.Ptr(), nullptr);
  // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move)
  ASSERT_EQ(owned_ptr.Owner(), nullptr);
}

TEST(OwnedPtr, MoveAssignmentOperator) {  // NOLINT
  Owner owner{42};
  auto owned_ptr = owner.Borrow();
  const auto* value = owned_ptr.Ptr();
  ASSERT_NE(value, nullptr);
  ASSERT_EQ(owned_ptr.Owner(), &owner);
  auto new_owned_ptr = std::move(owned_ptr);
  ASSERT_EQ(new_owned_ptr.Ptr(), value);
  ASSERT_EQ(new_owned_ptr.Owner(), &owner);
  // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move)
  ASSERT_EQ(owned_ptr.Ptr(), nullptr);
  // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move)
  ASSERT_EQ(owned_ptr.Owner(), nullptr);
}

TEST(OwnedPtr, ManualReturnToPool) {  // NOLINT
  Owner owner{42};
  ASSERT_EQ(owner.BorrowedSize(), 0);
  auto owned_ptr = owner.Borrow();
  ASSERT_EQ(owner.BorrowedSize(), 1);
  auto result = owner.Return(std::move(owned_ptr));
  ASSERT_TRUE(result.IsOk());
  ASSERT_EQ(owner.BorrowedSize(), 0);
}

TEST(OwnedPtr, AutomaticReturnToPool) {  // NOLINT
  Owner owner{42};
  ASSERT_EQ(owner.BorrowedSize(), 0);
  {
    auto owned_ptr = owner.Borrow();
    ASSERT_EQ(owner.BorrowedSize(), 1);
  }
  ASSERT_EQ(owner.BorrowedSize(), 0);
}

TEST(OwnedPtr, Take) {  // NOLINT
  Owner owner{42};
  ASSERT_EQ(owner.BorrowedSize(), 0);
  auto owned_ptr = owner.Borrow();
  auto* ptr = owned_ptr.Ptr();
  ASSERT_NE(ptr, nullptr);
  ASSERT_EQ(owned_ptr.Owner(), &owner);
  auto* unowned_ptr = owned_ptr.Take();
  ASSERT_EQ(owner.BorrowedSize(), 1);
  ASSERT_EQ(unowned_ptr, ptr);
  ASSERT_EQ(owned_ptr.Ptr(), nullptr);
  ASSERT_EQ(owned_ptr.Owner(), nullptr);
}

TEST(OwnedPtr, DereferenceOperator) {  // NOLINT
  constexpr int32 value{42};
  Owner owner{value};
  auto owned_ptr = owner.Borrow();
  ASSERT_EQ((*owned_ptr).Get(), value);
}

TEST(OwnedPtr, ArrowOperator) {  // NOLINT
  constexpr int32 value{42};
  Owner owner{value};
  auto owned_ptr = owner.Borrow();
  ASSERT_EQ(owned_ptr->Get(), value);
}

}  // namespace
