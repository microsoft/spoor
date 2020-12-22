// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

export {
  Event,
  FunctionInfo,
  InstrumentedFunctionMap,
  Timestamp,
  Trace,
} from '@microsoft/spoor-proto';
export {
  EVENT_SIZE_BYTES,
  Event as BinaryEvent,
  EventType as BinaryEventType,
  FOOTER_SIZE_BYTES,
  Footer as BinaryFooter,
  HEADER_SIZE_BYTES,
  Header as BinaryHeader,
  Trace as BinaryTrace,
} from './binaryTrace';
export {
  Event as ChromeEvent,
  EventType as ChromeEventType,
  SpoorEventArgs as ChromeTraceSpoorEventArgs,
  SpoorTraceOtherData as ChromeTraceSpoorTraceOtherData,
  Trace as ChromeTrace,
} from './chromeTrace';
export {mergeTraces} from './mergeTraces';
