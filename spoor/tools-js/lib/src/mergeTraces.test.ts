// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

import {
  BinaryEventType,
  Event,
  FunctionInfo,
  InstrumentedFunctionMap,
  Timestamp,
  Trace,
  mergeTraces,
} from '.';

function makeFunctionInfo(
  line: number,
  linkageName = 'linkage name',
  demangledName = 'demangled name',
  fileName = 'file name',
  directory = 'directory'
): typeof FunctionInfo {
  const functionInfo = new FunctionInfo();
  functionInfo.setLinkageName(linkageName);
  functionInfo.setDemangledName(demangledName);
  functionInfo.setFileName(fileName);
  functionInfo.setDirectory(directory);
  functionInfo.setLine(line);
  functionInfo.setInstrumented(true);
  return functionInfo;
}

test('merges empty input', () => {
  const date = new Date(0);
  const timestamp = new Timestamp();
  timestamp.fromDate(date);
  const mergedTrace = mergeTraces([], [], [], date);
  const expectedTrace = new Trace();
  expectedTrace.setEventsList([]);
  expectedTrace.setModulesList([]);
  expectedTrace.setCreatedAt(timestamp);
  expect(mergedTrace).toStrictEqual(expectedTrace);
});

test('merges spoor traces', () => {
  const module = 'module';
  const version = 0;
  const sessionId = '0x0000000000000001';
  const processId = '0x0000000000000002';
  const threadId = '0x0000000000000003';
  const functionAId = '0x000000000000000a';
  const functionBId = '0x000000000000000b';
  const eventATimestampNanosecond = 0;
  const eventBTimestampNanosecond = 1;
  const eventCTimestampNanosecond = 2;
  const eventDTimestampNanosecond = 3;
  const eventA = (() => {
    const event = new Event();
    event.setSessionId(sessionId);
    event.setProcessId(processId);
    event.setThreadId(threadId);
    event.setFunctionId(functionAId);
    event.setType(Event.Type.FUNCTION_ENTRY);
    const timestamp = new Timestamp();
    timestamp.setSeconds(0);
    timestamp.setNanos(eventATimestampNanosecond);
    event.setTimestamp(timestamp);
    return event;
  })();
  const eventB = (() => {
    const event = new Event();
    event.setSessionId(sessionId);
    event.setProcessId(processId);
    event.setThreadId(threadId);
    event.setFunctionId(functionAId);
    event.setType(Event.Type.FUNCTION_EXIT);
    const timestamp = new Timestamp();
    timestamp.setSeconds(0);
    timestamp.setNanos(eventBTimestampNanosecond);
    event.setTimestamp(timestamp);
    return event;
  })();
  const expectedEventC = (() => {
    const event = new Event();
    event.setSessionId(sessionId);
    event.setProcessId(processId);
    event.setThreadId(threadId);
    event.setFunctionId(functionBId);
    event.setType(Event.Type.FUNCTION_ENTRY);
    const timestamp = new Timestamp();
    timestamp.setSeconds(0);
    timestamp.setNanos(eventCTimestampNanosecond);
    event.setTimestamp(timestamp);
    return event;
  })();
  const expectedEventD = (() => {
    const event = new Event();
    event.setSessionId(sessionId);
    event.setProcessId(processId);
    event.setThreadId(threadId);
    event.setFunctionId(functionBId);
    event.setType(Event.Type.FUNCTION_EXIT);
    const timestamp = new Timestamp();
    timestamp.setSeconds(0);
    timestamp.setNanos(eventDTimestampNanosecond);
    event.setTimestamp(timestamp);
    return event;
  })();
  const functionAInfo = makeFunctionInfo(100);
  const functionBInfo = makeFunctionInfo(200);
  const traceA = (() => {
    const trace = new Trace();
    trace.setEventsList([eventA, eventB]);
    trace.setModulesList([module]);
    trace.getFunctionMapMap().set(functionAId, functionAInfo);
    const createdAt = new Timestamp();
    createdAt.fromDate(new Date(10));
    trace.setCreatedAt(createdAt);
    return trace;
  })();
  const traceB = {
    header: {
      version: BigInt(version),
      sessionId: BigInt(sessionId),
      processId: BigInt(processId),
      threadId: BigInt(threadId),
      systemClockTimestampNanoseconds: BigInt(0),
      steadyClockTimestampNanoseconds: BigInt(0),
      eventCount: 2,
    },
    events: [
      {
        eventType: BinaryEventType.FunctionEntry,
        functionId: BigInt(functionBId),
        steadyClockTimestampNanoseconds: BigInt(eventCTimestampNanosecond),
      },
      {
        eventType: BinaryEventType.FunctionExit,
        functionId: BigInt(functionBId),
        steadyClockTimestampNanoseconds: BigInt(eventDTimestampNanosecond),
      },
    ],
    footer: {},
  };
  const functionMapB = (() => {
    const functionMap = new InstrumentedFunctionMap();
    functionMap.setModuleId(module);
    functionMap.getFunctionMapMap().set(functionBId, functionBInfo);
    const createdAt = new Timestamp();
    createdAt.fromDate(new Date(20));
    functionMap.setCreatedAt(createdAt);
    return functionMap;
  })();

  const createdAtDate = new Date(30);
  const expectedTrace = (() => {
    const trace = new Trace();
    trace.setEventsList([eventA, eventB, expectedEventC, expectedEventD]);
    trace.setModulesList([module]);
    trace.getFunctionMapMap().set(functionAId, functionAInfo);
    trace.getFunctionMapMap().set(functionBId, functionBInfo);
    const createdAt = new Timestamp();
    createdAt.fromDate(createdAtDate);
    trace.setCreatedAt(createdAt);
    return trace;
  })();
  const merged = mergeTraces([traceA], [traceB], [functionMapB], createdAtDate);
  expect(merged).toStrictEqual(expectedTrace);
});

