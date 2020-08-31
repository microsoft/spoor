# Spoor
[![Build Status][build-status-badge]][build-status]

# Getting Started

## Requirements
* A C++ compiler that supports [C++20][c++20-compiler] such as Clang 10.
* [Bazel][bazel].

## Build
```
$ bazel build //...
```

## Test
```
$ bazel test //... --test_output=all
```

# Contributing

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
[code-of-conduct-faq]: https://opensource.microsoft.com/codeofconduct/faq/
[code-of-conduct]: https://opensource.microsoft.com/codeofconduct/
[opencode-email]: mailto:opencode@microsoft.com
