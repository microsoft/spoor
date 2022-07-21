# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''Shared logic for the `clang` and `clang++` wrappers.'''

import os
import pytest
import subprocess
import sys
from clang_clangxx_shared import main
from shared import Target
from test_util import plist_fixture, MOCK_BUILD_TOOLS
from unittest.mock import call, mock_open, patch, MagicMock


@patch('builtins.open',
       new_callable=mock_open,
       read_data=plist_fixture('xcframework_info.plist'))
@patch.dict('os.environ', {}, clear=True)
@patch('subprocess.Popen')
@patch('subprocess.run')
def test_compile_supported_target(run_mock, popen_mock, open_mock):
  build_tools = MOCK_BUILD_TOOLS
  target = Target('arm64-apple-ios15.0')
  input_file = 'input.cc'
  output_file = 'output.o'
  input_args = [
      build_tools.clangxx,
      '-target',
      target.string,
      '-c',
      input_file,
      '-o',
      output_file,
  ]

  run_print_phases_handle = MagicMock(stderr=f'''
                +- 0: input, "{input_file}", c++
             +- 1: preprocessor, {{0}}, c++-cpp-output
          +- 2: compiler, {{1}}, ir
       +- 3: backend, {{2}}, assembler
    +- 4: assembler, {{3}}, object
    5: bind-arch, "{target.architecture}", {{4}}, object
  '''.encode())
  run_mock.side_effect = [run_print_phases_handle]

  run_frontend_handle = MagicMock()
  run_frontend_handle.__enter__.return_value.returncode = os.EX_OK
  run_spoor_opt_handle = MagicMock()
  run_spoor_opt_handle.__enter__.return_value.returncode = os.EX_OK
  run_backend_handle = MagicMock()
  run_backend_handle.__enter__.return_value.returncode = os.EX_OK
  popen_mock.side_effect = [
      run_frontend_handle, run_spoor_opt_handle, run_backend_handle
  ]

  main(input_args, build_tools, build_tools.clangxx)

  print_phases_call = run_mock.call_args_list[0]
  frontend_call, spoor_opt_call, backend_call = popen_mock.call_args_list

  open_mock.assert_called_once_with(os.path.join(
      build_tools.spoor_frameworks_path, 'SpoorRuntime.xcframework/Info.plist'),
                                    mode='rb')

  expected_print_phases_call = call([build_tools.clangxx, '-ccc-print-phases'] +
                                    input_args[1:],
                                    check=True,
                                    capture_output=True)
  assert print_phases_call == expected_print_phases_call

  expected_frontend_call = call([
      build_tools.clangxx,
      '-target',
      target.string,
      '-c',
      input_file,
      '-o',
      '/dev/stdout',
      '-DSPOOR_INSTRUMENTATION_ENABLE_RUNTIME=1',
      '-DSPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME=1',
      '-DSPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION=1',
      '-D__SPOOR__=1',
      '-emit-llvm',
  ],
                                stdout=subprocess.PIPE)
  assert frontend_call == expected_frontend_call

  expected_spoor_opt_call = call(
      [build_tools.spoor_opt],
      env={
          'SPOOR_INSTRUMENTATION_MODULE_ID': 'output.bc',
          'SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE': 'output.spoor_symbols',
      },
      stdin=run_frontend_handle.__enter__.return_value.stdout,
      stdout=subprocess.PIPE)
  assert expected_spoor_opt_call == spoor_opt_call

  expected_backend_call = call(
      [
          build_tools.clangxx,
          '-x',
          'ir',
          '-',
          '-c',
          '-target',
          target.string,
          '-o',
          output_file,
      ],
      stdin=run_spoor_opt_handle.__enter__.return_value.stdout)
  assert backend_call == expected_backend_call


@patch('builtins.open',
       new_callable=mock_open,
       read_data=plist_fixture('xcframework_info.plist'))
@patch.dict('os.environ', {
    'SPOOR_INSTRUMENTATION_ENABLE_RUNTIME': '0',
    'SPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME': '0',
    'SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION': '0',
},
            clear=True)
