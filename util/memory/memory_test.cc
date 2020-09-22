#include <utility>

#include "util/memory/owned_ptr.h"
#include "util/memory/ptr_owner.h"
#include "gtest/gtest.h"
#include "util/numeric.h"

namespace {

class Value;

using PtrOwner = util::memory::PtrOwner<Value>;
using OwnedPtr = util::memory::OwnedPtr<Value>;

class Value {
 public:
  explicit Value(int64 value);
  auto Get() const -> int64;

 private:
  int64 value_;
};

class Owner : public PtrOwner {
 public:
  explicit Owner(int64 value);
  auto Borrow() -> OwnedPtr;
  virtual auto Return(OwnedPtr&& owned_ptr) -> PtrOwner::Result override;
  auto BorrowedSize() const -> uint64;

 protected:
  virtual auto Return(Value* value) -> PtrOwner::Result override;

 private:
  uint64 borrowed_size_;
  Value value_;
};

Value::Value(const int64 value) : value_{value} {}

auto Value::Get() const -> int64 {
  return value_;
};

Owner::Owner(const int64 value) : borrowed_size_{0}, value_{value} {}

auto Owner::Borrow() -> OwnedPtr {
  ++borrowed_size_;
  return {&value_, this};
}

auto Owner::Return(OwnedPtr&& owned_ptr) -> PtrOwner::Result {
  if (owned_ptr.Owner() != this || owned_ptr.Ptr() != &value_) {
    return PtrOwner::Error::kDoesNotOwnPtr;
  }
  --borrowed_size_;
  return PtrOwner::Result::Ok({});
}

auto Owner::BorrowedSize() const -> uint64 {
  return borrowed_size_;
}

auto Owner::Return(Value* value) -> PtrOwner::Result {
  if (value != &value_) {
    return PtrOwner::Error::kDoesNotOwnPtr;
  }
  --borrowed_size_;
  return PtrOwner::Result::Ok({});
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
  ASSERT_EQ(owned_ptr.Ptr(), nullptr);
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
  ASSERT_EQ(owned_ptr.Ptr(), nullptr);
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
  auto ptr = owned_ptr.Ptr();
  ASSERT_NE(ptr, nullptr);
  ASSERT_EQ(owned_ptr.Owner(), &owner);
  auto* unowned_ptr = owned_ptr.Take();
  ASSERT_EQ(owner.BorrowedSize(), 1);
  ASSERT_EQ(unowned_ptr, ptr);
  ASSERT_EQ(owned_ptr.Ptr(), nullptr);
  ASSERT_EQ(owned_ptr.Owner(), nullptr);
}

TEST(OwnedPtr, DereferenceOperator) {  // NOLINT
  const int64 value{42};
  Owner owner{value};
  auto owned_ptr = owner.Borrow();
  ASSERT_EQ((*owned_ptr).Get(), value);
}

TEST(OwnedPtr, ArrowOperator) {  // NOLINT
  const int64 value{42};
  Owner owner{value};
  auto owned_ptr = owner.Borrow();
  ASSERT_EQ(owned_ptr->Get(), value);
}

}  // namespace
