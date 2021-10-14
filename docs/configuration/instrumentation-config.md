# Instrumentation

Configure Spoor's instrumentation with environment variables.

## Source code

[spoor/instrumentation/config/][spoor-instrumentation-config]

## Instrumentation options

### SPOOR_INSTRUMENTATION_ENABLE_RUNTIME
Automatically enable Spoor's runtime.

**Default:** `true`

### SPOOR_INSTRUMENTATION_FILTERS_FILE
Path to the [filters file](#filters-file).

**Default:** Empty (no filters)

### SPOOR_INSTRUMENTATION_FORCE_BINARY_OUTPUT
Force printing binary data to the console.

**Default:** `false`

### SPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME
Automatically initialize Spoor's runtime. This is achieved by injecting a call
to `_spoor_runtime_Initialize()` at the start of `main`.

**Default:** `true`

### SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION
Inject Spoor instrumentation.

**Default:** `true`

### SPOOR_INSTRUMENTATION_MODULE_ID
Override the LLVM module's ID.

**Default:** Empty (do not override LLVM module ID)

### SPOOR_INSTRUMENTATION_OUTPUT_FILE
Spoor instrumentation symbols output file.

**Default:** `-` (stdout)

### SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE
Spoor instrumentation symbols output file. The path must be absolute and all
parent directories must exist. Tilde expansion is not supported.

**Default:** Empty (error)

### SPOOR_INSTRUMENTATION_OUTPUT_LANGUAGE
Language in which to output the transformed code.

**Options:** `ir` or `bitcode`.

**Default value:** `bitcode`

## Filters file

Policy: Do not instrument functions matching a blocklist rule that do not also
match an allow list rule.

A function must match all conditions within in a block or allow rule to apply.

Note: An empty rule matches everything.

Tip: Escape backslashes in regular expressions.

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

### rule_name
Optional. Helpful description of the rule. If a filter matches a function during
instrumentation (either blocking or allowing it), the rule's name is reported in
the emitted symbols metadata. If multiple rules apply to a function, only one of
the rules is reported.

### source_file_path
Optional. Regular expression (string) matching the source file path.

**Example source file paths**

```
/path/to/source.extension
```

```
<compiler-generated>
```

### function_demangled_name
Optional. Regular expression (string) matching the demangled function name.

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

### function_linkage_name
Optional. Regular expression (string) matching the function's linkage name.

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

### function_ir_instruction_count_lt
Optional. Integer. Matches all functions with an IR instruction count strictly
less than the provided value.

### function_ir_instruction_count_gt
Optional. Integer. Matches all functions with an IR instruction count strictly
greater than the provided value.

[spoor-instrumentation-config]: https://github.com/microsoft/spoor/tree/master/spoor/instrumentation/config
