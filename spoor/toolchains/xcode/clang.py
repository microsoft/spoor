#!/usr/bin/env python3
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''`clang` wrapper that injects Spoor instrumentation and links in Spoor's
runtime library.'''

from clang_clangxx_shared import parse_clang_clangxx_args
from shared import DEFAULT_CLANG, DEFAULT_CLANGXX, SPOOR_FRAMEWORKS_PATH
from shared import SPOOR_OPT
from shared import instrument_and_compile_ir
import subprocess
import sys


def main(argv, frontend_clang, spoor_opt, backend_clangxx, spoor_library_path):
  args_info = parse_clang_clangxx_args(argv[1:], spoor_library_path)
  clang_args = [frontend_clang] + args_info.args

  if args_info.instrument:
    with subprocess.Popen(clang_args, stdout=subprocess.PIPE) as clang_process:
      return instrument_and_compile_ir(clang_process, spoor_opt,
                                       backend_clangxx,
                                       args_info.output_files[0],
                                       args_info.target)
  else:
    with subprocess.Popen(clang_args) as clang_process:
      clang_process.wait()
      return clang_process.returncode


if __name__ == '__main__':
  return_code = main(sys.argv, DEFAULT_CLANG, SPOOR_OPT, DEFAULT_CLANGXX,
                     SPOOR_FRAMEWORKS_PATH)
  sys.exit(return_code)
