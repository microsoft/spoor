# Instrumentation

## Source code

[spoor/instrumentation/config/][spoor-instrumentation-config]

## Instrumentation options

Configure Spoor's instrumentation with environment variables.

!!! note "Config functions"
    Spoor does not instrument
    [user-defined config functions][user-defined-config] to prevent recursive
    initialization.

### Enable runtime

Automatically enable Spoor's runtime.

**Default:** `true`

Source               | Key
-------------------- | --------------------------------------
Environment variable | `SPOOR_INSTRUMENTATION_ENABLE_RUNTIME`

### Filters file

Path to the [filters file](#filters-file).

**Default:** Empty (no filters)

Source               | Key
-------------------- | ------------------------------------
Environment variable | `SPOOR_INSTRUMENTATION_FILTERS_FILE`

### Force binary output

Force printing binary data to the console.

**Default:** `false`

Source               | Key
-------------------- | -------------------------------------------
Environment variable | `SPOOR_INSTRUMENTATION_FORCE_BINARY_OUTPUT`

### Initialize runtime

Automatically initialize Spoor's runtime. This is achieved by injecting a call
to `_spoor_runtime_Initialize()` at the start of `main`.

**Default:** `true`

Source               | Key
-------------------- | ------------------------------------------
Environment variable | `SPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME`

### Inject instrumentation

Inject Spoor instrumentation.

**Default:** `true`

Source               | Key
-------------------- | ----------------------------------------------
Environment variable | `SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION`

### Module ID

Override the LLVM module's ID.

**Default:** Empty (do not override LLVM module ID)

Source               | Key
-------------------- | ---------------------------------
Environment variable | `SPOOR_INSTRUMENTATION_MODULE_ID`

### Output file

Spoor instrumentation symbols output file.

**Default:** `-` (stdout)

Source               | Key
-------------------- | -----------------------------------
Environment variable | `SPOOR_INSTRUMENTATION_OUTPUT_FILE`

### Output symbols file

Spoor instrumentation symbols output file. The path must be absolute and all
parent directories must exist. Tilde expansion is not supported.

**Default:** Empty (error)

Source               | Key
-------------------- | -------------------------------------------
Environment variable | `SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE`

### Output language

Language in which to output the transformed code.

**Options:** `ir` or `bitcode`.

**Default value:** `bitcode`

Source               | Key
-------------------- | ---------------------------------------
Environment variable | `SPOOR_INSTRUMENTATION_OUTPUT_LANGUAGE`

## Filters file

Policy: Do not instrument functions matching a blocklist rule that do not also
match an allow list rule.

A function must match all conditions within in a block or allow rule to apply.

!!! note "Match everything"
    An empty rule matches everything.

!!! info "Backslashes"
    Escape backslashes in regular expressions.

**Example filters file**

```toml
# filters.toml

[[block]]
rule_name = "Block small functions"
function_ir_instruction_count_lt = 100

[[block]]
rule_name = "Block React Native runtime"
demangled_name = "^facebook::jsc::.*"
source_file_path = ".*/Pods/React.*"

[[allow]]
rule_name = "Always instrument `main`"
linkage_name = "main"
```

The above config file blocks all functions that are less than 100 instructions,
**OR** functions whose source file matches the path `.*/Pods/React.*` **AND**
whose demangled name matches `^facebook::jsc::.*"`, **EXCEPT** for the main
function which is always instrumented (regardless of its source file path or
instruction count).

### Rule name

Optional. Helpful description of the rule. If a filter matches a function during
instrumentation (either blocking or allowing it), the rule's name is reported in
the emitted symbols metadata. If multiple rules apply to a function, only one of
the rules is reported.

Source      | Key
----------- | -----------
Config file | `rule_name`

### Source file path

Optional. Regular expression (string) matching the source file path.

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

Optional. Regular expression (string) matching the demangled function name.

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

Optional. Regular expression (string) matching the function's linkage name.

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

Optional. Integer. Matches all functions with an IR instruction count strictly
less than the provided value.

Source      | Key
----------- | ----------------------------------
Config file | `function_ir_instruction_count_lt`

### Function IR instruction count greater than

Optional. Integer. Matches all functions with an IR instruction count strictly
greater than the provided value.

Source      | Key
----------- | ----------------------------------
Config file | `function_ir_instruction_count_gt`

[spoor-instrumentation-config]: https://github.com/microsoft/spoor/tree/master/spoor/instrumentation/config
[user-defined-config]: /configuration/runtime/#configuration-file
