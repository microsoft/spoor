# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

import sys
from hello_lib import say_hello

name = sys.argv[1] if 1 < len(sys.argv) else None
message = hello_lib.say_hello(name)
print(message)
