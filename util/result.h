// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <optional>
#include <type_traits>
#include <utility>

namespace util::result {

// Use `None` to indicate an empty Ok or Err.
// `__attribute__((aligned(0)))` is not a power of two and `alignas(0)` is
// always ignored. NOLINTNEXTLINE(altera-struct-pack-align)
struct None {};

// `Result<T, E>` is the type used to return and propagate errors. It stores
// either the return value `T` representing a success or an error `E`.
template <class T, class E>
class Result {
 public:
  Result() = delete;
  // Construct `Result` when T != E. Single-argument constructors are
  // unavailable when T == E to avoid ambiguity.
  // Note: Not explicit. `Result` is often used as a return value making it
  // convenient and sensible to `return T{}` or `return E{}` when the return
  // type is `Result<T, E>`.
  template <class T2 = T, class = std::enable_if_t<!std::is_same_v<T2, E>>>
  constexpr Result(const T& value);  // NOLINT(google-explicit-constructor)
  template <class T2 = T, class = std::enable_if_t<!std::is_same_v<T2, E>>>
  constexpr Result(T&& value);  // NOLINT(google-explicit-constructor)
  template <class T2 = T, class E2 = E,
            class = std::enable_if_t<!std::is_same_v<T2, E2>>>
  constexpr Result(const E& error);  // NOLINT(google-explicit-constructor)
  template <class T2 = T, class E2 = E,
            class = std::enable_if_t<!std::is_same_v<T2, E2>>>
  constexpr Result(E&& error);  // NOLINT(google-explicit-constructor)

  // Construct `Result` when T == E or to aid readability.
  [[nodiscard]] constexpr static auto Ok(const T& value) -> Result<T, E>;
  [[nodiscard]] constexpr static auto Ok(T&& value) -> Result<T, E>;
  [[nodiscard]] constexpr static auto Err(const E& error) -> Result<T, E>;
  [[nodiscard]] constexpr static auto Err(E&& error) -> Result<T, E>;

  [[nodiscard]] constexpr auto IsOk() const -> bool;
  [[nodiscard]] constexpr auto IsErr() const -> bool;

  [[nodiscard]] constexpr auto Ok() const& -> const T&;
  [[nodiscard]] constexpr auto Ok() & -> T&;
  [[nodiscard]] constexpr auto Ok() const&& -> const T&&;
  [[nodiscard]] constexpr auto Ok() && -> T&&;
  [[nodiscard]] constexpr auto Err() const& -> const E&;
  [[nodiscard]] constexpr auto Err() & -> E&;
  [[nodiscard]] constexpr auto Err() const&& -> const E&&;
  [[nodiscard]] constexpr auto Err() && -> E&&;

  template <class U>
  [[nodiscard]] constexpr auto OkOr(U&& default_value) const& -> T;
  template <class U>
  [[nodiscard]] constexpr auto OkOr(U&& default_value) && -> T;
  template <class U>
  [[nodiscard]] constexpr auto ErrOr(U&& default_value) const& -> E;
  template <class U>
  [[nodiscard]] constexpr auto ErrOr(U&& default_value) && -> E;

 private:
  template <class T1, class T2, class E1, class E2>
  // False positive. NOLINTNEXTLINE(readability-redundant-declaration)
  friend constexpr auto operator==(const Result<T1, E1>& lhs,
                                   const Result<T2, E2>& rhs) -> bool;

  constexpr Result(std::optional<T>&& value, std::optional<E>&& error);

  std::optional<T> value_;
  std::optional<E> err_;
};

static_assert(std::is_constructible_v<Result<char, double>, char>);
static_assert(std::is_constructible_v<Result<char, double>, double>);
static_assert(!std::is_constructible_v<Result<char, char>, char>);

template <class T, class E>
template <class T2, class>
constexpr Result<T, E>::Result(const T& value)
    : value_{value}, err_{std::nullopt} {}

template <class T, class E>
template <class T2, class>
constexpr Result<T, E>::Result(T&& value)
    : value_{std::move(value)}, err_{std::nullopt} {}

template <class T, class E>
template <class T2, class E2, class>
constexpr Result<T, E>::Result(const E& error)
    : value_{std::nullopt}, err_{error} {}

template <class T, class E>
template <class T2, class E2, class>
constexpr Result<T, E>::Result(E&& error)
    : value_{std::nullopt}, err_{std::move(error)} {}

template <class T, class E>
constexpr Result<T, E>::Result(std::optional<T>&& value,
                               std::optional<E>&& error)
    : value_{std::move(value)}, err_{std::move(error)} {}

template <class T, class E>
constexpr auto Result<T, E>::Ok(const T& value) -> Result<T, E> {
  return Result{value, std::nullopt};
}

template <class T, class E>
constexpr auto Result<T, E>::Ok(T&& value) -> Result<T, E> {
  return Result{std::move(value), std::nullopt};
}

template <class T, class E>
constexpr auto Result<T, E>::Err(const E& error) -> Result<T, E> {
  return Result{std::nullopt, error};
}

template <class T, class E>
constexpr auto Result<T, E>::Err(E&& error) -> Result<T, E> {
  return Result{std::nullopt, std::move(error)};
}

template <class T, class E>
constexpr auto Result<T, E>::IsOk() const -> bool {
  return value_.has_value();
}

template <class T, class E>
constexpr auto Result<T, E>::IsErr() const -> bool {
  return err_.has_value();
}

template <class T, class E>
constexpr auto Result<T, E>::Ok() const& -> const T& {
  return value_.value();
}

template <class T, class E>
constexpr auto Result<T, E>::Ok() & -> T& {
  return value_.value();
}

template <class T, class E>
constexpr auto Result<T, E>::Ok() const&& -> const T&& {
  return std::move(value_.value());
}

template <class T, class E>
constexpr auto Result<T, E>::Ok() && -> T&& {
  return std::move(value_.value());
}

template <class T, class E>
constexpr auto Result<T, E>::Err() const& -> const E& {
  return err_.value();
}

template <class T, class E>
constexpr auto Result<T, E>::Err() & -> E& {
  return err_.value();
}

template <class T, class E>
constexpr auto Result<T, E>::Err() const&& -> const E&& {
  return std::move(err_.value());
}

template <class T, class E>
constexpr auto Result<T, E>::Err() && -> E&& {
  return std::move(err_.value());
}

template <class T, class E>
template <class U>
constexpr auto Result<T, E>::OkOr(U&& default_value) const& -> T {
  return value_.value_or(default_value);
}

template <class T, class E>
template <class U>
constexpr auto Result<T, E>::OkOr(U&& default_value) && -> T {
  return value_.value_or(default_value);
}

template <class T, class E>
template <class U>
constexpr auto Result<T, E>::ErrOr(U&& default_value) const& -> E {
  return err_.value_or(default_value);
}

template <class T, class E>
template <class U>
constexpr auto Result<T, E>::ErrOr(U&& default_value) && -> E {
  return err_.value_or(default_value);
}

template <class T1, class T2, class E1, class E2>
constexpr auto operator==(const Result<T1, E1>& lhs, const Result<T2, E2>& rhs)
    -> bool {
  return lhs.value_ == rhs.value_ && lhs.err_ == rhs.err_;
}

}  // namespace util::result
