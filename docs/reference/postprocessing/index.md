# Postprocessing

## Introduction

Spoor's postprocessing tool parses and symbolizes trace data generated at
runtime and converts these data into human-readable formats.

## Packaging

Spoor's postprocessing tools are packaged as a command line tool.

```
spoor --help
```

```
spoor: Parse and symbolize Spoor traces.

USAGE: spoor [options...] <search_paths...>

  Flags from spoor/tools/config/command_line_config.cc:
    --output_file (Output file.); default: "";
    --output_format (Data output format. Options: automatic, perfetto,
      spoor_symbols, csv. "automatic" detects the format from the output file's
      extension.); default: automatic;

Try --helpfull to get a list of all flags or --help=substring shows help for
flags which include specified substring in either in the name, or description or
path.
```

## Output formats

The command line tool infers the file output format from the `output_file`'s
extension.

### Perfetto

Outputs a [Perfetto][perfetto]-compatible trace file that can be visualized in
[Perfetto's trace viewer][perfetto-ui].

### Spoor symbols

Aggregates one or more `.spoor_symbols` files into a single `.spoor_symbols`
file.

### CSV

Aggregates one or more `.spoor_symbols` files, then outputs these symbols data
as a semicolon-delimited text file. This is useful for parsing the symbols data
in other programs.

[perfetto]: https://perfetto.dev/
[perfetto-ui]: https://ui.perfetto.dev/
