// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

import {add} from '@microsoft/spoor-lib';
import {parseArgs} from './command';

const args = parseArgs(process.argv.slice(2));
console.log(`${args.x} + ${args.y} = ${add(args.x, args.y)}`);
