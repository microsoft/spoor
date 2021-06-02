#!/usr/bin/env python3
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''`swiftc` wrapper that injects Spoor instrumentation and links in Spoor's
runtime library.'''

from shared import DEFAULT_SWIFTC, WRAPPED_SWIFT
import subprocess
import sys

DISABLE_BATCH_MODE_ARG = '-disable-batch-mode'
DRIVER_USE_FRONTEND_PATH_ARG = '-driver-use-frontend-path'
ENABLE_BATCH_MODE_ARG = '-enable-batch-mode'
WHOLE_MODULE_OPTIMIZATION_ARG = '-whole-module-optimization'


def _parse_swiftc_args(args, wrapped_swift):
  if WHOLE_MODULE_OPTIMIZATION_ARG in args:
    # Whole module optimization mode transforms the source code.
    raise ValueError('Whole module optimization is not supported yet.')
  # Batch mode does not transform the source code.
  args = list(filter(lambda arg: arg != ENABLE_BATCH_MODE_ARG, args))
  swiftc_args = [DRIVER_USE_FRONTEND_PATH_ARG, wrapped_swift] + args
  return swiftc_args


def main(argv, default_swiftc, wrapped_swift):
  swiftc_args = [default_swiftc] + _parse_swiftc_args(argv[1:], wrapped_swift)
  with subprocess.Popen(swiftc_args) as swiftc_process:
    swiftc_process.wait()
    return swiftc_process.returncode


if __name__ == '__main__':
  return_code = main(sys.argv, DEFAULT_SWIFTC, WRAPPED_SWIFT)
  sys.exit(return_code)
