# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''Tests for shared wrapper logic.'''

from shared import Target, arg_after, flatten, instrument_and_compile_ir
from unittest import mock
from unittest.mock import patch
import pytest
import subprocess
import sys


def test_target_parses_string_without_platform_variant():
  target_string = 'arm64-apple-ios15.0'
  target = Target(target_string)
  assert target.string == target_string
  assert target.architecture == 'arm64'
  assert target.vendor == 'apple'
  assert target.platform == 'ios'
  assert target.platform_version == '15.0'
  assert target.platform_variant is None


def test_target_parses_string_with_platform_variant():
  target_string = 'x86_64-apple-ios15.0-simulator'
  target = Target(target_string)
  assert target.string == target_string
  assert target.architecture == 'x86_64'
  assert target.vendor == 'apple'
  assert target.platform == 'ios'
  assert target.platform_version == '15.0'
  assert target.platform_variant == 'simulator'


def test_target_fails_to_parse_bad_target_string():
  target_string = 'foo'
  expected_error = f'Cannot parse target "{target_string}".'
  with pytest.raises(ValueError) as error:
    _ = Target(target_string)
  assert str(error.value) == expected_error


def test_arg_after():
  args = ['foo', '-x', 'bar', '-y', 'baz']
  assert arg_after('-x', args) == 'bar'
  assert arg_after('-y', args) == 'baz'


def test_flatten():
  input_list = [[1], [2], [3, 4]]
  expected_list = [1, 2, 3, 4]
  flattened_list = flatten(input_list)
  assert flattened_list == expected_list


@patch('subprocess.Popen')
def test_early_exit_on_frontend_process_failure(popen_mock):
  popen_handle = popen_mock.return_value.__enter__
  frontend_process_mock = mock.Mock()
  frontend_process_mock.returncode = 1
  spoor_opt_process_mock = mock.Mock()
  spoor_opt_process_mock.returncode = 0
  clangxx_process_mock = mock.Mock()
  clangxx_process_mock.returncode = 0
  popen_handle.side_effect = [
      frontend_process_mock, spoor_opt_process_mock, clangxx_process_mock
  ]
  with subprocess.Popen(['frontend'],
                        stdout=subprocess.PIPE) as frontend_process:
    return_code = instrument_and_compile_ir(frontend_process, 'spoor_opt',
                                            'clang++', 'a.o', 'target')

    frontend_process.stdout.close.assert_called_once()
    frontend_process.wait.assert_called_once()
    spoor_opt_process_mock.stdout.close.assert_not_called()
    spoor_opt_process_mock.wait.assert_not_called()
    clangxx_process_mock.stdout.close.assert_not_called()
    clangxx_process_mock.wait.assert_not_called()
    assert return_code == 1


@patch('subprocess.Popen')
def test_early_exit_on_spoor_opt_process_failure(popen_mock):
  popen_handle = popen_mock.return_value.__enter__
  frontend_process_mock = mock.Mock()
  frontend_process_mock.returncode = 0
  spoor_opt_process_mock = mock.Mock()
  spoor_opt_process_mock.returncode = 2
  clangxx_process_mock = mock.Mock()
  clangxx_process_mock.returncode = 0
  popen_handle.side_effect = [
      frontend_process_mock, spoor_opt_process_mock, clangxx_process_mock
  ]
  with subprocess.Popen(['frontend'],
                        stdout=subprocess.PIPE) as frontend_process:
    return_code = instrument_and_compile_ir(frontend_process, 'spoor_opt',
                                            'clang++', 'a.o', 'target')

    frontend_process.stdout.close.assert_called_once()
    frontend_process.wait.assert_called_once()
    spoor_opt_process_mock.stdout.close.assert_called_once()
    spoor_opt_process_mock.wait.assert_called_once()
    clangxx_process_mock.stdout.close.assert_not_called()
    clangxx_process_mock.wait.assert_not_called()
    assert return_code == 2


@patch('subprocess.Popen')
def test_exit_on_backend_process_failure(popen_mock):
  popen_handle = popen_mock.return_value.__enter__
  frontend_process_mock = mock.Mock()
  frontend_process_mock.returncode = 0
  spoor_opt_process_mock = mock.Mock()
  spoor_opt_process_mock.returncode = 0
  clangxx_process_mock = mock.Mock()
  clangxx_process_mock.returncode = 3
  popen_handle.side_effect = [
      frontend_process_mock, spoor_opt_process_mock, clangxx_process_mock
  ]
  with subprocess.Popen(['frontend'],
                        stdout=subprocess.PIPE) as frontend_process:
    return_code = instrument_and_compile_ir(frontend_process, 'spoor_opt',
                                            'clang++', 'a.o', 'target')

    frontend_process.stdout.close.assert_called_once()
    frontend_process.wait.assert_called_once()
    spoor_opt_process_mock.stdout.close.assert_called_once()
    spoor_opt_process_mock.wait.assert_called_once()
    clangxx_process_mock.wait.assert_called_once()
    assert return_code == 3


@patch('subprocess.Popen')
@patch.dict('os.environ', {})
def test_sets_environment_if_not_defined(popen_mock):
  popen_handle = popen_mock.return_value.__enter__
  frontend_process_mock = mock.Mock()
  frontend_process_mock.returncode = 0
  spoor_opt_process_mock = mock.Mock()
  spoor_opt_process_mock.returncode = 0
  clangxx_process_mock = mock.Mock()
  clangxx_process_mock.returncode = 0
  popen_handle.side_effect = [
      frontend_process_mock, spoor_opt_process_mock, clangxx_process_mock
  ]
  with subprocess.Popen(['frontend'],
                        stdout=subprocess.PIPE) as frontend_process:
    return_code = instrument_and_compile_ir(frontend_process, 'spoor_opt',
                                            'clang++', 'a.o', 'target')
    assert return_code == 0
    _, spoor_opt_args, _ = popen_mock.call_args_list
    expected_env = {
        'SPOOR_INSTRUMENTATION_MODULE_ID': 'a.o',
        'SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE': 'a.spoor_symbols',
    }
    assert expected_env.items() <= spoor_opt_args.kwargs['env'].items()


@patch('subprocess.Popen')
@patch.dict(
    'os.environ', {
        'SPOOR_INSTRUMENTATION_MODULE_ID': 'module_id',
        'SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE': 'map.spoor_symbols',
    })
def test_does_not_override_environment(popen_mock):
  popen_handle = popen_mock.return_value.__enter__
  frontend_process_mock = mock.Mock()
  frontend_process_mock.returncode = 0
  spoor_opt_process_mock = mock.Mock()
  spoor_opt_process_mock.returncode = 0
  clangxx_process_mock = mock.Mock()
  clangxx_process_mock.returncode = 0
  popen_handle.side_effect = [
      frontend_process_mock, spoor_opt_process_mock, clangxx_process_mock
  ]
  with subprocess.Popen(['frontend'],
                        stdout=subprocess.PIPE) as frontend_process:
    return_code = instrument_and_compile_ir(frontend_process, 'spoor_opt',
                                            'clang++', 'a.o', 'target')
    assert return_code == 0
    _, spoor_opt_args, _ = popen_mock.call_args_list
    expected_env = {
        'SPOOR_INSTRUMENTATION_MODULE_ID': 'module_id',
        'SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE': 'map.spoor_symbols',
    }
    assert expected_env.items() <= spoor_opt_args.kwargs['env'].items()


if __name__ == '__main__':
  sys.exit(pytest.main(sys.argv[1:]))
