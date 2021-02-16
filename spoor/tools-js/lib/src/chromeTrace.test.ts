// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

import {
  ChromeEvent,
  ChromeEventType,
  ChromeTrace,
  ChromeTraceSpoorEventArgs,
  ChromeTraceSpoorTraceOtherData,
  Event,
  FunctionInfo,
  Timestamp,
  Trace,
} from '.';

test('creates chrome event from spoor event and function info', () => {
  const sessionId = '1';
  const processId = '2';
  const threadId = '3';
  const functionId = '4';
  const seconds = 100;
  const nanos = 200;
  const timestamp = new Timestamp();
  timestamp.setSeconds(seconds);
  timestamp.setNanos(nanos);
  const linkageName = 'linkage name';
  const demangledName = 'demangled name';
  const fileName = 'file name';
  const directory = 'directory';
  const line = 42;
  const instrumented = true;

  const eventTypes = [
    [Event.Type.FUNCTION_ENTRY, 'B'],
    [Event.Type.FUNCTION_EXIT, 'E'],
  ];
  eventTypes.forEach(inAndExpectedEventType => {
    const [eventType, expectedEventType] = inAndExpectedEventType;
    const spoorEvent = new Event();
    spoorEvent.setSessionId(sessionId);
    spoorEvent.setProcessId(processId);
    spoorEvent.setThreadId(threadId);
    spoorEvent.setFunctionId(functionId);
    spoorEvent.setType(eventType);
    spoorEvent.setTimestamp(timestamp);

    const functionInfo = new FunctionInfo();
    functionInfo.setLinkageName(linkageName);
    functionInfo.setDemangledName(demangledName);
    functionInfo.setFileName(fileName);
    functionInfo.setDirectory(directory);
    functionInfo.setLine(line);
    functionInfo.setInstrumented(instrumented);

    const chromeEvent = ChromeEvent.fromEventAndFunctionInfo(
      spoorEvent,
      functionInfo
    );
    const expectedChromeEvent = {
      name: demangledName,
      ph: expectedEventType,
      ts: 1000000.0 * seconds + nanos / 1000.0,
      pid: processId,
      tid: threadId,
      args: {
        functionId: functionId,
        fileName: fileName,
        directory: directory,
        line: line,
        linkageName: linkageName,
      },
    };
    expect(chromeEvent).toStrictEqual(expectedChromeEvent);
  });
});

test('creates chrome event from spoor event only', () => {
  const sessionId = '1';
  const processId = '2';
  const threadId = '3';
  const functionId = '4';
  const seconds = 100;
  const nanos = 200;
  const timestamp = new Timestamp();
  timestamp.setSeconds(seconds);
  timestamp.setNanos(nanos);

  const spoorEvent = new Event();
  spoorEvent.setSessionId(sessionId);
  spoorEvent.setProcessId(processId);
  spoorEvent.setThreadId(threadId);
  spoorEvent.setFunctionId(functionId);
  spoorEvent.setType(Event.Type.FUNCTION_ENTRY);
  spoorEvent.setTimestamp(timestamp);

  const chromeEvent = ChromeEvent.fromEventAndFunctionInfo(spoorEvent);
  const expectedChromeEvent = {
    name: functionId,
    ph: 'B',
    ts: 1000000.0 * seconds + nanos / 1000.0,
    pid: processId,
    tid: threadId,
    args: {
      directory: undefined,
      fileName: undefined,
      functionId: functionId,
      line: undefined,
      linkageName: undefined,
    },
  };
  expect(chromeEvent).toStrictEqual(expectedChromeEvent);
});

