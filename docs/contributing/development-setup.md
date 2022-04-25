# Development Setup

## Requirements
* [LLVM 12][llvm-download] including `clang++`, `opt`, `clang-format`,
  `clang-tidy`, and `libc++`.
* [Python 3][python-download], and [Pylint][pylint] and [YAPF][yapf] to format
  and lint code.
* [Bazel][bazel] build system.
* [Material for MkDocs][mkdocs-material] to generate documentation.

!!! info "Bazel configuration"
    Check out the `bazelrc` files in the `.build` folder to help configure Spoor
    for your environment.

## Build

```bash
bazel build //...
```

## Test

```bash
bazel test //...
```

## Style and lint

### Format C++, Objective-C, and Python

```bash
bazel build //... --aspects toolchain/style/style.bzl%format --output_groups=report
```

### Format Starlark

```bash
bazel run //toolchain/style:buildifier
```

### Lint C++ and Python

```bash
bazel build //... --aspects toolchain/style/style.bzl%lint --output_groups=report
```

!!! info "CI toolchain"
    CI uses the LLVM toolchain when running the linter.

    ```bash
    --crosstool_top=//toolchain/crosstool:llvm_toolchain
    ```

### Add copyright header

```bash
./toolchain/copyright_header/add_copyright_header.sh
```

## Compilation database

Generate a `compile_commands.json` compilation database used by Clang Tidy for
linting and by some IDEs to offer code completion.
[Details][compilation-database-readme].

```bash
./toolchain/compilation_database/generate_compilation_database.sh
```

## Documentation

### Build 

```bash
mkdocs build
```

### Live preview server

```bash
mkdocs serve
```

```
INFO     -  Building documentation...
INFO     -  Cleaning site directory
INFO     -  Documentation built in 0.80 seconds
INFO     -  [00:00:00] Serving on http://127.0.0.1:8000/
```

[bazel]: https://bazel.build/
[compilation-database-readme]: https://github.com/microsoft/spoor/blob/master/toolchain/compilation_database/README.md
[llvm-download]: https://releases.llvm.org/download.html
[mkdocs-material]: https://squidfunk.github.io/mkdocs-material/getting-started/
[pylint]: https://www.pylint.org/
[python-download]: https://www.python.org/downloads/
[yapf]: https://github.com/google/yapf
[yarn]: https://classic.yarnpkg.com/
