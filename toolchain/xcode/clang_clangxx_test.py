# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''Tests for the `clang` and `clang++` wrappers.'''

from shared import arg_after
from unittest import mock
from unittest.mock import patch
import clang
import clangxx
import pytest
import subprocess
import sys


def _test_parses_compile_args(input_args, input_args_file, main, popen_mock):
  popen_handle = popen_mock.return_value.__enter__

  output_object_file = arg_after('-o', input_args)

  clang_clangxx_process_mock = mock.Mock()
  clang_clangxx_process_mock.returncode = 0
  spoor_opt_process_mock = mock.Mock()
  spoor_opt_process_mock.returncode = 0
  clangxx_process_mock = mock.Mock()
  clangxx_process_mock.returncode = 0
  popen_handle.side_effect = [
      clang_clangxx_process_mock, spoor_opt_process_mock, clangxx_process_mock
  ]

  return_code = main(['clang_clangxx'] + input_args, 'default_clang_clangxx',
                     'spoor_opt', 'default_clangxx', '/spoor/library/path')

  assert return_code == 0
  clang_clangxx_args, spoor_opt_args, clangxx_args = popen_mock.call_args_list

  assert len(clang_clangxx_args.args) == 1
  assert clang_clangxx_args.args[0][0] == 'default_clang_clangxx'
  assert '-target' in clang_clangxx_args.args[0]
  if 'ios' in input_args_file:
    assert 'apple-ios' in arg_after('-target', clang_clangxx_args.args[0])
  if 'macos' in input_args_file:
    assert 'apple-macos' in arg_after('-target', clang_clangxx_args.args[0])
  assert '-x' in clang_clangxx_args.args[0]
  assert arg_after('-x', clang_clangxx_args.args[0]) == 'objective-c'
  assert '-o' in clang_clangxx_args.args[0]
  assert arg_after('-o', clang_clangxx_args.args[0]) == '-'
  assert output_object_file not in clang_clangxx_args.args[0]
  assert '-emit-llvm' in clang_clangxx_args.args[0]
  assert clang_clangxx_args.kwargs['stdout'] == subprocess.PIPE
  clang_clangxx_process_mock.stdout.close.assert_called_once()
  clang_clangxx_process_mock.wait.assert_called_once()

  assert len(spoor_opt_args.args) == 1
  expected_spoor_opt_args = ['spoor_opt', '--output_language', 'ir']
  assert spoor_opt_args.args[0] == expected_spoor_opt_args
  assert spoor_opt_args.kwargs['stdin'] == clang_clangxx_process_mock.stdout
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


@patch('subprocess.Popen')
def test_parses_compile_args(popen_mock):
  input_args_files = [
      f'toolchain/xcode/test_data/build_args/{file}' for file in [
          'clang_compile_objc_ios_arm64_args.txt',
          'clang_compile_objc_ios_x86_64_args.txt',
          'clang_compile_objc_macos_x86_64_args.txt',
      ]
  ]
  for input_args_file in input_args_files:
    with open(input_args_file, encoding='utf-8', mode='r') as file:
      input_args = file.read().strip().split(' ')
      for main in [clang.main, clangxx.main]:
        _test_parses_compile_args(input_args, input_args_file, main, popen_mock)
        popen_mock.reset_mock()


def _test_parses_link_args(input_args, input_args_file, main, popen_mock):
  popen_handle = popen_mock.return_value.__enter__
  popen_handle.return_value.returncode = 0

  return_code = main(['clang_clangxx'] + input_args, 'default_clang_clangxx',
                     'spoor_opt', 'default_clangxx', '/spoor/library/path')

  popen_handle.assert_called_once()
  popen_handle.return_value.wait.assert_called_once()
  assert return_code == 0
  popen_args = popen_mock.call_args.args[0]
  assert popen_args[:-4] == ['default_clang_clangxx'] + input_args
  if 'ios' in input_args_file:
    assert popen_args[-4:] == [
        '-L/spoor/library/path', '-lspoor_runtime_ios', '-lc++',
        '-lspoor_runtime_default_config_ios'
    ]
  if 'macos' in input_args_file:
    assert popen_args[-4:] == [
        '-L/spoor/library/path', '-lspoor_runtime_macos', '-lc++',
        '-lspoor_runtime_default_config_macos'
    ]


