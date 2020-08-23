#ifndef SPOOR_UTIL_RESULT_H_
#define SPOOR_UTIL_RESULT_H_

#include <optional>
#include <type_traits>
#include <utility>

namespace util::result {

// Use `Void` to indicate an empty ok or err.
struct Void {};

template <class T, class E>
class Result final {
 public:
  Result() = delete;
  // Construct `Result` when T != E. Single-argument constructors are
  // unavailable when T == E to avoid ambiguity.
  // Note: Not explicit. `Result` is often used as a return value making it
  // convenient and sensible to `return T{}` or `return E{}` when the return
  // type is `Result<T, E>`.
  // NOLINTNEXTLINE(google-explicit-constructor)
  Result(const T&) requires(!std::is_same<T, E>::value);
  // NOLINTNEXTLINE(google-explicit-constructor)
  Result(T&&) requires(!std::is_same<T, E>::value);
  template <class T2 = T>  // NOLINTNEXTLINE(google-explicit-constructor)
  Result(const E& error) requires(!std::is_same<T2, E>::value);
  template <class T2 = T>  // NOLINTNEXTLINE(google-explicit-constructor)
  Result(E&& error) requires(!std::is_same<T2, E>::value);

  // Construct `Result` when T == E or to aid readability.
  constexpr static auto Ok(const T& value) -> Result<T, E>;
  constexpr static auto Ok(T&& value) -> Result<T, E>;
  constexpr static auto Err(const E& error) -> Result<T, E>;
  constexpr static auto Err(E&& error) -> Result<T, E>;

  // `Result` is intended to be immutable. Additionally, only providing `const`
  // accessors ensures that it cannot be corrupted by assigning `std::nullopt`
  // to `Ok()` or `Err()`.
  constexpr auto Ok() const& -> const std::optional<T>&;
  constexpr auto Ok() const&& -> const std::optional<T>&&;
  constexpr auto Err() const& -> const std::optional<E>&;
  constexpr auto Err() const&& -> const std::optional<E>&&;

  [[nodiscard]] constexpr auto IsOk() const -> bool;
  [[nodiscard]] constexpr auto IsErr() const -> bool;

  template <class T2, class E2>
  constexpr auto operator==(const Result<T2, E2>& other) const -> bool;

 private:
  Result(std::optional<T>&& value, std::optional<E>&& error);

  std::optional<T> value_;
  std::optional<E> err_;
};

template <class T, class E>
Result<T, E>::Result(const T& value) requires(!std::is_same<T, E>::value)
    : value_{value}, err_{std::nullopt} {}

template <class T, class E>
Result<T, E>::Result(T&& value) requires(!std::is_same<T, E>::value)
    : value_{value}, err_{std::nullopt} {}

template <class T, class E>
template <class T2>
Result<T, E>::Result(const E& error) requires(!std::is_same<T2, E>::value)
    : value_{std::nullopt}, err_{error} {}

template <class T, class E>
template <class T2>
Result<T, E>::Result(E&& error) requires(!std::is_same<T2, E>::value)
    : value_{std::nullopt}, err_{error} {}

template <class T, class E>
Result<T, E>::Result(std::optional<T>&& value, std::optional<E>&& error)
    : value_{value}, err_{error} {}

template <class T, class E>
constexpr auto Result<T, E>::Ok(const T& value) -> Result<T, E> {
  return Result{value, std::nullopt};
}

template <class T, class E>
constexpr auto Result<T, E>::Ok(T&& value) -> Result<T, E> {
  return Result{value, std::nullopt};
}

template <class T, class E>
constexpr auto Result<T, E>::Err(const E& error) -> Result<T, E> {
  return Result{std::nullopt, error};
}

template <class T, class E>
constexpr auto Result<T, E>::Err(E&& error) -> Result<T, E> {
  return Result{std::nullopt, error};
}

template <class T, class E>
constexpr auto Result<T, E>::Ok() const& -> const std::optional<T>& {
  return value_;
}

template <class T, class E>
constexpr auto Result<T, E>::Ok() const&& -> const std::optional<T>&& {
  return std::move(value_);
}

template <class T, class E>
constexpr auto Result<T, E>::Err() const& -> const std::optional<E>& {
  return err_;
}

template <class T, class E>
constexpr auto Result<T, E>::Err() const&& -> const std::optional<E>&& {
  return std::move(err_);
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
template <class T2, class E2>
constexpr auto Result<T, E>::operator==(const Result<T2, E2>& other) const
    -> bool {
  return value_ == other.value_ && err_ == other.err_;
}

}  // namespace util::result

#endif
