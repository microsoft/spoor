# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''Say hello to `name` or world.'''


def say_hello(name):
  if name is None:
    name = "world"
  return "Hello, {}!".format(name)
