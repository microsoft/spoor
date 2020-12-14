// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

import {parseArgs} from './command';

test('parses args', () => {
  const x = 1;
  const y = 2;
  const args = parseArgs(['--x', x.toString(), '--y', y.toString()]);
  expect(args.x).toBe(x);
  expect(args.y).toBe(y);
});
