// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Note: The format of these trace objects should match the structs in
// spoor/runtime/trace/trace.h.

// These "implementation details" are exported for use by downstream tests.
export const HEADER_SIZE_BYTES = 56;
export const EVENT_SIZE_BYTES = 16;
export const FOOTER_SIZE_BYTES = 1;

export class Header {
  constructor(
    readonly version: BigInt,
    readonly sessionId: BigInt,
    readonly processId: BigInt,
    readonly threadId: BigInt,
    readonly systemClockTimestampNanoseconds: BigInt,
    readonly steadyClockTimestampNanoseconds: BigInt,
    readonly eventCount: number
  ) {}
}

export enum EventType {
  FunctionExit = 0,
  FunctionEntry = 1,
}

export class Event {
  constructor(
    readonly eventType: EventType,
    readonly functionId: BigInt,
    readonly steadyClockTimestampNanoseconds: BigInt
  ) {}
}

// eslint-disable-next-line @typescript-eslint/no-empty-interface
export class Footer {}

export class Trace {
  constructor(
    readonly header: Header,
    readonly events: Event[],
    readonly footer: Footer
  ) {}

  static parseFromBuffer(buffer: Buffer) {
    const minimumSizeBytes = HEADER_SIZE_BYTES + FOOTER_SIZE_BYTES;
    if (buffer.byteLength < minimumSizeBytes) {
      throw new Error(
        'These binary data are not large enough to hold a trace. Expected ' +
          `size = at least ${minimumSizeBytes} bytes, actual size = ` +
          `${buffer.byteLength} bytes.`
      );
    }
    const headerBytes = buffer.slice(0, HEADER_SIZE_BYTES);
    const header = {
      version: headerBytes.readBigUInt64BE(0),
      sessionId: headerBytes.readBigUInt64BE(8),
      processId: headerBytes.readBigInt64BE(16),
      threadId: headerBytes.readBigUInt64BE(24),
      systemClockTimestampNanoseconds: headerBytes.readBigUInt64BE(32),
      steadyClockTimestampNanoseconds: headerBytes.readBigUInt64BE(40),
      eventCount: headerBytes.readInt32BE(48),
    };
    const expectedSizeBytes =
      HEADER_SIZE_BYTES +
      header.eventCount * EVENT_SIZE_BYTES +
      FOOTER_SIZE_BYTES;
    if (buffer.byteLength !== expectedSizeBytes) {
      throw new Error(
        'These binary data do not represent a trace. Expected size = ' +
          `${expectedSizeBytes} bytes, actual size = ${buffer.byteLength} ` +
          'bytes.'
      );
    }
    const events: Event[] = [];
    for (
      let eventOffset = HEADER_SIZE_BYTES;
      eventOffset < HEADER_SIZE_BYTES + header.eventCount * EVENT_SIZE_BYTES;
      eventOffset += EVENT_SIZE_BYTES
    ) {
      const eventBytes = buffer.slice(
        eventOffset,
        eventOffset + EVENT_SIZE_BYTES
      );
      const event: Event = {
        eventType: Number(eventBytes.readBigUInt64BE(8) >> 63n),
        functionId: eventBytes.readBigUInt64BE(0),
        steadyClockTimestampNanoseconds:
          eventBytes.readBigUInt64BE(8) & 0x7fffffffffffffffn,
      };
      events.push(event);
    }
    const footer = new Footer();
    return {
      header: header,
      events: events,
      footer: footer,
    };
  }
}
