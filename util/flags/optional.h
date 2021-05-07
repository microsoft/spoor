// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <optional>
#include <utility>

#include "absl/strings/string_view.h"

namespace util::flags {

// `std::optional<T>` and `absl::optional<T>` do not work with `absl::Flags`
// because their constructors are ambiguous with absl::string_view.

template <class T>
class Optional {
 public:
  constexpr Optional() = default;
  constexpr Optional(const T& item);  // NOLINT(google-explicit-constructor)
  constexpr Optional(T&& item);       // NOLINT(google-explicit-constructor)
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr Optional(std::optional<T> optional);

  [[nodiscard]] constexpr auto HasValue() const -> bool;
  [[nodiscard]] constexpr auto Value() const& -> const T&;
  [[nodiscard]] constexpr auto Value() & -> T&;
  [[nodiscard]] constexpr auto Value() const&& -> const T&&;
  [[nodiscard]] constexpr auto Value() && -> T&&;
  [[nodiscard]] constexpr auto ValueOr(T&& other) const& -> T;
  [[nodiscard]] constexpr auto ValueOr(T&& other) && -> T;
  [[nodiscard]] constexpr auto StdOptional() const -> std::optional<T>;

 private:
  std::optional<T> item_;
};

template <class T>
auto AbslParseFlag(absl::string_view user_value, Optional<T>* optional,
                   std::string* error) -> bool;

template <class T>
auto AbslUnparseFlag(const Optional<T>& optional_value) -> std::string;

template <class T>
constexpr Optional<T>::Optional(const T& item) : item_{item} {}

template <class T>
constexpr Optional<T>::Optional(T&& item) : item_{std::move(item)} {}

template <class T>
constexpr Optional<T>::Optional(std::optional<T> optional)
    : item_{std::move(optional)} {}

template <class T>
constexpr auto Optional<T>::HasValue() const -> bool {
  return item_.has_value();
}

template <class T>
constexpr auto Optional<T>::Value() const& -> const T& {
  return item_.value();
}

template <class T>
constexpr auto Optional<T>::Value() & -> T& {
  return item_.value();
}

template <class T>
constexpr auto Optional<T>::Value() const&& -> const T&& {
  return std::move(item_.value());
}

template <class T>
constexpr auto Optional<T>::Value() && -> T&& {
  return std::move(item_.value());
}

template <class T>
constexpr auto Optional<T>::ValueOr(T&& other) const& -> T {
  return item_.value_or(other);
}

template <class T>
constexpr auto Optional<T>::ValueOr(T&& other) && -> T {
  return item_.value_or(other);
}

template <class T>
constexpr auto Optional<T>::StdOptional() const -> std::optional<T> {
  if (item_.has_value()) return item_.value();
  return {};
}

template <class T>
auto AbslUnparseFlag(const Optional<T>& optional_value) -> std::string {
  if (optional_value.HasValue()) return std::string{optional_value.Value()};
  return "none";
}

}  // namespace util::flags
