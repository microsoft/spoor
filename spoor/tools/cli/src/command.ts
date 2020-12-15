// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

import yargs from 'yargs/yargs';

export function parseArgs(argv: string[]) {
  return yargs(argv).options({
    x: {type: 'number', required: true},
    y: {type: 'number', required: true},
  }).argv;
}
