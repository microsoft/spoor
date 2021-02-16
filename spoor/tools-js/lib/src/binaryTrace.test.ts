// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

import fs from 'fs';
import {
  BinaryEvent,
  BinaryEventType,
  BinaryFooter,
  BinaryHeader,
  BinaryTrace,
} from '.';

test('converts binary file to trace', () => {
  const expectedTrace = {
    header: {
      version: BigInt('0x1111111111111111'),
      sessionId: BigInt('0x2222222222222222'),
      processId: BigInt('0x3333333333333333'),
      threadId: BigInt('0x4444444444444444'),
      systemClockTimestampNanoseconds: BigInt('0x5555555555555555'),
      steadyClockTimestampNanoseconds: BigInt('0x6666666666666666'),
      eventCount: 8,
    },
    events: [
      {
        eventType: BinaryEventType.FunctionEntry,
        functionId: BigInt('0xaaaaaaaaaaaaaaaa'),
        steadyClockTimestampNanoseconds: BigInt('0x7ffffffffffffff0'),
      },
      {
        eventType: BinaryEventType.FunctionEntry,
        functionId: BigInt('0xbbbbbbbbbbbbbbbb'),
        steadyClockTimestampNanoseconds: BigInt('0x7ffffffffffffff1'),
      },
      {
        eventType: BinaryEventType.FunctionExit,
        functionId: BigInt('0xbbbbbbbbbbbbbbbb'),
        steadyClockTimestampNanoseconds: BigInt('0x7ffffffffffffff2'),
      },
      {
        eventType: BinaryEventType.FunctionEntry,
        functionId: BigInt('0xcccccccccccccccc'),
        steadyClockTimestampNanoseconds: BigInt('0x7ffffffffffffff3'),
      },
      {
        eventType: BinaryEventType.FunctionEntry,
        functionId: BigInt('0xcccccccccccccccc'),
        steadyClockTimestampNanoseconds: BigInt('0x7ffffffffffffff4'),
      },
      {
        eventType: BinaryEventType.FunctionExit,
        functionId: BigInt('0xcccccccccccccccc'),
        steadyClockTimestampNanoseconds: BigInt('0x7ffffffffffffff5'),
      },
      {
        eventType: BinaryEventType.FunctionExit,
        functionId: BigInt('0xcccccccccccccccc'),
        steadyClockTimestampNanoseconds: BigInt('0x7ffffffffffffff6'),
      },
      {
        eventType: BinaryEventType.FunctionExit,
        functionId: BigInt('0xaaaaaaaaaaaaaaaa'),
        steadyClockTimestampNanoseconds: BigInt('0x7ffffffffffffff7'),
      },
    ],
    footer: new BinaryFooter(),
  };
  const buffer = fs.readFileSync('./test_data/trace.spoor_trace');
  const trace = BinaryTrace.parseFromBuffer(buffer);
  expect(trace).toStrictEqual(expectedTrace);
});

test('throws if file is too small', () => {
  const buffer = fs.readFileSync(
    './test_data/smaller_than_header_and_footer.spoor_trace'
  );
  const expectedMessage =
    'These binary data are not large enough to hold a trace. Expected size = ' +
    'at least 57 bytes, actual size = 55 bytes.';
  expect(() => BinaryTrace.parseFromBuffer(buffer)).toThrow(expectedMessage);
});

test('throws if file cannot exactly hold the reported trace count', () => {
  const filesAndSize = [
    ['./test_data/unexpected_size_too_small.spoor_trace', 184],
    ['./test_data/unexpected_size_too_big.spoor_trace', 186],
  ];
  for (const [file, size] of filesAndSize) {
    const buffer = fs.readFileSync(file);
    const expectedMessage =
      'These binary data do not represent a trace. Expected size = 185 ' +
      `bytes, actual size = ${size} bytes.`;
    expect(() => BinaryTrace.parseFromBuffer(buffer)).toThrow(expectedMessage);
  }
});

test('bonus usage to maximize coverage', () => {
  const header = new BinaryHeader(
    BigInt(0),
    BigInt(0),
    BigInt(0),
    BigInt(0),
    BigInt(0),
    BigInt(0),
    0
  );
  const event = new BinaryEvent(
    BinaryEventType.FunctionEntry,
    BigInt(0),
    BigInt(0)
  );
  const footer = new BinaryFooter();
  new BinaryTrace(header, [event], footer);
});
