#include "util/result.h"

#include "gtest/gtest.h"
#include "util/numeric.h"

namespace {

using util::result::Result;

enum class OkType { kOk };
enum class ErrType { kErr };

TEST(Result, OkExplicitLvalueConstructor) {  // NOLINT
  const auto ok = OkType::kOk;
  const Result<OkType, ErrType> result{ok};
  ASSERT_EQ(result.Ok(), ok);
  ASSERT_FALSE(result.IsErr());
}

TEST(Result, OkImplicitLvalueConstructor) {  // NOLINT
  const auto ok = OkType::kOk;
  const Result<OkType, ErrType> result = ok;
  ASSERT_EQ(result.Ok(), ok);
  ASSERT_FALSE(result.IsErr());
}

TEST(Result, OkExplicitRvalueConstructor) {  // NOLINT
  const Result<OkType, ErrType> result{OkType::kOk};
  ASSERT_EQ(result.Ok(), OkType::kOk);
  ASSERT_FALSE(result.IsErr());
}

TEST(Result, OkImplicitRvalueConstructor) {  // NOLINT
  const Result<OkType, ErrType> result = OkType::kOk;
  ASSERT_EQ(result.Ok(), OkType::kOk);
  ASSERT_FALSE(result.IsErr());
}

TEST(Result, ErrorExplicitLvalueConstructor) {  // NOLINT
  const auto err = ErrType::kErr;
  const Result<OkType, ErrType> result{err};
  ASSERT_FALSE(result.IsOk());
  ASSERT_EQ(result.Err(), err);
}

TEST(Result, ErrorImplicitLvalueConstructor) {  // NOLINT
  const auto err = ErrType::kErr;
  const Result<OkType, ErrType> result = err;
  ASSERT_FALSE(result.IsOk());
  ASSERT_EQ(result.Err(), err);
}

TEST(Result, ErrorExplicitRvalueConstructor) {  // NOLINT
  const Result<OkType, ErrType> result{ErrType::kErr};
  ASSERT_FALSE(result.IsOk());
  ASSERT_EQ(result.Err(), ErrType::kErr);
}

TEST(Result, ErrorImplicitRvalueConstructor) {  // NOLINT
  const auto err = ErrType::kErr;
  const Result<OkType, ErrType> result = err;
  ASSERT_FALSE(result.IsOk());
  ASSERT_EQ(result.Err(), err);
}

TEST(Result, OkStaticLvalueConstructor) {  // NOLINT
  const int64 ok{42};
  const auto result = Result<int64, int64>::Ok(ok);
  ASSERT_EQ(result.Ok(), ok);
  ASSERT_FALSE(result.IsErr());
}

TEST(Result, OkStaticRvalueConstructor) {  // NOLINT
  const auto result = Result<int64, int64>::Ok(42);
  ASSERT_EQ(result.Ok(), 42);
  ASSERT_FALSE(result.IsErr());
}

TEST(Result, ErrStaticLvalueConstructor) {  // NOLINT
  const int64 err{42};
  const auto result = Result<int64, int64>::Err(err);
  ASSERT_FALSE(result.IsOk());
  ASSERT_EQ(result.Err(), err);
}

TEST(Result, ErrStaticRvalueConstructor) {  // NOLINT
  const auto result = Result<int64, int64>::Err(42);
  ASSERT_FALSE(result.IsOk());
  ASSERT_EQ(result.Err(), 42);
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

TEST(Result, OkConstLvalue) {  // NOLINT
  const Result<OkType, ErrType> result{OkType::kOk};
  ASSERT_EQ(result.Ok(), OkType::kOk);
}

TEST(Result, OkLvalue) {  // NOLINT
  Result<int64, ErrType> result{42};
  ASSERT_EQ(result.Ok(), 42);
  result.Ok() += 1;
  ASSERT_EQ(result.Ok(), 43);
}

TEST(Result, OkConstRvalue) {  // NOLINT
  Result<int64, ErrType> result{42};
  // NOLINTNEXTLINE(performance-move-const-arg)
  ASSERT_EQ(std::move(result).Ok(), 42);
}

TEST(Result, OkRvalue) {  // NOLINT
  const Result<int64, ErrType> result{42};
  // NOLINTNEXTLINE(performance-move-const-arg)
  ASSERT_EQ(std::move(result).Ok(), 42);
}

TEST(Result, ErrConstLvalue) {  // NOLINT
  const Result<OkType, ErrType> result{ErrType::kErr};
  ASSERT_EQ(result.Err(), ErrType::kErr);
}

TEST(Result, ErrLvalue) {  // NOLINT
  Result<OkType, int64> result{42};
  ASSERT_EQ(result.Err(), 42);
  result.Err() += 1;
  ASSERT_EQ(result.Err(), 43);
}

TEST(Result, ErrConstRvalue) {  // NOLINT
  const Result<OkType, int64> result{42};
  // NOLINTNEXTLINE(performance-move-const-arg)
  ASSERT_EQ(std::move(result).Err(), 42);
}

TEST(Result, ErrRvalue) {  // NOLINT
  Result<OkType, int64> result{42};
  // NOLINTNEXTLINE(performance-move-const-arg)
  ASSERT_EQ(std::move(result).Err(), 42);
}

TEST(Result, OkOrLvalue) {  // NOLINT
  const Result<int64, ErrType> result{ErrType::kErr};
  const auto x = result.OkOr(42);
  ASSERT_EQ(x, 42);
}

TEST(Result, OkOrRvalue) {  // NOLINT
  Result<int64, ErrType> result{ErrType::kErr};
  // NOLINTNEXTLINE(performance-move-const-arg)
  const auto x = std::move(result).OkOr(42);
  ASSERT_EQ(x, 42);
}

TEST(Result, ErrOrLvalue) {  // NOLINT
  const Result<OkType, int64> result{OkType::kOk};
  const auto x = result.ErrOr(42);
  ASSERT_EQ(x, 42);
}

TEST(Result, ErrOrRvalue) {  // NOLINT
  Result<OkType, int64> result{OkType::kOk};
  // NOLINTNEXTLINE(performance-move-const-arg)
  const auto x = std::move(result).ErrOr(42);
  ASSERT_EQ(x, 42);
}

TEST(Result, EqualityOperator) {  // NOLINT
  using Result = Result<int64, int64>;
  ASSERT_EQ(Result::Ok(0), Result::Ok(0));
  ASSERT_NE(Result::Ok(0), Result::Ok(1));
  ASSERT_EQ(Result::Err(0), Result::Err(0));
  ASSERT_NE(Result::Err(0), Result::Err(1));
  ASSERT_NE(Result::Ok(0), Result::Err(0));
}

}  // namespace
