#!/usr/bin/env python3
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''`clang` wrapper that injects Spoor instrumentation and links in Spoor's
runtime library.'''

from clang_clangxx_shared import main
from shared import BuildTools
import sys

if __name__ == '__main__':
  build_tools = BuildTools.get()
  main(sys.argv, build_tools, build_tools.clang)
