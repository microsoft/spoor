# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''Tests for the `clang` and `clang++` wrappers.'''

from shared import CLANG_CLANGXX_EMBED_BITCODE_ARG
from shared import CLANG_CLANGXX_EMBED_BITCODE_MARKER_ARG
from shared import SPOOR_INSTRUMENTATION_ENABLE_RUNTIME_KEY
from shared import SPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME_KEY
from shared import SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION_KEY
from shared import Target, arg_after
from unittest import mock
from unittest.mock import mock_open, patch
import clang
import clangxx
import pytest
import shlex
import subprocess
import sys


def _read_xctoolchain_info_plist():
  plist_path = ('spoor/toolchains/xcode/test_data/info_plists/'
                'spoor_runtime_xctoolchain_info.plist')
  with open(plist_path, mode='rb') as file:
    return file.read()


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
                     'spoor_opt', 'default_clangxx',
                     '/path/to/spoor/frameworks')

  assert return_code == 0
  clang_clangxx_args, spoor_opt_args, clangxx_args = popen_mock.call_args_list

  assert len(clang_clangxx_args.args) == 1
  assert clang_clangxx_args.args[0][0] == 'default_clang_clangxx'
  assert '-target' in clang_clangxx_args.args[0]
  if 'ios' in input_args_file:
    assert 'apple-ios' in arg_after('-target', clang_clangxx_args.args[0])
  if 'macos' in input_args_file:
    assert 'apple-macos' in arg_after('-target', clang_clangxx_args.args[0])
  if 'watchos' in input_args_file:
    assert 'apple-watchos' in arg_after('-target', clang_clangxx_args.args[0])
  assert '-D__SPOOR__=1' in clang_clangxx_args.args[0]
  assert ('-DSPOOR_INSTRUMENTATION_ENABLE_RUNTIME=1'
          in clang_clangxx_args.args[0])
  assert ('-DSPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME=1'
          in clang_clangxx_args.args[0])
  assert ('-DSPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION=1'
          in clang_clangxx_args.args[0])
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
@patch.dict('os.environ', {})
def test_parses_compile_args(popen_mock):
  input_args_files = [
      f'spoor/toolchains/xcode/test_data/build_args/{file}' for file in [
          'clang_compile_objc_ios_arm64_args.txt',
          'clang_compile_objc_ios_x86_64_args.txt',
          # TODO(#257): Reintroduce macOS and watchOS support.
          # 'clang_compile_objc_macos_x86_64_args.txt',
          # 'clang_compile_objc_watchos_arm64_args.txt',
          # 'clang_compile_objc_watchos_x86_64_args.txt',
      ]
  ]
  for input_args_file in input_args_files:
    with open(input_args_file, encoding='utf-8', mode='r') as file:
      input_args = shlex.split(file.read())
      for main in [clang.main, clangxx.main]:
        with patch('builtins.open', mock_open()) as open_mock:
          _test_parses_compile_args(input_args, input_args_file, main,
                                    popen_mock)
          open_mock.assert_not_called()
        popen_mock.reset_mock()


def _get_compile_args(input_args, main, popen_mock):
  popen_handle = popen_mock.return_value.__enter__

  clang_clangxx_process_mock = mock.Mock()
  clang_clangxx_process_mock.returncode = 0
  spoor_opt_process_mock = mock.Mock()
  spoor_opt_process_mock.returncode = 0
  clangxx_process_mock = mock.Mock()
  clangxx_process_mock.returncode = 0
  popen_handle.side_effect = [
      clang_clangxx_process_mock, spoor_opt_process_mock, clangxx_process_mock
  ]

  _ = main(['clang_clangxx'] + input_args, 'default_clang_clangxx', 'spoor_opt',
           'default_clangxx', '/path/to/spoor/frameworks')

  clang_clangxx_args, _, _ = popen_mock.call_args_list
  assert len(clang_clangxx_args.args) == 1
  return clang_clangxx_args.args[0]