test('computes binary system timestamp', () => {
  const version = '0x0000000000000000';
  const sessionId1 = '0x0000000000000001';
  const sessionId2 = '0x0000000000000002';
  const processId1 = '0x0000000000000001';
  const processId2 = '0x0000000000000002';
  const threadId1 = '0x0000000000000001';
  const threadId2 = '0x0000000000000002';
  const threadId3 = '0x0000000000000003';
  const threadId4 = '0x0000000000000004';
  const functionId = '0x000000000000000a';

  const traces = [
    {
      header: {
        version: BigInt(version),
        sessionId: BigInt(sessionId1),
        processId: BigInt(processId1),
        threadId: BigInt(threadId1),
        systemClockTimestampNanoseconds: BigInt(1000000),
        steadyClockTimestampNanoseconds: BigInt(1000),
        eventCount: 2,
      },
      events: [
        {
          eventType: BinaryEventType.FunctionEntry,
          functionId: BigInt(functionId),
          steadyClockTimestampNanoseconds: BigInt(1001),
        },
        {
          eventType: BinaryEventType.FunctionExit,
          functionId: BigInt(functionId),
          steadyClockTimestampNanoseconds: BigInt(1002),
        },
      ],
      footer: {},
    },
    {
      header: {
        version: BigInt(version),
        sessionId: BigInt(sessionId1),
        processId: BigInt(processId1),
        threadId: BigInt(threadId2),
        systemClockTimestampNanoseconds: BigInt(2222222),
        steadyClockTimestampNanoseconds: BigInt(2222),
        eventCount: 2,
      },
      events: [
        {
          eventType: BinaryEventType.FunctionEntry,
          functionId: BigInt(functionId),
          steadyClockTimestampNanoseconds: BigInt(2100),
        },
        {
          eventType: BinaryEventType.FunctionExit,
          functionId: BigInt(functionId),
          steadyClockTimestampNanoseconds: BigInt(2200),
        },
      ],
      footer: {},
    },
    {
      header: {
        version: BigInt(version),
        sessionId: BigInt(sessionId1),
        processId: BigInt(processId2),
        threadId: BigInt(threadId3),
        systemClockTimestampNanoseconds: BigInt(3000000),
        steadyClockTimestampNanoseconds: BigInt(3000),
        eventCount: 2,
      },
      events: [
        {
          eventType: BinaryEventType.FunctionEntry,
          functionId: BigInt(functionId),
          steadyClockTimestampNanoseconds: BigInt(3001),
        },
        {
          eventType: BinaryEventType.FunctionExit,
          functionId: BigInt(functionId),
          steadyClockTimestampNanoseconds: BigInt(3002),
        },
      ],
      footer: {},
    },
    {
      header: {
        version: BigInt(version),
        sessionId: BigInt(sessionId2),
        processId: BigInt(processId1),
        threadId: BigInt(threadId4),
        systemClockTimestampNanoseconds: BigInt(4000000),
        steadyClockTimestampNanoseconds: BigInt(4000),
        eventCount: 2,
      },
      events: [
        {
          eventType: BinaryEventType.FunctionEntry,
          functionId: BigInt(functionId),
          steadyClockTimestampNanoseconds: BigInt(4001),
        },
        {
          eventType: BinaryEventType.FunctionExit,
          functionId: BigInt(functionId),
          steadyClockTimestampNanoseconds: BigInt(4002),
        },
      ],
      footer: {},
    },
  ];

  const createdAtDate = new Date(30);
  const expectedTrace = (() => {
    const events = [
      (() => {
        const event = new Event();
        event.setSessionId(sessionId1);
        event.setProcessId(processId1);
        event.setThreadId(threadId1);
        event.setFunctionId(functionId);
        event.setType(Event.Type.FUNCTION_ENTRY);
        const timestamp = new Timestamp();
        timestamp.setSeconds(0);
        timestamp.setNanos(1000001);
        event.setTimestamp(timestamp);
        return event;
      })(),
      (() => {
        const event = new Event();
        event.setSessionId(sessionId1);
        event.setProcessId(processId1);
        event.setThreadId(threadId1);
        event.setFunctionId(functionId);
        event.setType(Event.Type.FUNCTION_EXIT);
        const timestamp = new Timestamp();
        timestamp.setSeconds(0);
        timestamp.setNanos(1000002);
        event.setTimestamp(timestamp);
        return event;
      })(),
      (() => {
        const event = new Event();
        event.setSessionId(sessionId1);
        event.setProcessId(processId1);
        event.setThreadId(threadId2);
        event.setFunctionId(functionId);
        event.setType(Event.Type.FUNCTION_ENTRY);
        const timestamp = new Timestamp();
        timestamp.setSeconds(0);
        timestamp.setNanos(1001100);
        event.setTimestamp(timestamp);
        return event;
      })(),
      (() => {
        const event = new Event();
        event.setSessionId(sessionId1);
        event.setProcessId(processId1);
        event.setThreadId(threadId2);
        event.setFunctionId(functionId);
        event.setType(Event.Type.FUNCTION_EXIT);
        const timestamp = new Timestamp();
        timestamp.setSeconds(0);
        timestamp.setNanos(1001200);
        event.setTimestamp(timestamp);
        return event;
      })(),
      (() => {
        const event = new Event();
        event.setSessionId(sessionId1);
        event.setProcessId(processId2);
        event.setThreadId(threadId3);
        event.setFunctionId(functionId);
        event.setType(Event.Type.FUNCTION_ENTRY);
        const timestamp = new Timestamp();
        timestamp.setSeconds(0);
        timestamp.setNanos(3000001);
        event.setTimestamp(timestamp);
        return event;
      })(),
      (() => {
        const event = new Event();
        event.setSessionId(sessionId1);
        event.setProcessId(processId2);
        event.setThreadId(threadId3);
        event.setFunctionId(functionId);
        event.setType(Event.Type.FUNCTION_EXIT);
        const timestamp = new Timestamp();
        timestamp.setSeconds(0);
        timestamp.setNanos(3000002);
        event.setTimestamp(timestamp);
        return event;
      })(),
      (() => {
        const event = new Event();
        event.setSessionId(sessionId2);
        event.setProcessId(processId1);
        event.setThreadId(threadId4);
        event.setFunctionId(functionId);
        event.setType(Event.Type.FUNCTION_ENTRY);
        const timestamp = new Timestamp();
        timestamp.setSeconds(0);
        timestamp.setNanos(4000001);
        event.setTimestamp(timestamp);
        return event;
      })(),
      (() => {
        const event = new Event();
        event.setSessionId(sessionId2);
        event.setProcessId(processId1);
        event.setThreadId(threadId4);
        event.setFunctionId(functionId);
        event.setType(Event.Type.FUNCTION_EXIT);
        const timestamp = new Timestamp();
        timestamp.setSeconds(0);
        timestamp.setNanos(4000002);
        event.setTimestamp(timestamp);
        return event;
      })(),
    ];

    const trace = new Trace();
    trace.setEventsList(events);
    trace.setModulesList([]);
    const createdAt = new Timestamp();
    createdAt.fromDate(createdAtDate);
    trace.setCreatedAt(createdAt);
    return trace;
  })();
  const merged = mergeTraces([], traces, [], createdAtDate);
  expect(merged.getEventsList()).toStrictEqual(expectedTrace.getEventsList());
});

