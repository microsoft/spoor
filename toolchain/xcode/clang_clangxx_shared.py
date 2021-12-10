# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''Shared logic for the `clang` and `clang++` wrapper.'''

from shared import CLANG_CLANGXX_LANGUAGE_ARG, CLANG_CLANGXX_OUTPUT_FILE_ARG
from shared import CLANG_CLANGXX_TARGET_ARG
from shared import CLANG_CLANGXX_ONLY_PREPROCESS_COMPILE_AND_ASSEMBLE_ARG
from shared import OBJECT_FILE_EXTENSION, LLVM_IR_LANGUAGE
from shared import ArgsInfo, flatten
import argparse

CLANG_CLANGXX_EMIT_LLVM_ARG = '-emit-llvm'
CLANG_CLANGXX_INCREMENTAL_LINK_ARG = '-r'
CLANG_CLANGXX_LIBRARY_SEARCH_PATH_ARG = '-L'
CLANG_CLANGXX_LINK_LIBRARY_ARG = '-l'
CLANG_CLANGXX_OUTPUT_STDOUT_VALUE = '-'
LIB_CXX_LIBRARY = 'c++'
SUPPORTED_LANGUAGES = {
    'c', 'c++', LLVM_IR_LANGUAGE, 'objective-c', 'objective-c++'
}


def _runtime_library_for_target(target):
  if 'ios' in target:
    return 'spoor_runtime_ios'
  elif 'macos' in target:
    return 'spoor_runtime_macos'
  raise ValueError(f'Unsupported target {target}.')


def _runtime_config_for_target(target):
  if 'ios' in target:
    return 'spoor_runtime_default_config_ios'
  elif 'macos' in target:
    return 'spoor_runtime_default_config_macos'
  raise ValueError(f'Unsupported target {target}.')


def _link_spoor_runtime(args):
  return CLANG_CLANGXX_ONLY_PREPROCESS_COMPILE_AND_ASSEMBLE_ARG not in args \
          and CLANG_CLANGXX_INCREMENTAL_LINK_ARG not in args


def _compiling(language, output_files):
  if language not in SUPPORTED_LANGUAGES:
    return False
  if not output_files:
    return False
  return all(f.endswith(OBJECT_FILE_EXTENSION) for f in output_files)


def parse_clang_clangxx_args(args, spoor_library_path):
  parser = argparse.ArgumentParser(add_help=False)
  parser.add_argument(CLANG_CLANGXX_OUTPUT_FILE_ARG,
                      action='append',
                      dest='output_files',
                      nargs=1)
  parser.add_argument(CLANG_CLANGXX_TARGET_ARG, dest='target', nargs=1)
  parser.add_argument(CLANG_CLANGXX_LANGUAGE_ARG, dest='language', nargs=1)
  known_args, other_args = parser.parse_known_args(args)

  if not known_args.target:
    raise ValueError('No target was supplied.')
  target = known_args.target[0]

  output_files = flatten(known_args.output_files)

  language = known_args.language[0] if known_args.language and 0 < len(
      known_args.language) else None

  if not _compiling(language, output_files):
    clang_args = args
    if _link_spoor_runtime(clang_args):
      runtime_library = _runtime_library_for_target(target)
      runtime_config_library = _runtime_config_for_target(target)
      clang_args += [
          f'{CLANG_CLANGXX_LIBRARY_SEARCH_PATH_ARG}{spoor_library_path}',
          f'{CLANG_CLANGXX_LINK_LIBRARY_ARG}{runtime_library}',
          f'{CLANG_CLANGXX_LINK_LIBRARY_ARG}{LIB_CXX_LIBRARY}',
          # Link order matters. The runtime config library must be linked
          # *after* the target's object files to ensure that any user-provided
          # configurations are not overridden by the (weak) default config
          # symbols.
          f'{CLANG_CLANGXX_LINK_LIBRARY_ARG}{runtime_config_library}',
      ]
    return ArgsInfo(clang_args, output_files, target, instrument=False)

  if len(output_files) != 1:
    message = f'Expected exactly one output file, got {len(output_files)}.'
    raise ValueError(message)
  output_file = output_files[0]

  clang_args = [CLANG_CLANGXX_LANGUAGE_ARG, language]
  clang_args += [CLANG_CLANGXX_TARGET_ARG, target]
  clang_args += other_args
  clang_args += [
      CLANG_CLANGXX_OUTPUT_FILE_ARG, CLANG_CLANGXX_OUTPUT_STDOUT_VALUE
  ]
  clang_args += [CLANG_CLANGXX_EMIT_LLVM_ARG]

  return ArgsInfo(clang_args, [output_file], target, instrument=True)
