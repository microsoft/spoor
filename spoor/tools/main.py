#!/usr/bin/env python3
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

import argparse
import datetime
import glob
import json
import numpy
import os
import sys
from spoor.proto.spoor_pb2 import Event, Trace, InstrumentedFunctionMap
from google.protobuf.timestamp_pb2 import Timestamp
from google.protobuf.json_format import MessageToJson

HEADER_SIZE_BYTES = 56
EVENT_SIZE_BYTES = 16
FOOTER_SIZE_BYTES = 1

def main(argv):
    # see https://docs.python.org/3/library/argparse.html#argumentparser-objects
    # for a list of options
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers()
    cat_parser = subparsers.add_parser('cat')
    # cat_parser.add_argument('-r', '--recursive', action='store_true')
    cat_parser.add_argument('files', nargs='+', type=str)
    args = parser.parse_args(argv[1:])

    files = []
    for file_glob in args.files:
        file_glob = os.path.expanduser(file_glob)
        for file in glob.glob(file_glob, recursive=True):
            files.append(file)

    # print(len(files), file=sys.stderr)
    # print(len(set(files)), file=sys.stderr)

    function_map = {}
    count = 0
    for file_name in sorted(filter(lambda f: f.endswith('.spoor_function_map'), files)):
        instrumented_function_map = InstrumentedFunctionMap()
        with open(file_name, 'rb') as fd:
            instrumented_function_map.ParseFromString(fd.read())
        for function_id in instrumented_function_map.function_map:
            count += 1
            # if function_id in function_map:
            #     # print(f'Duplicate function id {hex(function_id)}', file=sys.stderr)
            #     continue
            function_info = instrumented_function_map.function_map[function_id]
            function_map[function_id] = {
                'linkage_name': function_info.linkage_name,
                'demangled_name': function_info.demangled_name,
                'file_name': function_info.file_name,
                'directory': function_info.directory,
                'line': function_info.line,
                # 'instrumented': function_info.instrumented,
            }
    # print(f'len(function_map) = {len(set(function_map))}', file=sys.stderr)
    # print(f'count = {count}', file=sys.stderr)
    # function_map_end = datetime.datetime.now()

    print('[', end='')
    events = []
    for file_name in sorted(filter(lambda f: f.endswith('.spoor_trace'), files)):
        trace_file_start = datetime.datetime.now()

        version, session_id, process_id, thread_id, system_clock_timestamp, steady_clock_timestamp, event_count = numpy.fromfile(file_name, count=1, dtype=numpy.dtype('>i8, >u8, >i8, >u8, >i8, >i8, >i4'))[0]
        process_id = process_id.item()
        thread_id = thread_id.item()
        all_event_data = numpy.fromfile(file_name, offset=HEADER_SIZE_BYTES, count=event_count, dtype=numpy.dtype('>u8, >u8'))
        for event_data in all_event_data:
            function_id, event_type_and_timestamp = event_data
            function_id = function_id.item()
            event_type = event_type_and_timestamp >> numpy.uint64(63)
            timestamp_ns = event_type_and_timestamp & numpy.uint64(0x7fffffffffffffff)
            timestamp_us = timestamp_ns.item() / 1000.0

            if timestamp_us < (241978381657.536 - 100.0):
                continue

            if not function_id in function_map:
                print(f'ID {hex(function_id)} not in function map', file=sys.stderr)
                continue
            function_info = function_map[function_id]
            if event_type:
                ph = 'B'
            else:
                ph = 'E'

            # if function_info['demangled_name'] == '<compiler-generated>':
            #     continue

            print('{', end='')
            print('"name":', end='')
            print(json.dumps(function_info['demangled_name']), end='')
            print(',"ph":"', end='')
            print(ph, end='')
            print('","ts":', end='')
            print(timestamp_us, end='')
            print(',"pid":', end='')
            print(process_id, end='')
            print(',"tid":', end='')
            print(thread_id, end='')
            print(',"args":{', end='')
            print('"functionId":', end='')
            print(function_id, end='')
            print(',"fileName":"', end='')
            print(function_info['file_name'], end='')
            # print('","directory":"', end='')
            # print(function_info['directory'], end='')
            print('","line":', end='')
            print(function_info['line'], end='')
            # print(',"linkageName":', end='')
            # print(json.dumps(function_info['linkage_name']), end='')
            print('}', end='')
            print('},')#, end='')
            # events.append({
            # print(json.dumps({
            #     'name': function_info['demangled_name'],
            #     'ph': ph,
            #     'ts': timestamp_us,
            #     'pid': process_id,
            #     'tid': thread_id,
            #     'args': {
            #         'functionId': function_id,
            #         'fileName': function_info['file_name'],
            #         'directory': function_info['directory'],
            #         'line': function_info['line'],
            #         'linkageName': function_info['linkage_name'],
            #     },
            # }), end='')
            # print(',', end='')
        trace_file_end = datetime.datetime.now()
        # print(f'trace file {event_count} events: {trace_file_end - trace_file_start}', file=sys.stderr)
    print('{}]', end='')

    # print(json.dumps(events))
    # with open('/Users/lelandjansen/Desktop/myamazingapp_chrome_trace.json', 'w') as file:
    #     json.dump(events, file)

    return











                
