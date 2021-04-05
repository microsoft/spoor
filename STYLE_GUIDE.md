# Spoor Style Guide

## General

### Comments
Use code comments sparingly. Comments should explain *why* a decision was made,
not *what* the code is doing.

Comments should be complete English sentences including the necessary articles
("a", "an", and "the") and punctuation, and should follow
[Microsoft's Writing Style Guide][microsoft-writing-style-guide].

### Inequality operators
Prefer using the `<` `<=>` or operators over `<=`, `>`, and `>=`.

## C++

Follow [Google's C++ Style Guide][google-cpp-style-guide].

### Function declarations

Exception: Use trailing return types for all function, method, and labmda
declarations. Functions and methods may not use automatic return type inference.

```c++
// ✅ Do this
auto Add(const int64 a, const int64 b) -> int64;

// ❌ Not this
int64 Add(const int64 a, const int64 b);
```

### Header guard

Exception: Use `#pragma once` instead of `#define` guard for header files. This
project's tooling does not support automatically enforcing the header guard
convention where the `#define` guard's identifier mirrors the file's path to
guarantee uniqueness. Despite being non-standard, `#pragma once` carries
advantages such as reducing the maintenance burden when renaming and moving
files, and in some cases improves compilation speed.

```c++
// ✅ Do this
#pragma once

// ❌ Not this
#ifndef SPOOR_PATH_TO_HEADER_H_
#define SPOOR_PATH_TO_HEADER_H_
...
#endif
```

### Initialization

Prefer list (curly brace) initialization over other forms. List initialization
does not allow type narrowing making it safer than its alternatives.

```c++
MyClass::MyClass(const int64 value) : value_{value} {...}

const int64 x{42};
const MyClass my_class{x};
```

### Error handling

Do not use C++ exceptions for error handling. In fact, Spoor disables exceptions
by compiling with `-fno-exceptions`.

Instead, handle errors with Spoor's [`Result<Value, Error>`][util-result-h]
type used to return either a value on success or an error otherwise.

### Numeric values

Use numeric types with a defined size. For convenience, integer types without
the `_t` postifx are defined in [util/numeric.h][util-numeric-h]. Prefer using
these `typedef`s over the values defined in `<cstdint>`.

```c++
// ✅ Do this
#include "util/numerics.h"

int64 x{42};

// ❌ Not this
#include <cstdint>

int x{42};
int64_t y{7};
```

### Tools

Use [Clang Format][clang-format] and [Clang Tidy][clang-tidy] to style and lint
code.

In special circumstances it might make sense to ignore lint warnings, for
example, when using an external library macro where the lint warning is
unavoidable. Explicitly list the lint rules to ignore instead of using a blanket
`NOLINT` or `NOLINTNEXTLINE`.

```c++
// ✅ Do this
int64 a[] = {1, 2, 3};  // NOLINT(modernize-avoid-c-arrays)

// ❌ Not this
int64 a[] = {1, 2, 3};  // NOLINT
```

Exception: Google Test and Abseil flags library macros.

```c++
// ✅ Okay
TEST(Foo, Bar) { // NOLINT
  ...
}

// ✅ Okay
ABSL_FLAG(  // NOLINT
    std::string, my_flag, {}, "Description...");
```

## Objective-C

Follow Google's [Objective-C Style Guide](https://google.github.io/styleguide/objcguide.html).

## Starlark (Bazel)

Follow [Starlark's Style Guide][starlark-style-guide].

Use [Buildifier][buildifier] to style and lint Starlark files.

## Shell

Follow [Google's Shell Style Guide][google-shell-style-guide].

### Shebang

Exception: Use the shebang `#!/usr/bin/env bash` which is more portable.

```bash
# ✅ Do this
#!/usr/bin/env bash

# ❌ Not this
#!/bin/bash
```

## Markdown

Follow [Google's Markdown Style Guide][google-markdown-style-guide].

### Character line limit

Each line should be at most 80 characters long.

Exception: URLs and tables which cannot be wrapped.

### Links

Use reference-style links for long URLs. Use `kebab-case` for reference names,
place references at the end of the file, and alphabetize the list.

```markdown
✅ Do this
Follow [Google's Markdown Style Guide][google-markdown-style-guide].

[google-markdown-style-guide]: https://google.github.io/styleguide/docguide/style.html#document-layout

❌ Not this
Follow
[Google's Markdown Style Guide](https://google.github.io/styleguide/docguide/style.html#document-layout).
```

[buildifier]: https://github.com/bazelbuild/buildtools/blob/master/buildifier/README.md
[clang-format]: https://clang.llvm.org/docs/ClangFormat.html
[clang-tidy]: https://clang.llvm.org/extra/clang-tidy/
[google-cpp-style-guide]: https://google.github.io/styleguide/cppguide.html
[google-objc-style-guide]: https://google.github.io/styleguide/objcguide.html
[google-markdown-style-guide]: https://google.github.io/styleguide/docguide/style.html#document-layout
[google-shell-style-guide]: https://google.github.io/styleguide/shellguide.html
[microsoft-writing-style-guide]: https://docs.microsoft.com/en-us/style-guide/welcome/
[rust-result]: https://doc.rust-lang.org/std/result/
[starlark-style-guide]: https://docs.bazel.build/versions/master/skylark/bzl-style.html
[util-numeric-h]: util/numeric.h
[util-result-h]: util/result.h
