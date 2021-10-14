# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''Tests for the `swift` wrapper.'''

from shared import arg_after
from unittest import mock
from unittest.mock import patch
import subprocess
import pytest
import swift
import sys


@patch('swiftc.subprocess.Popen')
def test_parses_swift_compile_args(popen_mock):
  popen_handle = popen_mock.return_value.__enter__
  input_args_files = [
      f'toolchain/xcode/test_data/build_args/{file}' for file in [
          'swift_compile_swift_ios_arm64_args.txt',
          'swift_compile_swift_ios_x86_64_args.txt',
          'swift_compile_swift_macos_x86_64_args.txt',
      ]
  ]
  for input_args_file in input_args_files:
    with open(input_args_file, encoding='utf-8', mode='r') as file:
      input_args = file.read().strip().split(' ')
      output_object_file = arg_after('-o', input_args)

      swift_process_mock = mock.Mock()
      swift_process_mock.returncode = 0
      spoor_opt_process_mock = mock.Mock()
      spoor_opt_process_mock.returncode = 0
      clangxx_process_mock = mock.Mock()
      clangxx_process_mock.returncode = 0
      popen_handle.side_effect = [
          swift_process_mock,
          spoor_opt_process_mock,
          clangxx_process_mock,
      ]

      return_code = swift.main(['swift'] + input_args, 'default_swift',
                               'spoor_opt', 'default_clangxx')

      assert return_code == 0
      assert len(popen_mock.call_args_list) == 3
      swift_args, spoor_opt_args, clangxx_args = popen_mock.call_args_list

      assert len(swift_args.args) == 1
      assert swift_args.args[0][0] == 'default_swift'
      assert '-target' in swift_args.args[0]
      if 'ios' in input_args_file:
        assert 'apple-ios' in arg_after('-target', swift_args.args[0])
      if 'macos' in input_args_file:
        assert 'apple-macos' in arg_after('-target', swift_args.args[0])
      assert '-o' not in swift_args.args[0]
      assert output_object_file not in swift_args.args[0]
      assert '-emit-ir' in swift_args.args[0]
      assert swift_args.kwargs['stdout'] == subprocess.PIPE
      swift_process_mock.stdout.close.assert_called_once()
      swift_process_mock.wait.assert_called_once()

      assert len(spoor_opt_args.args) == 1
      expected_spoor_opt_args = ['spoor_opt', '--output_language', 'ir']
      assert spoor_opt_args.args[0] == expected_spoor_opt_args
      assert spoor_opt_args.kwargs['stdin'] == swift_process_mock.stdout
      assert spoor_opt_args.kwargs['stdout'] == subprocess.PIPE
      expected_env = {
          'SPOOR_INSTRUMENTATION_MODULE_ID':
              output_object_file,
          'SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE':
              output_object_file[:-2] + '.spoor_symbols',
      }
      assert expected_env.items() <= spoor_opt_args.kwargs['env'].items()
      spoor_opt_process_mock.stdout.close.assert_called_once()
      spoor_opt_process_mock.wait.assert_called_once()

      assert len(clangxx_args.args) == 1
      expected_clangxx_args = [
          'default_clangxx',
          '-target',
          arg_after('-target', input_args),
          '-c',
          '-x',
          'ir',
          '-',
          '-o',
          output_object_file,
      ]
      assert clangxx_args.args[0] == expected_clangxx_args
      assert clangxx_args.kwargs['stdin'] == spoor_opt_process_mock.stdout
      clangxx_process_mock.wait.assert_called_once()

      popen_mock.reset_mock()


@patch('swift.subprocess.Popen')
def test_parses_swift_non_compile_args(popen_mock):
  popen_handle = popen_mock.return_value.__enter__
  input_args = ['foo', 'bar', 'baz']
  assert '-o' not in input_args
  popen_handle.return_value.returncode = 0
  return_code = swift.main(['swift'] + input_args, 'default_swift', 'spoor_opt',
                           'default_clangxx')
  popen_handle.assert_called_once()
  popen_handle.return_value.wait.assert_called_once()
  assert return_code == 0
  assert len(popen_mock.call_args.args) == 1
  assert popen_mock.call_args.args[0] == ['default_swift'] + input_args


@patch('swift.subprocess.Popen')
def test_non_compile_return_code(popen_mock):
  popen_handle = popen_mock.return_value.__enter__
  input_args = ['foo', 'bar', 'baz']
  assert '-o' not in input_args
  for expected_return_code in range(3):
    popen_handle.return_value.returncode = expected_return_code
    return_code = swift.main(['swift'] + input_args, 'default_swift',
                             'spoor_opt', 'default_clangxx')
    popen_handle.assert_called_once()
    popen_handle.return_value.wait.assert_called_once()
    assert return_code == expected_return_code
    popen_mock.reset_mock()


def test_raises_exception_with_no_target():
  with pytest.raises(ValueError) as error:
    swift.main(['swift', '-o', 'a.o'], 'default_swift', 'spoor_opt',
               'default_clangxx')
    assert error == 'No target was supplied.'


def test_raises_exception_with_multiple_outputs_when_compiling():
  args = ['-target', 'arm64-apple-ios15.0', '-o', 'a.o', '-o', 'b.o']
  with pytest.raises(ValueError) as error:
    swift.main(['swift'] + args, 'default_swift', 'spoor_opt',
               'default_clangxx')
    assert error == 'Expected exactly one output file, got 2.'


if __name__ == '__main__':
  sys.exit(pytest.main(sys.argv[1:]))
