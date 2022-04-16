# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''Shared logic for the wrapper scripts.'''

import functools
import operator
import os
import pathlib
import subprocess
import re

DEVELOPER_PATH = os.getenv('DEVELOPER_DIR',
                           '/Applications/Xcode.app/Contents/Developer')
DEFAULT_TOOLCHAIN_PATH = f'{DEVELOPER_PATH}/Toolchains/XcodeDefault.xctoolchain'
DEFAULT_CLANG = f'{DEFAULT_TOOLCHAIN_PATH}/usr/bin/clang'
DEFAULT_CLANGXX = f'{DEFAULT_TOOLCHAIN_PATH}/usr/bin/clang++'
DEFAULT_SWIFT = f'{DEFAULT_TOOLCHAIN_PATH}/usr/bin/swift'
DEFAULT_SWIFTC = f'{DEFAULT_TOOLCHAIN_PATH}/usr/bin/swiftc'

# This path is correct when inside the (generated) toolchain.
CUSTOM_TOOLCHAIN_PATH = pathlib.Path(
    *pathlib.Path(__file__).parent.absolute().parts[:-2])
SPOOR_OPT = f'{CUSTOM_TOOLCHAIN_PATH}/spoor/bin/spoor_opt'
SPOOR_FRAMEWORKS_PATH = f'{CUSTOM_TOOLCHAIN_PATH}/spoor/frameworks'
WRAPPED_SWIFT = f'{CUSTOM_TOOLCHAIN_PATH}/usr/bin/swift'

CLANG_CLANGXX_EMBED_BITCODE_ARG = '-fembed-bitcode'
CLANG_CLANGXX_EMBED_BITCODE_MARKER_ARG = '-fembed-bitcode-marker'
CLANG_CLANGXX_INPUT_STDIN_VALUE = '-'
CLANG_CLANGXX_LANGUAGE_ARG = '-x'
CLANG_CLANGXX_ONLY_PREPROCESS_COMPILE_AND_ASSEMBLE_ARG = '-c'
CLANG_CLANGXX_OUTPUT_FILE_ARG = '-o'
CLANG_CLANGXX_TARGET_ARG = '-target'
LLVM_IR_LANGUAGE = 'ir'
OBJECT_FILE_EXTENSION = '.o'
EXIT_SUCCESS = 0
INSTRUMENTATION_MODULE_ID_KEY = 'SPOOR_INSTRUMENTATION_MODULE_ID'
SPOOR_OPT_OUTPUT_LANGUGAGE_ARG = '--output_language'

# SPOOR_INSTRUMENTATION_* keys and default values should match
# spoor/instrumentation/config/config.h.
SPOOR_INSTRUMENTATION_ENABLE_RUNTIME_DEFAULT_VALUE = True
SPOOR_INSTRUMENTATION_ENABLE_RUNTIME_KEY = \
    'SPOOR_INSTRUMENTATION_ENABLE_RUNTIME'
SPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME_DEFAULT_VALUE = True
SPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME_KEY = \
    'SPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME'
SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION_DEFAULT_VALUE = True
SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION_KEY = \
    'SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION'
SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE_KEY = \
    'SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE'
SPOOR_SYMBOLS_FILE_EXTENSION = 'spoor_symbols'


class ArgsInfo:

  def __init__(self, args, output_files, target, instrument):
    self.args = args
    self.output_files = output_files
    self.target = target
    self.instrument = instrument


class Target:
  '''Parses a target string's components.'''

  def __init__(self, target_string):
    components = target_string.split('-')
    if len(components) == 3:
      architecture, vendor, platform = components
      platform_variant = None
    elif len(components) == 4:
      architecture, vendor, platform, platform_variant = components
    else:
      raise ValueError(f'Cannot parse target "{target_string}".')
    self.string = target_string
    self.architecture = architecture
    self.vendor = vendor
    platform_components = re.search('([a-zA-Z]+)([0-9.]+)', platform)
    self.platform = platform_components.group(1)
    self.platform_version = platform_components.group(2)
    self.platform_variant = platform_variant


def arg_after(arg, args):
  return args[args.index(arg) + 1]


def flatten(nested_list):
  return functools.reduce(operator.iconcat, nested_list or [], [])


def _make_clangxx_args(target, output_file):
  clangxx_args = [CLANG_CLANGXX_TARGET_ARG, target]
  clangxx_args += [CLANG_CLANGXX_ONLY_PREPROCESS_COMPILE_AND_ASSEMBLE_ARG]
  clangxx_args += [CLANG_CLANGXX_LANGUAGE_ARG, LLVM_IR_LANGUAGE]
  clangxx_args += [CLANG_CLANGXX_INPUT_STDIN_VALUE]
  clangxx_args += [CLANG_CLANGXX_OUTPUT_FILE_ARG, output_file]
  return clangxx_args


def _make_spoor_opt_args():
  return [SPOOR_OPT_OUTPUT_LANGUGAGE_ARG, LLVM_IR_LANGUAGE]


def _make_spoor_opt_env(env, output_file):
  if INSTRUMENTATION_MODULE_ID_KEY not in env:
    env[INSTRUMENTATION_MODULE_ID_KEY] = output_file
  if SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE_KEY not in env:
    path = pathlib.Path(output_file)
    env[SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE_KEY] = str(
        path.with_suffix(f'.{SPOOR_SYMBOLS_FILE_EXTENSION}'))
  return env


def instrument_and_compile_ir(frontend_process, spoor_opt, backend_clangxx,
                              output_file, target):
  spoor_opt_args = [spoor_opt] + _make_spoor_opt_args()
  clangxx_args = [backend_clangxx] + _make_clangxx_args(target, output_file)
  spoor_opt_env = _make_spoor_opt_env(os.environ.copy(), output_file)

  with subprocess.Popen(spoor_opt_args,
                        stdin=frontend_process.stdout,
                        stdout=subprocess.PIPE,
                        env=spoor_opt_env) as spoor_opt_process:
    frontend_process.stdout.close()
    frontend_process.wait()
    if frontend_process.returncode != EXIT_SUCCESS:
      return frontend_process.returncode
    with subprocess.Popen(clangxx_args,
                          stdin=spoor_opt_process.stdout) as clangxx_process:
      spoor_opt_process.stdout.close()
      spoor_opt_process.wait()
      if spoor_opt_process.returncode != EXIT_SUCCESS:
        return spoor_opt_process.returncode
      clangxx_process.wait()
      return clangxx_process.returncode
