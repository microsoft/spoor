#!/usr/bin/env python3
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''`swiftc` wrapper that injects Spoor instrumentation and links in Spoor's
runtime library.'''

from shared import SPOOR_INSTRUMENTATION_XCODE_OUTPUT_FILE_MAP_KEY, BuildTools
import argparse
import json
import os
import pathlib
import subprocess
import sys


def main(argv, build_tools):
  args = argv[1:]

  parser = argparse.ArgumentParser()
  parser.add_argument('-output-file-map', dest='output_file_map_path')
  known_args, _ = parser.parse_known_args(args)

  if known_args.output_file_map_path is None:
    raise NotImplementedError(
        "Spoor's swiftc wrapper requires an output file map.")

  with open(known_args.output_file_map_path, 'r+', encoding='utf-8') as file:
    output_file_map = json.load(file)
    for source, outputs in output_file_map.items():
      if 'llvm-bc' in outputs:
        llvm_bc_path = pathlib.Path(outputs['llvm-bc'])
        output_file_map[source]['spoor-symbols'] = str(
            llvm_bc_path.with_suffix('.spoor_symbols'))
        output_file_map[source]['instrumented-llvm-bc'] = str(
            llvm_bc_path.with_suffix('.instrumented.bc'))
    file.seek(0)
    file.truncate(0)
    json.dump(output_file_map, file)

  env = os.environ.copy()
  env[SPOOR_INSTRUMENTATION_XCODE_OUTPUT_FILE_MAP_KEY] = \
      known_args.output_file_map_path

  swiftc_args = [
      build_tools.swiftc,
      '-driver-use-frontend-path',
      build_tools.spoor_swift,
  ] + args + [
      '-emit-bc',
  ]

  subprocess.run(swiftc_args, env=env, check=True)


if __name__ == '__main__':
  main(sys.argv, BuildTools.get())