@patch('subprocess.Popen')
@patch('subprocess.run')
def test_compile_supported_target_env_override(run_mock, popen_mock, open_mock):
  build_tools = MOCK_BUILD_TOOLS
  target = Target('arm64-apple-ios15.0')
  input_file = 'input.cc'
  output_file = 'output.o'
  input_args = [
      build_tools.clangxx,
      '-target',
      target.string,
      '-c',
      input_file,
      '-o',
      output_file,
  ]

  run_print_phases_handle = MagicMock(stderr=f'''
                +- 0: input, "{input_file}", c++
             +- 1: preprocessor, {{0}}, c++-cpp-output
          +- 2: compiler, {{1}}, ir
       +- 3: backend, {{2}}, assembler
    +- 4: assembler, {{3}}, object
    5: bind-arch, "{target.architecture}", {{4}}, object
  '''.encode())
  run_mock.side_effect = [run_print_phases_handle]

  run_frontend_handle = MagicMock()
  run_frontend_handle.__enter__.return_value.returncode = os.EX_OK
  run_spoor_opt_handle = MagicMock()
  run_spoor_opt_handle.__enter__.return_value.returncode = os.EX_OK
  run_backend_handle = MagicMock()
  run_backend_handle.__enter__.return_value.returncode = os.EX_OK
  popen_mock.side_effect = [
      run_frontend_handle, run_spoor_opt_handle, run_backend_handle
  ]

  main(input_args, build_tools, build_tools.clangxx)

  print_phases_call = run_mock.call_args_list[0]
  frontend_call, spoor_opt_call, backend_call = popen_mock.call_args_list

  open_mock.assert_called_once_with(os.path.join(
      build_tools.spoor_frameworks_path, 'SpoorRuntime.xcframework/Info.plist'),
                                    mode='rb')

  expected_print_phases_call = call([build_tools.clangxx, '-ccc-print-phases'] +
                                    input_args[1:],
                                    check=True,
                                    capture_output=True)
  assert print_phases_call == expected_print_phases_call

  expected_frontend_call = call([
      build_tools.clangxx,
      '-target',
      target.string,
      '-c',
      input_file,
      '-o',
      '/dev/stdout',
      '-DSPOOR_INSTRUMENTATION_ENABLE_RUNTIME=0',
      '-DSPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME=0',
      '-DSPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION=0',
      '-D__SPOOR__=1',
      '-emit-llvm',
  ],
                                stdout=subprocess.PIPE)
  assert frontend_call == expected_frontend_call

  expected_spoor_opt_call = call(
      [build_tools.spoor_opt],
      env={
          'SPOOR_INSTRUMENTATION_ENABLE_RUNTIME': '0',
          'SPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME': '0',
          'SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION': '0',
          'SPOOR_INSTRUMENTATION_MODULE_ID': 'output.bc',
          'SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE': 'output.spoor_symbols',
      },
      stdin=run_frontend_handle.__enter__.return_value.stdout,
      stdout=subprocess.PIPE)
  assert expected_spoor_opt_call == spoor_opt_call

  expected_backend_call = call(
      [
          build_tools.clangxx,
          '-x',
          'ir',
          '-',
          '-c',
          '-target',
          target.string,
          '-o',
          output_file,
      ],
      stdin=run_spoor_opt_handle.__enter__.return_value.stdout)
  assert backend_call == expected_backend_call


@patch('builtins.open',
       new_callable=mock_open,
       read_data=plist_fixture('xcframework_info.plist'))
