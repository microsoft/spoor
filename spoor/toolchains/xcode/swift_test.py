# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''Tests for the `swift` wrapper.'''

from shared import Target, SPOOR_INSTRUMENTATION_XCODE_OUTPUT_FILE_MAP_KEY
from swift import main
from test_util import MOCK_BUILD_TOOLS
from test_util import build_intermediate_fixture, plist_fixture
from unittest.mock import call, mock_open, patch
import pytest
import sys


@patch('subprocess.run')
@patch.dict('os.environ', {
    SPOOR_INSTRUMENTATION_XCODE_OUTPUT_FILE_MAP_KEY:
        '/path/to/module-OutputFileMap.json'
},
            clear=True)
def test_runs_swift(run_mock):
  build_tools = MOCK_BUILD_TOOLS
  target = Target('arm64-apple-ios15.0')
  input_args = [
      build_tools.swift,
      '-target',
      target.string,
      '-primary-file',
      '/path/to/a.swift',
      '-primary-file',
      '/path/to/b.swift',
      'arg0',
      'arg1',
      '-o',
      '/path/to/a.bc',
      '-o',
      '/path/to/b.bc',
  ]

  open_output_file_map_handle = mock_open(read_data=build_intermediate_fixture(
      'modified-module-OutputFileMap.json'))
  open_xcframework_info_handle = mock_open(
      read_data=plist_fixture('xcframework_info.plist'))

  with patch('builtins.open') as open_mock:
    open_mock.side_effect = [
        open_output_file_map_handle.return_value,
        open_xcframework_info_handle.return_value,
    ]

    main(input_args, build_tools)

  expected_compile_all_source_to_bitcode_call = call([
      build_tools.swift,
      '-target',
      'arm64-apple-ios15.0',
      '-primary-file',
      '/path/to/a.swift',
      '-primary-file',
      '/path/to/b.swift',
      'arg0',
      'arg1',
      '-o',
      '/path/to/a.bc',
      '-o',
      '/path/to/b.bc',
  ],
                                                     check=True)
  expected_instrument_a_call = call(
      [
          'spoor_opt',
          '/path/to/a.bc',
          '--output_file=/path/to/a.instrumented.bc',
          '--output_symbols_file=/path/to/a.spoor_symbols',
          '--output_language=bitcode',
      ],
      env={
          '_SPOOR_INSTRUMENTATION_XCODE_OUTPUT_FILE_MAP':
              '/path/to/module-OutputFileMap.json',
          'SPOOR_INSTRUMENTATION_MODULE_ID':
              '/path/to/a.o',
          'SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE':
              '/path/to/a.spoor_symbols',
      },
      check=True)
  expected_compile_a_to_object_call = call([
      'clang++',
      '-c',
      '-target',
      'arm64-apple-ios15.0',
      '/path/to/a.instrumented.bc',
      '-o',
      '/path/to/a.o',
  ],
                                           check=True)
  expected_instrument_b_call = call(
      [
          'spoor_opt',
          '/path/to/b.bc',
          '--output_file=/path/to/b.instrumented.bc',
          '--output_symbols_file=/path/to/b.spoor_symbols',
          '--output_language=bitcode',
      ],
      env={
          '_SPOOR_INSTRUMENTATION_XCODE_OUTPUT_FILE_MAP':
              '/path/to/module-OutputFileMap.json',
          'SPOOR_INSTRUMENTATION_MODULE_ID':
              '/path/to/b.o',
          'SPOOR_INSTRUMENTATION_OUTPUT_SYMBOLS_FILE':
              '/path/to/b.spoor_symbols',
      },
      check=True)
  expected_compile_b_to_object_call = call([
      'clang++',
      '-c',
      '-target',
      'arm64-apple-ios15.0',
      '/path/to/b.instrumented.bc',
      '-o',
      '/path/to/b.o',
  ],
                                           check=True)

  assert len(run_mock.call_args_list) == 5

  assert run_mock.call_args_list[
      0] == expected_compile_all_source_to_bitcode_call

  assert expected_instrument_a_call in run_mock.call_args_list
  assert expected_compile_a_to_object_call in run_mock.call_args_list
  assert (run_mock.call_args_list.index(expected_instrument_a_call) <
          run_mock.call_args_list.index(expected_compile_a_to_object_call))

  assert expected_instrument_b_call in run_mock.call_args_list
  assert expected_compile_b_to_object_call in run_mock.call_args_list
  assert (run_mock.call_args_list.index(expected_instrument_b_call) <
          run_mock.call_args_list.index(expected_compile_b_to_object_call))

  open_output_file_map_call, open_xcframework_info_call = \
          open_mock.call_args_list

  expected_open_output_file_map_call = call(
      '/path/to/module-OutputFileMap.json', 'r', encoding='utf-8')
  assert open_output_file_map_call == expected_open_output_file_map_call

  expected_open_xcframework_info_call = call(
      '/path/to/frameworks/SpoorRuntime.xcframework/Info.plist', mode='rb')
  assert open_xcframework_info_call == expected_open_xcframework_info_call


