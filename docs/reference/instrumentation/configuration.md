# Instrumentation Configuration

Configure Spoor's instrumentation with command line flags, environment
variables, or a configuration file.

!!! info "Precedence"
    1. Command line flags (`spoor_opt`).
    2. Environment variables.
    3. Config file.

!!! info "Config functions"
    Spoor does not instrument
    [user-defined config functions][runtime-configuration-file] to prevent
    recursive initialization.

## Instrumentation options

### Enable runtime

Automatically enable Spoor's runtime.

**Type:** `bool`

**Default:** `true`

Source               | Key
-------------------- | --------------------------------------
Command line         | `--enable_runtime`
Environment variable | `SPOOR_INSTRUMENTATION_ENABLE_RUNTIME`
Config file          | `enable_runtime`

### Filters file

Path to the [filters file](#filters-file).

**Type:** `string`

**Default:** Empty (no filters)

Source               | Key
-------------------- | ------------------------------------
Command line         | `--filters_file`
Environment variable | `SPOOR_INSTRUMENTATION_FILTERS_FILE`
Config file          | `filters_file`

### Force binary output

Force printing binary data to the console.

**Type:** `bool`

**Default:** `false`

Source               | Key
-------------------- | -------------------------------------------
Command line         | `--force_binary_output`
Environment variable | `SPOOR_INSTRUMENTATION_FORCE_BINARY_OUTPUT`
Config file          | `force_binary_output`

### Initialize runtime

Automatically initialize Spoor's runtime. This is achieved by injecting a call
to `_spoor_runtime_Initialize()` at the start of `main`.

**Type:** `bool`

**Default:** `true`

Source               | Key
-------------------- | ------------------------------------------
Command line         | `--initialize_runtime`
Environment variable | `SPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME`
Config file          | `initialize_runtime`

### Inject instrumentation

Inject Spoor instrumentation.

**Type:** `bool`

**Default:** `true`

Source               | Key
-------------------- | ----------------------------------------------
Command line         | `--inject_instrumentation`
Environment variable | `SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION`
Config file          | `inject_instrumentation`

### Module ID

Override the LLVM module's ID.

**Type:** `string`

**Default:** Empty (do not override LLVM module ID)

Source               | Key
-------------------- | ---------------------------------
Command line         | `--module_id`
Environment variable | `SPOOR_INSTRUMENTATION_MODULE_ID`
Config file          | `module_id`

### Output file

Spoor instrumentation symbols output file. All parent directories must exist.

**Type:** `string`

**Default:** `-` (stdout)

Source               | Key
-------------------- | -----------------------------------
Command line         | `--output_file`
Environment variable | `SPOOR_INSTRUMENTATION_OUTPUT_FILE`
Config file          | `output_file`

### Output language

Language in which to output the transformed code.

**Type:** `string`

**Options:** `ir` or `bitcode`.

**Default value:** `bitcode`

Source               | Key
-------------------- | ---------------------------------------
Command line         | `--output_language`
Environment variable | `SPOOR_INSTRUMENTATION_OUTPUT_LANGUAGE`
Config file          | `output_language`

### Output symbols file

Spoor instrumentation symbols output file. All parent directories must exist.

**Type:** `string`

**Default:** Empty (error)

Source               | Key
-------------------- | -------------------------------------------
Command line         | `--output_symbols_file`
Environment variable | `SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE`
Config file          | `output_symbols_file`

## Xcode toolchain

### Preprocessor macros

Spoor's `clang` and `clang++` toolchain wrappers export the following
preprocessor macros.

Name        | Value
------------|------
`__SPOOR__` | `1`

Additionally, the `clang` and `clang++` toolchain wrappers forward the following
configuration values as a preprocessor macros. Their values are adopted from the
configured value or take on the default value if not specified.

* [`SPOOR_INSTRUMENTATION_ENABLE_RUNTIME`][enable-runtime]
* [`SPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME`][initialize-runtime]
* [`SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION`][inject-instrumentation]

## Configuration file

Spoor's instrumentation tools search for a configuration file called
`spoor_config.toml` from the present working directory up to root.

**Example configuration file**

```toml
# spoor_config.toml

enable_runtime = true
filters_file = "/path/to/spoor_filters.toml"
force_binary_output = false
initialize_runtime = true
inject_instrumentation = true
module_id = "Module"
output_file = "/path/to/output.perfetto"
output_language = "bitcode"
output_symbols_file = "/path/to/output.spoor_symbols"
```

## Filters file

Policy: Do not instrument functions matching a blocklist rule that do not also
match an allow list rule.

A function must match all conditions within in a block or allow rule to apply.

All configurations are optional.

!!! note "Match everything"
    An empty rule matches everything.

!!! info "Backslashes"
    Escape backslashes in regular expressions.

**Example filters file**

```toml
# spoor_filters.toml

[[block]]
rule_name = "Block small functions"
function_ir_instruction_count_lt = 100

[[block]]
rule_name = "Block React Native runtime"
demangled_name = "^facebook::jsc::.*"
source_file_path = ".*/Pods/React.*"

[[allow]]
rule_name = "Always instrument `main`"
function_linkage_name = "main"
```

The above config file blocks all functions that are less than 100 instructions,
**OR** functions whose source file matches the path `.*/Pods/React.*` **AND**
whose demangled name matches `^facebook::jsc::.*"`, **EXCEPT** for the main
function which is always instrumented (regardless of its source file path or
instruction count).

### Rule name

Helpful description of the rule. If a filter matches a function during
instrumentation (either blocking or allowing it), the rule's name is reported in
the emitted symbols metadata. If multiple rules apply to a function, only one of
the rules is reported.

**Type:** `string`

Source      | Key
----------- | -----------
Config file | `rule_name`

### Source file path

Regular expression matching the source file path.

**Type:** `string`

Source      | Key
----------- | ------------------
Config file | `source_file_path`

**Example source file paths**

```
/path/to/source.extension
```

```
<compiler-generated>
```

### Demangled function name

Regular expression matching the demangled function name.

**Type:** `string`

Source      | Key
----------- | -------------------------
Config file | `function_demangled_name`

**Example demangled function names**

```
main
```

```
void std::__1::sort<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > >(std::__1::__wrap_iter<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >*>, std::__1::__wrap_iter<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >*>)
```

```
app_ios.Bootstrap.appDidFinish(with: Swift.Optional<Swift.Dictionary<__C.UIApplicationLaunchOptionsKey, Any>>) -> ()
```

```
+[RCTCxxBridge runRunLoop]
```

### Function linkage name

Regular expression matching the function's linkage name.

**Type:** `string`

Source      | Key
----------- | -----------------------
Config file | `function_linkage_name`

**Example function linkage names**

```
main
```

```
_ZNSt3__1L4sortINS_12basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEEEEvNS_11__wrap_iterIPT_EESA_
```
```
$s7app_ios9BootstrapC0A9DidFinish4withySDySo29UIApplicationLaunchOptionsKeyaypGSg_tF
```

```
+[RCTCxxBridge runRunLoop]
```

### Function IR instruction count less than

Matches all functions with an IR instruction count strictly less than the
provided value.

**Type:** `int32`

Source      | Key
----------- | ----------------------------------
Config file | `function_ir_instruction_count_lt`

### Function IR instruction count greater than

Matches all functions with an IR instruction count strictly greater than the
provided value.

**Type:** `int32`

Source      | Key
----------- | ----------------------------------
Config file | `function_ir_instruction_count_gt`

[enable-runtime]: #enable-runtime
[initialize-runtime]: #initialize-runtime
[inject-instrumentation]: #inject-instrumentation
[runtime-configuration-file]: /reference/runtime/configuration#configuration-file
