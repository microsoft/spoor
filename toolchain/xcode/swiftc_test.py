# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''Tests for the `swiftc` wrapper.'''

from unittest.mock import patch
import pytest
import swiftc
import sys


@patch('swiftc.subprocess.Popen')
def test_parses_swiftc_args_and_disables_batch_mode(popen_mock):
  popen_handle = popen_mock.return_value.__enter__
  input_args_files = [
      f'toolchain/xcode/test_data/build_args/{file}' for file in [
          'swiftc_driver_swift_ios_arm64_args.txt',
          'swiftc_driver_swift_ios_x86_64_args.txt',
          'swiftc_driver_swift_macos_x86_64_args.txt',
      ]
  ]
  for input_args_file in input_args_files:
    with open(input_args_file, 'r') as file:
      input_args = file.read().strip().split(' ')
      popen_handle.return_value.returncode = 0
      return_code = swiftc.main(['swiftc'] + input_args, 'default_swiftc',
                                'wrapped_swift')
      popen_handle.assert_called_once()
      popen_handle.return_value.wait.assert_called_once()
      assert len(popen_mock.call_args.args) == 1
      popen_args = popen_mock.call_args.args[0]
      assert popen_args[:3] == [
          'default_swiftc', '-driver-use-frontend-path', 'wrapped_swift'
      ]
      assert '-enable-batch-mode' in input_args
      input_args.remove('-enable-batch-mode')
      assert popen_args[3:] == input_args
      assert return_code == 0
      popen_mock.reset_mock()


@patch('swiftc.subprocess.Popen')
def test_raises_when_compiling_with_whole_module_optimization(popen_mock):
  popen_handle = popen_mock.return_value.__enter__
  input_args_file = 'toolchain/xcode/test_data/build_args/' + \
          'swiftc_driver_swift_ios_arm64_whole_module_args.txt'
  with open(input_args_file, 'r') as file:
    input_args = file.read().strip().split(' ')
    with pytest.raises(ValueError) as error:
      swiftc.main(['swiftc'] + input_args, 'default_swiftc', 'wrapped_swift')
      assert error == 'Whole module optimization is not supported yet.'
      popen_handle.assert_not_called()


@patch('swiftc.subprocess.Popen')
def test_returns_subprocess_return_code(popen_mock):
  popen_handle = popen_mock.return_value.__enter__
  input_args = ['foo', 'bar', 'baz']
  for expected_return_code in range(3):
    popen_handle.return_value.returncode = expected_return_code
    return_code = swiftc.main(['swiftc'] + input_args, 'default_swiftc',
                              'wrapped_swift')
    popen_handle.assert_called_once()
    assert return_code == expected_return_code
    popen_mock.reset_mock()


if __name__ == '__main__':
  sys.exit(pytest.main(sys.argv[1:]))
