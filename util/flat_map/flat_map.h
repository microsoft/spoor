// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <initializer_list>
#include <optional>
#include <utility>

namespace util::flat_map {

template <class Key, class Value, std::size_t Size>
class FlatMap {
 public:
  using ValueType = std::pair<Key, Value>;
  using Iterator = ValueType*;
  using ConstIterator = const ValueType*;

  constexpr FlatMap(std::initializer_list<ValueType> data);

  [[nodiscard]] constexpr auto FirstValueForKey(const Key& key) const
      -> std::optional<Value>;
  [[nodiscard]] constexpr auto FirstKeyForValue(const Value& value) const
      -> std::optional<Key>;
  [[nodiscard]] constexpr auto Keys() const -> std::array<Key, Size>;
  [[nodiscard]] constexpr auto Values() const -> std::array<Value, Size>;

  // NOLINTNEXTLINE(readability-identifier-naming)
  [[nodiscard]] constexpr auto size() const -> decltype(Size);
  // NOLINTNEXTLINE(readability-identifier-naming)
  constexpr auto begin() const -> ConstIterator;
  // NOLINTNEXTLINE(readability-identifier-naming)
  constexpr auto begin() -> Iterator;
  // NOLINTNEXTLINE(readability-identifier-naming)
  constexpr auto cbegin() const -> ConstIterator;
  // NOLINTNEXTLINE(readability-identifier-naming)
  constexpr auto end() const -> ConstIterator;
  // NOLINTNEXTLINE(readability-identifier-naming)
  constexpr auto end() -> Iterator;
  // NOLINTNEXTLINE(readability-identifier-naming)
  constexpr auto cend() const -> ConstIterator;

 private:
  std::array<ValueType, Size> data_;

  template <std::size_t... Indices>
  constexpr FlatMap(std::initializer_list<ValueType>& data,
                    std::index_sequence<Indices...> /*unused*/);
};

template <class Key, class Value, std::size_t Size>
constexpr FlatMap<Key, Value, Size>::FlatMap(
    std::initializer_list<ValueType> data)
    : FlatMap{data, std::make_index_sequence<Size>()} {}

template <class Key, class Value, std::size_t Size>
template <std::size_t... Indices>
constexpr FlatMap<Key, Value, Size>::FlatMap(
    std::initializer_list<ValueType>& data,
    std::index_sequence<Indices...> /*unused*/)
    : data_{*(data.begin() + Indices)...} {}

template <class Key, class Value, std::size_t Size>
constexpr auto FlatMap<Key, Value, Size>::FirstValueForKey(const Key& key) const
    -> std::optional<Value> {
  const auto iterator = std::find_if(
      std::cbegin(data_), std::cend(data_),
      [&key](const auto& key_value) { return key_value.first == key; });
  if (iterator == std::cend(data_)) return {};
  return iterator->second;
}

template <class Key, class Value, std::size_t Size>
constexpr auto FlatMap<Key, Value, Size>::FirstKeyForValue(
    const Value& value) const -> std::optional<Key> {
  const auto iterator = std::find_if(
      std::cbegin(data_), std::cend(data_),
      [&value](const auto& key_value) { return key_value.second == value; });
  if (iterator == std::cend(data_)) return {};
  return iterator->first;
}

template <class Key, class Value, std::size_t Size>
constexpr auto FlatMap<Key, Value, Size>::Keys() const
    -> std::array<Key, Size> {
  std::array<Key, Size> keys{};
  std::transform(std::cbegin(data_), std::cend(data_), std::begin(keys),
                 [](const auto& key_value) { return key_value.first; });
  return keys;
}

template <class Key, class Value, std::size_t Size>
constexpr auto FlatMap<Key, Value, Size>::Values() const
    -> std::array<Value, Size> {
  std::array<Value, Size> values{};
  std::transform(std::cbegin(data_), std::cend(data_), std::begin(values),
                 [](const auto& key_value) { return key_value.second; });
  return values;
}

template <class Key, class Value, std::size_t Size>
constexpr auto FlatMap<Key, Value, Size>::size() const -> decltype(Size) {
  return Size;
}

template <class Key, class Value, std::size_t Size>
constexpr auto FlatMap<Key, Value, Size>::begin() const -> ConstIterator {
  return data_.begin();
}

template <class Key, class Value, std::size_t Size>
constexpr auto FlatMap<Key, Value, Size>::begin() -> Iterator {
  return data_.begin();
}

template <class Key, class Value, std::size_t Size>
constexpr auto FlatMap<Key, Value, Size>::cbegin() const -> ConstIterator {
  return data_.cbegin();
}

template <class Key, class Value, std::size_t Size>
constexpr auto FlatMap<Key, Value, Size>::end() const -> ConstIterator {
  return data_.end();
}

template <class Key, class Value, std::size_t Size>
constexpr auto FlatMap<Key, Value, Size>::end() -> Iterator {
  return data_.end();
}

template <class Key, class Value, std::size_t Size>
constexpr auto FlatMap<Key, Value, Size>::cend() const -> ConstIterator {
  return data_.cend();
}

}  // namespace util::flat_map
