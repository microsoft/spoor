#!/usr/bin/env python3
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''`swift` wrapper that injects Spoor instrumentation and links in Spoor's
runtime library.'''

from shared import BuildTools, RuntimeFramework, Target, make_spoor_opt_env
from shared import SPOOR_INSTRUMENTATION_XCODE_OUTPUT_FILE_MAP_KEY
import argparse
import concurrent.futures
import json
import multiprocessing
import os
import subprocess
import sys


def _compile_bitcode(build_tools, target, files):
  clangxx_args = [
      build_tools.clangxx,
      '-c',
      '-target',
      target.string,
      files['llvm-bc'],
      '-o',
      files['object'],
  ]
  subprocess.run(clangxx_args, check=True)


def _instrument_and_compile_bitcode(build_tools, target, files):
  # Swift writes to stdout when emitting LLVM IR and to a file when emitting
  # bitcode. Additionally, there is no output delimitation when compiling with
  # batched mode. Therefore, the pipe approach is not suitable for Swift and we
  # need to write the bitcode to an intermediate file.
  spoor_opt_args = [
      build_tools.spoor_opt,
      files['llvm-bc'],
      f'--output_file={files["instrumented-llvm-bc"]}',
      f'--output_symbols_file={files["spoor-symbols"]}',
      '--output_language=bitcode',
  ]
  spoor_opt_env = make_spoor_opt_env(os.environ.copy(), files['object'],
                                     files['spoor-symbols'])
  subprocess.run(spoor_opt_args, env=spoor_opt_env, check=True)

  clangxx_args = [
      build_tools.clangxx,
      '-c',
      '-target',
      target.string,
      files['instrumented-llvm-bc'],
      '-o',
      files['object'],
  ]
  subprocess.run(clangxx_args, check=True)


def main(argv, build_tools):
  args = argv[1:]

  subprocess.run([build_tools.swift] + args, check=True)

  output_file_map_path = os.environ.get(
      SPOOR_INSTRUMENTATION_XCODE_OUTPUT_FILE_MAP_KEY, None)
  invoked_by_spoor_swiftc_driver = output_file_map_path is not None
  if not invoked_by_spoor_swiftc_driver:
    return

  parser = argparse.ArgumentParser()
  parser.add_argument('-target')
  parser.add_argument('-o', action='append', dest='output_files', nargs='+')
  known_args, _ = parser.parse_known_args(args)

  if known_args.output_files is None:
    return
  output_files = {files[0] for files in known_args.output_files}

  with open(output_file_map_path, 'r', encoding='utf-8') as file:
    output_file_map = {
        source: outputs
        for source, outputs in json.load(file).items()
        if 'llvm-bc' in outputs and outputs['llvm-bc'] in output_files
    }

  if len(output_file_map) == 0:
    return

  target = Target(known_args.target) if known_args.target else None
  if target is None:
    raise NotImplementedError(
        "Spoor's swift wrapper requires an explicit target.")

  runtime_framework = RuntimeFramework.get(build_tools.spoor_frameworks_path,
                                           target)
  if runtime_framework is None:
    print(
        f'Warning: Skipping instrumentation for unsupported target "{target}".',
        file=sys.stderr)
    action = _compile_bitcode
  else:
    action = _instrument_and_compile_bitcode

  thread_pool_size = min(2 * multiprocessing.cpu_count(), len(output_file_map))
  with concurrent.futures.ThreadPoolExecutor(thread_pool_size) as executor:
    futures = []
    for _, outputs in output_file_map.items():
      future = executor.submit(action, build_tools, target, outputs)
      futures.append(future)
    for future in futures:
      _ = future.result()


if __name__ == '__main__':
  main(sys.argv, BuildTools.get())