@patch('subprocess.Popen')
@patch('builtins.open')
def test_forwards_env_configs(open_mock, popen_mock):
  config_unspecified = {}
  config_empty = {
      SPOOR_INSTRUMENTATION_ENABLE_RUNTIME_KEY: '',
      SPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME_KEY: '',
      SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION_KEY: '',
  }
  config_disabled = {
      SPOOR_INSTRUMENTATION_ENABLE_RUNTIME_KEY: 'false',
      SPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME_KEY: 'NO',
      SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION_KEY: '0',
  }
  config_enabled = {
      SPOOR_INSTRUMENTATION_ENABLE_RUNTIME_KEY: 'true',
      SPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME_KEY: 'YES',
      SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION_KEY: '1',
  }
  input_args = [
      '-target', 'arm64-apple-ios15.0', '-x', 'c', '-o', 'foo.o', 'foo'
  ]
  for main in [clang.main, clangxx.main]:
    for env in [config_unspecified, config_empty, config_enabled]:
      with patch.dict('os.environ', env, clear=True):
        args = _get_compile_args(input_args, main, popen_mock)
        assert '-DSPOOR_INSTRUMENTATION_ENABLE_RUNTIME=1' in args
        assert '-DSPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME=1' in args
        assert '-DSPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION=1' in args
        open_mock.assert_not_called()
        open_mock.reset_mock()
        popen_mock.reset_mock()
    for env in [config_disabled]:
      with patch.dict('os.environ', env, clear=True):
        args = _get_compile_args(input_args, main, popen_mock)
        assert '-DSPOOR_INSTRUMENTATION_ENABLE_RUNTIME=0' in args
        assert '-DSPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME=0' in args
        assert '-DSPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION=0' in args
        open_mock.assert_not_called()
        open_mock.reset_mock()
        popen_mock.reset_mock()


def _test_parses_link_args(input_args, main, popen_mock):
  popen_handle = popen_mock.return_value.__enter__
  popen_handle.return_value.returncode = 0

  spoor_frameworks_path = '/path/to/spoor/frameworks'
  return_code = main(['clang_clangxx'] + input_args, 'default_clang_clangxx',
                     'spoor_opt', 'default_clangxx', spoor_frameworks_path)

  popen_handle.assert_called_once()
  popen_handle.return_value.wait.assert_called_once()
  assert return_code == 0
  popen_args = popen_mock.call_args.args[0]
  target = Target(arg_after('-target', popen_args))
  assert popen_args[:-3] == ['default_clang_clangxx'] + input_args
  framework_search_path = popen_args[-3]
  assert framework_search_path.startswith(
      f'-F{spoor_frameworks_path}/SpoorRuntime.xcframework/')
  assert target.architecture in framework_search_path
  assert target.platform in framework_search_path
  assert popen_args[-2:] == ['-framework', 'SpoorRuntime']


@patch('subprocess.Popen')
@patch.dict('os.environ', {})
def test_parses_link_args(popen_mock):
  input_args_files = [
      f'spoor/toolchains/xcode/test_data/build_args/{file}' for file in [
          'clang_link_objc_ios_arm64_args.txt',
          'clang_link_objc_ios_x86_64_args.txt',
          # TODO(#257): Reintroduce macOS and watchOS support.
          # 'clang_link_objc_macos_x86_64_args.txt',
          # 'clang_link_objc_watchos_arm64_args.txt',
          # 'clang_link_objc_watchos_x86_64_args.txt',
          'clang_link_swift_ios_arm64_args.txt',
          'clang_link_swift_ios_x86_64_args.txt',
          # TODO(#257): Reintroduce macOS and watchOS support.
          # 'clang_link_swift_macos_x86_64_args.txt',
          # 'clang_link_swift_watchos_arm64_args.txt',
          # 'clang_link_swift_watchos_x86_64_args.txt',
      ]
  ]
  for input_args_file in input_args_files:
    with open(input_args_file, encoding='utf-8', mode='r') as file:
      input_args = shlex.split(file.read())
      input_args = list(
          filter((CLANG_CLANGXX_EMBED_BITCODE_ARG).__ne__, input_args))
      input_args = list(
          filter((CLANG_CLANGXX_EMBED_BITCODE_MARKER_ARG).__ne__, input_args))
      for main in [clang.main, clangxx.main]:
        with patch(
            'builtins.open',
            mock_open(read_data=_read_xctoolchain_info_plist())) as open_mock:
          _test_parses_link_args(input_args, main, popen_mock)
          open_mock.assert_called_once_with(
              '/path/to/spoor/frameworks/SpoorRuntime.xcframework/Info.plist',
              mode='rb')
        popen_mock.reset_mock()


def _test_does_not_link_spoor_for_incremental_link(main, popen_mock):
  input_args = [
      '-target', 'arm64-apple-ios15.0', '-o', 'a.o', '-r', 'foo', 'bar'
  ]
  popen_handle = popen_mock.return_value.__enter__
  popen_handle.return_value.returncode = 0

  return_code = main(['clang_clangxx'] + input_args, 'default_clang_clangxx',
                     'spoor_opt', 'default_clangxx',
                     '/path/to/spoor/frameworks')

  popen_handle.assert_called_once()
  popen_handle.return_value.wait.assert_called_once()
  assert return_code == 0
  popen_args = popen_mock.call_args.args[0]
  assert popen_args == ['default_clang_clangxx'] + input_args


