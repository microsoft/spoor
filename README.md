# Spoor
[![Build Status][build-status-badge]][build-status]

# Getting Started

## Requirements
* Compiler instrumentation and runtime library
  * A C++ compiler that supports [C++20][c++20-compiler] such as Clang 10.
  * [Bazel][bazel].
  * [Clang Format][clang-format] and [Clang Tidy][clang-tidy] to style and lint
    code.
* Post-processing tools
  * [Node.js][nodejs]
  * [yarn][yarn]
  * [protoc][protoc-installation]

## Build
```
$ bazel build //...          # Compiler instrumentation and runtime library
$ yarn workspaces run build  # Post-processing tools
```

## Test
```
$ bazel test //... --test_output=all  # Compiler instrumentation and runtime library
$ yarn workspaces run test            # Post-processing tools
```

## Run
```
$ yarn workspace @microsoft/spoor-cli start --help  # Post-processing tools CLI
```

## Style and lint
```
$ bazel run //toolchain/style:buildifier                # Format Starlark files
$ ./toolchain/style/clang_format.sh                     # Format C++ and Protobuf files
$ ./toolchain/style/clang_tidy.sh                       # Lint C++ files
$ yarn lint                                             # Lint JavaScript and TypeScript files
$ ./toolchain/copyright_header/add_copyright_header.sh  # Add copyright header
```

## Compilation database
Generate a `compile_commands.json` compilation database used by Clang Tidy for
linting and by some IDEs to offer code completion.
[Details][compilation-database-readme].

```
$ ./toolchain/compilation_database/generate_compilation_database.sh
```

# Contributing

Please review Spoor's [Style Guide][style-guide].

This project welcomes contributions and suggestions.  Most contributions require
you to agree to a Contributor License Agreement (CLA) declaring that you have
the right to, and actually do, grant us the rights to use your contribution. For
details, visit https://cla.opensource.microsoft.com.

When you submit a pull request, a CLA bot will automatically determine whether
you need to provide a CLA and decorate the PR appropriately (e.g., status check,
comment). Simply follow the instructions provided by the bot. You will only need
to do this once across all repos using our CLA.

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
[yarn]: https://classic.yarnpkg.com/
