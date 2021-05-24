# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''Say hello!'''

import sys
from hello_lib import say_hello

if __name__ == '__main__':
  name = sys.argv[1] if 1 < len(sys.argv) else None
  message = say_hello(name)
  print(message)
