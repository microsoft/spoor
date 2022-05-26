# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''Tests for shared wrapper logic.'''

from shared import BuildTools, Target, RuntimeFramework, make_spoor_opt_env
from unittest.mock import mock_open, patch
import pytest
import sys


def _plist_fixture(file_name):
  plist_path = f'spoor/toolchains/xcode/test_data/info_plists/{file_name}'
  with open(plist_path, mode='rb') as file:
    return file.read()


@patch('shared.__file__', '/path/to/spoor/toolchain/usr/bin/shared.py')
@patch('subprocess.run')
@patch.dict('os.environ', {})
def test_get_developer_path(run_mock):
  developer_dir = '/Applications/Xcode.app/Contents/Developer'
  run_mock.return_value.stdout = f'{developer_dir}\n'

  spoor_toolchain_dir = '/path/to/spoor/toolchain'
  default_toolchain_dir = f'{developer_dir}/Toolchains/XcodeDefault.xctoolchain'

  build_tools = BuildTools.get()

  run_mock.assert_called_once_with(['xcode-select', '--print-path'],
                                   capture_output=True,
                                   text=True,
                                   check=True)

  assert build_tools.clang == f'{default_toolchain_dir}/usr/bin/clang'
  assert build_tools.clangxx == f'{default_toolchain_dir}/usr/bin/clang++'
  assert build_tools.swift == f'{default_toolchain_dir}/usr/bin/swift'
  assert build_tools.swiftc == f'{default_toolchain_dir}/usr/bin/swiftc'
  assert build_tools.spoor_opt == f'{spoor_toolchain_dir}/spoor/bin/spoor_opt'
  assert (build_tools.spoor_frameworks_path ==
          f'{spoor_toolchain_dir}/spoor/frameworks')
  assert build_tools.spoor_swift == f'{spoor_toolchain_dir}/usr/bin/swift'


@patch('shared.__file__', '/path/to/spoor/toolchain/usr/bin/shared.py')
@patch('subprocess.run')
@patch.dict('os.environ', {'DEVELOPER_DIR': '/path/to/developer/dir'})
def test_get_developer_path_env_override(run_mock):
  toolchain_dir = '/path/to/developer/dir/Toolchains/XcodeDefault.xctoolchain'
  spoor_toolchain_dir = '/path/to/spoor/toolchain'

  build_tools = BuildTools.get()

  run_mock.assert_not_called()

  assert build_tools.clang == f'{toolchain_dir}/usr/bin/clang'
  assert build_tools.clangxx == f'{toolchain_dir}/usr/bin/clang++'
  assert build_tools.swift == f'{toolchain_dir}/usr/bin/swift'
  assert build_tools.swiftc == f'{toolchain_dir}/usr/bin/swiftc'
  assert build_tools.spoor_opt == f'{spoor_toolchain_dir}/spoor/bin/spoor_opt'
  assert (build_tools.spoor_frameworks_path ==
          f'{spoor_toolchain_dir}/spoor/frameworks')
  assert build_tools.spoor_swift == f'{spoor_toolchain_dir}/usr/bin/swift'


def test_target():
  target_a = Target('x86_64-apple-ios15.0-simulator')
  assert target_a.architecture == 'x86_64'
  assert target_a.vendor == 'apple'
  assert target_a.platform == 'ios'
  assert target_a.platform_version == '15.0'
  assert target_a.platform_variant == 'simulator'

  target_b = Target('arm64-apple-ios15.0')
  assert target_b.architecture == 'arm64'
  assert target_b.vendor == 'apple'
  assert target_b.platform == 'ios'
  assert target_b.platform_version == '15.0'
  assert target_b.platform_variant is None

  with pytest.raises(ValueError) as error:
    _ = Target('bad-target')
  assert str(error.value) == 'Cannot parse target "bad-target"'


@patch('builtins.open',
       new_callable=mock_open,
       read_data=_plist_fixture('xcframework_info.plist'))
