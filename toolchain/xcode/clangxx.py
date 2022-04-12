#!/usr/bin/env python3
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''`clang++` wrapper that injects Spoor instrumentation and links in Spoor's
runtime library.'''

from clang_clangxx_shared import parse_clang_clangxx_args
from shared import DEFAULT_CLANGXX, SPOOR_FRAMEWORKS_PATH, SPOOR_OPT
from shared import instrument_and_compile_ir
import subprocess
import sys


def main(argv, frontend_clangxx, spoor_opt, backend_clangxx,
         spoor_library_path):
  args_info = parse_clang_clangxx_args(argv[1:], spoor_library_path)
  clangxx_args = [frontend_clangxx] + args_info.args

  if args_info.instrument:
    with subprocess.Popen(clangxx_args,
                          stdout=subprocess.PIPE) as clangxx_process:
      return instrument_and_compile_ir(clangxx_process, spoor_opt,
                                       backend_clangxx,
                                       args_info.output_files[0],
                                       args_info.target)
  else:
    with subprocess.Popen(clangxx_args) as clangxx_process:
      clangxx_process.wait()
      return clangxx_process.returncode


if __name__ == '__main__':
  return_code = main(sys.argv, DEFAULT_CLANGXX, SPOOR_OPT, DEFAULT_CLANGXX,
                     SPOOR_FRAMEWORKS_PATH)
  sys.exit(return_code)
