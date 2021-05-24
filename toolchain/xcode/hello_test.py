# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''Tests for `hello_lib`.'''

import pytest
import sys
import hello_lib


def test_say_hello():
  assert hello_lib.say_hello(None) == 'Hello, world!'
  assert hello_lib.say_hello('name') == 'Hello, name!'


if __name__ == '__main__':
  sys.exit(pytest.main(sys.argv[1:]))
