// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

import fs from 'fs';
import yargs from 'yargs';
import {
  FOOTER_SIZE_BYTES,
  HEADER_SIZE_BYTES,
  InstrumentedFunctionMap,
  Trace,
} from '@microsoft/spoor-lib';
import {SpoorFileExtension} from './cat';

const NONEXISTENT_FILE = 'does_not_exist.spoor';

beforeEach(() => {
  jest
    .spyOn(fs, 'existsSync')
    .mockImplementation(path => path !== NONEXISTENT_FILE);
  jest.spyOn(fs.promises, 'readFile').mockImplementation(async path => {
    const fileExtension = (path as string).split('.').pop();
    switch (fileExtension) {
      case SpoorFileExtension.Trace: {
        const trace = new Trace();
        return trace.serializeBinary();
      }
      case SpoorFileExtension.BinaryTrace: {
        const trace = Buffer.alloc(HEADER_SIZE_BYTES + FOOTER_SIZE_BYTES);
        return trace;
      }
      case SpoorFileExtension.InstrumentedFunctionMap: {
        const functionMap = new InstrumentedFunctionMap();
        return functionMap.serializeBinary();
      }
      case 'txt': {
        return Buffer.alloc(1);
      }
      default: {
        throw new Error(`Unsupported file type '${path}'`);
      }
    }
  });
});

afterEach(() => jest.restoreAllMocks());

test('help', async () => {
  const parser = yargs.command(require('./cat')).help();
  const output = await new Promise(resolve => {
    parser.parse(
      'cat --help',
      (err: Error, argv: yargs.Argv, output: string) => {
        resolve(output);
      }
    );
  });
  expect(output).toContain('cat <files...>');
  expect(output).toContain(
    'Concatenate Spoor files and write the result to stdout.'
  );
});

test('handles proto output', async () => {
  const args = [
    'cat',
    '--format',
    'proto',
    'trace.spoor',
    'trace.spoor_trace',
    'functions.spoor_function_map',
  ];
  const command = require('./cat');
  const parser = yargs.command(command).help();
  const argv = await parser.parse(args);
  const stdout = jest
    .spyOn(process.stdout, 'write')
    .mockImplementation(() => true);
  await command.handler(argv);
  const output = stdout.mock.calls[0][0];
  expect(output).not.toHaveLength(0);
  const trace = Trace.deserializeBinary(output);
  expect(trace.getEventsList()).toHaveLength(0);
});

test('uses proto output if not specified', async () => {
  const args = [
    'cat',
    'trace.spoor',
    'trace.spoor_trace',
    'functions.spoor_function_map',
  ];
  const command = require('./cat');
  const parser = yargs.command(command).help();
  const argv = await parser.parse(args);
  const stdout = jest
    .spyOn(process.stdout, 'write')
    .mockImplementation(() => true);
  await command.handler(argv);
  const output = stdout.mock.calls[0][0];
  expect(output).not.toHaveLength(0);
  const trace = Trace.deserializeBinary(output);
  expect(trace.getEventsList()).toHaveLength(0);
});

test('handles json output', async () => {
  const args = [
    'cat',
    '--format',
    'json',
    'trace.spoor',
    'trace.spoor_trace',
    'functions.spoor_function_map',
  ];
  const command = require('./cat');
  const parser = yargs.command(command).help();
  const argv = await parser.parse(args);
  const stdout = jest
    .spyOn(process.stdout, 'write')
    .mockImplementation(() => true);
  await command.handler(argv);
  const output = stdout.mock.calls[0][0];
  const trace = JSON.parse(output as string);
  const keys = Object.keys(trace);
  ['createdAt', 'eventsList', 'functionMapMap', 'modulesList'].forEach(key =>
    expect(keys).toContain(key)
  );
});

test('handles chrome output', async () => {
  const args = [
    'cat',
    '--format',
    'chrome',
    'trace.spoor',
    'trace.spoor_trace',
    'functions.spoor_function_map',
  ];
  const command = require('./cat');
  const parser = yargs.command(command).help();
  const argv = await parser.parse(args);
  const stdout = jest
    .spyOn(process.stdout, 'write')
    .mockImplementation(() => true);
  await command.handler(argv);
  const output = stdout.mock.calls[0][0];
  const trace = JSON.parse(output as string);
  const keys = Object.keys(trace);
  ['displayTimeUnit', 'traceEvents', 'otherData'].forEach(key =>
    expect(keys).toContain(key)
  );
});

test('handles none output', async () => {
  const args = [
    'cat',
    '--format',
    'none',
    'trace.spoor',
    'trace.spoor_trace',
    'functions.spoor_function_map',
  ];
  const command = require('./cat');
  const parser = yargs.command(command).help();
  const argv = await parser.parse(args);
  const stdout = jest
    .spyOn(process.stdout, 'write')
    .mockImplementation(() => true);
  await command.handler(argv);
  expect(stdout).toBeCalledTimes(0);
});

test('handles unknown output type', async () => {
  const args = [
    'cat',
    '--format',
    'something',
    'trace.spoor',
    'trace.spoor_trace',
    'functions.spoor_function_map',
  ];
  const parser = yargs.command(require('./cat')).help();
  const output = await new Promise(resolve => {
    parser.parse(args, (err: Error, argv: yargs.Argv, output: string) => {
      resolve(output);
    });
  });
  expect(output).toContain('Invalid values');
});

test('handles no input files', async () => {
  const args = ['cat'];
  const parser = yargs.command(require('./cat')).help();
  const output = await new Promise(resolve => {
    parser.parse(args, (err: Error, argv: yargs.Argv, output: string) => {
      resolve(output);
    });
  });
  expect(output).toContain(
    'Not enough non-option arguments: got 0, need at least 1'
  );
});

test('throws with unknown output format type bypassing parser', async () => {
  const argv = {
    format: 'something',
    files: ['trace.spoor', 'trace.spoor_trace', 'functions.spoor_function_map'],
  };
  const command = require('./cat');
  await expect(command.handler(argv)).rejects.toThrow(
    "Unknown output format 'something'"
  );
});

test('handles bad file extension', async () => {
  const args = [
    'cat',
    'trace.spoor',
    'trace.spoor_trace',
    'functions.spoor_function_map',
    'bad_file.txt',
  ];
  const parser = yargs.command(require('./cat')).help();
  const output = await new Promise(resolve => {
    parser.parse(args, (err: Error, argv: yargs.Argv, output: string) => {
      resolve(output);
    });
  });
  expect(output).toContain("Unsupported file extension '.txt'.");
});

test('throws with bad file bypassing parser', async () => {
  const argv = {
    format: 'proto',
    files: [
      'trace.spoor',
      'trace.spoor_trace',
      'functions.spoor_function_map',
      'bad_file.txt',
    ],
  };
  const command = require('./cat');
  await expect(command.handler(argv)).rejects.toThrow(
    "Cannot parse the file 'bad_file.txt'"
  );
});

test('handles nonexistent file', async () => {
  const args = [
    'cat',
    'trace.spoor',
    'trace.spoor_trace',
    'functions.spoor_function_map',
    NONEXISTENT_FILE,
  ];
  const parser = yargs.command(require('./cat')).help();
  const output = await new Promise(resolve => {
    parser.parse(args, (err: Error, argv: yargs.Argv, output: string) => {
      resolve(output);
    });
  });
  expect(output).toContain(`No such file '${NONEXISTENT_FILE}'.`);
});
