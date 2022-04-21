# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''Shared logic for the `clang` and `clang++` wrapper.'''

from shared import ArgsInfo, Target, flatten
from shared import CLANG_CLANGXX_LANGUAGE_ARG, CLANG_CLANGXX_OUTPUT_FILE_ARG
from shared import CLANG_CLANGXX_ONLY_PREPROCESS_COMPILE_AND_ASSEMBLE_ARG
from shared import CLANG_CLANGXX_EMBED_BITCODE_ARG
from shared import CLANG_CLANGXX_EMBED_BITCODE_MARKER_ARG
from shared import CLANG_CLANGXX_TARGET_ARG
from shared import OBJECT_FILE_EXTENSION, LLVM_IR_LANGUAGE
from shared import SPOOR_INSTRUMENTATION_ENABLE_RUNTIME_DEFAULT_VALUE
from shared import SPOOR_INSTRUMENTATION_ENABLE_RUNTIME_KEY
from shared import SPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME_DEFAULT_VALUE
from shared import SPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME_KEY
from shared import SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION_DEFAULT_VALUE
from shared import SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION_KEY
import plistlib
import argparse
import os

CLANG_CLANGXX_ANALYZE_ARG = '--analyze'
CLANG_CLANGXX_EMIT_LLVM_ARG = '-emit-llvm'
CLANG_CLANGXX_FRAMEWORK_SEARCH_PATH_ARG = '-F'
CLANG_CLANGXX_INCREMENTAL_LINK_ARG = '-r'
CLANG_CLANGXX_LINK_FRAMEWORK_ARG = '-framework'
CLANG_CLANGXX_OUTPUT_STDOUT_VALUE = '-'
CLANG_CLANGXX_PREPROCESSOR_DEFINE = '-D'
INFO_PLIST_FILE_NAME = 'Info.plist'
SPOOR_CONSTANT_PREPROCESSOR_MACROS = {'__SPOOR__': 1}
SPOOR_RUNTIME_XCFRAMEWORK_NAME = 'SpoorRuntime.xcframework'
SUPPORTED_LANGUAGES = {
    'c', 'c++', LLVM_IR_LANGUAGE, 'objective-c', 'objective-c++'
}
XCFRAMEWORK_INFO_PLIST_AVAILABLE_LIBRARIES_KEY = 'AvailableLibraries'
XCFRAMEWORK_INFO_PLIST_LIBRARY_IDENTIFIER_KEY = 'LibraryIdentifier'
XCFRAMEWORK_INFO_PLIST_LIBRARY_PATH_KEY = 'LibraryPath'
XCFRAMEWORK_INFO_PLIST_SUPPORTED_ARCHITECTURES_KEY = 'SupportedArchitectures'
XCFRAMEWORK_INFO_PLIST_SUPPORTED_PLATFORM_KEY = 'SupportedPlatform'
XCFRAMEWORK_INFO_PLIST_SUPPORTED_PLATFORM_VARIANT_KEY = \
    'SupportedPlatformVariant'
XCFRAMEWORK_INFO_PLIST_XCFRAMEWORK_FORMAT_VERSION_KEY = \
    'XCFrameworkFormatVersion'


# distutils.util.strtobool is deprecated.
def _strtobool(value):
  truthy = {'y', 'yes', 't', 'true', 'on', '1'}
  falsy = {'n', 'no', 'f', 'false', 'off', '0'}
  value = value.lower().strip()
  if value in truthy:
    return 1
  if value in falsy:
    return 0
  raise ValueError(f'Invalid truth value "{value}"')


def _compiling(language, output_files):
  if language not in SUPPORTED_LANGUAGES:
    return False
  if not output_files:
    return False
  return all(f.endswith(OBJECT_FILE_EXTENSION) for f in output_files)


def _preprocessor_macros(env):
  configs = {
      SPOOR_INSTRUMENTATION_ENABLE_RUNTIME_KEY:
          SPOOR_INSTRUMENTATION_ENABLE_RUNTIME_DEFAULT_VALUE,
      SPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME_KEY:
          SPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME_DEFAULT_VALUE,
      SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION_KEY:
          SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION_DEFAULT_VALUE,
  }
  macros = SPOOR_CONSTANT_PREPROCESSOR_MACROS
  for key, default_value in configs.items():
    if key in env and env[key]:
      macros[key] = _strtobool(env[key])
    else:
      macros[key] = int(default_value)
  return macros


