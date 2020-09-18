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

Additionally, please follow the
[C++ and Bazel Best Practices][c++-bazel-best-practices].

### Function declarations

Exception: Use trailing return types for all function, method, and labmda
declarations. Functions and methods may not use automatic return type inference.

```c++
// ✅ Do this
auto Add(const int64 a, const int64 b) -> int64;

// ❌ Not this
int64 Add(const int64 a, const int64 b);
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
type used to return either a value on success or an error otherwise. This is
technique is inspired by [Rust's `Result`][rust-result].

### Numeric values

Use numeric types with a defined size (e.g. `int64` instead of `int`). For
convenience, integer types without the `_t` postifx are defined in
[util/numeric.h][util-numeric-h]. Prefer using these `typedef`s over the values
defined in `<cstdint>`.
 
### Tools

Use [Clang Format][clang-format] and [Clang Tidy][clang-tidy] to style and lint
code.

In special circumstances it might make sense to ignore lint warnings, for
example, when calling an external library where the lint warning is unavoidable.
Explicitly list the lint rules to ignore instead of using a blanket `NOLINT` or
`NOLINTNEXTLINE`.

```c++
// ✅ Do this
int32_t a[] = {1, 2, 3};  // NOLINT(modernize-avoid-c-arrays)

// ❌ Not this
int32_t a[] = {1, 2, 3};  // NOLINT
```

Exception: Google Test and Gflags library macros.

```c++
// ✅ Okay
TEST(Foo, Bar) { // NOLINT
  ...
}

// ✅ Okay
DEFINE_string(  // NOLINT
    my_flag, "", "Description...");
```

## Starlark (Bazel)

Follow [Starlark's Style Guide][starlark-style-guide].

## Bash

Just do your best.

[clang-format]: https://clang.llvm.org/docs/ClangFormat.html
[clang-tidy]: https://clang.llvm.org/extra/clang-tidy/
[c++-bazel-best-practices]: https://docs.bazel.build/versions/master/bazel-and-cpp.html#best-practices
[google-cpp-style-guide]: https://google.github.io/styleguide/cppguide.html
[microsoft-writing-style-guide]: https://docs.microsoft.com/en-us/style-guide/welcome/
[rust-result]: https://doc.rust-lang.org/std/result/
[starlark-style-guide]: https://docs.bazel.build/versions/master/skylark/bzl-style.html
[util-numeric-h]: util/numeric.h
[util-result-h]: util/result.h
