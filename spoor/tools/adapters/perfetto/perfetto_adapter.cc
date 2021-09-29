// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/tools/adapters/perfetto/perfetto_adapter.h"

#include <algorithm>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "google/protobuf/repeated_field.h"
#include "gsl/gsl"
#include "protos/perfetto/common/builtin_clock.pb.h"
#include "protos/perfetto/trace/clock_snapshot.pb.h"
#include "protos/perfetto/trace/trace.pb.h"
#include "protos/perfetto/trace/trace_packet.pb.h"
#include "protos/perfetto/trace/track_event/source_location.pb.h"
#include "protos/perfetto/trace/track_event/track_event.pb.h"
#include "spoor/instrumentation/symbols/symbols.pb.h"
#include "spoor/tools/adapters/perfetto/perfetto_adapter_private.h"
#include "util/numeric.h"

namespace spoor::tools::adapters::perfetto {

using spoor::runtime::trace::FunctionId;
using PerfettoBuiltinClock = ::perfetto::protos::BuiltinClock;
using PerfettoClockSnapshot = ::perfetto::protos::ClockSnapshot;
using PerfettoEventName = ::perfetto::protos::EventName;
using PerfettoSourceLocation = ::perfetto::protos::SourceLocation;
using PerfettoTrace = ::perfetto::protos::Trace;
using PerfettoTracePacket = ::perfetto::protos::TracePacket;
using PerfettoTrackEvent = ::perfetto::protos::TrackEvent;
using SpoorEvent = spoor::runtime::trace::Event;
using SpoorFunctionInfo = spoor::instrumentation::symbols::FunctionInfo;
using SpoorHeader = spoor::runtime::trace::Header;
using SpoorRepeatedFunctionInfos =
    google::protobuf::RepeatedPtrField<SpoorFunctionInfo>;
using SpoorSymbols = spoor::instrumentation::symbols::Symbols;
using SpoorTraceFile = spoor::runtime::trace::TraceFile;

constexpr auto kPrimaryTraceClock{PerfettoBuiltinClock::BUILTIN_CLOCK_REALTIME};
constexpr std::string_view kSeparator{"; "};
constexpr std::string_view kUnknown{"unknown"};
constexpr std::string_view kConflictFormat{
    "%d-way conflict for function ID %#016x: %s"};
// Perfetto skips interned data with long strings. The limit is around 50k
// characters.
constexpr std::string_view::size_type kMaxNameLength{1'024};

auto Truncate(const std::string& string) -> std::string {
  constexpr std::string_view ellipsis{"..."};
  static_assert(ellipsis.size() < kMaxNameLength);
  if (string.size() <= kMaxNameLength) return string;
  return absl::StrCat(string.substr(0, kMaxNameLength - ellipsis.size()),
                      ellipsis);
}

auto MakeFunctionName(const FunctionId function_id,
                      const SpoorRepeatedFunctionInfos& function_infos)
    -> std::string {
  const auto joined =
      absl::StrJoin(function_infos, kSeparator,
                    [function_id](auto* out, const auto& function_info) {
                      const auto name = [&] {
                        if (function_info.has_demangled_name()) {
                          return function_info.demangled_name();
                        }
                        if (function_info.has_linkage_name()) {
                          return function_info.linkage_name();
                        }
                        return absl::StrFormat("%#016x", function_id);
                      }();
                      out->append(name);
                    });
  if (function_infos.size() < 2) return Truncate(joined);
  const auto formatted = absl::StrFormat(kConflictFormat, function_infos.size(),
                                         function_id, joined);
  return Truncate(formatted);
}

auto MakeFileName(const FunctionId function_id,
                  const SpoorRepeatedFunctionInfos& function_infos)
    -> std::string {
  const auto joined = absl::StrJoin(
      function_infos, kSeparator, [&](auto* out, const auto& function_info) {
        const auto path = [&] {
          const auto append_line = function_infos.size() != 1;
          std::filesystem::path path{};
          if (function_info.has_directory()) {
            path = function_info.directory();
            path.make_preferred();
          }
          if (function_info.has_file_name()) {
            path /= function_info.file_name();
            if (append_line && function_info.has_line()) {
              path += absl::StrFormat(":%d", function_info.line());
            }
          }
          if (path.empty()) return std::string{kUnknown};
          return path.string();
        }();
        out->append(path);
      });
  if (function_infos.size() < 2) return Truncate(joined);
  const auto formatted = absl::StrFormat(kConflictFormat, function_infos.size(),
                                         function_id, joined);
  return Truncate(formatted);
}

auto MakeEventName(const FunctionId function_id,
                   const SpoorRepeatedFunctionInfos& function_infos)
    -> PerfettoEventName {
  PerfettoEventName event_name{};
  event_name.set_iid(function_id);
  event_name.set_name(MakeFunctionName(function_id, function_infos));
  return event_name;
}

auto MakeSourceLocation(const FunctionId function_id,
                        const SpoorRepeatedFunctionInfos& function_infos)
    -> PerfettoSourceLocation {
  PerfettoSourceLocation source_location{};
  source_location.set_iid(function_id);
  source_location.set_file_name(MakeFileName(function_id, function_infos));
  source_location.set_function_name(
      MakeFunctionName(function_id, function_infos));
  if (function_infos.size() == 1) {
    const auto& function_info = std::cbegin(function_infos);
    if (function_info->has_line()) {
      source_location.set_line_number(function_info->line());
    }
  }
  return source_location;
}

auto MakePerfettoTrackEventTracePacket(const SpoorHeader& spoor_header,
                                       const SpoorEvent& spoor_event)
    -> PerfettoTracePacket {
  PerfettoTracePacket packet{};
  packet.set_trusted_packet_sequence_id(kTrustedPacketSequenceId);
  packet.set_timestamp(spoor_event.steady_clock_timestamp);
  packet.set_timestamp_clock_id(PerfettoBuiltinClock::BUILTIN_CLOCK_MONOTONIC);
  packet.set_sequence_flags(PerfettoTracePacket::SEQ_NEEDS_INCREMENTAL_STATE);

  auto* track_event = packet.mutable_track_event();
  const auto track_uuid =
      internal::TrackUuid(spoor_header.process_id, spoor_header.thread_id);
  track_event->set_track_uuid(track_uuid);

  switch (static_cast<SpoorEvent::Event::Type>(spoor_event.type)) {
    case SpoorEvent::Type::kFunctionEntry: {
      const auto function_id = spoor_event.payload_1;
      track_event->set_type(PerfettoTrackEvent::TYPE_SLICE_BEGIN);
      track_event->set_name_iid(function_id);
      track_event->set_source_location_iid(function_id);
      break;
    }
    case SpoorEvent::Type::kFunctionExit: {
      track_event->set_type(PerfettoTrackEvent::TYPE_SLICE_END);
      break;
    }
  }
  return packet;
}

auto MakePerfettoClockSnapshotTracePacket(const SpoorHeader& spoor_header)
    -> PerfettoTracePacket {
  PerfettoClockSnapshot::Clock steady_clock{};
  steady_clock.set_clock_id(PerfettoBuiltinClock::BUILTIN_CLOCK_MONOTONIC);
  steady_clock.set_timestamp(spoor_header.steady_clock_timestamp);

  PerfettoClockSnapshot::Clock system_clock{};
  system_clock.set_clock_id(PerfettoBuiltinClock::BUILTIN_CLOCK_REALTIME);
  system_clock.set_timestamp(spoor_header.system_clock_timestamp);

  PerfettoTracePacket packet{};
  packet.set_trusted_packet_sequence_id(kTrustedPacketSequenceId);
  auto* clock_snapshot = packet.mutable_clock_snapshot();
  auto* clocks = clock_snapshot->mutable_clocks();
  clocks->Reserve(2);
  clocks->Add(std::move(steady_clock));
  clocks->Add(std::move(system_clock));
  clock_snapshot->set_primary_trace_clock(kPrimaryTraceClock);
  return packet;
}

auto MakePerfettoTrackDescriptorPackets(
    const std::vector<SpoorTraceFile>& trace_files)
    -> std::vector<PerfettoTracePacket> {
  std::unordered_set<uint64> uuids{};
  std::vector<PerfettoTracePacket> trace_packets{};
  for (const auto& trace_file : trace_files) {
    const auto& header = trace_file.header;
    const auto uuid = internal::TrackUuid(header.process_id, header.thread_id);
    if (uuids.contains(uuid)) continue;
    PerfettoTracePacket packet{};
    auto* track_descriptor = packet.mutable_track_descriptor();
    track_descriptor->set_uuid(uuid);
    auto* thread_descriptor = track_descriptor->mutable_thread();
    thread_descriptor->set_pid(gsl::narrow_cast<int32>(header.process_id));
    thread_descriptor->set_tid(gsl::narrow_cast<int32>(header.thread_id));
    trace_packets.emplace_back(std::move(packet));
  }
  return trace_packets;
}

auto SpoorTraceToPerfettoTrace(const std::vector<SpoorTraceFile>& trace_files,
                               const SpoorSymbols& symbols) -> PerfettoTrace {
  const auto event_count = std::transform_reduce(
      std::cbegin(trace_files), std::cend(trace_files), 0,
      [](const auto size_a, const auto size_b) { return size_a + size_b; },
      [](const auto& trace_file) { return trace_file.events.size(); });
  if (event_count < 1) return {};

  std::unordered_set<FunctionId> called_function_ids{};
  std::for_each(
      std::cbegin(trace_files), std::cend(trace_files),
      [&called_function_ids](const auto& trace_file) {
        std::for_each(
            std::cbegin(trace_file.events), std::cend(trace_file.events),
            [&called_function_ids](const auto& event) {
              switch (static_cast<SpoorEvent::Event::Type>(event.type)) {
                case SpoorEvent::Type::kFunctionEntry:
                case SpoorEvent::Type::kFunctionExit: {
                  called_function_ids.emplace(event.payload_1);
                  break;
                }
              }
            });
      });

  auto head_packet = [&] {
    PerfettoTracePacket packet{};
    packet.set_trusted_packet_sequence_id(kTrustedPacketSequenceId);
    packet.set_previous_packet_dropped(true);
    packet.set_sequence_flags(
        PerfettoTracePacket::SEQ_INCREMENTAL_STATE_CLEARED);
    auto* trace_config = packet.mutable_trace_config();
    auto* data_source = trace_config->mutable_builtin_data_sources();
    data_source->set_primary_trace_clock(kPrimaryTraceClock);
    const auto& symbols_table = symbols.function_symbols_table();
    auto* interned_data = packet.mutable_interned_data();
    auto* event_names = interned_data->mutable_event_names();
    event_names->Reserve(gsl::narrow_cast<int>(called_function_ids.size()));
    auto* source_locations = interned_data->mutable_source_locations();
    source_locations->Reserve(
        gsl::narrow_cast<int>(called_function_ids.size()));
    std::for_each(
        std::cbegin(symbols_table), std::cend(symbols_table),
        [&](const auto& key_value) {
          const auto& [function_id, function_infos] = key_value;
          const auto& repeated_function_infos = function_infos.function_infos();
          if (!called_function_ids.contains(function_id)) return;
          event_names->Add(MakeEventName(function_id, repeated_function_infos));
          source_locations->Add(
              MakeSourceLocation(function_id, repeated_function_infos));
        });
    return packet;
  }();

  auto track_descriptor_packets =
      MakePerfettoTrackDescriptorPackets(trace_files);

  PerfettoTrace trace{};
  auto* packets = trace.mutable_packet();
  packets->Reserve(gsl::narrow_cast<int>(1 + track_descriptor_packets.size() +
                                         trace_files.size() + event_count));
  packets->Add(std::move(head_packet));
  std::move(std::begin(track_descriptor_packets),
            std::end(track_descriptor_packets),
            RepeatedFieldBackInserter(packets));
  std::for_each(
      std::cbegin(trace_files), std::cend(trace_files),
      [packets](const auto& trace_file) {
        packets->Add(MakePerfettoClockSnapshotTracePacket(trace_file.header));
        std::transform(
            std::cbegin(trace_file.events), std::cend(trace_file.events),
            RepeatedFieldBackInserter(packets),
            [&header = trace_file.header](const auto& event) {
              return MakePerfettoTrackEventTracePacket(header, event);
            });
      });
  return trace;
}

}  // namespace spoor::tools::adapters::perfetto
