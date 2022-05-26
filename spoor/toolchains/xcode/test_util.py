# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''Test utilities.'''

from shared import BuildTools
import os

MOCK_BUILD_TOOLS = BuildTools('clang', 'clang++', 'swift', 'swiftc',
                              'spoor_opt', '/path/to/frameworks', 'spoor_swift')


def build_intermediate_fixture(file_name):
  plist_path = os.path.join(
      'spoor/toolchains/xcode/test_data/build_intermediates', file_name)
  with open(plist_path, mode='rb') as file:
    return file.read()


def plist_fixture(file_name):
  plist_path = f'spoor/toolchains/xcode/test_data/info_plists/{file_name}'
  with open(plist_path, mode='rb') as file:
    return file.read()