def _link_spoor_runtime(args):
  return (CLANG_CLANGXX_ONLY_PREPROCESS_COMPILE_AND_ASSEMBLE_ARG not in args and
          CLANG_CLANGXX_INCREMENTAL_LINK_ARG not in args and
          CLANG_CLANGXX_ANALYZE_ARG not in args)


def _runtime_framework(spoor_frameworks_path, target):
  supported_xcframework_format_versions = set(['1.0'])
  supported_target_vendors = set(['apple'])
  plist_path = (f'{spoor_frameworks_path}/{SPOOR_RUNTIME_XCFRAMEWORK_NAME}/'
                f'{INFO_PLIST_FILE_NAME}')

  with open(plist_path, mode='rb') as file:
    data = file.read()
    plist = plistlib.loads(data)
    format_version = plist[
        XCFRAMEWORK_INFO_PLIST_XCFRAMEWORK_FORMAT_VERSION_KEY]
    if format_version not in supported_xcframework_format_versions:
      raise ValueError(
          f'Unknown xcframework format version "{format_version}".')

    if target.vendor not in supported_target_vendors:
      raise ValueError(f'Unsupported vendor "{target.vendor}".')

    for library in plist[XCFRAMEWORK_INFO_PLIST_AVAILABLE_LIBRARIES_KEY]:
      identifier = library[XCFRAMEWORK_INFO_PLIST_LIBRARY_IDENTIFIER_KEY]
      path = library[XCFRAMEWORK_INFO_PLIST_LIBRARY_PATH_KEY]
      framework_name = path[:-1 * len('.framework')]
      architectures = library[
          XCFRAMEWORK_INFO_PLIST_SUPPORTED_ARCHITECTURES_KEY]
      platform = library[XCFRAMEWORK_INFO_PLIST_SUPPORTED_PLATFORM_KEY]
      if XCFRAMEWORK_INFO_PLIST_SUPPORTED_PLATFORM_VARIANT_KEY in library:
        platform_variant = library[
            XCFRAMEWORK_INFO_PLIST_SUPPORTED_PLATFORM_VARIANT_KEY]
      else:
        platform_variant = None
      if (platform == target.platform and
          platform_variant == target.platform_variant and
          target.architecture in architectures):
        framework_path = (f'{spoor_frameworks_path}/'
                          f'{SPOOR_RUNTIME_XCFRAMEWORK_NAME}/{identifier}')
        return (framework_path, framework_name)
    raise ValueError(f'No runtime library for target "{target.string}".')


def parse_clang_clangxx_args(args, spoor_frameworks_path):
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
  target = Target(known_args.target[0])

  output_files = flatten(known_args.output_files)

  language = known_args.language[0] if known_args.language and 0 < len(
      known_args.language) else None

  if not _compiling(language, output_files):
    # TODO(265): Support embedded bitcode when the Runtime xcframework also
    # supports embedded bitcode.
    clang_args = list(filter((CLANG_CLANGXX_EMBED_BITCODE_ARG).__ne__, args))
    clang_args = list(
        filter((CLANG_CLANGXX_EMBED_BITCODE_MARKER_ARG).__ne__, clang_args))
    if _link_spoor_runtime(clang_args):
      framework_path, framework_name = _runtime_framework(
          spoor_frameworks_path, target)
      clang_args += [
          f'{CLANG_CLANGXX_FRAMEWORK_SEARCH_PATH_ARG}{framework_path}',
          CLANG_CLANGXX_LINK_FRAMEWORK_ARG,
          framework_name,
      ]
    return ArgsInfo(clang_args, output_files, target.string, instrument=False)

  if len(output_files) != 1:
    message = f'Expected exactly one output file, got {len(output_files)}.'
    raise ValueError(message)
  output_file = output_files[0]

  clang_args = [CLANG_CLANGXX_LANGUAGE_ARG, language]
  clang_args += [CLANG_CLANGXX_TARGET_ARG, target.string]
  clang_args += sorted([
      f'{CLANG_CLANGXX_PREPROCESSOR_DEFINE}{key}={value}'
      for key, value in _preprocessor_macros(os.environ.copy()).items()
  ])
  clang_args += other_args
  clang_args += [
      CLANG_CLANGXX_OUTPUT_FILE_ARG, CLANG_CLANGXX_OUTPUT_STDOUT_VALUE
  ]
  clang_args += [CLANG_CLANGXX_EMIT_LLVM_ARG]

  return ArgsInfo(clang_args, [output_file], target.string, instrument=True)