@patch.dict('os.environ', {}, clear=True)
@patch('subprocess.Popen')
@patch('subprocess.run')
def test_raises_exception_on_frontend_failure(run_mock, popen_mock, open_mock):
  build_tools = MOCK_BUILD_TOOLS
  target = Target('arm64-apple-ios15.0')
  input_file = 'input.cc'
  output_file = 'output.o'
  input_args = [
      build_tools.clangxx,
      '-target',
      target.string,
      '-c',
      input_file,
      '-o',
      output_file,
  ]

  run_print_phases_handle = MagicMock(stderr=f'''
                +- 0: input, "{input_file}", c++
             +- 1: preprocessor, {{0}}, c++-cpp-output
          +- 2: compiler, {{1}}, ir
       +- 3: backend, {{2}}, assembler
    +- 4: assembler, {{3}}, object
    5: bind-arch, "{target.architecture}", {{4}}, object
  '''.encode())
  run_mock.side_effect = [run_print_phases_handle]

  run_frontend_handle = MagicMock()
  run_frontend_handle.__enter__.return_value.returncode = os.EX_SOFTWARE
  run_spoor_opt_handle = MagicMock()
  popen_mock.side_effect = [run_frontend_handle, run_spoor_opt_handle]

  with pytest.raises(subprocess.CalledProcessError) as error:
    main(input_args, build_tools, build_tools.clangxx)
  assert error.value.returncode == os.EX_SOFTWARE

  expected_frontend_call_args = [
      build_tools.clangxx,
      '-target',
      target.string,
      '-c',
      input_file,
      '-o',
      '/dev/stdout',
      '-DSPOOR_INSTRUMENTATION_ENABLE_RUNTIME=1',
      '-DSPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME=1',
      '-DSPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION=1',
      '-D__SPOOR__=1',
      '-emit-llvm',
  ]
  assert error.value.cmd == expected_frontend_call_args

  print_phases_call = run_mock.call_args_list[0]
  frontend_call, spoor_opt_call = popen_mock.call_args_list

  open_mock.assert_called_once_with(os.path.join(
      build_tools.spoor_frameworks_path, 'SpoorRuntime.xcframework/Info.plist'),
                                    mode='rb')

  expected_print_phases_call = call([build_tools.clangxx, '-ccc-print-phases'] +
                                    input_args[1:],
                                    check=True,
                                    capture_output=True)
  assert print_phases_call == expected_print_phases_call

  expected_frontend_call = call(expected_frontend_call_args,
                                stdout=subprocess.PIPE)
  assert frontend_call == expected_frontend_call

  expected_spoor_opt_call = call(
      [build_tools.spoor_opt],
      env={
          'SPOOR_INSTRUMENTATION_MODULE_ID': 'output.bc',
          'SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE': 'output.spoor_symbols',
      },
      stdin=run_frontend_handle.__enter__.return_value.stdout,
      stdout=subprocess.PIPE)
  assert expected_spoor_opt_call == spoor_opt_call


@patch('builtins.open',
       new_callable=mock_open,
       read_data=plist_fixture('xcframework_info.plist'))
