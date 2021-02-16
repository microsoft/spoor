// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

import assert from 'assert';
import fs from 'fs';
import * as yargs from 'yargs';
import {
  BinaryTrace,
  ChromeTrace,
  InstrumentedFunctionMap,
  Trace,
  mergeTraces,
} from '@microsoft/spoor-lib';

export enum OutputFormat {
  Proto = 'proto',
  Json = 'json',
  Chrome = 'chrome',
  None = 'none',
}

export enum SpoorFileExtension {
  Trace = 'spoor',
  BinaryTrace = 'spoor_trace',
  InstrumentedFunctionMap = 'spoor_function_map',
}

interface Arguments {
  format: OutputFormat;
  files: string[];
}

type YargArgv = yargs.Arguments<
  yargs.Omit<{format: OutputFormat} & {files: string | undefined}, 'files'> & {
    files: yargs.ToArray<string | undefined>;
  }
>;

const joinedSpoorFileExtensionsDescription = (() => {
  const values = Object.values(SpoorFileExtension);
  assert(2 < values.length);
  const extensions = values.map(extension => `.${extension}`);
  const last = extensions.pop();
  return `${extensions.join(', ')}, and ${last}`;
})();

exports.command = 'cat <files...>';

exports.describe = (() => {
  return 'Concatenate Spoor files and write the result to stdout.';
})();

exports.builder = (yargs: yargs.Argv) => {
  yargs
    .option('format', {
      describe: 'Output format',
      choices: Object.values(OutputFormat),
      default: OutputFormat.Proto,
      type: 'string',
    })
    .positional('files', {
      describe:
        'Spoor files to concatenate. Supported file extensions: ' +
        `${joinedSpoorFileExtensionsDescription}.`,
      type: 'string',
      normalize: true,
    })
    .array('files')
    .check((args: YargArgv) => {
      const files = args.files as string[];
      return files.every((file: string) => {
        const fileExtension = file.split('.').pop() as SpoorFileExtension;
        if (!Object.values(SpoorFileExtension).includes(fileExtension)) {
          throw new Error(
            `Unsupported file extension '.${fileExtension}'. Supported file ` +
              `extensions: ${joinedSpoorFileExtensionsDescription}.`
          );
        }
        // Yargs' builder check does not support async.
        if (!fs.existsSync(file)) {
          throw new Error(`No such file '${file}'.`);
        }
        return true;
      });
    });
};

exports.handler = async (args: Arguments) => {
  const traces = new Array<typeof Trace>();
  const binaryTraces = new Array<BinaryTrace>();
  const functionMaps = new Array<typeof InstrumentedFunctionMap>();

  await Promise.all(
    args.files.map(async (file: string) => {
      const fileExtension = file.split('.').pop();
      switch (fileExtension) {
        case SpoorFileExtension.Trace: {
          const buffer = await fs.promises.readFile(file);
          const trace = Trace.deserializeBinary(buffer);
          traces.push(trace);
          break;
        }
        case SpoorFileExtension.BinaryTrace: {
          const buffer = await fs.promises.readFile(file);
          const binaryTrace = BinaryTrace.parseFromBuffer(buffer);
          binaryTraces.push(binaryTrace);
          break;
        }
        case SpoorFileExtension.InstrumentedFunctionMap: {
          const buffer = await fs.promises.readFile(file);
          const functionMap = InstrumentedFunctionMap.deserializeBinary(buffer);
          functionMaps.push(functionMap);
          break;
        }
        default: {
          throw new Error(`Cannot parse the file '${file}'`);
        }
      }
    })
  );

  const trace = mergeTraces(traces, binaryTraces, functionMaps);

  const format = args.format;
  switch (format) {
    case OutputFormat.Proto: {
      process.stdout.write(trace.serializeBinary());
      break;
    }
    case OutputFormat.Json: {
      process.stdout.write(JSON.stringify(trace.toObject()));
      break;
    }
    case OutputFormat.Chrome: {
      const chromeTrace = ChromeTrace.fromSpoorTrace(trace);
      process.stdout.write(JSON.stringify(chromeTrace));
      break;
    }
    case OutputFormat.None: {
      break;
    }
    default: {
      throw new Error(`Unknown output format '${format}'`);
    }
  }
};
