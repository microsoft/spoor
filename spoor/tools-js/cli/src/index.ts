// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

import yargs from 'yargs/yargs';

function main(argv: string[]) {
  yargs(argv.slice(2)).command(require('./commands/cat')).demandCommand().help()
    .argv;
}

main(process.argv);