@patch.dict('os.environ', {}, clear=True)
@patch('subprocess.Popen')
@patch('subprocess.run')
def test_raises_exception_on_spoor_opt_failure(run_mock, popen_mock, open_mock):
  build_tools = MOCK_BUILD_TOOLS
  target = Target('arm64-apple-ios15.0')
  input_file = 'input.cc'
  output_file = 'output.o'
  input_args = [
      build_tools.clangxx,
      '-target',
      target.string,
      '-c',
      input_file,
      '-o',
      output_file,
  ]

  run_print_phases_handle = MagicMock(stderr=f'''
                +- 0: input, "{input_file}", c++
             +- 1: preprocessor, {{0}}, c++-cpp-output
          +- 2: compiler, {{1}}, ir
       +- 3: backend, {{2}}, assembler
    +- 4: assembler, {{3}}, object
    5: bind-arch, "{target.architecture}", {{4}}, object
  '''.encode())
  run_mock.side_effect = [run_print_phases_handle]

  run_frontend_handle = MagicMock()
  run_frontend_handle.__enter__.return_value.returncode = os.EX_OK
  run_spoor_opt_handle = MagicMock()
  run_spoor_opt_handle.__enter__.return_value.returncode = os.EX_SOFTWARE
  run_backend_handle = MagicMock()
  popen_mock.side_effect = [
      run_frontend_handle, run_spoor_opt_handle, run_backend_handle
  ]

  with pytest.raises(subprocess.CalledProcessError) as error:
    main(input_args, build_tools, build_tools.clangxx)
  assert error.value.returncode == os.EX_SOFTWARE

  expected_spoor_opt_call_args = [build_tools.spoor_opt]
  assert error.value.cmd == expected_spoor_opt_call_args

  print_phases_call = run_mock.call_args_list[0]
  frontend_call, spoor_opt_call, backend_call = popen_mock.call_args_list

  open_mock.assert_called_once_with(os.path.join(
      build_tools.spoor_frameworks_path, 'SpoorRuntime.xcframework/Info.plist'),
                                    mode='rb')

  expected_print_phases_call = call([build_tools.clangxx, '-ccc-print-phases'] +
                                    input_args[1:],
                                    check=True,
                                    capture_output=True)
  assert print_phases_call == expected_print_phases_call

  expected_frontend_call = call([
      build_tools.clangxx,
      '-target',
      target.string,
      '-c',
      input_file,
      '-o',
      '/dev/stdout',
      '-DSPOOR_INSTRUMENTATION_ENABLE_RUNTIME=1',
      '-DSPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME=1',
      '-DSPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION=1',
      '-D__SPOOR__=1',
      '-emit-llvm',
  ],
                                stdout=subprocess.PIPE)
  assert frontend_call == expected_frontend_call

  expected_spoor_opt_call = call(
      expected_spoor_opt_call_args,
      env={
          'SPOOR_INSTRUMENTATION_MODULE_ID': 'output.bc',
          'SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE': 'output.spoor_symbols',
      },
      stdin=run_frontend_handle.__enter__.return_value.stdout,
      stdout=subprocess.PIPE)
  assert expected_spoor_opt_call == spoor_opt_call

  expected_backend_call = call(
      [
          build_tools.clangxx,
          '-x',
          'ir',
          '-',
          '-c',
          '-target',
          target.string,
          '-o',
          output_file,
      ],
      stdin=run_spoor_opt_handle.__enter__.return_value.stdout)
  assert backend_call == expected_backend_call


@patch('builtins.open',
       new_callable=mock_open,
       read_data=plist_fixture('xcframework_info.plist'))
@patch.dict('os.environ', {}, clear=True)
@patch('subprocess.Popen')
@patch('subprocess.run')
def test_raises_exception_on_backend_failure(run_mock, popen_mock, open_mock):
  build_tools = MOCK_BUILD_TOOLS
  target = Target('arm64-apple-ios15.0')
  input_file = 'input.cc'
  output_file = 'output.o'
  input_args = [
      build_tools.clangxx,
      '-target',
      target.string,
      '-c',
      input_file,
      '-o',
      output_file,
  ]

  run_print_phases_handle = MagicMock(stderr=f'''
                +- 0: input, "{input_file}", c++
             +- 1: preprocessor, {{0}}, c++-cpp-output
          +- 2: compiler, {{1}}, ir
       +- 3: backend, {{2}}, assembler
    +- 4: assembler, {{3}}, object
    5: bind-arch, "{target.architecture}", {{4}}, object
  '''.encode())
  run_mock.side_effect = [run_print_phases_handle]

  run_frontend_handle = MagicMock()
  run_frontend_handle.__enter__.return_value.returncode = os.EX_OK
  run_spoor_opt_handle = MagicMock()
  run_spoor_opt_handle.__enter__.return_value.returncode = os.EX_OK
  run_backend_handle = MagicMock()
  run_backend_handle.__enter__.return_value.returncode = os.EX_SOFTWARE
  popen_mock.side_effect = [
      run_frontend_handle, run_spoor_opt_handle, run_backend_handle
  ]

  with pytest.raises(subprocess.CalledProcessError) as error:
    main(input_args, build_tools, build_tools.clangxx)
  assert error.value.returncode == os.EX_SOFTWARE

  expected_backend_call_args = [
      build_tools.clangxx,
      '-x',
      'ir',
      '-',
      '-c',
      '-target',
      target.string,
      '-o',
      output_file,
  ]
  assert error.value.cmd == expected_backend_call_args

  print_phases_call = run_mock.call_args_list[0]
  frontend_call, spoor_opt_call, backend_call = popen_mock.call_args_list

  open_mock.assert_called_once_with(os.path.join(
      build_tools.spoor_frameworks_path, 'SpoorRuntime.xcframework/Info.plist'),
                                    mode='rb')

  expected_print_phases_call = call([build_tools.clangxx, '-ccc-print-phases'] +
                                    input_args[1:],
                                    check=True,
                                    capture_output=True)
  assert print_phases_call == expected_print_phases_call

  expected_frontend_call = call([
      build_tools.clangxx,
      '-target',
      target.string,
      '-c',
      input_file,
      '-o',
      '/dev/stdout',
      '-DSPOOR_INSTRUMENTATION_ENABLE_RUNTIME=1',
      '-DSPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME=1',
      '-DSPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION=1',
      '-D__SPOOR__=1',
      '-emit-llvm',
  ],
                                stdout=subprocess.PIPE)
  assert frontend_call == expected_frontend_call

  expected_spoor_opt_call = call(
      [build_tools.spoor_opt],
      env={
          'SPOOR_INSTRUMENTATION_MODULE_ID': 'output.bc',
          'SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE': 'output.spoor_symbols',
      },
      stdin=run_frontend_handle.__enter__.return_value.stdout,
      stdout=subprocess.PIPE)
  assert expected_spoor_opt_call == spoor_opt_call

  expected_backend_call = call(
      expected_backend_call_args,
      stdin=run_spoor_opt_handle.__enter__.return_value.stdout)
  assert backend_call == expected_backend_call