def test_get_runtime_framework(_):
  frameworks_path = '/path/to/frameworks'

  simulator_target = Target('x86_64-apple-ios15.0-simulator')
  simulator_framework = RuntimeFramework.get(frameworks_path, simulator_target)
  assert simulator_framework.name == 'SpoorRuntime'
  assert (
      simulator_framework.path ==
      f'{frameworks_path}/SpoorRuntime.xcframework/ios-arm64_x86_64-simulator')

  device_target = Target('arm64-apple-ios15.0')
  simulator_framework = RuntimeFramework.get(frameworks_path, device_target)
  assert simulator_framework.name == 'SpoorRuntime'
  assert (simulator_framework.path ==
          f'{frameworks_path}/SpoorRuntime.xcframework/ios-arm64')


@patch('builtins.open',
       new_callable=mock_open,
       read_data=_plist_fixture('xcframework_info_unknown_version.plist'))
def test_get_runtime_framework_unknown_xcframework_version(_):
  frameworks_path = '/path/to/frameworks'
  target = Target('arm64-apple-ios15.0')

  with pytest.raises(ValueError) as error:
    _ = RuntimeFramework.get(frameworks_path, target)
  assert str(error.value) == 'Unknown XCFramework format version "2.0"'


@patch('builtins.open',
       new_callable=mock_open,
       read_data=_plist_fixture('xcframework_info.plist'))
def test_get_runtime_framework_unsupported_vendor(_):
  frameworks_path = '/path/to/frameworks'
  target = Target('arm64-orange-ios15.0')

  framework = RuntimeFramework.get(frameworks_path, target)
  assert framework is None


@patch('builtins.open',
       new_callable=mock_open,
       read_data=_plist_fixture('xcframework_info.plist'))
def test_get_runtime_framework_unsupported_architecture(_):
  frameworks_path = '/path/to/frameworks'
  target = Target('riscv-apple-ios15.0')
  framework = RuntimeFramework.get(frameworks_path, target)
  assert framework is None


@patch('builtins.open',
       new_callable=mock_open,
       read_data=_plist_fixture('xcframework_info.plist'))
def test_get_runtime_framework_unsupported_platform(_):
  frameworks_path = '/path/to/frameworks'
  target = Target('arm64-apple-jetpackos15.0')
  framework = RuntimeFramework.get(frameworks_path, target)
  assert framework is None


@patch('builtins.open',
       new_callable=mock_open,
       read_data=_plist_fixture('xcframework_info.plist'))
def test_get_runtime_framework_unsupported_platform_version(_):
  frameworks_path = '/path/to/frameworks'
  target = Target('arm64-apple-ios15.0-emulator')
  framework = RuntimeFramework.get(frameworks_path, target)
  assert framework is None


def test_make_spoor_opt_env():
  module_id = 'module_id'
  output_symbols_file = '/path/to/symbols.spoor_symbols'
  env = {}
  new_env = make_spoor_opt_env(env, module_id, output_symbols_file)
  assert new_env['SPOOR_INSTRUMENTATION_MODULE_ID'] == module_id
  assert (new_env['SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE'] ==
          output_symbols_file)


def test_make_spoor_opt_env_does_not_override_env():
  module_id = 'module_id'
  output_symbols_file = '/path/to/symbols.spoor_symbols'
  env = {
      'SPOOR_INSTRUMENTATION_MODULE_ID':
          'module_id',
      'SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE':
          '/path/to/symbols.spoor_symbols',
  }
  new_env = make_spoor_opt_env(env, 'new_module_id',
                               '/path/to/new_symbols.spoor_symbols')
  assert new_env['SPOOR_INSTRUMENTATION_MODULE_ID'] == module_id
  assert (new_env['SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE'] ==
          output_symbols_file)


if __name__ == '__main__':
  sys.exit(pytest.main(sys.argv[1:]))
