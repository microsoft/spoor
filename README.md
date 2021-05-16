# Spoor [![Build Status][build-status-badge]][build-status]

# Getting Started

## Requirements
* A C++ compiler that supports [C++20][c++20-compiler] such as Clang 12.
* Python 3.
* [Bazel][bazel].
* [Clang Format][clang-format], [Clang Tidy][clang-tidy], and [YAPF][yapf] to
  format and lint code.

## Build
```
$ bazel build //...
```

## Test
```
$ bazel test //...
```

## Style and lint

### Format C++, Objective-C, Python
```
$ bazel build //... --aspects toolchain/style/style.bzl%format --output_groups=report
```

### Format Starlark
```
$ bazel run //toolchain/style:buildifier
```

### Lint C++
```
$ bazel build //... --aspects toolchain/style/style.bzl%lint --output_groups=report
```

### Add copyright header
```
$ ./toolchain/copyright_header/add_copyright_header.sh
```

## Compilation database
Generate a `compile_commands.json` compilation database used by Clang Tidy for
linting and by some IDEs to offer code completion.
[Details][compilation-database-readme].

```
$ ./toolchain/compilation_database/generate_compilation_database.sh
```

# Contributing

## Style guide

Please review Spoor's [Style Guide][style-guide].

## Contributor License Agreement

This project welcomes contributions and suggestions.  Most contributions require
you to agree to a Contributor License Agreement (CLA) declaring that you have
the right to, and actually do, grant us the rights to use your contribution. For
details, visit https://cla.opensource.microsoft.com.

When you submit a pull request, a CLA bot will automatically determine whether
you need to provide a CLA and decorate the PR appropriately (e.g., status check,
comment). Simply follow the instructions provided by the bot. You will only need
to do this once across all repos using our CLA.

## Code of conduct

This project has adopted the
[Microsoft Open Source Code of Conduct][code-of-conduct]. For more information
see the [Code of Conduct FAQ][code-of-conduct-faq] or contact
[opencode@microsoft.com][opencode-email] with any additional questions or
comments.

[bazel]: https://bazel.build/
[build-status-badge]: https://outlookmobile.visualstudio.com/Github/_apis/build/status/microsoft.spoor
[build-status]: https://outlookmobile.visualstudio.com/Github/_build?definitionId=89
[c++20-compiler]: https://en.cppreference.com/w/cpp/compiler_support
[clang-format]: https://clang.llvm.org/docs/ClangFormat.html
[clang-tidy]: https://clang.llvm.org/extra/clang-tidy/
[code-of-conduct-faq]: https://opensource.microsoft.com/codeofconduct/faq/
[code-of-conduct]: https://opensource.microsoft.com/codeofconduct/
[compilation-database-readme]: toolchain/compilation_database/README.md
[nodejs]: https://nodejs.org/
[opencode-email]: mailto:opencode@microsoft.com
[protoc-installation]: https://grpc.io/docs/protoc-installation/
[style-guide]: STYLE_GUIDE.md
[yapf]: https://github.com/google/yapf
[yarn]: https://classic.yarnpkg.com/
