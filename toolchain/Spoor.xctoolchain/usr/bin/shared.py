# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

import functools
import operator
import os
import pathlib
import subprocess

DEVELOPER_PATH = \
        os.getenv('DEVELOPER_DIR', '/Applications/Xcode.app/Contents/Developer')
DEFAULT_TOOLCHAIN_PATH = f'{DEVELOPER_PATH}/Toolchains/XcodeDefault.xctoolchain'

CLANGXX_INPUT_STDIN_VALUE = '-'
CLANGXX_LANGUAGE_ARG = '-x'
CLANGXX_ONLY_PREPROCESS_COMPILE_AND_ASSEMBLE_ARG = '-c'
CLANGXX_OUTPUT_FILE_ARG = '-o'
CLANGXX_TARGET_ARG = '-target'
CUSTOM_TOOLCHAIN_PATH = \
        pathlib.Path(*pathlib.Path(__file__).parent.absolute().parts[:-2])
DEFAULT_CLANG = f'{DEFAULT_TOOLCHAIN_PATH}/usr/bin/clang'
DEFAULT_CLANGXX = f'{DEFAULT_TOOLCHAIN_PATH}/usr/bin/clang++'
DEFAULT_OPT = subprocess.run(['which', 'opt'], capture_output=True, text=True) \
        .stdout.strip() or '/usr/local/opt/llvm/bin/opt'
DEFAULT_SWIFT = f'{DEFAULT_TOOLCHAIN_PATH}/usr/bin/swift'
DEFAULT_SWIFTC = f'{DEFAULT_TOOLCHAIN_PATH}/usr/bin/swiftc'
INSTRUMENTATION_PASS_LIBRARY_PATH = \
        f'{CUSTOM_TOOLCHAIN_PATH}/spoor/libspoor_instrumentation.dylib'
INSTRUMENTATION_PASS_NAMES = ['inject-spoor-runtime']
LLVM_IR_LANGUAGE = 'ir'
OBJECT_FILE_EXTENSION = '.o'
OK_RETURN_CODE = 0
OPT_INPUT_STDIN_VALUE = '-'
OPT_LOAD_PASS_PLUGIN_ARG = '-load-pass-plugin'
OPT_PASSES_ARG = '-passes'
OPT_WRITE_OUTPUT_AS_LLVM_ASSEMBLY_ARG = '-S'
SPOOR_INSTRUMENTATION_MODULE_ID_KEY = 'SPOOR_INSTRUMENTATION_MODULE_ID'
SPOOR_LIBRARY_PATH = f'{CUSTOM_TOOLCHAIN_PATH}/spoor'
WRAPPED_SWIFT = f'{CUSTOM_TOOLCHAIN_PATH}/usr/bin/swift'

def flatten(list):
    return functools.reduce(operator.iconcat, list or [], [])


def instrument_and_compile_ir(frontend_process, output_file, target):
    opt_args = [
        DEFAULT_OPT,
        OPT_INPUT_STDIN_VALUE,
        OPT_WRITE_OUTPUT_AS_LLVM_ASSEMBLY_ARG,
        f'{OPT_LOAD_PASS_PLUGIN_ARG}={INSTRUMENTATION_PASS_LIBRARY_PATH}',
        f'{OPT_PASSES_ARG}={",".join(INSTRUMENTATION_PASS_NAMES)}',
    ]
    clangxx_args = [
        DEFAULT_CLANGXX,
        CLANGXX_TARGET_ARG, target,
        CLANGXX_ONLY_PREPROCESS_COMPILE_AND_ASSEMBLE_ARG,
        CLANGXX_LANGUAGE_ARG, LLVM_IR_LANGUAGE,
        CLANGXX_INPUT_STDIN_VALUE,
        CLANGXX_OUTPUT_FILE_ARG, output_file,
    ]

    env = os.environ.copy()
    env[SPOOR_INSTRUMENTATION_MODULE_ID_KEY] = output_file

    opt_process = subprocess.Popen(opt_args, stdin=frontend_process.stdout,
            stdout=subprocess.PIPE, env=env)
    frontend_process.stdout.close()
    frontend_process.wait()
    if frontend_process.returncode != OK_RETURN_CODE:
        return frontend_process.returncode
    clangxx_process = subprocess.Popen(clangxx_args, stdin=opt_process.stdout)
    opt_process.stdout.close()
    opt_process.wait()
    if opt_process.returncode != OK_RETURN_CODE:
        return opt_process.returncode
    clangxx_process.wait()
    opt_process.wait()
    if opt_process.returncode != OK_RETURN_CODE:
        return opt_process.returncode
    clangxx_process.wait()
    return clangxx_process.returncode
