#!/usr/bin/env python3
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''`swift` wrapper that injects Spoor instrumentation and links in Spoor's
runtime library.'''

from shared import DEFAULT_CLANGXX, DEFAULT_SWIFT, SPOOR_OPT
from shared import OBJECT_FILE_EXTENSION
from shared import ArgsInfo, flatten, instrument_and_compile_ir
import argparse
import subprocess
import sys

SWIFT_EMIT_IR_ARG = '-emit-ir'
SWIFT_OUTPUT_FILE_ARG = '-o'
SWIFT_TARGET_ARG = '-target'


def _compiling(output_files):
  if not output_files:
    return False
  return all(f.endswith(OBJECT_FILE_EXTENSION) for f in output_files)


def _parse_swift_args(args):
  parser = argparse.ArgumentParser()
  parser.add_argument(
      SWIFT_OUTPUT_FILE_ARG,
      action='append',
      dest='output_files',
      nargs=1,
  )
  parser.add_argument(
      SWIFT_TARGET_ARG,
      dest='target',
      nargs=1,
  )
  known_args, other_args = parser.parse_known_args(args)

  target = known_args.target[0] if known_args.target else None

  output_files = flatten(known_args.output_files)
  if not _compiling(output_files):
    return ArgsInfo(args, output_files, target, instrument=False)

  if not target:
    raise ValueError('No target was supplied.')

  if len(output_files) != 1:
    message = f'Expected exactly one output file, got {len(output_files)}.'
    raise ValueError(message)

  swift_args = other_args
  swift_args += [SWIFT_TARGET_ARG, target]
  swift_args += [SWIFT_EMIT_IR_ARG]
  return ArgsInfo(swift_args, output_files, target, instrument=True)


def main(argv, frontend_swift, spoor_opt, backend_clangxx):
  args_info = _parse_swift_args(argv[1:])
  swift_args = [frontend_swift] + args_info.args

  if args_info.instrument:
    with subprocess.Popen(swift_args, stdout=subprocess.PIPE) as swift_process:
      return instrument_and_compile_ir(swift_process, spoor_opt,
                                       backend_clangxx,
                                       args_info.output_files[0],
                                       args_info.target)
  else:
    with subprocess.Popen(swift_args) as swift_process:
      swift_process.wait()
      return swift_process.returncode


if __name__ == '__main__':
  return_code = main(sys.argv, DEFAULT_SWIFT, SPOOR_OPT, DEFAULT_CLANGXX)
  sys.exit(return_code)