@patch('subprocess.Popen')
@patch.dict('os.environ', {})
def test_clang_does_not_link_spoor_for_incremental_link(popen_mock):
  for main in [clang.main, clangxx.main]:
    _test_does_not_link_spoor_for_incremental_link(main, popen_mock)
    popen_mock.reset_mock()


def _test_does_not_embed_bitcode(main, popen_mock):
  input_args = [
      '-target',
      'arm64-apple-ios15.0',
      '-fembed-bitcode',
      '-fembed-bitcode-marker',
      '-o',
      'lib.a',
  ]
  parsed_input_args = [
      '-target',
      'arm64-apple-ios15.0',
      '-o',
      'lib.a',
      '-F/path/to/spoor/frameworks/SpoorRuntime.xcframework/ios-arm64',
      '-framework',
      'SpoorRuntime',
  ]
  popen_handle = popen_mock.return_value.__enter__
  popen_handle.return_value.returncode = 0

  return_code = main(['clang_clangxx'] + input_args, 'default_clang_clangxx',
                     'spoor_opt', 'default_clangxx',
                     '/path/to/spoor/frameworks')

  popen_handle.assert_called_once()
  popen_handle.return_value.wait.assert_called_once()
  assert return_code == 0
  popen_args = popen_mock.call_args.args[0]
  print(popen_args)
  print(parsed_input_args)
  assert popen_args == ['default_clang_clangxx'] + parsed_input_args


@patch('subprocess.Popen')
@patch.dict('os.environ', {})
def test_does_not_embed_bitcode(popen_mock):
  for main in [clang.main, clangxx.main]:
    with patch(
        'builtins.open',
        mock_open(read_data=_read_xctoolchain_info_plist())) as open_mock:
      _test_does_not_embed_bitcode(main, popen_mock)
      open_mock.assert_called_once_with(
          '/path/to/spoor/frameworks/SpoorRuntime.xcframework/Info.plist',
          mode='rb')
      popen_mock.reset_mock()


def _test_does_not_link_analyze(main, popen_mock):
  input_args = [
      '-target',
      'arm64-apple-ios15.0',
      '--analyze',
      'source.m',
      '-o',
      'out.plist',
  ]
  popen_handle = popen_mock.return_value.__enter__
  popen_handle.return_value.returncode = 0

  return_code = main(['clang_clangxx'] + input_args, 'default_clang_clangxx',
                     'spoor_opt', 'default_clangxx',
                     '/path/to/spoor/frameworks')

  popen_handle.assert_called_once()
  popen_handle.return_value.wait.assert_called_once()
  assert return_code == 0
  popen_args = popen_mock.call_args.args[0]
  assert popen_args == ['default_clang_clangxx'] + input_args


@patch('subprocess.Popen')
@patch.dict('os.environ', {})
def test_clang_does_not_link_analyze(popen_mock):
  for main in [clang.main, clangxx.main]:
    _test_does_not_link_analyze(main, popen_mock)
    popen_mock.reset_mock()


def _test_link_return_code(main, popen_mock):
  input_args = [
      '-target', 'arm64-apple-ios15.0', '-o', 'a.o', '-r', 'foo', 'bar'
  ]
  for expected_return_code in range(3):
    popen_handle = popen_mock.return_value.__enter__
    popen_handle.return_value.returncode = expected_return_code

    return_code = main(['clang_clangxx'] + input_args, 'default_clang_clangxx',
                       'spoor_opt', 'default_clangxx',
                       '/path/to/spoor/frameworks')

    popen_handle.assert_called_once()
    popen_handle.return_value.wait.assert_called_once()
    assert return_code == expected_return_code
    popen_mock.reset_mock()


@patch('subprocess.Popen')
@patch.dict('os.environ', {})
def test_clang_link_return_code(popen_mock):
  for main in [clang.main, clangxx.main]:
    _test_link_return_code(main, popen_mock)


def _test_raises_error_linking_unsupported_target(main):
  targets = [
      'armv7k-apple-ios1.0',
      'arm64-apple-jetpack1.0',
      'arm64-apple-ios1.0-other',
  ]
  for target in targets:
    input_args = ['-target', target, '-o', 'a.o', 'foo', 'bar']
    expected_error = f'No runtime library for target "{target}".'
    with patch(
        'builtins.open',
        mock_open(read_data=_read_xctoolchain_info_plist())) as open_mock:
      with pytest.raises(ValueError) as error:
        main(['clang_clangxx'] + input_args, 'default_clang_clangxx',
             'spoor_opt', 'default_clangxx', '/path/to/spoor/frameworks')
      assert str(error.value) == expected_error
      open_mock.assert_called_once_with(
          '/path/to/spoor/frameworks/SpoorRuntime.xcframework/Info.plist',
          mode='rb')