@patch('subprocess.Popen')
def test_parses_link_args(popen_mock):
  input_args_files = [
      f'toolchain/xcode/test_data/build_args/{file}' for file in [
          'clang_link_objc_ios_arm64_args.txt',
          'clang_link_objc_ios_x86_64_args.txt',
          'clang_link_objc_macos_x86_64_args.txt',
          'clang_link_swift_ios_arm64_args.txt',
          'clang_link_swift_ios_x86_64_args.txt',
          'clang_link_swift_macos_x86_64_args.txt',
      ]
  ]
  for input_args_file in input_args_files:
    with open(input_args_file, encoding='utf-8', mode='r') as file:
      input_args = file.read().strip().split(' ')
      for main in [clang.main, clangxx.main]:
        _test_parses_link_args(input_args, input_args_file, main, popen_mock)
        popen_mock.reset_mock()


def _test_does_not_link_spoor_for_incremental_link(main, popen_mock):
  input_args = [
      '-target', 'arm64-apple-ios15.0', '-o', 'a.o', '-r', 'foo', 'bar'
  ]
  popen_handle = popen_mock.return_value.__enter__
  popen_handle.return_value.returncode = 0

  return_code = main(['clang_clangxx'] + input_args, 'default_clang_clangxx',
                     'spoor_opt', 'default_clangxx', '/spoor/library/path')

  popen_handle.assert_called_once()
  popen_handle.return_value.wait.assert_called_once()
  assert return_code == 0
  popen_args = popen_mock.call_args.args[0]
  assert popen_args == ['default_clang_clangxx'] + input_args


@patch('subprocess.Popen')
def test_clang_does_not_link_spoor_for_incremental_link(popen_mock):
  for main in [clang.main, clangxx.main]:
    _test_does_not_link_spoor_for_incremental_link(main, popen_mock)
    popen_mock.reset_mock()


def _test_link_return_code(main, popen_mock):
  input_args = [
      '-target', 'arm64-apple-ios15.0', '-o', 'a.o', '-r', 'foo', 'bar'
  ]
  for expected_return_code in range(3):
    popen_handle = popen_mock.return_value.__enter__
    popen_handle.return_value.returncode = expected_return_code

    return_code = main(['clang_clangxx'] + input_args, 'default_clang_clangxx',
                       'spoor_opt', 'default_clangxx', '/spoor/library/path')

    popen_handle.assert_called_once()
    popen_handle.return_value.wait.assert_called_once()
    assert return_code == expected_return_code
    popen_mock.reset_mock()


@patch('subprocess.Popen')
def test_clang_link_return_code(popen_mock):
  for main in [clang.main, clangxx.main]:
    _test_link_return_code(main, popen_mock)


def test_raises_exception_with_no_target():
  input_args = ['foo', '-x', 'bar', '-y', 'baz']
  expected_error = 'No target was supplied.'
  with pytest.raises(ValueError) as error:
    clang.main(input_args, 'clang', 'spoor_opt', 'clang++',
               '/path/to/spoor/lib')
    assert error == expected_error
  with pytest.raises(ValueError) as error:
    clangxx.main(input_args, 'clang++', 'spoor_opt', 'clang++',
                 '/path/to/spoor/lib')
    assert error == expected_error


def test_raises_exception_with_unsupported_target():
  input_args = ['-target', 'arm64-apple-jetpack1.0']
  expected_error = 'Unsupported target arm64-apple-jetpack1.0.'
  with pytest.raises(ValueError) as error:
    clang.main(input_args, 'clang', 'spoor_opt', 'clang++',
               '/spoor/library/path')
    assert error == expected_error
  with pytest.raises(ValueError) as error:
    clangxx.main(input_args, 'clang++', 'spoor_opt', 'clang++',
                 '/spoor/library/path')
    assert error == expected_error


def test_raises_exception_with_multiple_outputs_when_compiling():
  input_args = [
      '-target', 'arm64-apple-ios15.0', '-x', 'objective-c', '-o', 'a.o', '-o',
      'b.o'
  ]
  expected_error = 'Expected exactly one output file, got 2.'
  with pytest.raises(ValueError) as error:
    clang.main(input_args, 'clang', 'spoor_opt', 'clang++',
               '/spoor/library/path')
    assert error == expected_error
  with pytest.raises(ValueError) as error:
    clangxx.main(input_args, 'clang++', 'spoor_opt', 'clang++',
                 '/spoor/library/path')
    assert error == expected_error


if __name__ == '__main__':
  sys.exit(pytest.main(sys.argv[1:]))
