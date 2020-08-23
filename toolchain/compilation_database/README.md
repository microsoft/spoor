# Compilation database

This module generates a JSON Compilation Database representing how to replay
individual compilations for each C++ file in the project. The compilation
database is used by Clang Tidy for linting and by some IDEs to offer code
completion.

Further reading:
[JSON Compilation Database Format Specification][llvm-compilation-database-spec]

## tl;dr
Run the following incantation from the project's root directory.
```
$ ./toolchain/compilation_database/generate_compilation_database.sh
$ python -m json.tool compile_commands.json # Optionally view the formatted JSON
```

## Strategy

Some build systems offer built-in compilation database generation functionality
such as CMake with its `CMAKE_EXPORT_COMPILE_COMMANDS` flag. Bazel has no such
feature meaning that Spoor must generate its own.

Spoor's strategy uses a [Bazel action listener][bazel-extra-action] that listens
to build events and outputs an intermediate Protocol Buffer for each
`CppCompile` mnemonic. Each intermediate file contains the command to compile a
single source file. Next, we aggregate the intermediate files and inject the
execution directory which was not available during the previous step. Finally,
we format the aggregated result JSON and write the file to
`compile_commands.json` in Spoor's workspace root.

Although the approach requires regenerating the compilation database on each
run, it is more reliable than supporting incremental updates while still
demonstrating reasonable performance for small project.

[bazel-action-listener]: https://docs.bazel.build/versions/master/be/extra-actions.html
[llvm-compilation-database-spec]: https://clang.llvm.org/docs/JSONCompilationDatabase.html