test('sorts events by timestamp', () => {
  const version = '0x0000000000000000';
  const sessionId = '0x0000000000000001';
  const processId = '0x0000000000000002';
  const threadId = '0x0000000000000003';
  const functionId = '0x0000000000000004';
  const traces = [
    {
      header: {
        version: BigInt(version),
        sessionId: BigInt(sessionId),
        processId: BigInt(processId),
        threadId: BigInt(threadId),
        systemClockTimestampNanoseconds: BigInt(2),
        steadyClockTimestampNanoseconds: BigInt(2),
        eventCount: 1,
      },
      events: [
        {
          eventType: BinaryEventType.FunctionEntry,
          functionId: BigInt(functionId),
          steadyClockTimestampNanoseconds: BigInt(1000000000),
        },
      ],
      footer: {},
    },
    {
      header: {
        version: BigInt(version),
        sessionId: BigInt(sessionId),
        processId: BigInt(processId),
        threadId: BigInt(threadId),
        systemClockTimestampNanoseconds: BigInt(1),
        steadyClockTimestampNanoseconds: BigInt(1),
        eventCount: 1,
      },
      events: [
        {
          eventType: BinaryEventType.FunctionEntry,
          functionId: BigInt(functionId),
          steadyClockTimestampNanoseconds: BigInt(1),
        },
      ],
      footer: {},
    },
    {
      header: {
        version: BigInt(version),
        sessionId: BigInt(sessionId),
        processId: BigInt(processId),
        threadId: BigInt(threadId),
        systemClockTimestampNanoseconds: BigInt(0),
        steadyClockTimestampNanoseconds: BigInt(0),
        eventCount: 1,
      },
      events: [
        {
          eventType: BinaryEventType.FunctionEntry,
          functionId: BigInt(functionId),
          steadyClockTimestampNanoseconds: BigInt(0),
        },
      ],
      footer: {},
    },
  ];
  const expectedEvents = [
    (() => {
      const event = new Event();
      event.setSessionId(sessionId);
      event.setProcessId(processId);
      event.setThreadId(threadId);
      event.setFunctionId(functionId);
      event.setType(Event.Type.FUNCTION_ENTRY);
      const timestamp = new Timestamp();
      timestamp.setSeconds(0);
      timestamp.setNanos(0);
      event.setTimestamp(timestamp);
      return event;
    })(),
    (() => {
      const event = new Event();
      event.setSessionId(sessionId);
      event.setProcessId(processId);
      event.setThreadId(threadId);
      event.setFunctionId(functionId);
      event.setType(Event.Type.FUNCTION_ENTRY);
      const timestamp = new Timestamp();
      timestamp.setSeconds(0);
      timestamp.setNanos(1);
      event.setTimestamp(timestamp);
      return event;
    })(),
    (() => {
      const event = new Event();
      event.setSessionId(sessionId);
      event.setProcessId(processId);
      event.setThreadId(threadId);
      event.setFunctionId(functionId);
      event.setType(Event.Type.FUNCTION_ENTRY);
      const timestamp = new Timestamp();
      timestamp.setSeconds(1);
      timestamp.setNanos(0);
      event.setTimestamp(timestamp);
      return event;
    })(),
  ];
  const merged = mergeTraces([], traces, []);
  expect(merged.getEventsList()).toStrictEqual(expectedEvents);
});