@patch.dict('os.environ', {})
def test_raises_error_linking_unsupported_target():
  for main in [clang.main, clangxx.main]:
    _test_raises_error_linking_unsupported_target(main)


def _test_raises_error_with_unsupported_vendor(main):
  input_args = ['-target', 'arm64-unknown-ios15.0']
  expected_error = 'Unsupported vendor "unknown".'
  with patch('builtins.open',
             mock_open(read_data=_read_xctoolchain_info_plist())) as open_mock:
    with pytest.raises(ValueError) as error:
      main(['default_clang_clangxx'] + input_args, 'clang', 'spoor_opt',
           'clang++', '/path/to/spoor/frameworks')
    assert str(error.value) == expected_error
    open_mock.assert_called_once_with(
        '/path/to/spoor/frameworks/SpoorRuntime.xcframework/Info.plist',
        mode='rb')


@patch.dict('os.environ', {})
def test_raises_error_with_unsupported_vendor():
  for main in [clang.main, clangxx.main]:
    _test_raises_error_with_unsupported_vendor(main)


def _test_raises_error_with_unsupported_plist_version(main):
  plist_path = ('spoor/toolchains/xcode/test_data/info_plists/'
                'unknown_xctoolchain_version.plist')
  input_args = ['-target', 'arm64-apple-ios15.0']
  expected_error = 'Unknown xcframework format version "2.0".'
  with open(plist_path, mode='rb') as file:
    with patch('builtins.open', mock_open(read_data=file.read())) as open_mock:
      with pytest.raises(ValueError) as error:
        main(['default_clang_clangxx'] + input_args, 'clang', 'spoor_opt',
             'clang++', '/path/to/spoor/frameworks')
      assert str(error.value) == expected_error
      open_mock.assert_called_once_with(
          '/path/to/spoor/frameworks/SpoorRuntime.xcframework/Info.plist',
          mode='rb')


@patch.dict('os.environ', {})
def test_raises_error_with_unsupported_plist_veresion():
  for main in [clang.main, clangxx.main]:
    _test_raises_error_with_unsupported_plist_version(main)


@patch.dict('os.environ', {})
def test_raises_exception_with_no_target():
  input_args = ['foo', '-x', 'bar', '-y', 'baz']
  expected_error = 'No target was supplied.'
  for main in [clang.main, clangxx.main]:
    with pytest.raises(ValueError) as error:
      main(['default_clang_clangxx'] + input_args, 'clang', 'spoor_opt',
           'clang++', '/path/to/spoor/frameworks')
    assert str(error.value) == expected_error


@patch.dict('os.environ', {})
def test_raises_exception_with_unsupported_target():
  input_args = ['-target', 'arm64-apple-jetpack1.0']
  expected_error = 'No runtime library for target "arm64-apple-jetpack1.0".'
  for main in [clang.main, clangxx.main]:
    with patch(
        'builtins.open',
        mock_open(read_data=_read_xctoolchain_info_plist())) as open_mock:
      with pytest.raises(ValueError) as error:
        main(['default_clang_clangxx'] + input_args, 'clang', 'spoor_opt',
             'clang++', '/path/to/spoor/frameworks')
      assert str(error.value) == expected_error
      open_mock.assert_called_once_with(
          '/path/to/spoor/frameworks/SpoorRuntime.xcframework/Info.plist',
          mode='rb')


@patch('builtins.open')
@patch.dict('os.environ', {})
def test_raises_exception_with_multiple_outputs_when_compiling(open_mock):
  input_args = [
      '-target', 'arm64-apple-ios15.0', '-x', 'objective-c', '-o', 'a.o', '-o',
      'b.o'
  ]
  expected_error = 'Expected exactly one output file, got 2.'
  for main in [clang.main, clangxx.main]:
    with pytest.raises(ValueError) as error:
      main(['default_clang_clangxx'] + input_args, 'clang', 'spoor_opt',
           'clang++', '/path/to/spoor/frameworks')
    assert str(error.value) == expected_error
    open_mock.assert_not_called()
    open_mock.reset_mock()


if __name__ == '__main__':
  sys.exit(pytest.main(sys.argv[1:]))
