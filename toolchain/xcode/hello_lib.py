# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.


def say_hello(name):
    if name is None:
        name = "world"
    return "Hello, {}!".format(name)
