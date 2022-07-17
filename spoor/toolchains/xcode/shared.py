# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''Shared logic for the wrapper scripts.'''

import os
import pathlib
import plistlib
import re
import subprocess

# Keys and default values should match spoor/instrumentation/config/config.h.
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

# The key should match spoor/instrumentation/config/env_source.h.
SPOOR_INSTRUMENTATION_MODULE_ID_KEY = 'SPOOR_INSTRUMENTATION_MODULE_ID'

SPOOR_INSTRUMENTATION_XCODE_OUTPUT_FILE_MAP_KEY = \
    '_SPOOR_INSTRUMENTATION_XCODE_OUTPUT_FILE_MAP'


class BuildTools:
  '''Paths to the toolchain's build tools.'''

  def __init__(self, clang, clangxx, swift, swiftc, spoor_opt,
               spoor_frameworks_path, spoor_swift):
    self.clang = clang
    self.clangxx = clangxx
    self.swift = swift
    self.swiftc = swiftc
    self.spoor_opt = spoor_opt
    self.spoor_frameworks_path = spoor_frameworks_path
    self.spoor_swift = spoor_swift

  @staticmethod
  def get():
    developer_path = BuildTools._get_developer_path()
    default_toolchain_path = \
        f'{developer_path}/Toolchains/XcodeDefault.xctoolchain'
    clang = f'{default_toolchain_path}/usr/bin/clang'
    clangxx = f'{default_toolchain_path}/usr/bin/clang++'
    swift = f'{default_toolchain_path}/usr/bin/swift'
    swiftc = f'{default_toolchain_path}/usr/bin/swiftc'

    custom_toolchain_path = BuildTools._get_custom_toolchain_path()
    spoor_opt = f'{custom_toolchain_path}/spoor/bin/spoor_opt'
    spoor_frameworks_path = f'{custom_toolchain_path}/spoor/frameworks'
    spoor_swift = f'{custom_toolchain_path}/usr/bin/swift'

    return BuildTools(clang, clangxx, swift, swiftc, spoor_opt,
                      spoor_frameworks_path, spoor_swift)

  @staticmethod
  def _get_developer_path():
    developer_dir = os.getenv('DEVELOPER_DIR')
    if developer_dir:
      return developer_dir
    result = subprocess.run(['xcode-select', '--print-path'],
                            capture_output=True,
                            text=True,
                            check=True)
    return result.stdout.strip()

  @staticmethod
  def _get_custom_toolchain_path():
    # This path is correct when inside the (generated) toolchain.
    return pathlib.Path(*pathlib.Path(__file__).parent.absolute().parts[:-2])


class Target:
  '''Represents a build target platform and architecture.

  Example
  string = 'arm64-apple-ios15.0-simulator'
  architecture = 'arm64'
  vendor = 'apple'
  platform = 'ios'
  platform_version = '15.0'
  platform_variant = 'simulator'
  '''

  def __init__(self, target_string):
    components = target_string.split('-')
    if len(components) == 3:
      architecture, vendor, platform = components
      platform_variant = None
    elif len(components) == 4:
      architecture, vendor, platform, platform_variant = components
    else:
      raise ValueError(f'Cannot parse target "{target_string}"')
    self.string = target_string
    self.architecture = architecture
    self.vendor = vendor
    platform_components = re.search('([a-zA-Z]+)([0-9.]+)', platform)
    self.platform = platform_components.group(1)
    self.platform_version = platform_components.group(2)
    self.platform_variant = platform_variant

  def __str__(self):
    return self.string


class RuntimeFramework:
  '''Spoor's runtime framework information.'''

  def __init__(self, framework_path, framework_name):
    self.path = framework_path
    self.name = framework_name

  @staticmethod
  def get(spoor_frameworks_path, target):
    supported_xcframework_format_versions = {'1.0'}
    supported_target_vendors = {'apple'}
    plist_path = f'{spoor_frameworks_path}/SpoorRuntime.xcframework/Info.plist'

    if target.vendor not in supported_target_vendors:
      return None

    with open(plist_path, mode='rb') as file:
      data = file.read()
      plist = plistlib.loads(data)
      format_version = plist['XCFrameworkFormatVersion']
      if format_version not in supported_xcframework_format_versions:
        raise ValueError(
            f'Unknown XCFramework format version "{format_version}"')

      for library in plist['AvailableLibraries']:
        identifier = library['LibraryIdentifier']
        path = library['LibraryPath']
        framework_name = str(pathlib.Path(path).with_suffix(''))
        architectures = library['SupportedArchitectures']
        platform = library['SupportedPlatform']
        if 'SupportedPlatformVariant' in library:
          platform_variant = library['SupportedPlatformVariant']
        else:
          platform_variant = None
        if (platform == target.platform and
            platform_variant == target.platform_variant and
            target.architecture in architectures):
          framework_path = \
              f'{spoor_frameworks_path}/SpoorRuntime.xcframework/{identifier}'
          return RuntimeFramework(framework_path, framework_name)
    return None


def make_spoor_opt_env(env, module_id, output_symbols_file):
  if SPOOR_INSTRUMENTATION_MODULE_ID_KEY not in env:
    env[SPOOR_INSTRUMENTATION_MODULE_ID_KEY] = module_id
  if SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE_KEY not in env:
    env[SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE_KEY] = output_symbols_file
  return env