@patch('subprocess.run')
@patch.dict('os.environ', {}, clear=True)
def test_does_not_instrument_if_not_invoked_by_swiftc_driver(run_mock):
  build_tools = MOCK_BUILD_TOOLS
  target = Target('arm64-apple-ios15.0')

  input_args = [
      build_tools.swift,
      '-target',
      target.string,
      '-primary-file',
      '/path/to/a.swift',
      '-primary-file',
      '/path/to/b.swift',
      'arg0',
      'arg1',
      '-o',
      '/path/to/a.bc',
      '-o',
      '/path/to/b.bc',
  ]

  main(input_args, build_tools)

  run_mock.assert_called_once_with(input_args, check=True)


@patch('subprocess.run')
@patch.dict('os.environ', {
    SPOOR_INSTRUMENTATION_XCODE_OUTPUT_FILE_MAP_KEY:
        '/path/to/module-OutputFileMap.json'
},
            clear=True)
def test_does_not_instrument_without_output_files(run_mock):
  build_tools = MOCK_BUILD_TOOLS
  target = Target('arm64-apple-ios15.0')

  input_args = [
      build_tools.swift, '-target', target.string, 'arg0', 'arg1', 'arg2'
  ]

  main(input_args, build_tools)

  run_mock.assert_called_once_with(input_args, check=True)


@patch('subprocess.run')
@patch.dict('os.environ', {
    SPOOR_INSTRUMENTATION_XCODE_OUTPUT_FILE_MAP_KEY:
        '/path/to/module-OutputFileMap.json'
},
            clear=True)
def test_does_not_instrument_unsupported_target(run_mock):
  build_tools = MOCK_BUILD_TOOLS
  target = Target('arm64-apple-jetpack1.0')

  input_args = [
      build_tools.swift,
      '-target',
      target.string,
      '-primary-file',
      '/path/to/a.swift',
      '-primary-file',
      '/path/to/b.swift',
      'arg0',
      'arg1',
      '-o',
      '/path/to/a.bc',
      '-o',
      '/path/to/b.bc',
  ]

  open_output_file_map_handle = mock_open(read_data=build_intermediate_fixture(
      'modified-module-OutputFileMap.json'))
  open_xcframework_info_handle = mock_open(
      read_data=plist_fixture('xcframework_info.plist'))

  with patch('builtins.open') as open_mock:
    open_mock.side_effect = [
        open_output_file_map_handle.return_value,
        open_xcframework_info_handle.return_value,
    ]

    main(input_args, build_tools)

  open_output_file_map_call, open_xcframework_info_call = \
          open_mock.call_args_list

  assert len(run_mock.call_args_list) == 3

  expected_compile_all_source_to_bitcode_call = call([
      'swift',
      '-target',
      'arm64-apple-jetpack1.0',
      '-primary-file',
      '/path/to/a.swift',
      '-primary-file',
      '/path/to/b.swift',
      'arg0',
      'arg1',
      '-o',
      '/path/to/a.bc',
      '-o',
      '/path/to/b.bc',
  ],
                                                     check=True)
  assert run_mock.call_args_list[
      0] == expected_compile_all_source_to_bitcode_call

  expected_compile_a_to_object_call = call([
      'clang++',
      '-c',
      '-target',
      'arm64-apple-jetpack1.0',
      '/path/to/a.bc',
      '-o',
      '/path/to/a.o',
  ],
                                           check=True)
  assert expected_compile_a_to_object_call in run_mock.call_args_list

  expected_compile_b_to_object_call = call([
      'clang++',
      '-c',
      '-target',
      'arm64-apple-jetpack1.0',
      '/path/to/b.bc',
      '-o',
      '/path/to/b.o',
  ],
                                           check=True)
  assert expected_compile_b_to_object_call in run_mock.call_args_list

  expected_open_output_file_map_call = call(
      '/path/to/module-OutputFileMap.json', 'r', encoding='utf-8')
  assert open_output_file_map_call == expected_open_output_file_map_call

  expected_open_xcframework_info_call = call(
      '/path/to/frameworks/SpoorRuntime.xcframework/Info.plist', mode='rb')
  assert open_xcframework_info_call == expected_open_xcframework_info_call


@patch('builtins.open',
       new_callable=mock_open,
       read_data=build_intermediate_fixture('module-OutputFileMap.json'))
@patch('subprocess.run')
@patch.dict('os.environ', {
    SPOOR_INSTRUMENTATION_XCODE_OUTPUT_FILE_MAP_KEY:
        '/path/to/module-OutputFileMap.json'
},
            clear=True)
def test_fails_without_explicit_target_argument(run_mock, open_mock):
  build_tools = MOCK_BUILD_TOOLS
  input_args = [
      build_tools.swift,
      '-primary-file',
      '/path/to/a.swift',
      '-primary-file',
      '/path/to/b.swift',
      'arg0',
      'arg1',
      '-o',
      '/path/to/a.bc',
      '-o',
      '/path/to/b.bc',
  ]

  with pytest.raises(NotImplementedError) as error:
    main(input_args, build_tools)
  assert (str(
      error.value) == "Spoor's swift wrapper requires an explicit target.")

  run_mock.assert_called_once_with(input_args, check=True)
  open_mock.assert_called_once_with('/path/to/module-OutputFileMap.json',
                                    'r',
                                    encoding='utf-8')


if __name__ == '__main__':
  sys.exit(pytest.main(sys.argv[1:]))
