// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

import {
  Event,
  FunctionInfo,
  Timestamp,
  Trace,
  InstrumentedFunctionMap,
} from '@microsoft/spoor-proto';
import {Trace as BinaryTrace} from './binaryTrace';

function to64BitHex(bigInt: BigInt): string {
  return `0x${bigInt.toString(16).padStart(16, '0')}`;
}

function nanosecondsToTimestamp(nanoseconds: BigInt): typeof Timestamp {
  const seconds = BigInt(nanoseconds) / 1000000000n;
  const remainderNanoseconds = BigInt(nanoseconds) % 1000000000n;
  const timestamp = new Timestamp();
  timestamp.setSeconds(Number(seconds));
  timestamp.setNanos(Number(remainderNanoseconds));
  return timestamp;
}

export function mergeTraces(
  traces: typeof Trace[],
  binaryTraces: BinaryTrace[],
  functionMaps: typeof InstrumentedFunctionMap[],
  createdAt: Date = new Date()
): typeof Trace {
  const combinedTrace = new Trace();

  const combinedEvents = traces
    .flatMap(trace => trace.getEventsList())
    .filter(event => event !== undefined);
  // Maps (session id, process id) to the earliest
  // (system clock timestamp, steady clock timestamp).
  const traceGroupToTimestamp = new Map<string, [BigInt, BigInt]>();
  for (const trace of binaryTraces) {
    // Using the joined string representation allows for map key uniqueness.
    const sessionId = trace.header.sessionId.toString();
    const processId = trace.header.processId.toString();
    const key = sessionId + processId;
    const systemClockTimestamp = trace.header.systemClockTimestampNanoseconds;
    const steadyClockTimestamp = trace.header.steadyClockTimestampNanoseconds;
    let value = traceGroupToTimestamp.get(key);
    if (value === undefined) {
      traceGroupToTimestamp.set(key, [
        systemClockTimestamp,
        steadyClockTimestamp,
      ]);
    } else {
      const [existingSystemClockTimestamp] = value;
      if (systemClockTimestamp < existingSystemClockTimestamp) {
        value = [systemClockTimestamp, steadyClockTimestamp];
      }
    }
  }
  for (const binaryTrace of binaryTraces) {
    const sessionId = binaryTrace.header.sessionId.toString();
    const processId = binaryTrace.header.processId.toString();
    const key = sessionId + processId;
    const [
      systemClockTimestamp,
      steadyClockTimestamp,
    ] = traceGroupToTimestamp.get(key) as [BigInt, BigInt];
    const steadyClockOffset =
      BigInt(systemClockTimestamp) - BigInt(steadyClockTimestamp);
    for (const binaryEvent of binaryTrace.events) {
      const event = new Event();
      event.setSessionId(to64BitHex(binaryTrace.header.sessionId));
      event.setProcessId(to64BitHex(binaryTrace.header.processId));
      event.setThreadId(to64BitHex(binaryTrace.header.threadId));
      event.setFunctionId(to64BitHex(binaryEvent.functionId));
      event.setType(binaryEvent.eventType);
      const timestampNanoseconds =
        BigInt(binaryEvent.steadyClockTimestampNanoseconds) + steadyClockOffset;
      event.setTimestamp(nanosecondsToTimestamp(timestampNanoseconds));
      combinedEvents.push(event);
    }
  }
  combinedEvents.sort((a, b) => {
    const timestampA = a.getTimestamp();
    const timestampB = b.getTimestamp();
    const seconds = timestampA.getSeconds() - timestampB.getSeconds();
    if (seconds === 0) return timestampA.getNanos() - timestampB.getNanos();
    return seconds;
  });
  combinedTrace.setEventsList(combinedEvents);

  const addFunctionsToCombinedTraceFunctionMap = (
    functionMap: Map<string, typeof FunctionInfo>
  ) => {
    const functions = functionMap.entries();
    for (const [functionId, functionInfo] of functions) {
      const existingValue = combinedTrace.getFunctionMapMap().get(functionId);
      if (existingValue !== undefined && existingValue !== functionInfo) {
        throw new Error(
          `Conflicting entries found for function ID '${functionId}.'`
        );
      }
      combinedTrace.getFunctionMapMap().set(functionId, functionInfo);
    }
  };
  traces.forEach(trace =>
    addFunctionsToCombinedTraceFunctionMap(trace.getFunctionMapMap())
  );
  functionMaps.forEach(functionMap =>
    addFunctionsToCombinedTraceFunctionMap(functionMap.getFunctionMapMap())
  );

  const modules = (() => {
    const modules = traces
      .flatMap(trace => trace.getModulesList())
      .concat(functionMaps.map(map => map.getModuleId()));
    const uniqueModules = [...new Set(modules)];
    return uniqueModules.sort();
  })();
  combinedTrace.setModulesList(modules);

  const timestamp = new Timestamp();
  timestamp.fromDate(createdAt);
  combinedTrace.setCreatedAt(timestamp);

  return combinedTrace;
}