@patch('builtins.open',
       new_callable=mock_open,
       read_data=plist_fixture('xcframework_info.plist'))
@patch('subprocess.run')
def test_compile_unsupported_target(run_mock, open_mock):
  build_tools = MOCK_BUILD_TOOLS
  target = Target('arm64-apple-jetpackos1.0')
  input_file = 'input.cc'
  output_file = 'output.o'
  input_args = [
      build_tools.clangxx,
      '-target',
      target.string,
      '-c',
      input_file,
      '-o',
      output_file,
  ]

  run_print_phases_handle = MagicMock(stderr=f'''
                +- 0: input, "{input_file}", c++
             +- 1: preprocessor, {{0}}, c++-cpp-output
          +- 2: compiler, {{1}}, ir
       +- 3: backend, {{2}}, assembler
    +- 4: assembler, {{3}}, object
    5: bind-arch, "{target.architecture}", {{4}}, object
  '''.encode())
  run_compile_handle = MagicMock()
  run_mock.side_effect = [run_print_phases_handle, run_compile_handle]

  main(input_args, build_tools, build_tools.clangxx)

  print_phases_call, compile_call = run_mock.call_args_list

  open_mock.assert_called_once_with(os.path.join(
      build_tools.spoor_frameworks_path, 'SpoorRuntime.xcframework/Info.plist'),
                                    mode='rb')

  expected_print_phases_call = call([build_tools.clangxx, '-ccc-print-phases'] +
                                    input_args[1:],
                                    check=True,
                                    capture_output=True)
  assert print_phases_call == expected_print_phases_call

  compile_build_call = call(input_args, check=True)
  assert compile_call == compile_build_call


@patch('builtins.open',
       new_callable=mock_open,
       read_data=plist_fixture('xcframework_info.plist'))
@patch('subprocess.run')
def test_link_supported_target(run_mock, open_mock):
  build_tools = MOCK_BUILD_TOOLS
  target = Target('arm64-apple-ios15.0')
  input_file = 'input.o'
  output_file = 'output'
  input_args = [
      build_tools.clangxx,
      '-target',
      target.string,
      input_file,
      '-o',
      output_file,
  ]

  run_print_phases_handle = MagicMock(stderr=f'''
       +- 0: input, "{input_file}", object
    +- 1: linker, {{0}}, image
    2: bind-arch, "{target.architecture}", {{1}}, image
  '''.encode())
  run_link_handle = MagicMock()
  run_mock.side_effect = [run_print_phases_handle, run_link_handle]

  main(input_args, build_tools, build_tools.clangxx)

  print_phases_call, link_call = run_mock.call_args_list

  open_mock.assert_called_once_with(os.path.join(
      build_tools.spoor_frameworks_path, 'SpoorRuntime.xcframework/Info.plist'),
                                    mode='rb')

  expected_print_phases_call = call([build_tools.clangxx, '-ccc-print-phases'] +
                                    input_args[1:],
                                    check=True,
                                    capture_output=True)
  assert print_phases_call == expected_print_phases_call

  expected_link_call = call([build_tools.clangxx] + input_args[1:] + [
      (f'-F{build_tools.spoor_frameworks_path}/SpoorRuntime.xcframework/'
       f'{target.platform}-{target.architecture}'),
      '-framework',
      'SpoorRuntime',
  ],
                            check=True)
  assert link_call == expected_link_call


