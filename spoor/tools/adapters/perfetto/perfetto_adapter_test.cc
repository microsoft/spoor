// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/tools/adapters/perfetto/perfetto_adapter.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "city_hash/city.h"
#include "google/protobuf/repeated_field.h"
#include "google/protobuf/util/message_differencer.h"
#include "google/protobuf/util/time_util.h"
#include "gsl/gsl"
#include "gtest/gtest.h"
#include "protos/perfetto/common/builtin_clock.pb.h"
#include "protos/perfetto/trace/clock_snapshot.pb.h"
#include "protos/perfetto/trace/trace.pb.h"
#include "protos/perfetto/trace/trace_packet.pb.h"
#include "protos/perfetto/trace/track_event/source_location.pb.h"
#include "protos/perfetto/trace/track_event/track_event.pb.h"
#include "spoor/instrumentation/symbols/symbols.pb.h"
#include "spoor/runtime/trace/trace.h"
#include "spoor/tools/adapters/perfetto/perfetto_adapter_private.h"
#include "util/numeric.h"

namespace {

using google::protobuf::util::MessageDifferencer;
using google::protobuf::util::TimeUtil;
using perfetto::protos::BuiltinClock;
using perfetto::protos::ClockSnapshot;
using perfetto::protos::EventName;
using perfetto::protos::SourceLocation;
using perfetto::protos::TracePacket;
using perfetto::protos::TrackEvent;
using spoor::instrumentation::symbols::FunctionInfo;
using spoor::instrumentation::symbols::Symbols;
using spoor::runtime::trace::CompressionStrategy;
using spoor::runtime::trace::Event;
using spoor::runtime::trace::EventCount;
using spoor::runtime::trace::EventType;
using spoor::runtime::trace::FunctionId;
using spoor::runtime::trace::kEndianness;
using spoor::runtime::trace::kMagicNumber;
using spoor::runtime::trace::kTraceFileVersion;
using spoor::runtime::trace::ProcessId;
using spoor::runtime::trace::SessionId;
using spoor::runtime::trace::ThreadId;
using spoor::runtime::trace::TraceFile;
using spoor::tools::adapters::perfetto::kTrustedPacketSequenceId;
using spoor::tools::adapters::perfetto::SpoorTraceToPerfettoTrace;
using spoor::tools::adapters::perfetto::internal::TrackUuid;
using PerfettoTrace = perfetto::protos::Trace;

static_assert(kTrustedPacketSequenceId != 0);  // Perfetto requirement.

TEST(PerfettoAdapter, HandlesZeroEvents) {  // NOLINT
  const Symbols symbols{};
  const std::vector<TraceFile> trace_files{};
  const PerfettoTrace expected_perfetto_trace{};
  const auto perfetto_trace = SpoorTraceToPerfettoTrace(trace_files, symbols);
  ASSERT_TRUE(
      MessageDifferencer::Equals(perfetto_trace, expected_perfetto_trace));
}

TEST(PerfettoAdapter, AdaptsEntryExitTraces) {  // NOLINT
  constexpr SessionId session_id{1};
  constexpr ProcessId process_id{2};
  constexpr ThreadId thread_a_id{10};
  constexpr ThreadId thread_b_id{11};
  constexpr FunctionId function_a_id{0};
  constexpr FunctionId function_b_id{1};
  constexpr FunctionId function_c{2};
  const auto thread_a_track_uuid = TrackUuid(process_id, thread_a_id);
  const auto thread_b_track_uuid = TrackUuid(process_id, thread_b_id);

  const Symbols symbols{};

  const auto trace_files = [&] {
    std::vector<Event> thread_a_events{
        {
            .steady_clock_timestamp = 0,
            .payload_1 = function_a_id,
            .type = static_cast<EventType>(Event::Type::kFunctionEntry),
            .payload_2 = 0,
        },
        {
            .steady_clock_timestamp = 1,
            .payload_1 = function_b_id,
            .type = static_cast<EventType>(Event::Type::kFunctionEntry),
            .payload_2 = 0,
        },
        {
            .steady_clock_timestamp = 2,
            .payload_1 = function_b_id,
            .type = static_cast<EventType>(Event::Type::kFunctionExit),
            .payload_2 = 0,
        },
        {
            .steady_clock_timestamp = 3,
            .payload_1 = function_a_id,
            .type = static_cast<EventType>(Event::Type::kFunctionExit),
            .payload_2 = 0,
        },
        {
            .steady_clock_timestamp = 4,
            .payload_1 = function_c,
            .type = static_cast<EventType>(Event::Type::kFunctionEntry),
            .payload_2 = 0,
        },
        {
            .steady_clock_timestamp = 5,
            .payload_1 = function_c,
            .type = static_cast<EventType>(Event::Type::kFunctionExit),
            .payload_2 = 0,
        },
    };
    std::vector<Event> thread_b_events{
        {
            .steady_clock_timestamp = 2,
            .payload_1 = function_a_id,
            .type = static_cast<EventType>(Event::Type::kFunctionEntry),
            .payload_2 = 0,
        },
        {
            .steady_clock_timestamp = 4,
            .payload_1 = function_a_id,
            .type = static_cast<EventType>(Event::Type::kFunctionExit),
            .payload_2 = 0,
        },
    };
    const auto thread_a_event_count = thread_a_events.size();
    const auto thread_b_event_count = thread_b_events.size();
    return std::vector<TraceFile>{
        {
            .header =
                {
                    .magic_number = kMagicNumber,
                    .endianness = kEndianness,
                    .compression_strategy = CompressionStrategy::kNone,
                    .version = kTraceFileVersion,
                    .session_id = session_id,
                    .process_id = process_id,
                    .thread_id = thread_a_id,
                    .system_clock_timestamp = 7,
                    .steady_clock_timestamp = 6,
                    .event_count =
                        gsl::narrow_cast<EventCount>(thread_a_event_count),
                    .padding = {},
                },
            .events = std::move(thread_a_events),
        },
        {
            .header =
                {
                    .magic_number = kMagicNumber,
                    .endianness = kEndianness,
                    .compression_strategy = CompressionStrategy::kNone,
                    .version = kTraceFileVersion,
                    .session_id = session_id,
                    .process_id = process_id,
                    .thread_id = thread_b_id,
                    .system_clock_timestamp = 9,
                    .steady_clock_timestamp = 8,
                    .event_count =
                        gsl::narrow_cast<EventCount>(thread_b_event_count),
                    .padding = {},
                },
            .events = std::move(thread_b_events),
        },
    };
  }();
  const auto expected_perfetto_trace = [&] {
    std::vector packets{
        [&] {
          TracePacket packet{};
          packet.set_trusted_packet_sequence_id(kTrustedPacketSequenceId);
          packet.set_previous_packet_dropped(true);
          packet.set_sequence_flags(TracePacket::SEQ_INCREMENTAL_STATE_CLEARED);
          auto* trace_config = packet.mutable_trace_config();
          auto* builtin_data_sources =
              trace_config->mutable_builtin_data_sources();
          builtin_data_sources->set_primary_trace_clock(
              BuiltinClock::BUILTIN_CLOCK_REALTIME);
          packet.mutable_interned_data();
          return packet;
        }(),
        [&] {
          TracePacket packet{};
          auto* track_descriptor = packet.mutable_track_descriptor();
          track_descriptor->set_uuid(TrackUuid(process_id, thread_a_id));
          auto* thread_descriptor = track_descriptor->mutable_thread();
          thread_descriptor->set_pid(process_id);
          thread_descriptor->set_tid(thread_a_id);
          return packet;
        }(),
        [&] {
          TracePacket packet{};
          auto* track_descriptor = packet.mutable_track_descriptor();
          track_descriptor->set_uuid(TrackUuid(process_id, thread_b_id));
          auto* thread_descriptor = track_descriptor->mutable_thread();
          thread_descriptor->set_pid(process_id);
          thread_descriptor->set_tid(thread_b_id);
          return packet;
        }(),
        [&] {
          ClockSnapshot::Clock steady_clock{};
          steady_clock.set_clock_id(BuiltinClock::BUILTIN_CLOCK_MONOTONIC);
          steady_clock.set_timestamp(6);
          ClockSnapshot::Clock system_clock{};
          system_clock.set_clock_id(BuiltinClock::BUILTIN_CLOCK_REALTIME);
          system_clock.set_timestamp(7);
          TracePacket packet{};
          packet.set_trusted_packet_sequence_id(kTrustedPacketSequenceId);
          auto* clock_snapshot = packet.mutable_clock_snapshot();
          auto* clocks = clock_snapshot->mutable_clocks();
          clocks->Add(std::move(steady_clock));
          clocks->Add(std::move(system_clock));
          clock_snapshot->set_primary_trace_clock(
              BuiltinClock::BUILTIN_CLOCK_REALTIME);
          return packet;
        }(),
        [&] {
          TracePacket packet{};
          packet.set_trusted_packet_sequence_id(kTrustedPacketSequenceId);
          packet.set_timestamp(0);
          packet.set_timestamp_clock_id(BuiltinClock::BUILTIN_CLOCK_MONOTONIC);
          packet.set_sequence_flags(TracePacket::SEQ_NEEDS_INCREMENTAL_STATE);
          auto& track_event = *packet.mutable_track_event();
          track_event.set_track_uuid(thread_a_track_uuid);
          track_event.set_type(TrackEvent::TYPE_SLICE_BEGIN);
          track_event.set_name_iid(function_a_id);
          track_event.set_source_location_iid(function_a_id);
          return packet;
        }(),
        [&] {
          TracePacket packet{};
          packet.set_trusted_packet_sequence_id(kTrustedPacketSequenceId);
          packet.set_timestamp(1);
          packet.set_timestamp_clock_id(BuiltinClock::BUILTIN_CLOCK_MONOTONIC);
          packet.set_sequence_flags(TracePacket::SEQ_NEEDS_INCREMENTAL_STATE);
          auto& track_event = *packet.mutable_track_event();
          track_event.set_track_uuid(thread_a_track_uuid);
          track_event.set_type(TrackEvent::TYPE_SLICE_BEGIN);
          track_event.set_name_iid(function_b_id);
          track_event.set_source_location_iid(function_b_id);
          return packet;
        }(),
        [&] {
          TracePacket packet{};
          packet.set_trusted_packet_sequence_id(kTrustedPacketSequenceId);
          packet.set_timestamp(2);
          packet.set_timestamp_clock_id(BuiltinClock::BUILTIN_CLOCK_MONOTONIC);
          packet.set_sequence_flags(TracePacket::SEQ_NEEDS_INCREMENTAL_STATE);
          auto& track_event = *packet.mutable_track_event();
          track_event.set_track_uuid(thread_a_track_uuid);
          track_event.set_type(TrackEvent::TYPE_SLICE_END);
          return packet;
        }(),
        [&] {
          TracePacket packet{};
          packet.set_trusted_packet_sequence_id(kTrustedPacketSequenceId);
          packet.set_timestamp(3);
          packet.set_timestamp_clock_id(BuiltinClock::BUILTIN_CLOCK_MONOTONIC);
          packet.set_sequence_flags(TracePacket::SEQ_NEEDS_INCREMENTAL_STATE);
          auto& track_event = *packet.mutable_track_event();
          track_event.set_track_uuid(thread_a_track_uuid);
          track_event.set_type(TrackEvent::TYPE_SLICE_END);
          return packet;
        }(),
        [&] {
          TracePacket packet{};
          packet.set_trusted_packet_sequence_id(kTrustedPacketSequenceId);
          packet.set_timestamp(4);
          packet.set_timestamp_clock_id(BuiltinClock::BUILTIN_CLOCK_MONOTONIC);
          packet.set_sequence_flags(TracePacket::SEQ_NEEDS_INCREMENTAL_STATE);
          auto& track_event = *packet.mutable_track_event();
          track_event.set_track_uuid(thread_a_track_uuid);
          track_event.set_type(TrackEvent::TYPE_SLICE_BEGIN);
          track_event.set_name_iid(function_c);
          track_event.set_source_location_iid(function_c);
          return packet;
        }(),
        [&] {
          TracePacket packet{};
          packet.set_trusted_packet_sequence_id(kTrustedPacketSequenceId);
          packet.set_timestamp(5);
          packet.set_timestamp_clock_id(BuiltinClock::BUILTIN_CLOCK_MONOTONIC);
          packet.set_sequence_flags(TracePacket::SEQ_NEEDS_INCREMENTAL_STATE);
          auto& track_event = *packet.mutable_track_event();
          track_event.set_track_uuid(thread_a_track_uuid);
          track_event.set_type(TrackEvent::TYPE_SLICE_END);
          return packet;
        }(),
        [&] {
          ClockSnapshot::Clock steady_clock{};
          steady_clock.set_clock_id(BuiltinClock::BUILTIN_CLOCK_MONOTONIC);
          steady_clock.set_timestamp(8);
          ClockSnapshot::Clock system_clock{};
          system_clock.set_clock_id(BuiltinClock::BUILTIN_CLOCK_REALTIME);
          system_clock.set_timestamp(9);
          TracePacket packet{};
          packet.set_trusted_packet_sequence_id(kTrustedPacketSequenceId);
          auto* clock_snapshot = packet.mutable_clock_snapshot();
          auto* clocks = clock_snapshot->mutable_clocks();
          clocks->Add(std::move(steady_clock));
          clocks->Add(std::move(system_clock));
          clock_snapshot->set_primary_trace_clock(
              BuiltinClock::BUILTIN_CLOCK_REALTIME);
          return packet;
        }(),
        [&] {
          TracePacket packet{};
          packet.set_trusted_packet_sequence_id(kTrustedPacketSequenceId);
          packet.set_timestamp(2);
          packet.set_timestamp_clock_id(BuiltinClock::BUILTIN_CLOCK_MONOTONIC);
          packet.set_sequence_flags(TracePacket::SEQ_NEEDS_INCREMENTAL_STATE);
          auto& track_event = *packet.mutable_track_event();
          track_event.set_track_uuid(thread_b_track_uuid);
          track_event.set_type(TrackEvent::TYPE_SLICE_BEGIN);
          track_event.set_name_iid(function_a_id);
          track_event.set_source_location_iid(function_a_id);
          return packet;
        }(),
        [&] {
          TracePacket packet{};
          packet.set_trusted_packet_sequence_id(kTrustedPacketSequenceId);
          packet.set_timestamp(4);
          packet.set_timestamp_clock_id(BuiltinClock::BUILTIN_CLOCK_MONOTONIC);
          packet.set_sequence_flags(TracePacket::SEQ_NEEDS_INCREMENTAL_STATE);
          auto& track_event = *packet.mutable_track_event();
          track_event.set_track_uuid(thread_b_track_uuid);
          track_event.set_type(TrackEvent::TYPE_SLICE_END);
          return packet;
        }(),
    };

    PerfettoTrace trace{};
    auto* repeated_packets = trace.mutable_packet();
    std::move(std::begin(packets), std::end(packets),
              RepeatedFieldBackInserter(repeated_packets));
    return trace;
  }();
  const auto perfetto_trace = SpoorTraceToPerfettoTrace(trace_files, symbols);
  ASSERT_TRUE(
      MessageDifferencer::Equals(perfetto_trace, expected_perfetto_trace));
}

TEST(PerfettoAdapter, AdaptsSymbolsToInternedData) {  // NOLINT
  const std::string module_id{"module_id"};
  constexpr SessionId session_id{1};
  constexpr ProcessId process_id{2};
  constexpr ThreadId thread_id{10};
  constexpr FunctionId function_a_id{0};
  constexpr FunctionId function_b_id{1};
  const std::string function_a_linkage_name{"function_a"};
  const std::string function_b_linkage_name{"function_b"};
  const std::string function_a_demangled_name{"FuctionA()"};
  const std::string function_b_demangled_name{"FunctionB()"};
  const std::string function_a_file_name{"file_a.source"};
  const std::string function_b_file_name{"file_b.source"};
  const std::string function_a_directory{"/path/to/a/"};
  const std::string function_b_directory{"/path/to/b/"};
  constexpr int32 function_a_line_number{1};
  constexpr int32 function_b_line_number{2};
  constexpr int32 function_a_ir_instruction_count{100};
  constexpr int32 function_b_ir_instruction_count{200};

  const auto symbols = [&] {
    std::vector<std::pair<FunctionId, FunctionInfo>> function_infos{
        {
            function_a_id,
            [&] {
              FunctionInfo function_info{};
              function_info.set_module_id(module_id);
              function_info.set_linkage_name(function_a_linkage_name);
              function_info.set_demangled_name(function_a_demangled_name);
              function_info.set_file_name(function_a_file_name);
              function_info.set_directory(function_a_directory);
              function_info.set_line(function_a_line_number);
              function_info.set_ir_instruction_count(
                  function_a_ir_instruction_count);
              function_info.set_instrumented(true);
              *function_info.mutable_created_at() =
                  TimeUtil::NanosecondsToTimestamp(1);
              return function_info;
            }(),
        },
        {
            function_b_id,
            [&] {
              FunctionInfo function_info{};
              function_info.set_module_id(module_id);
              function_info.set_linkage_name(function_b_linkage_name);
              function_info.set_demangled_name(function_b_demangled_name);
              function_info.set_file_name(function_b_file_name);
              function_info.set_directory(function_b_directory);
              function_info.set_line(function_b_line_number);
              function_info.set_ir_instruction_count(
                  function_b_ir_instruction_count);
              function_info.set_instrumented(true);
              *function_info.mutable_created_at() =
                  TimeUtil::NanosecondsToTimestamp(2);
              return function_info;
            }(),
        },
    };

    Symbols symbols{};
    auto& function_symbols_table = *symbols.mutable_function_symbols_table();
    for (auto& [function_id, function_info] : function_infos) {
      auto& repeated_function_infos = function_symbols_table[function_id];
      *repeated_function_infos.add_function_infos() = std::move(function_info);
    }
    return symbols;
  }();

  const auto trace_files = [&] {
    std::vector<Event> events{
        {
            .steady_clock_timestamp = 0,
            .payload_1 = function_a_id,
            .type = static_cast<EventType>(Event::Type::kFunctionEntry),
            .payload_2 = 0,
        },
        {
            .steady_clock_timestamp = 1,
            .payload_1 = function_b_id,
            .type = static_cast<EventType>(Event::Type::kFunctionEntry),
            .payload_2 = 0,
        },
    };
    const auto event_count = events.size();
    return std::vector<TraceFile>{{
        .header =
            {
                .magic_number = kMagicNumber,
                .endianness = kEndianness,
                .compression_strategy = CompressionStrategy::kNone,
                .version = kTraceFileVersion,
                .session_id = session_id,
                .process_id = process_id,
                .thread_id = thread_id,
                .system_clock_timestamp = 0,
                .steady_clock_timestamp = 0,
                .event_count = gsl::narrow_cast<EventCount>(event_count),
                .padding = {},
            },
        .events = std::move(events),
    }};
  }();

  const auto expected_head_packet = [&] {
    std::vector<EventName> event_names{
        [&] {
          EventName event_name{};
          event_name.set_iid(function_a_id);
          event_name.set_name(function_a_demangled_name);
          return event_name;
        }(),
        [&] {
          EventName event_name{};
          event_name.set_iid(function_b_id);
          event_name.set_name(function_b_demangled_name);
          return event_name;
        }(),
    };
    std::vector<SourceLocation> source_locations{
        [&] {
          SourceLocation source_location{};
          source_location.set_iid(function_a_id);
          source_location.set_file_name(
              absl::StrCat(function_a_directory, function_a_file_name));
          source_location.set_function_name(function_a_demangled_name);
          source_location.set_line_number(function_a_line_number);
          return source_location;
        }(),
        [&] {
          SourceLocation source_location{};
          source_location.set_iid(function_b_id);
          source_location.set_file_name(
              absl::StrCat(function_b_directory, function_b_file_name));
          source_location.set_function_name(function_b_demangled_name);
          source_location.set_line_number(function_b_line_number);
          return source_location;
        }(),
    };
    TracePacket packet{};
    packet.set_trusted_packet_sequence_id(kTrustedPacketSequenceId);
    packet.set_previous_packet_dropped(true);
    packet.set_sequence_flags(TracePacket::SEQ_INCREMENTAL_STATE_CLEARED);
    auto* trace_config = packet.mutable_trace_config();
    auto* builtin_data_sources = trace_config->mutable_builtin_data_sources();
    builtin_data_sources->set_primary_trace_clock(
        BuiltinClock::BUILTIN_CLOCK_REALTIME);
    auto* interned_data = packet.mutable_interned_data();
    std::move(std::begin(event_names), std::end(event_names),
              RepeatedFieldBackInserter(interned_data->mutable_event_names()));
    std::move(
        std::begin(source_locations), std::end(source_locations),
        RepeatedFieldBackInserter(interned_data->mutable_source_locations()));
    return packet;
  }();

  const auto head_packet = [&] {
    auto perfetto_trace = SpoorTraceToPerfettoTrace(trace_files, symbols);
    auto head_packet = perfetto_trace.mutable_packet()->begin();
    auto* interned_data = head_packet->mutable_interned_data();
    auto& event_names = *interned_data->mutable_event_names();
    auto& source_locations = *interned_data->mutable_source_locations();
    // Iteration order through over a Protocol Buffer map is not defined.
    std::sort(
        std::begin(event_names), std::end(event_names),
        [](const auto& lhs, const auto& rhs) { return lhs.iid() < rhs.iid(); });
    std::sort(
        std::begin(source_locations), std::end(source_locations),
        [](const auto& lhs, const auto& rhs) { return lhs.iid() < rhs.iid(); });
    return *head_packet;
  }();
  ASSERT_TRUE(MessageDifferencer::Equals(head_packet, expected_head_packet));
}

TEST(PerfettoAdapter, DoesNotAddUnusedFunctionsToInternedData) {  // NOLINT
  const std::string module_id{"module_id"};
  constexpr SessionId session_id{1};
  constexpr ProcessId process_id{2};
  constexpr ThreadId thread_id{10};
  constexpr FunctionId function_a_id{0};
  constexpr FunctionId function_b_id{1};
  const std::string function_a_linkage_name{"function_a"};
  const std::string function_b_linkage_name{"function_b"};
  const std::string function_a_demangled_name{"FuctionA()"};
  const std::string function_b_demangled_name{"FunctionB()"};
  const std::string function_a_file_name{"file_a.source"};
  const std::string function_b_file_name{"file_b.source"};
  const std::string function_a_directory{"/path/to/a/"};
  const std::string function_b_directory{"/path/to/b/"};
  constexpr int32 function_a_line_number{1};
  constexpr int32 function_b_line_number{2};
  constexpr int32 function_a_ir_instruction_count{100};
  constexpr int32 function_b_ir_instruction_count{200};

  const auto symbols = [&] {
    std::vector<std::pair<FunctionId, FunctionInfo>> function_infos{
        {
            function_a_id,
            [&] {
              FunctionInfo function_info{};
              function_info.set_module_id(module_id);
              function_info.set_linkage_name(function_a_linkage_name);
              function_info.set_demangled_name(function_a_demangled_name);
              function_info.set_file_name(function_a_file_name);
              function_info.set_directory(function_a_directory);
              function_info.set_line(function_a_line_number);
              function_info.set_ir_instruction_count(
                  function_a_ir_instruction_count);
              function_info.set_instrumented(true);
              *function_info.mutable_created_at() =
                  TimeUtil::NanosecondsToTimestamp(1);
              return function_info;
            }(),
        },
        {
            function_b_id,
            [&] {
              FunctionInfo function_info{};
              function_info.set_module_id(module_id);
              function_info.set_linkage_name(function_b_linkage_name);
              function_info.set_demangled_name(function_b_demangled_name);
              function_info.set_file_name(function_b_file_name);
              function_info.set_directory(function_b_directory);
              function_info.set_line(function_b_line_number);
              function_info.set_ir_instruction_count(
                  function_b_ir_instruction_count);
              function_info.set_instrumented(true);
              *function_info.mutable_created_at() =
                  TimeUtil::NanosecondsToTimestamp(2);
              return function_info;
            }(),
        },
    };

    Symbols symbols{};
    auto& function_symbols_table = *symbols.mutable_function_symbols_table();
    for (auto& [function_id, function_info] : function_infos) {
      auto& repeated_function_infos = function_symbols_table[function_id];
      *repeated_function_infos.add_function_infos() = std::move(function_info);
    }
    return symbols;
  }();

  const auto trace_files = [&] {
    std::vector<Event> events{
        {
            .steady_clock_timestamp = 0,
            .payload_1 = function_a_id,
            .type = static_cast<EventType>(Event::Type::kFunctionEntry),
            .payload_2 = 0,
        },
    };
    const auto event_count = events.size();
    return std::vector<TraceFile>{{
        .header =
            {
                .magic_number = kMagicNumber,
                .endianness = kEndianness,
                .compression_strategy = CompressionStrategy::kNone,
                .version = kTraceFileVersion,
                .session_id = session_id,
                .process_id = process_id,
                .thread_id = thread_id,
                .system_clock_timestamp = 0,
                .steady_clock_timestamp = 0,
                .event_count = gsl::narrow_cast<EventCount>(event_count),
                .padding = {},
            },
        .events = std::move(events),
    }};
  }();

  const auto expected_head_packet = [&] {
    TracePacket packet{};
    packet.set_trusted_packet_sequence_id(kTrustedPacketSequenceId);
    packet.set_previous_packet_dropped(true);
    packet.set_sequence_flags(TracePacket::SEQ_INCREMENTAL_STATE_CLEARED);

    auto* trace_config = packet.mutable_trace_config();
    auto* builtin_data_sources = trace_config->mutable_builtin_data_sources();
    builtin_data_sources->set_primary_trace_clock(
        BuiltinClock::BUILTIN_CLOCK_REALTIME);

    auto* interned_data = packet.mutable_interned_data();

    auto* event_names = interned_data->mutable_event_names();
    auto* event_name = event_names->Add();
    event_name->set_iid(function_a_id);
    event_name->set_name(function_a_demangled_name);

    auto* source_locations = interned_data->mutable_source_locations();
    auto* source_location = source_locations->Add();
    source_location->set_iid(function_a_id);
    source_location->set_file_name(
        absl::StrCat(function_a_directory, function_a_file_name));
    source_location->set_function_name(function_a_demangled_name);
    source_location->set_line_number(function_a_line_number);

    return packet;
  }();

  auto perfetto_trace = SpoorTraceToPerfettoTrace(trace_files, symbols);
  auto* packets = perfetto_trace.mutable_packet();
  ASSERT_GE(packets->size(), 1);
  const auto head_packet = *packets->begin();
  ASSERT_TRUE(MessageDifferencer::Equals(head_packet, expected_head_packet));
}

// NOLINTNEXTLINE
TEST(PerfettoAdapter, HandlesMultipleFunctionInfosForSingleFunctionId) {
  const std::string module_id{"module_id"};
  constexpr SessionId session_id{1};
  constexpr ProcessId process_id{2};
  constexpr ThreadId thread_id{10};
  constexpr FunctionId function_a_id{0};
  constexpr FunctionId function_b_id{0};
  ASSERT_EQ(function_a_id, function_b_id);
  const std::string function_a_linkage_name{"function_a"};
  const std::string function_b_linkage_name{"function_b"};
  const std::string function_a_demangled_name{"FuctionA()"};
  const std::string function_b_demangled_name{"FunctionB()"};
  const std::string function_a_file_name{"file_a.source"};
  const std::string function_b_file_name{"file_b.source"};
  const std::string function_a_directory{"/path/to/a/"};
  const std::string function_b_directory{"/path/to/b/"};
  constexpr int32 function_a_line_number{1};
  constexpr int32 function_b_line_number{2};
  constexpr int32 function_a_ir_instruction_count{100};
  constexpr int32 function_b_ir_instruction_count{200};

  const auto symbols = [&] {
    std::vector<std::pair<FunctionId, FunctionInfo>> function_infos{
        {
            function_a_id,
            [&] {
              FunctionInfo function_info{};
              function_info.set_module_id(module_id);
              function_info.set_linkage_name(function_a_linkage_name);
              function_info.set_demangled_name(function_a_demangled_name);
              function_info.set_file_name(function_a_file_name);
              function_info.set_directory(function_a_directory);
              function_info.set_line(function_a_line_number);
              function_info.set_ir_instruction_count(
                  function_a_ir_instruction_count);
              function_info.set_instrumented(true);
              *function_info.mutable_created_at() =
                  TimeUtil::NanosecondsToTimestamp(1);
              return function_info;
            }(),
        },
        {
            function_b_id,
            [&] {
              FunctionInfo function_info{};
              function_info.set_module_id(module_id);
              function_info.set_linkage_name(function_b_linkage_name);
              function_info.set_demangled_name(function_b_demangled_name);
              function_info.set_file_name(function_b_file_name);
              function_info.set_directory(function_b_directory);
              function_info.set_line(function_b_line_number);
              function_info.set_ir_instruction_count(
                  function_b_ir_instruction_count);
              function_info.set_instrumented(true);
              *function_info.mutable_created_at() =
                  TimeUtil::NanosecondsToTimestamp(2);
              return function_info;
            }(),
        },
    };

    Symbols symbols{};
    auto& function_symbols_table = *symbols.mutable_function_symbols_table();
    for (auto& [function_id, function_info] : function_infos) {
      auto& repeated_function_infos = function_symbols_table[function_id];
      *repeated_function_infos.add_function_infos() = std::move(function_info);
    }
    return symbols;
  }();

  const auto trace_files = [&] {
    std::vector<Event> events{
        {
            .steady_clock_timestamp = 0,
            .payload_1 = function_a_id,
            .type = static_cast<EventType>(Event::Type::kFunctionEntry),
            .payload_2 = 0,
        },
    };
    const auto event_count = events.size();
    return std::vector<TraceFile>{{
        .header =
            {
                .magic_number = kMagicNumber,
                .endianness = kEndianness,
                .compression_strategy = CompressionStrategy::kNone,
                .version = kTraceFileVersion,
                .session_id = session_id,
                .process_id = process_id,
                .thread_id = thread_id,
                .system_clock_timestamp = 0,
                .steady_clock_timestamp = 0,
                .event_count = gsl::narrow_cast<EventCount>(event_count),
                .padding = {},
            },
        .events = std::move(events),
    }};
  }();

  const auto expected_head_packet = [&] {
    TracePacket packet{};
    packet.set_trusted_packet_sequence_id(kTrustedPacketSequenceId);
    packet.set_previous_packet_dropped(true);
    packet.set_sequence_flags(TracePacket::SEQ_INCREMENTAL_STATE_CLEARED);

    auto* trace_config = packet.mutable_trace_config();
    auto* builtin_data_sources = trace_config->mutable_builtin_data_sources();
    builtin_data_sources->set_primary_trace_clock(
        BuiltinClock::BUILTIN_CLOCK_REALTIME);

    auto* interned_data = packet.mutable_interned_data();

    const auto conflict = absl::StrFormat(
        "2-way conflict for function ID %#016x: ", function_a_id);

    auto* event_names = interned_data->mutable_event_names();
    auto* event_name = event_names->Add();
    event_name->set_iid(function_a_id);
    event_name->set_name(absl::StrCat(conflict, function_a_demangled_name, "; ",
                                      function_b_demangled_name));

    auto* source_locations = interned_data->mutable_source_locations();
    auto* source_location = source_locations->Add();
    source_location->set_iid(function_a_id);
    source_location->set_file_name(
        absl::StrCat(conflict, function_a_directory, function_a_file_name, ":",
                     function_a_line_number, "; ", function_b_directory,
                     function_b_file_name, ":", function_b_line_number));
    source_location->set_function_name(absl::StrCat(
        conflict, function_a_demangled_name, "; ", function_b_demangled_name));

    return packet;
  }();

  auto perfetto_trace = SpoorTraceToPerfettoTrace(trace_files, symbols);
  auto* packets = perfetto_trace.mutable_packet();
  ASSERT_GE(packets->size(), 1);
  const auto head_packet = *packets->begin();
  ASSERT_TRUE(MessageDifferencer::Equals(head_packet, expected_head_packet));
}

TEST(PerfettoAdapter, HandlesFunctionNameFallback) {  // NOLINT
  const std::string module_id{"module_id"};
  constexpr SessionId session_id{1};
  constexpr ProcessId process_id{2};
  constexpr ThreadId thread_id{10};
  constexpr FunctionId function_a_id{0};
  constexpr FunctionId function_b_id{1};
  const std::string function_a_linkage_name{"function_a"};
  const std::string function_a_expected_function_name{function_a_linkage_name};
  const auto function_b_expected_function_name =
      absl::StrFormat("%#016x", function_b_id);
  const std::string function_a_file_name{"file_a.source"};
  const std::string function_b_file_name{"file_b.source"};
  const std::string function_a_directory{"/path/to/a/"};
  const std::string function_b_directory{"/path/to/b/"};
  constexpr int32 function_a_line_number{1};
  constexpr int32 function_b_line_number{2};
  constexpr int32 function_a_ir_instruction_count{100};
  constexpr int32 function_b_ir_instruction_count{200};

  const auto symbols = [&] {
    std::vector<std::pair<FunctionId, FunctionInfo>> function_infos{
        {
            function_a_id,
            [&] {
              FunctionInfo function_info{};
              function_info.set_module_id(module_id);
              function_info.set_linkage_name(function_a_linkage_name);
              function_info.set_file_name(function_a_file_name);
              function_info.set_directory(function_a_directory);
              function_info.set_line(function_a_line_number);
              function_info.set_ir_instruction_count(
                  function_a_ir_instruction_count);
              function_info.set_instrumented(true);
              *function_info.mutable_created_at() =
                  TimeUtil::NanosecondsToTimestamp(1);
              return function_info;
            }(),
        },
        {
            function_b_id,
            [&] {
              FunctionInfo function_info{};
              function_info.set_module_id(module_id);
              function_info.set_file_name(function_b_file_name);
              function_info.set_directory(function_b_directory);
              function_info.set_line(function_b_line_number);
              function_info.set_ir_instruction_count(
                  function_b_ir_instruction_count);
              function_info.set_instrumented(true);
              *function_info.mutable_created_at() =
                  TimeUtil::NanosecondsToTimestamp(2);
              return function_info;
            }(),
        },
    };

    Symbols symbols{};
    auto& function_symbols_table = *symbols.mutable_function_symbols_table();
    for (auto& [function_id, function_info] : function_infos) {
      auto& repeated_function_infos = function_symbols_table[function_id];
      *repeated_function_infos.add_function_infos() = std::move(function_info);
    }
    return symbols;
  }();

  const auto trace_files = [&] {
    std::vector<Event> events{
        {
            .steady_clock_timestamp = 0,
            .payload_1 = function_a_id,
            .type = static_cast<EventType>(Event::Type::kFunctionEntry),
            .payload_2 = 0,
        },
        {
            .steady_clock_timestamp = 0,
            .payload_1 = function_b_id,
            .type = static_cast<EventType>(Event::Type::kFunctionEntry),
            .payload_2 = 0,
        },
    };
    const auto event_count = events.size();
    return std::vector<TraceFile>{{
        .header =
            {
                .magic_number = kMagicNumber,
                .endianness = kEndianness,
                .compression_strategy = CompressionStrategy::kNone,
                .version = kTraceFileVersion,
                .session_id = session_id,
                .process_id = process_id,
                .thread_id = thread_id,
                .system_clock_timestamp = 0,
                .steady_clock_timestamp = 0,
                .event_count = gsl::narrow_cast<EventCount>(event_count),
                .padding = {},
            },
        .events = std::move(events),
    }};
  }();

  const auto expected_head_packet = [&] {
    std::vector<EventName> event_names{
        [&] {
          EventName event_name{};
          event_name.set_iid(function_a_id);
          event_name.set_name(function_a_expected_function_name);
          return event_name;
        }(),
        [&] {
          EventName event_name{};
          event_name.set_iid(function_b_id);
          event_name.set_name(function_b_expected_function_name);
          return event_name;
        }(),
    };
    std::vector<SourceLocation> source_locations{
        [&] {
          SourceLocation source_location{};
          source_location.set_iid(function_a_id);
          source_location.set_file_name(
              absl::StrCat(function_a_directory, function_a_file_name));
          source_location.set_function_name(function_a_expected_function_name);
          source_location.set_line_number(function_a_line_number);
          return source_location;
        }(),
        [&] {
          SourceLocation source_location{};
          source_location.set_iid(function_b_id);
          source_location.set_file_name(
              absl::StrCat(function_b_directory, function_b_file_name));
          source_location.set_function_name(function_b_expected_function_name);
          source_location.set_line_number(function_b_line_number);
          return source_location;
        }(),
    };
    TracePacket packet{};
    packet.set_trusted_packet_sequence_id(kTrustedPacketSequenceId);
    packet.set_previous_packet_dropped(true);
    packet.set_sequence_flags(TracePacket::SEQ_INCREMENTAL_STATE_CLEARED);
    auto* trace_config = packet.mutable_trace_config();
    auto* builtin_data_sources = trace_config->mutable_builtin_data_sources();
    builtin_data_sources->set_primary_trace_clock(
        BuiltinClock::BUILTIN_CLOCK_REALTIME);
    auto* interned_data = packet.mutable_interned_data();
    std::move(std::begin(event_names), std::end(event_names),
              RepeatedFieldBackInserter(interned_data->mutable_event_names()));
    std::move(
        std::begin(source_locations), std::end(source_locations),
        RepeatedFieldBackInserter(interned_data->mutable_source_locations()));
    return packet;
  }();

  const auto head_packet = [&] {
    auto perfetto_trace = SpoorTraceToPerfettoTrace(trace_files, symbols);
    auto head_packet = perfetto_trace.mutable_packet()->begin();
    auto* interned_data = head_packet->mutable_interned_data();
    auto& event_names = *interned_data->mutable_event_names();
    auto& source_locations = *interned_data->mutable_source_locations();
    // Iteration order through over a Protocol Buffer map is not defined.
    std::sort(
        std::begin(event_names), std::end(event_names),
        [](const auto& lhs, const auto& rhs) { return lhs.iid() < rhs.iid(); });
    std::sort(
        std::begin(source_locations), std::end(source_locations),
        [](const auto& lhs, const auto& rhs) { return lhs.iid() < rhs.iid(); });
    return *head_packet;
  }();

  ASSERT_TRUE(MessageDifferencer::Equals(head_packet, expected_head_packet));
}

TEST(PerfettoAdapter, TruncatesLongNames) {  // NOLINT
  const std::string module_id{"module_id"};
  constexpr SessionId session_id{1};
  constexpr ProcessId process_id{2};
  constexpr ThreadId thread_id{10};
  constexpr FunctionId function_id{0};
  const std::string linkage_name{"function_a"};
  const std::string demangled_name(1'024 + 1, 'x');
  const auto expected_demangled_name =
      absl::StrCat(std::string(1'024 - 3, 'x'), "...");
  const std::string file_name(1'024 + 1, 'x');
  const std::string directory{"/path/to/a/"};
  const auto expected_file_name = absl::StrCat(
      directory, std::string(1'024 - directory.size() - 3, 'x'), "...");
  constexpr int32 line_number{1};
  constexpr int32 ir_instruction_count{100};

  const auto symbols = [&] {
    std::vector<std::pair<FunctionId, FunctionInfo>> function_infos{{
        function_id,
        [&] {
          FunctionInfo function_info{};
          function_info.set_module_id(module_id);
          function_info.set_linkage_name(linkage_name);
          function_info.set_demangled_name(demangled_name);
          function_info.set_file_name(file_name);
          function_info.set_directory(directory);
          function_info.set_line(line_number);
          function_info.set_ir_instruction_count(ir_instruction_count);
          function_info.set_instrumented(true);
          *function_info.mutable_created_at() =
              TimeUtil::NanosecondsToTimestamp(1);
          return function_info;
        }(),
    }};

    Symbols symbols{};
    auto& function_symbols_table = *symbols.mutable_function_symbols_table();
    for (auto& [function_id, function_info] : function_infos) {
      auto& repeated_function_infos = function_symbols_table[function_id];
      *repeated_function_infos.add_function_infos() = std::move(function_info);
    }
    return symbols;
  }();

  const auto trace_files = [&] {
    std::vector<Event> events{{
        .steady_clock_timestamp = 0,
        .payload_1 = function_id,
        .type = static_cast<EventType>(Event::Type::kFunctionEntry),
        .payload_2 = 0,
    }};
    const auto event_count = events.size();
    return std::vector<TraceFile>{{
        .header =
            {
                .magic_number = kMagicNumber,
                .endianness = kEndianness,
                .compression_strategy = CompressionStrategy::kNone,
                .version = kTraceFileVersion,
                .session_id = session_id,
                .process_id = process_id,
                .thread_id = thread_id,
                .system_clock_timestamp = 0,
                .steady_clock_timestamp = 0,
                .event_count = gsl::narrow_cast<EventCount>(event_count),
                .padding = {},
            },
        .events = std::move(events),
    }};
  }();

  const auto expected_head_packet = [&] {
    std::vector<EventName> event_names{[&] {
      EventName event_name{};
      event_name.set_iid(function_id);
      event_name.set_name(expected_demangled_name);
      return event_name;
    }()};
    std::vector<SourceLocation> source_locations{[&] {
      SourceLocation source_location{};
      source_location.set_iid(function_id);
      source_location.set_file_name(expected_file_name);
      source_location.set_function_name(expected_demangled_name);
      source_location.set_line_number(line_number);
      return source_location;
    }()};
    TracePacket packet{};
    packet.set_trusted_packet_sequence_id(kTrustedPacketSequenceId);
    packet.set_previous_packet_dropped(true);
    packet.set_sequence_flags(TracePacket::SEQ_INCREMENTAL_STATE_CLEARED);
    auto* trace_config = packet.mutable_trace_config();
    auto* builtin_data_sources = trace_config->mutable_builtin_data_sources();
    builtin_data_sources->set_primary_trace_clock(
        BuiltinClock::BUILTIN_CLOCK_REALTIME);
    auto* interned_data = packet.mutable_interned_data();
    std::move(std::begin(event_names), std::end(event_names),
              RepeatedFieldBackInserter(interned_data->mutable_event_names()));
    std::move(
        std::begin(source_locations), std::end(source_locations),
        RepeatedFieldBackInserter(interned_data->mutable_source_locations()));
    return packet;
  }();

  auto perfetto_trace = SpoorTraceToPerfettoTrace(trace_files, symbols);
  auto* packets = perfetto_trace.mutable_packet();
  ASSERT_GE(packets->size(), 1);
  const auto head_packet = *packets->begin();
  ASSERT_TRUE(MessageDifferencer::Equals(head_packet, expected_head_packet));
}

}  // namespace
