// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

const spoorProto = require('../dist/spoor_pb');
const google_protobuf_timestamp_pb = require('google-protobuf/google/protobuf/timestamp_pb.js');
export const Event = spoorProto.Event;
export const FunctionInfo = spoorProto.FunctionInfo;
export const InstrumentedFunctionMap = spoorProto.InstrumentedFunctionMap;
export const Timestamp = google_protobuf_timestamp_pb.Timestamp;
export const Trace = spoorProto.Trace;