@patch('builtins.open',
       new_callable=mock_open,
       read_data=plist_fixture('xcframework_info.plist'))
@patch('subprocess.run')
def test_link_unsupported_target(run_mock, open_mock):
  build_tools = MOCK_BUILD_TOOLS
  target = Target('arm64-apple-jetpack1.0')
  input_file = 'input.o'
  output_file = 'output'
  input_args = [
      build_tools.clangxx,
      '-target',
      target.string,
      input_file,
      '-o',
      output_file,
  ]

  run_print_phases_handle = MagicMock(stderr=f'''
       +- 0: input, "{input_file}", object
    +- 1: linker, {{0}}, image
    2: bind-arch, "{target.architecture}", {{1}}, image
  '''.encode())
  run_link_handle = MagicMock()
  run_mock.side_effect = [run_print_phases_handle, run_link_handle]

  main(input_args, build_tools, build_tools.clangxx)

  print_phases_call, link_call = run_mock.call_args_list

  open_mock.assert_called_once_with(os.path.join(
      build_tools.spoor_frameworks_path, 'SpoorRuntime.xcframework/Info.plist'),
                                    mode='rb')

  expected_print_phases_call = call([build_tools.clangxx, '-ccc-print-phases'] +
                                    input_args[1:],
                                    check=True,
                                    capture_output=True)
  assert print_phases_call == expected_print_phases_call

  expected_link_call = call(input_args, check=True)
  assert link_call == expected_link_call


def test_fails_without_explicit_output_file():
  build_tools = MOCK_BUILD_TOOLS
  target = Target('arm64-apple-ios15.0')
  input_args = [build_tools.clangxx, '-target', target.string, 'input.cc']

  with pytest.raises(NotImplementedError) as error:
    main(input_args, build_tools, build_tools.clangxx)
  assert (str(error.value) ==
          "Spoor's clang and clang++ wrappers require an explicit output file.")


@patch('subprocess.run')
def test_simultaneous_compile_link_fails(run_mock):
  build_tools = MOCK_BUILD_TOOLS
  target = Target('arm64-apple-ios15.0')
  input_file = 'input.cc'
  output_file = 'output.o'
  input_args = [
      build_tools.clangxx,
      '-target',
      target.string,
      input_file,
      '-o',
      output_file,
  ]
  run_mock.return_value.stderr = f'''
                   +- 0: input, "{input_file}", c++
                +- 1: preprocessor, {{0}}, c++-cpp-output
             +- 2: compiler, {{1}}, ir
          +- 3: backend, {{2}}, assembler
       +- 4: assembler, {{3}}, object
    +- 5: linker, {{4}}, image
    6: bind-arch, "{target.architecture}", {{5}}, image
  '''.encode()

  with pytest.raises(NotImplementedError) as error:
    main(input_args, build_tools, build_tools.clangxx)
  assert (str(
      error.value) == 'Compiling and linking simultaneously is not supported.')
  run_mock.assert_called_once_with([build_tools.clangxx, '-ccc-print-phases'] +
                                   input_args[1:],
                                   check=True,
                                   capture_output=True)


def test_infers_host_target_if_not_specified():
  build_tools = MOCK_BUILD_TOOLS
  input_file = 'input.o'
  output_file = 'output'
  input_args = [build_tools.clangxx, input_file, '-o', output_file]

  with pytest.raises(NotImplementedError) as error:
    main(input_args, build_tools, build_tools.clangxx)
  assert (str(error.value) ==
          "Spoor's clang and clang++ wrappers require an explicit target.")


if __name__ == '__main__':
  sys.exit(pytest.main(sys.argv[1:]))
