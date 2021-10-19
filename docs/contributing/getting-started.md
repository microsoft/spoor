# Getting Started

## Requirements
* [LLVM 12][llvm-download] including `clang++`, `opt`, `clang-format`,
  `clang-tidy`, and `libc++`.
* [Python 3][python-download], and [Pylint][pylint] and [YAPF][yapf] to format
  and lint code.
* [Bazel][bazel] build system.
* [Material for MkDocs][mkdocs-material] to generate documentation.

## Build

```
bazel build //...
```

## Test

```
bazel test //...
```

## Style and lint

### Format C++, Objective-C, and Python

```
bazel build //... --aspects toolchain/style/style.bzl%format --output_groups=report
```

### Format Starlark

```
bazel run //toolchain/style:buildifier
```

### Lint C++ and Python

```
bazel build //... --aspects toolchain/style/style.bzl%lint --output_groups=report
```

### Add copyright header

```
./toolchain/copyright_header/add_copyright_header.sh
```

## Compilation database

Generate a `compile_commands.json` compilation database used by Clang Tidy for
linting and by some IDEs to offer code completion.
[Details][compilation-database-readme].

```
./toolchain/compilation_database/generate_compilation_database.sh
```

## Documentation

### Build 

```
mkdocs build
```

### Live preview server

```
mkdocs serve
```

[bazel]: https://bazel.build/
[compilation-database-readme]: https://github.com/microsoft/spoor/blob/master/toolchain/compilation_database/README.md
[llvm-download]: https://releases.llvm.org/download.html
[mkdocs-material]: https://squidfunk.github.io/mkdocs-material/getting-started/
[pylint]: https://www.pylint.org/
[python-download]: https://www.python.org/downloads/
[yapf]: https://github.com/google/yapf
[yarn]: https://classic.yarnpkg.com/