#         if file_name.endswith('.spoor_trace.lz4'):
#             raise NotImplementedError('lz4 compression is not yet supported')
# 
#         elif file_name.endswith('.spoor_function_map'):
#             instrumented_function_map = InstrumentedFunctionMap()
#             with open(file_name, 'rb') as fd:
#                 instrumented_function_map.ParseFromString(fd.read())
#             # print(MessageToJson(instrumented_function_map))
#             # trace.modules.append(instrumented_function_map.module_id)
#             for function_id in instrumented_function_map.function_map:
#                 function_info = instrumented_function_map.function_map[function_id]
#                 function_map[function_id] = {
#                     'linkage_name': function_info.linkage_name,
#                     'demangled_name': function_info.demangled_name,
#                     'file_name': function_info.file_name,
#                     'directory': function_info.directory,
#                     'line': function_info.line,
#                     # 'instrumented': funciton_info.instrumented,
#                 }
# 
#     for file_name in files:
#         if file_name.endswith('.spoor_trace'):
#             # TODO: investigate https://docs.python.org/3/library/struct.html
# 
#             version, session_id, process_id, thread_id, system_clock_timestamp, steady_clock_timestamp, event_count = numpy.fromfile(file_name, count=1, dtype=numpy.dtype('>i8, >u8, >i8, >u8, >i8, >i8, >i4'))[0]
#             HEADER_SIZE_BYTES = 56
#             EVENT_SIZE_BYTES = 16
#             FOOTER_SIZE_BYTES = 1
#             all_event_data = numpy.fromfile(file_name, offset=HEADER_SIZE_BYTES, count=event_count, dtype=numpy.dtype('>u8, >u8'))
#             for event_data in all_event_data:
#                 function_id, event_type_and_timestamp = event_data
#                 event_type = event_type_and_timestamp >> numpy.uint64(63)
#                 timestamp_ns = event_type_and_timestamp & numpy.uint64(0x7fffffffffffffff)
#                 event = Event()
#                 event.session_id = session_id.item()
#                 event.process_id = process_id.item()
#                 event.thread_id = thread_id.item()
#                 event.function_id = function_id.item()
#                 event.type = event_type.item()
#                 event.timestamp.FromNanoseconds(timestamp_ns.item())
#                 trace.events.append(event)
#                 
#         if file_name.endswith('.spoor_trace.lz4'):
#             raise NotImplementedError('lz4 compression is not yet supported')
# 
#         elif file_name.endswith('.spoor_function_map'):
#             instrumented_function_map = InstrumentedFunctionMap()
#             with open(file_name, 'rb') as fd:
#                 instrumented_function_map.ParseFromString(fd.read())
#             # print(MessageToJson(instrumented_function_map))
#             # trace.modules.append(instrumented_function_map.module_id)
#             for function_id in instrumented_function_map.function_map:
#                 function_info = instrumented_function_map.function_map[function_id]
#                 function_map[function_id] = {
#                     'linkage_name': function_info.linkage_name,
#                     'demangled_name': function_info.demangled_name,
#                     'file_name': function_info.file_name,
#                     'directory': function_info.directory,
#                     'line': function_info.line,
#                     # 'instrumented': funciton_info.instrumented,
#                 }
# 
# 
#     trace.created_at.FromDatetime(datetime.datetime.now())
# 
#     chrome = []
#     for event in trace.events:
#         function_id = event.function_id
#         if function_id not in function_map:
#             continue
#         function_info = function_map[function_id]
#         if event.type == Event.Type.FUNCTION_ENTRY:
#             ph = 'B'
#         else:
#             ph = 'E'
#         ts = event.timestamp.ToMicroseconds()
#         pid = event.process_id
#         tid = event.thread_id
#         chrome.append({
#             'name': function_info['demangled_name'],
#             'ph': ph,
#             'ts': ts,
#             'pid': pid,
#             'tid': tid,
#             # 'args': {
#             #     'functionId': function_id,
#             #     'fileName': function_info['file_name'],
#             #     'directory': function_info['directory'],
#             #     'line': function_info['line'],
#             #     'linkageName': function_info['linkage_name'],
#             # },
#         })

    # print(len(sorted(list(function_map.keys()))))
    # print(len(sorted(list(set(function_ids)))))
    # print(json.dumps(chrome))


if __name__ == '__main__':
    main(sys.argv)
