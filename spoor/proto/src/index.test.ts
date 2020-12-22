// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

import {
  Event,
  FunctionInfo,
  InstrumentedFunctionMap,
  Timestamp,
  Trace,
} from '.';

test('Imports and serializes trace', () => {
  const functionId = '10';
  const moduleId = 'module id';

  const functionInfo = new FunctionInfo();
  functionInfo.setLinkageName('linkage name');
  functionInfo.setDemangledName('demangled name');
  functionInfo.setFileName('file name');
  functionInfo.setDirectory('directory');
  functionInfo.setLine(42);
  functionInfo.setInstrumented(true);

  const instrumentedFunctionMap = new InstrumentedFunctionMap();
  instrumentedFunctionMap.setModuleId(moduleId);
  instrumentedFunctionMap.getFunctionMapMap().set(functionId, functionInfo);
  instrumentedFunctionMap.setCreatedAt(new Timestamp());

  const event = new Event();
  event.setSessionId('1');
  event.setProcessId('2');
  event.setThreadId('3');
  event.setFunctionId(functionId);
  event.setType(Event.Type.FUNCTION_ENTRY);
  event.setTimestamp(new Timestamp());

  const trace = new Trace();
  trace.setEventsList([event]);
  trace.setModulesList([moduleId]);
  trace.getFunctionMapMap().set(functionId, functionInfo);
  trace.setCreatedAt(new Timestamp());

  [
    [FunctionInfo, functionInfo],
    [InstrumentedFunctionMap, instrumentedFunctionMap],
    [Event, event],
    [Trace, trace],
  ].forEach(type_and_message => {
    const [type, message] = type_and_message;
    const bytes = message.serializeBinary();
    const parsed = type.deserializeBinary(bytes);
    expect(parsed).toStrictEqual(message);
  });
});