test('sorts and deduplicates modules', () => {
  const modules = [...Array(100).keys()].map(module =>
    String(module).padStart(2, '0')
  );
  const functionMaps = modules
    .slice()
    .reverse()
    .map(module => {
      const functionMap = new InstrumentedFunctionMap();
      functionMap.setModuleId(module);
      return functionMap;
    });
  const merged = mergeTraces([], [], functionMaps.concat(functionMaps));
  expect(merged.getModulesList()).toStrictEqual(modules);
});

test('throws error with conflicting function id info', () => {
  const module = 'module';
  const functionId = 'function id';
  const functionInfoA = makeFunctionInfo(1);
  const functionInfoB = makeFunctionInfo(1);
  const functionMapA = (() => {
    const functionMap = new InstrumentedFunctionMap();
    functionMap.setModuleId(module);
    functionMap.getFunctionMapMap().set(functionId, functionInfoA);
    const timestamp = new Timestamp();
    timestamp.fromDate(new Date(0));
    functionMap.setCreatedAt(timestamp);
    return functionMap;
  })();
  const functionMapB = (() => {
    const functionMap = new InstrumentedFunctionMap();
    functionMap.setModuleId(module);
    functionMap.getFunctionMapMap().set(functionId, functionInfoB);
    const timestamp = new Timestamp();
    timestamp.fromDate(new Date(0));
    functionMap.setCreatedAt(timestamp);
    return functionMap;
  })();

  const merged = mergeTraces([], [], [functionMapA, functionMapA]);
  expect(merged.getFunctionMapMap()).toStrictEqual(
    functionMapA.getFunctionMapMap()
  );
  expect(() => mergeTraces([], [], [functionMapA, functionMapB])).toThrow(
    `Conflicting entries found for function ID '${functionId}.'`
  );
});
