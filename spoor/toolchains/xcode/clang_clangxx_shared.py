# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
'''Shared logic for the `clang` and `clang++` wrappers.'''

from enum import Enum
from shared import RuntimeFramework, Target, make_spoor_opt_env
from shared import SPOOR_INSTRUMENTATION_ENABLE_RUNTIME_DEFAULT_VALUE
from shared import SPOOR_INSTRUMENTATION_ENABLE_RUNTIME_KEY
from shared import SPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME_DEFAULT_VALUE
from shared import SPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME_KEY
from shared import SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION_DEFAULT_VALUE
from shared import SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION_KEY
import sys
import re
import os
import subprocess
import argparse
import pathlib


class BuildPhase:
  '''Models Clang's driver build phases.'''

  class Type(Enum):
    ANALYZER = 'analyzer'
    ASSEMBLER = 'assembler'
    BACKEND = 'backend'
    COMPILER = 'compiler'
    LINKER = 'linker'
    LIPO = 'lipo'
    PREPROCESSOR = 'preprocessor'
    BIND_ARCH = 'bind-arch'
    INPUT = 'input'

  def __init__(self, phase_id, phase_type, input_file, dependency_ids, language,
               arch):
    self.phase_id = phase_id
    self.phase_type = phase_type
    self.input_file = input_file
    self.dependency_ids = dependency_ids
    self.language = language
    self.arch = arch

  @staticmethod
  def get(frontend_clang_clangxx, args):
    phases_result = subprocess.run(
        [frontend_clang_clangxx, '-ccc-print-phases'] + args,
        check=True,
        capture_output=True)
    # `phases_result` can contain invalid unicode characters.
    raw_phases = phases_result.stderr.decode('utf-8', 'ignore')
    return BuildPhase._parse(raw_phases)

  @staticmethod
  def _parse(raw_phases):
    phases = []
    for raw_phase in raw_phases.strip().splitlines():
      raw_phase = raw_phase.strip(' |+-')
      expression = r'^(\d+): (.+?),'
      result = re.search(expression, raw_phase)
      phase_id = int(result.group(1))
      phase_type = BuildPhase.Type(result.group(2))
      if phase_type in {
          BuildPhase.Type.ANALYZER, BuildPhase.Type.ASSEMBLER,
          BuildPhase.Type.BACKEND, BuildPhase.Type.COMPILER,
          BuildPhase.Type.LINKER, BuildPhase.Type.LIPO,
          BuildPhase.Type.PREPROCESSOR
      }:
        expression = r'^\d+: .*, {(.+)}, (.+)$'
        result = re.search(expression, raw_phase)
        raw_dependencies = result.group(1)
        dependency_ids = {int(dep) for dep in raw_dependencies.split(', ')}
        language = result.group(2)
        phase = BuildPhase(phase_id, phase_type, None, dependency_ids, language,
                           None)
      elif phase_type == BuildPhase.Type.BIND_ARCH:
        expression = r'^\d+: bind-arch, "(.+)", {(.*)}, (.+)$'
        result = re.search(expression, raw_phase)
        arch = result.group(1)
        raw_dependencies = result.group(2)
        dependency_ids = {int(dep) for dep in raw_dependencies.split(', ')}
        language = result.group(3)
        phase = BuildPhase(phase_id, phase_type, None, dependency_ids, language,
                           arch)
      elif phase_type == BuildPhase.Type.INPUT:
        expression = r'^\d+: input, "(.*)", (.+)'
        result = re.search(expression, raw_phase)
        input_file = result.group(1)
        language = result.group(2)
        phase = BuildPhase(phase_id, phase_type, input_file, {}, language, None)
      else:
        raise ValueError(f'Unhandled Clang driver phase "{phase_type}"')
      phases.append(phase)
    return phases


# distutils.util.strtobool is deprecated.
def _strtobool(value):
  truthy = {'y', 'yes', 't', 'true', 'on', '1'}
  falsy = {'n', 'no', 'f', 'false', 'off', '0'}
  value = value.lower().strip()
  if value in truthy:
    return 1
  if value in falsy:
    return 0
  raise ValueError(f'Invalid truth value "{value}"')


