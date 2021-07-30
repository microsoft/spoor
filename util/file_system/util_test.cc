// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "util/file_system/util.h"

#include <array>
#include <cstring>
#include <string>
#include <string_view>

#include "gtest/gtest.h"
#include "util/env/env.h"

namespace {

using util::file_system::ExpandTilde;

constexpr std::string_view kHome{"/usr/you"};

auto GetEnv(const char* key) -> const char* {
  if (key == nullptr) return nullptr;
  if (std::strncmp(key, util::env::kHomeKey.data(),
                   util::env::kHomeKey.size()) == 0) {
    return kHome.data();
  }
  return nullptr;
}

TEST(ExpandTilde, ExpandsTilde) {  // NOLINT
  constexpr auto path{"~/path/to/file.extension"};
  const auto expanded = ExpandTilde(path, GetEnv);
  ASSERT_EQ(expanded, "/usr/you/path/to/file.extension");
}

TEST(ExpandTilde, DoesNotExpandTilde) {  // NOLINT
  const std::array<std::string, 3> paths{{
      "/usr/you/path/to/file.extension",
      "~path/to/file.extension",
      "/usr/you/~/to/file.extension",
  }};
  for (const auto& path : paths) {
    const auto expanded = ExpandTilde(path, GetEnv);
    ASSERT_EQ(expanded, path);
  }
}

TEST(ExpandTilde, HandlesNonexistentHomeEnvironmentVariable) {  // NOLINT
  constexpr auto path{"~/path/to/file.extension"};
  const auto expanded =
      ExpandTilde(path, [](const char* /*unused*/) { return nullptr; });
  ASSERT_EQ(expanded, path);
}

}  // namespace
