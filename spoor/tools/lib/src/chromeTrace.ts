// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Chrome Trace Event Format specification:
// https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU
// This module implements a subset of the spec for Spoor's use case.

import {
  Event as SpoorEvent,
  FunctionInfo,
  Trace as SpoorTrace,
} from '@microsoft/spoor-proto';

export class SpoorEventArgs {
  constructor(
    readonly functionId: string,
    readonly fileName?: string,
    readonly directory?: string,
    readonly line?: number,
    readonly linkageName?: string
  ) {}
}

export enum EventType {
  FunctionEntry = 'B',
  FunctionExit = 'E',
}

export class Event {
  constructor(
    readonly name: string,
    readonly ph: EventType,
    readonly ts: number,
    readonly pid: number | string,
    readonly tid: number | string,
    readonly args: SpoorEventArgs
  ) {}

  static fromEventAndFunctionInfo(
    spoorEvent: typeof SpoorEvent,
    functionInfo?: typeof FunctionInfo
  ): Event {
    const name = (() => {
      const functionId = spoorEvent.getFunctionId();
      if (functionInfo === undefined) return functionId;
      const demangledName = functionInfo.getDemangledName();
      return demangledName === '' ? functionId : demangledName;
    })();
    const ph = (() => {
      const eventType = spoorEvent.getType();
      switch (eventType) {
        case SpoorEvent.Type.FUNCTION_EXIT:
          return EventType.FunctionExit;
        case SpoorEvent.Type.FUNCTION_ENTRY:
          return EventType.FunctionEntry;
        default:
          throw new Error(`Unknown event type '${eventType}'.`);
      }
    })();
    const ts = (() => {
      const timestamp = spoorEvent.getTimestamp();
      const seconds = timestamp.getSeconds();
      const remainderNanoseconds = timestamp.getNanos();
      const microseconds = 1000000.0 * seconds + remainderNanoseconds / 1000.0;
      return microseconds;
    })();
    const pid = spoorEvent.getProcessId();
    const tid = spoorEvent.getThreadId();
    const args = (() => {
      const fileName = (() => {
        if (functionInfo === undefined) return undefined;
        const fileName = functionInfo.getFileName();
        return fileName === '' ? undefined : fileName;
      })();
      const directory = (() => {
        if (functionInfo === undefined) return undefined;
        const directory = functionInfo.getDirectory();
        return directory === '' ? undefined : directory;
      })();
      const line = (() => {
        if (functionInfo === undefined) return undefined;
        const line = functionInfo.getLine();
        return line < 1 ? undefined : line;
      })();
      const linkageName = (() => {
        if (functionInfo === undefined) return undefined;
        const linkageName = functionInfo.getLinkageName();
        return linkageName === '' ? undefined : linkageName;
      })();
      return {
        functionId: spoorEvent.getFunctionId(),
        fileName: fileName,
        directory: directory,
        line: line,
        linkageName: linkageName,
      };
    })();
    return {
      name: name,
      ph: ph,
      ts: ts,
      pid: pid,
      tid: tid,
      args: args,
    };
  }
}

export class SpoorTraceOtherData {
  constructor(readonly createdAt: string, readonly modules: string[]) {}
}

export class Trace {
  constructor(
    readonly traceEvents: Event[],
    readonly displayTimeUnit?: string,
    readonly otherData?: SpoorTraceOtherData
  ) {}

  static fromSpoorTrace(spoorTrace: typeof SpoorTrace) {
    const functionMap = spoorTrace.getFunctionMapMap();
    const traceEvents = spoorTrace
      .getEventsList()
      .map((spoorEvent: typeof SpoorEvent) => {
        const functionInfo = functionMap.get(spoorEvent.getFunctionId());
        return Event.fromEventAndFunctionInfo(spoorEvent, functionInfo);
      });
    const displayTimeUnit = 'ns';
    const otherData = {
      createdAt: spoorTrace.getCreatedAt().toDate().toString(),
      modules: spoorTrace.getModulesList(),
    };
    return {
      traceEvents: traceEvents,
      displayTimeUnit: displayTimeUnit,
      otherData: otherData,
    };
  }
}