def _preprocessor_macros(env):
  configs = {
      SPOOR_INSTRUMENTATION_ENABLE_RUNTIME_KEY:
          SPOOR_INSTRUMENTATION_ENABLE_RUNTIME_DEFAULT_VALUE,
      SPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME_KEY:
          SPOOR_INSTRUMENTATION_INITIALIZE_RUNTIME_DEFAULT_VALUE,
      SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION_KEY:
          SPOOR_INSTRUMENTATION_INJECT_INSTRUMENTATION_DEFAULT_VALUE,
  }
  macros = {'__SPOOR__': 1}
  for key, default_value in configs.items():
    if key in env and env[key]:
      macros[key] = _strtobool(env[key])
    else:
      macros[key] = int(default_value)
  return macros


def _compile(args, target, output_file, build_tools, frontend_clang_clangxx):
  frontend_args = [frontend_clang_clangxx] + args + sorted([
      f'-D{key}={value}'
      for key, value in _preprocessor_macros(os.environ).items()
  ])
  frontend_args.append('-emit-llvm')
  frontend_args[frontend_args.index('-o') + 1] = '/dev/stdout'

  spoor_opt_args = [build_tools.spoor_opt]
  spoor_opt_env = make_spoor_opt_env(
      os.environ.copy(), str(pathlib.Path(output_file).with_suffix('.bc')),
      str(pathlib.Path(output_file).with_suffix('.spoor_symbols')))

  backend_args = [
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

  with subprocess.Popen(frontend_args, stdout=subprocess.PIPE) as frontend:
    with subprocess.Popen(spoor_opt_args,
                          env=spoor_opt_env,
                          stdin=frontend.stdout,
                          stdout=subprocess.PIPE) as spoor_opt:
      subprocess.run(backend_args, stdin=spoor_opt.stdout, check=True)


def _link(args, runtime_framework, frontend_clang_clangxx):
  link_args = [frontend_clang_clangxx] + args + [
      f'-F{runtime_framework.path}',
      '-framework',
      runtime_framework.name,
  ]
  subprocess.run(link_args, check=True)


def main(argv, build_tools, frontend_clang_clangxx):
  args = argv[1:]
  args = [
      arg for arg in args
      if arg not in ('-fembed-bitcode-marker', '-fembed-bitcode')
  ]

  parser = argparse.ArgumentParser(add_help=False, allow_abbrev=False)
  parser.add_argument('-target')
  parser.add_argument('-o', dest='output_file')
  known_args, _ = parser.parse_known_args(argv)

  target = Target(known_args.target) if known_args.target else None
  if target is None:
    raise NotImplementedError(
        "Spoor's clang and clang++ wrappers require an explicit target.")

  if known_args.output_file is None:
    raise NotImplementedError(
        "Spoor's clang and clang++ wrappers require an explicit output file.")

  phases = BuildPhase.get(frontend_clang_clangxx, args)
  phase_types = {phase.phase_type for phase in phases}

  if (BuildPhase.Type.COMPILER in phase_types and
      BuildPhase.Type.LINKER in phase_types):
    raise NotImplementedError(
        'Compiling and linking simultaneously is not supported.')

  runtime_framework = RuntimeFramework.get(build_tools.spoor_frameworks_path,
                                           target)
  if runtime_framework is None:
    print(
        f'Warning: Skipping instrumentation for unsupported target "{target}".',
        file=sys.stderr)
    subprocess.run([frontend_clang_clangxx] + args, check=True)
    return

  if BuildPhase.Type.COMPILER in phase_types:
    _compile(args, target, known_args.output_file, build_tools,
             frontend_clang_clangxx)
    return

  if BuildPhase.Type.LINKER in phase_types:
    _link(args, runtime_framework, frontend_clang_clangxx)
    return

  subprocess.run([frontend_clang_clangxx] + args, check=True)
