#include "util/result.h"

#include <gtest/gtest.h>

#include <type_traits>

#include "util/numeric.h"

namespace {

using util::result::Result;

enum class OkType { kOk };

enum class ErrType { kErr };

TEST(Result, DefaultConstructorAvailability) {  // NOLINT
  ASSERT_FALSE((std::is_default_constructible_v<Result<OkType, ErrType>>));
}

TEST(Result, ParameterizedSingleArgumentConstructorAvailability) {  // NOLINT
  using util::result::Void;
  ASSERT_TRUE((std::is_constructible_v<Result<OkType, ErrType>, OkType>));
  ASSERT_TRUE((std::is_constructible_v<Result<OkType, ErrType>, ErrType>));
  ASSERT_FALSE((std::is_constructible_v<Result<Void, Void>, Void>));
}

TEST(Result, ParameterizedTwoArgumentConstructorAvailability) {  // NOLINT
  ASSERT_FALSE(
      (std::is_constructible_v<Result<OkType, ErrType>, OkType, ErrType>));
}

TEST(Result, CopyConstructorAvailability) {  // NOLINT
  ASSERT_TRUE((std::is_copy_constructible_v<Result<OkType, ErrType>>));
}

TEST(Result, MoveConstructorAvailability) {  // NOLINT
  ASSERT_TRUE((std::is_move_constructible_v<Result<OkType, ErrType>>));
}

TEST(Result, CopyAssignmentAvailability) {  // NOLINT
  ASSERT_TRUE((std::is_copy_assignable_v<Result<OkType, ErrType>>));
}

TEST(Result, MoveAssignmentAvailability) {  // NOLINT
  ASSERT_TRUE((std::is_move_assignable_v<Result<OkType, ErrType>>));
}

TEST(Result, DestructorAvailability) {  // NOLINT
  ASSERT_TRUE((std::is_destructible_v<Result<OkType, ErrType>>));
  ASSERT_FALSE((std::has_virtual_destructor_v<Result<OkType, ErrType>>));
}

TEST(Result, Swappable) {  // NOLINT
  ASSERT_TRUE((std::is_swappable_v<Result<OkType, ErrType>>));
}

TEST(Result, Polymorphism) {  // NOLINT
  ASSERT_FALSE((std::is_polymorphic_v<Result<OkType, ErrType>>));
}

TEST(Result, Final) {  // NOLINT
  ASSERT_TRUE((std::is_final_v<Result<OkType, ErrType>>));
}

TEST(Result, OkExplicitLvalueConstructor) {  // NOLINT
  const auto ok = OkType::kOk;
  const Result<OkType, ErrType> result{ok};
  ASSERT_EQ(result.Ok().value(), ok);
  ASSERT_FALSE(result.Err().has_value());
}

TEST(Result, OkImplicitLvalueConstructor) {  // NOLINT
  const auto ok = OkType::kOk;
  const Result<OkType, ErrType> result = ok;
  ASSERT_EQ(result.Ok().value(), ok);
  ASSERT_FALSE(result.Err().has_value());
}

TEST(Result, OkExplicitRvalueConstructor) {  // NOLINT
  const Result<OkType, ErrType> result{OkType::kOk};
  ASSERT_EQ(result.Ok().value(), OkType::kOk);
  ASSERT_FALSE(result.Err().has_value());
}

TEST(Result, OkImplicitRvalueConstructor) {  // NOLINT
  const Result<OkType, ErrType> result = OkType::kOk;
  ASSERT_EQ(result.Ok().value(), OkType::kOk);
  ASSERT_FALSE(result.Err().has_value());
}

TEST(Result, ErrorExplicitLvalueConstructor) {  // NOLINT
  const auto err = ErrType::kErr;
  const Result<OkType, ErrType> result{err};
  ASSERT_FALSE(result.Ok().has_value());
  ASSERT_EQ(result.Err().value(), err);
}

TEST(Result, ErrorImplicitLvalueConstructor) {  // NOLINT
  const auto err = ErrType::kErr;
  const Result<OkType, ErrType> result = err;
  ASSERT_FALSE(result.Ok().has_value());
  ASSERT_EQ(result.Err().value(), err);
}

TEST(Result, ErrorExplicitRvalueConstructor) {  // NOLINT
  const Result<OkType, ErrType> result{ErrType::kErr};
  ASSERT_FALSE(result.Ok().has_value());
  ASSERT_EQ(result.Err().value(), ErrType::kErr);
}

TEST(Result, ErrorImplicitRvalueConstructor) {  // NOLINT
  const auto err = ErrType::kErr;
  const Result<OkType, ErrType> result = err;
  ASSERT_FALSE(result.Ok().has_value());
  ASSERT_EQ(result.Err().value(), err);
}

TEST(Result, OkStaticLvalueConstructor) {  // NOLINT
  const int32 ok{42};
  const auto result = Result<int32, int32>::Ok(ok);
  ASSERT_EQ(result.Ok().value(), ok);
  ASSERT_FALSE(result.Err().has_value());
}

TEST(Result, OkStaticRvalueConstructor) {  // NOLINT
  const auto result = Result<int32, int32>::Ok(42);
  ASSERT_EQ(result.Ok().value(), 42);
  ASSERT_FALSE(result.Err().has_value());
}

TEST(Result, ErrStaticLvalueConstructor) {  // NOLINT
  const int32 err{42};
  const auto result = Result<int32, int32>::Err(err);
  ASSERT_FALSE(result.Ok().has_value());
  ASSERT_EQ(result.Err().value(), err);
}

TEST(Result, ErrStaticRvalueConstructor) {  // NOLINT
  const auto result = Result<int32, int32>::Err(42);
  ASSERT_FALSE(result.Ok().has_value());
  ASSERT_EQ(result.Err().value(), 42);
}

TEST(Result, OkLvalue) {  // NOLINT
  Result<OkType, ErrType> result{OkType::kOk};
  ASSERT_FALSE((std::is_assignable_v<decltype(result.Ok()), std::nullopt_t>));
  ASSERT_FALSE((std::is_assignable_v<decltype(result.Ok().value()), OkType>));
}

TEST(Result, OkConstLvalue) {  // NOLINT
  const Result<OkType, ErrType> result{OkType::kOk};
  ASSERT_EQ(result.Ok().value(), OkType::kOk);
}

TEST(Result, OkRvalue) {  // NOLINT
  ASSERT_FALSE((std::is_assignable_v<
                decltype(Result<OkType, ErrType>{OkType::kOk}.Ok().value()),
                OkType>));
}

TEST(Result, OkConstRvalue) {  // NOLINT
  const auto ok = OkType::kOk;
  ASSERT_EQ((Result<OkType, ErrType>{ok}.Ok().value()), ok);
}

TEST(Result, ErrLvalue) {  // NOLINT
  Result<OkType, ErrType> result{ErrType::kErr};
  ASSERT_FALSE((std::is_assignable_v<decltype(result.Err()), std::nullopt_t>));
  ASSERT_FALSE((std::is_assignable_v<decltype(result.Err().value()), ErrType>));
}

TEST(Result, ErrConstLvalue) {  // NOLINT
  const Result<OkType, ErrType> result{ErrType::kErr};
  ASSERT_EQ(result.Err().value(), ErrType::kErr);
}

TEST(Result, ErrRvalue) {  // NOLINT
  ASSERT_FALSE((std::is_assignable_v<
                decltype(Result<OkType, ErrType>{ErrType::kErr}.Err().value()),
                ErrType>));
}

TEST(Result, ErrConstRvalue) {  // NOLINT
  const auto err = ErrType::kErr;
  ASSERT_EQ((Result<OkType, ErrType>{err}.Err().value()), err);
}

TEST(Result, IsOk) {  // NOLINT
  const Result<OkType, ErrType> ok_result{OkType::kOk};
  ASSERT_TRUE(ok_result.IsOk());
  const Result<OkType, ErrType> err_result{ErrType::kErr};
  ASSERT_FALSE(err_result.IsOk());
}

TEST(Result, IsErr) {  // NOLINT
  const Result<OkType, ErrType> ok_result{OkType::kOk};
  ASSERT_FALSE(ok_result.IsErr());
  const Result<OkType, ErrType> err_result{ErrType::kErr};
  ASSERT_TRUE(err_result.IsErr());
}

TEST(Result, EqualityOperator) {  // NOLINT
  const auto ok_result_0_a = Result<int32, int32>::Ok(0);
  const auto ok_result_0_b = Result<int32, int32>::Ok(0);
  ASSERT_EQ(ok_result_0_a, ok_result_0_b);
  const auto ok_result_1 = Result<int32, int32>::Ok(1);
  ASSERT_NE(ok_result_0_a, ok_result_1);
  const auto err_result_0_a = Result<int32, int32>::Err(0);
  const auto err_result_0_b = Result<int32, int32>::Err(0);
  ASSERT_EQ(err_result_0_a, err_result_0_b);
  const auto err_result_1 = Result<int32, int32>::Err(1);
  ASSERT_NE(err_result_0_a, err_result_1);
  ASSERT_NE(ok_result_0_a, err_result_0_a);
}

}  // namespace