test('creates chrome trace from spoor event only', () => {
  const module = 'module';
  const linkageName = 'linkage name';
  const demangledName = 'demangled name';
  const fileName = 'file name';
  const directory = 'directory';
  const line = 42;
  const instrumented = true;
  const createdAt = new Timestamp();
  createdAt.setSeconds(3);
  createdAt.setNanos(0);
  const sessionId = '1';
  const processId = '2';
  const threadId = '3';
  const functionId = '4';
  const timestampA = new Timestamp();
  timestampA.setSeconds(1);
  timestampA.setNanos(0);
  const timestampB = new Timestamp();
  timestampB.setSeconds(2);
  timestampB.setNanos(0);

  const functionInfo = new FunctionInfo();
  functionInfo.setLinkageName(linkageName);
  functionInfo.setDemangledName(demangledName);
  functionInfo.setFileName(fileName);
  functionInfo.setDirectory(directory);
  functionInfo.setLine(line);
  functionInfo.setInstrumented(instrumented);

  const spoorEventA = new Event();
  spoorEventA.setSessionId(sessionId);
  spoorEventA.setProcessId(processId);
  spoorEventA.setThreadId(threadId);
  spoorEventA.setFunctionId(functionId);
  spoorEventA.setType(Event.Type.FUNCTION_ENTRY);
  spoorEventA.setTimestamp(timestampA);

  const spoorEventB = new Event();
  spoorEventB.setSessionId(sessionId);
  spoorEventB.setProcessId(processId);
  spoorEventB.setThreadId(threadId);
  spoorEventB.setFunctionId(functionId);
  spoorEventB.setType(Event.Type.FUNCTION_EXIT);
  spoorEventB.setTimestamp(timestampB);

  const spoorTrace = new Trace();
  spoorTrace.setEventsList([spoorEventA, spoorEventB]);
  spoorTrace.setModulesList([module]);
  spoorTrace.getFunctionMapMap().set(functionId, functionInfo);
  spoorTrace.setCreatedAt(createdAt);

  const chromeTrace = ChromeTrace.fromSpoorTrace(spoorTrace);
  const expectedArgs = {
    directory: directory,
    fileName: fileName,
    functionId: functionId,
    line: line,
    linkageName: linkageName,
  };
  const expectedCreatedAt = new Date(0);
  expectedCreatedAt.setUTCSeconds(
    createdAt.getSeconds() + createdAt.getNanos() / 1000000000.0
  );
  const expectedChromeTrace = {
    traceEvents: [
      {
        name: demangledName,
        ph: 'B',
        ts:
          1000000.0 * timestampA.getSeconds() + timestampA.getNanos() / 1000.0,
        pid: processId,
        tid: threadId,
        args: expectedArgs,
      },
      {
        name: demangledName,
        ph: 'E',
        ts:
          1000000.0 * timestampB.getSeconds() + timestampB.getNanos() / 1000.0,
        pid: processId,
        tid: threadId,
        args: expectedArgs,
      },
    ],
    displayTimeUnit: 'ns',
    otherData: {
      createdAt: expectedCreatedAt.toString(),
      modules: [module],
    },
  };
  expect(chromeTrace).toStrictEqual(expectedChromeTrace);
});

test('throws with unknown event type', () => {
  const badEventType = 'bad event type';
  const spoorEvent = new Event();
  spoorEvent.setSessionId('session id');
  spoorEvent.setProcessId('process id');
  spoorEvent.setThreadId('thread id');
  spoorEvent.setFunctionId('function id');
  spoorEvent.setType(badEventType);
  spoorEvent.setTimestamp(new Timestamp());
  expect(() => ChromeEvent.fromEventAndFunctionInfo(spoorEvent)).toThrow(
    `Unknown event type '${badEventType}'.`
  );
});

test('converts empty function info strings to undefined', () => {
  const sessionId = 'session id';
  const processId = 'process id';
  const threadId = 'thread id';
  const functionId = 'function id';

  const spoorEvent = new Event();
  spoorEvent.setSessionId(sessionId);
  spoorEvent.setProcessId(processId);
  spoorEvent.setThreadId(threadId);
  spoorEvent.setFunctionId(functionId);
  spoorEvent.setType(Event.Type.FUNCTION_ENTRY);
  spoorEvent.setTimestamp(new Timestamp());

  const functionInfo = new FunctionInfo();
  functionInfo.setLinkageName('');
  functionInfo.setDemangledName('');
  functionInfo.setFileName('');
  functionInfo.setDirectory('');
  functionInfo.setInstrumented(true);

  const chromeEvent = ChromeEvent.fromEventAndFunctionInfo(
    spoorEvent,
    functionInfo
  );
  const expectedChromeEvent = {
    name: functionId,
    ph: ChromeEventType.FunctionEntry,
    ts: 0,
    pid: processId,
    tid: threadId,
    args: {
      directory: undefined,
      fileName: undefined,
      functionId: functionId,
      line: undefined,
      linkageName: undefined,
    },
  };
  expect(chromeEvent).toStrictEqual(expectedChromeEvent);
});

test('bonus usage to maximize coverage', () => {
  const spoorEventArgs = new ChromeTraceSpoorEventArgs('', '', '', 0, '');
  const event = new ChromeEvent(
    '',
    ChromeEventType.FunctionEntry,
    0,
    '',
    '',
    spoorEventArgs
  );
  const spoorTraceOtherData = new ChromeTraceSpoorTraceOtherData('', ['']);
  new ChromeTrace([event], 'ns', spoorTraceOtherData);
});
