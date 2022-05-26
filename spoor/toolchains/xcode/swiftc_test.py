# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''Tests for the `swiftc` wrapper.'''

from shared import SPOOR_INSTRUMENTATION_XCODE_OUTPUT_FILE_MAP_KEY
from swiftc import main
from test_util import build_intermediate_fixture, MOCK_BUILD_TOOLS
from unittest.mock import mock_open, patch
import json
import pytest
import sys


@patch('subprocess.run')
@patch.dict('os.environ', {}, clear=True)
def test_runs_swiftc(run_mock):
  build_tools = MOCK_BUILD_TOOLS
  output_file_map_path = '/path/to/module-OutputFileMap.json'
  input_args = [
      build_tools.swiftc,
      'arg0',
      '-flag',
      'value',
      '-output-file-map',
      output_file_map_path,
      'arg1',
  ]

  expected_written_data = build_intermediate_fixture(
      'modified-module-OutputFileMap.json')

  with patch(
      'builtins.open',
      mock_open(read_data=build_intermediate_fixture(
          'module-OutputFileMap.json'))) as open_mock:
    main(input_args, build_tools)

    open_mock.return_value.seek.assert_called_once_with(0)
    open_mock.return_value.truncate.assert_called_once_with(0)

    written_data = ''.join([
        call.args[0]
        for call in open_mock.return_value.mock_calls
        if str(call).startswith('call.write')
    ])
    assert json.loads(written_data) == json.loads(expected_written_data)

  run_mock.assert_called_once_with(
      [
          build_tools.swiftc, '-driver-use-frontend-path',
          build_tools.spoor_swift
      ] + input_args[1:] + ['-emit-bc'],
      env={
          SPOOR_INSTRUMENTATION_XCODE_OUTPUT_FILE_MAP_KEY: output_file_map_path
      },
      check=True)


def test_fails_without_output_file_map():
  build_tools = MOCK_BUILD_TOOLS
  input_args = [build_tools.swiftc, 'arg0', '-flag', 'value', 'arg1']

  with pytest.raises(NotImplementedError) as error:
    main(input_args, build_tools)
  assert (str(
      error.value) == "Spoor's swiftc wrapper requires an output file map.")


if __name__ == '__main__':
  sys.exit(pytest.main(sys.argv[1:]))
