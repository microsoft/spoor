#include <algorithm>
#include <cstdlib>
#include <execution>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "google/protobuf/repeated_field.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/util/message_differencer.h"
#include "gsl/gsl"
#include "protos/perfetto/trace/clock_snapshot.pb.h"
#include "protos/perfetto/trace/trace.pb.h"
#include "protos/perfetto/trace/trace_packet.pb.h"
#include "protos/perfetto/trace/track_event/source_location.pb.h"
#include "protos/perfetto/trace/track_event/thread_descriptor.pb.h"
#include "protos/perfetto/trace/track_event/track_descriptor.pb.h"
#include "protos/perfetto/trace/track_event/track_event.pb.h"
#include "spoor/proto/spoor.pb.h"
#include "spoor/runtime/trace/trace_file_reader.h"
#include "util/env/env.h"
#include "util/numeric.h"
#include "util/result.h"

using google::protobuf::util::MessageDifferencer;
using perfetto::protos::EventName;
using perfetto::protos::SourceLocation;
using perfetto::protos::ThreadDescriptor;
using perfetto::protos::Trace;
using perfetto::protos::TracePacket;
using perfetto::protos::TrackDescriptor;
using perfetto::protos::TrackEvent;
using spoor::FunctionInfo;
using spoor::InstrumentedFunctionMap;
using spoor::runtime::trace::FunctionId;
using spoor::runtime::trace::TraceFile;
using spoor::runtime::trace::TraceFileReader;
using SpoorEvent = spoor::runtime::trace::Event;
using SourceLocationMap = std::unordered_map<FunctionId, SourceLocation>;
using CompressionStrategy = util::compression::Strategy;

namespace spoor::tools {

constexpr uint32 kTrustedPacketSequenceId{1};
constexpr uint64 kLargest31BitPrime{2'147'483'647};
// constexpr uint64 kLargest32BitPrime{4'294'967'291};
constexpr std::string_view kUnknown{"unknown"};

enum class Error {
  kFailedToOpenFile,
  kFailedToParse,
};

auto ReadSpoorInstrumentedFunctionMap(const std::filesystem::path& path)
    -> util::result::Result<InstrumentedFunctionMap, Error> {
  std::ifstream file{};
  file.open(path);
  if (!file.is_open()) return Error::kFailedToOpenFile;

  InstrumentedFunctionMap instrumented_function_map{};
  const auto success = instrumented_function_map.ParseFromIstream(&file);
  if (!success) return Error::kFailedToParse;

  return instrumented_function_map;
}

auto ReadSpoorTraceFile(const std::filesystem::path& path)
    -> util::result::Result<TraceFile, Error> {
  std::ifstream file{};
  file.open(path);
  if (!file.is_open()) return Error::kFailedToOpenFile;

  // FileReader file_reader{};
  TraceFileReader trace_file_reader{
      {/*.file_reader = &file_reader, */ .initial_buffer_capacity = 0}};
  const auto trace_file_result = trace_file_reader.Read(path, true);
  if (trace_file_result.IsErr()) return Error::kFailedToParse;

  return trace_file_result.Ok();
}

auto ReduceSpoorInstrumentedFunctionMapInto(
    const InstrumentedFunctionMap& instrumented_function_map,
    std::unordered_map<FunctionId, std::vector<FunctionInfo>>*
        function_info_map) -> void {
  for (const auto& [function_id, function_info] :
       instrumented_function_map.function_map()) {
    auto iterator = function_info_map->find(function_id);
    if (iterator == std::cend(*function_info_map)) {
      function_info_map->insert({function_id, {function_info}});
    } else {
      // Function ID collisions are not expected.
      auto& existing_function_infos = iterator->second;
      const auto item = std::find_if(
          std::cbegin(existing_function_infos),
          std::cend(existing_function_infos),
          [function_info = function_info](const auto& existing_function_info) {
            return MessageDifferencer::Equals(function_info,
                                              existing_function_info);
          });
      if (item == std::cend(existing_function_infos)) {
        existing_function_infos.emplace_back(function_info);
      }
    }
  }
}

auto MakeFunctionName(const FunctionId function_id,
                      const FunctionInfo& function_info) -> std::string {
  if (function_info.has_demangled_name()) {
    return function_info.demangled_name();
  }
  if (function_info.has_linkage_name()) {
    return function_info.linkage_name();
  }
  return absl::StrFormat("%#016x", function_id);
};

auto MakeFunctionName(const FunctionId function_id,
                      const std::vector<FunctionInfo> function_infos)
    -> std::string {
  std::vector<std::string> function_names{};
  function_names.reserve(function_infos.size());
  std::transform(std::cbegin(function_infos), std::cend(function_infos),
                 std::back_inserter(function_names),
                 [function_id](const auto& function_info) {
                   return MakeFunctionName(function_id, function_info);
                 });
  return absl::StrJoin(function_names, "; ");
}

auto MakeFileName(const FunctionInfo& function_info, const bool append_line)
    -> std::string {
  std::filesystem::path path{};
  if (function_info.has_directory()) {
    path = function_info.directory();
    path.make_preferred();
  }
  if (function_info.has_file_name()) {
    path /= function_info.file_name();
    if (append_line && function_info.has_line() && function_info.line() != 0) {
      path += absl::StrFormat(":%d", function_info.line());
    }
  }
  if (path.empty()) return std::string{kUnknown};
  return path.string();
}

auto MakeFileName(const std::vector<FunctionInfo>& function_infos)
    -> std::string {
  std::vector<std::string> file_names{};
  file_names.reserve(function_infos.size());
  const auto append_line = function_infos.size() != 1;
  std::transform(std::cbegin(function_infos), std::cend(function_infos),
                 std::back_inserter(file_names),
                 [append_line](const auto& function_info) {
                   return MakeFileName(function_info, append_line);
                 });
  return absl::StrJoin(file_names, "; ");
}

auto TransformSpoorFunctionInfoMapToPerfettoSourceLocation(
    const std::pair<FunctionId, std::vector<FunctionInfo>>& function_info)
    -> SourceLocation {
  const auto& [function_id, function_infos] = function_info;

  SourceLocation source_location{};
  // source_location.set_iid(function_id);
  if (function_infos.empty()) return source_location;
  source_location.set_file_name(MakeFileName(function_infos));
  source_location.set_function_name(
      MakeFunctionName(function_id, function_infos));
  if (function_infos.size() == 1) {
    const auto& function_info = function_infos.front();
    if (function_info.has_line()) {
      source_location.set_line_number(function_info.line());
    }
  }

  return source_location;
}

auto TransformSpoorEventsToPerfettoTracePackets(
    const TraceFile& trace_file,
    const std::unordered_map<FunctionId, std::vector<FunctionInfo>>&
        function_info_map) -> std::vector<TracePacket> {
  std::vector<TracePacket> trace_packets{};
  trace_packets.reserve(trace_file.events.size());
  std::transform(
      std::cbegin(trace_file.events), std::cend(trace_file.events),
      std::back_inserter(trace_packets),
      [&trace_file, &function_info_map](const auto& spoor_event) {
        TracePacket trace_packet{};
        trace_packet.set_trusted_packet_sequence_id(kTrustedPacketSequenceId);
        // trace_packet.set_sequence_flags(
        //     TracePacket::SEQ_NEEDS_INCREMENTAL_STATE);

        auto* track_descriptor = trace_packet.mutable_track_descriptor();
        auto* thread_descriptor = track_descriptor->mutable_thread();
        thread_descriptor->set_pid(
            static_cast<int32>(trace_file.process_id % kLargest31BitPrime));
        thread_descriptor->set_tid(
            static_cast<int32>(trace_file.thread_id % kLargest31BitPrime));
        trace_packet.set_timestamp(spoor_event.steady_clock_timestamp);

        auto* track_event = trace_packet.mutable_track_event();
        switch (static_cast<SpoorEvent::Event::Type>(spoor_event.type)) {
          case SpoorEvent::Type::kFunctionEntry: {
            const auto function_id = spoor_event.payload_1;
            track_event->set_type(TrackEvent::TYPE_SLICE_BEGIN);
            auto it = function_info_map.find(function_id);
            if (it != std::cend(function_info_map)) {
              const auto function_infos = it->second;
              track_event->set_name(
                MakeFunctionName(function_id, function_infos));
            }
            // track_event->set_name_iid(function_id);
            track_event->set_source_location_iid(function_id);
            break;
          }
          case SpoorEvent::Type::kFunctionExit: {
            // const auto function_id = spoor_event.payload_1;
            track_event->set_type(TrackEvent::TYPE_SLICE_END);
            // track_event->set_name_iid(function_id);
            // track_event->set_source_location_iid(function_id);
            break;
          }
        }
        return trace_packet;
      });
  return trace_packets;
}

}  // namespace spoor::tools

auto main(const int argc, const char** argv) -> int {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <search_paths>\n";
    return EXIT_FAILURE;
  }

  std::vector<std::filesystem::path> function_map_files{};
  std::vector<std::filesystem::path> spoor_trace_files{};
  for (auto argi{1}; argi < argc; ++argi) {
    const auto* const arg = argv[argi];
    std::cerr << "Searching " << arg << '\n';
    std::filesystem::recursive_directory_iterator directory_iterator{
        arg, std::filesystem::directory_options::skip_permission_denied};
    for (const auto& directory_entry : directory_iterator) {
      if (!std::filesystem::is_regular_file(directory_entry)) continue;
      const auto& path = directory_entry.path();
      if (path.extension() == ".spoor_function_map") {  // TODO constant
        function_map_files.emplace_back(path);
      } else if (path.extension() == ".spoor_trace") {  // TODO constant
        spoor_trace_files.emplace_back(path);
      }
    }
  }

  std::cerr << "# function map files = " << function_map_files.size() << '\n';
  std::cerr << "# spoor trace files = " << spoor_trace_files.size() << '\n';

  std::vector<InstrumentedFunctionMap> instrumented_function_maps{};
  instrumented_function_maps.reserve(function_map_files.size());
  std::for_each(std::cbegin(function_map_files), std::cend(function_map_files),
                [&instrumented_function_maps](const auto& path) {
                  const auto result =
                      spoor::tools::ReadSpoorInstrumentedFunctionMap(path);
                  if (result.IsOk()) {
                    instrumented_function_maps.emplace_back(result.Ok());
                  } else {
                    switch (result.Err()) {
                      case spoor::tools::Error::kFailedToOpenFile: {
                        std::cerr << "Failed to open " << path << '\n';
                        break;
                      }
                      case spoor::tools::Error::kFailedToParse: {
                        std::cerr << "Failed to parse " << path << '\n';
                        break;
                      }
                    }
                  }
                });

  std::cerr << "# instrumented functions maps = "
            << instrumented_function_maps.size() << '\n';

  std::vector<TraceFile> trace_files{};
  trace_files.reserve(spoor_trace_files.size());
  std::for_each(std::cbegin(spoor_trace_files), std::cend(spoor_trace_files),
                [&trace_files](const auto& path) {
                  const auto result = spoor::tools::ReadSpoorTraceFile(path);
                  if (result.IsOk()) {
                    trace_files.emplace_back(result.Ok());
                  } else {
                    switch (result.Err()) {
                      case spoor::tools::Error::kFailedToOpenFile: {
                        std::cerr << "Failed to open " << path << '\n';
                        break;
                      }
                      case spoor::tools::Error::kFailedToParse: {
                        std::cerr << "Failed to parse " << path << '\n';
                        break;
                      }
                    }
                  }
                });
  std::cerr << "# trace files = " << trace_files.size() << '\n';
  const auto events_size = std::transform_reduce(
      std::cbegin(trace_files), std::cend(trace_files), 0,
      [](const auto size_a, const auto size_b) { return size_a + size_b; },
      [](const auto& trace_file) { return trace_file.events.size(); });
  std::cerr << "# events = " << events_size << '\n';

  std::unordered_set<FunctionId> called_function_ids{};
  std::for_each(
      std::cbegin(trace_files), std::cend(trace_files),
      [&called_function_ids](const auto& trace_file) {
        std::for_each(
            std::cbegin(trace_file.events), std::cend(trace_file.events),
            [&called_function_ids](const auto& event) {
              switch (static_cast<SpoorEvent::Event::Type>(event.type)) {
                case SpoorEvent::Type::kFunctionEntry: {
                  called_function_ids.emplace(event.payload_1);
                  break;
                }
                case SpoorEvent::Type::kFunctionExit: {
                  called_function_ids.emplace(event.payload_1);
                  break;
                }
              }
            });
      });
  std::cerr << "# called functions = " << called_function_ids.size() << '\n';

  std::unordered_map<FunctionId, std::vector<FunctionInfo>> function_info_map{};
  const auto function_info_size = std::transform_reduce(
      std::cbegin(instrumented_function_maps),
      std::cend(instrumented_function_maps), 0,
      [](const auto size_a, const auto size_b) { return size_a + size_b; },
      [](const auto& instrumented_function_map) {
        return instrumented_function_map.function_map().size();
      });
  function_info_map.reserve(function_info_size);
  std::for_each(std::cbegin(instrumented_function_maps),
                std::cend(instrumented_function_maps),
                [&function_info_map](const auto& instrumented_function_map) {
                  spoor::tools::ReduceSpoorInstrumentedFunctionMapInto(
                      instrumented_function_map, &function_info_map);
                });
  std::cerr << "# function infos = " << function_info_map.size() << '\n';

  TracePacket interned_data_trace_packet{};
  interned_data_trace_packet.set_trusted_packet_sequence_id(
      spoor::tools::kTrustedPacketSequenceId);
  // interned_data_trace_packet.set_sequence_flags(
  //     TracePacket::SEQ_NEEDS_INCREMENTAL_STATE);
  auto* interned_data = interned_data_trace_packet.mutable_interned_data();
  auto* event_names = interned_data->mutable_event_names();
  event_names->Reserve(function_info_map.size());
  std::for_each(std::cbegin(function_info_map), std::cend(function_info_map),
                [&called_function_ids, event_names](const auto& key_value) {
                  const auto& [function_id, function_info] = key_value;
                  if (!called_function_ids.contains(function_id)) return;
                  const auto name = spoor::tools::MakeFunctionName(
                      function_id, function_info);
                  EventName event_name{};
                  event_name.set_iid(function_id);
                  event_name.set_name(name);
                  event_names->Add(std::move(event_name));
                });
  std::cerr << "# event names = " << event_names->size() << '\n';
  auto* source_locations = interned_data->mutable_source_locations();
  source_locations->Reserve(function_info_map.size());
  std::for_each(
      std::cbegin(function_info_map), std::cend(function_info_map),
      [&called_function_ids, source_locations](const auto& key_value) {
        const auto& [function_id, _] = key_value;
        if (!called_function_ids.contains(function_id)) return;
        source_locations->Add(
            spoor::tools::TransformSpoorFunctionInfoMapToPerfettoSourceLocation(
                key_value));
      });
  // std::transform(
  //     std::cbegin(function_info_map), std::cend(function_info_map),
  //     RepeatedFieldBackInserter(source_locations),
  // spoor::tools::TransformSpoorFunctionInfoMapToPerfettoSourceLocation);
  std::cerr << "# source locations = " << source_locations->size() << '\n';

  Trace trace{};
  auto* packets = trace.mutable_packet();
  packets->Reserve(1 + events_size);
  packets->Add(std::move(interned_data_trace_packet));
  std::for_each(std::cbegin(trace_files), std::cend(trace_files),
                [packets, &function_info_map](const auto& trace_file) {
                  const auto new_packets =
                      spoor::tools::TransformSpoorEventsToPerfettoTracePackets(
                          trace_file, function_info_map);
                  // TODO transform and pass in extra info to avoid copy
                  std::copy(std::cbegin(new_packets), std::cend(new_packets),
                            RepeatedFieldBackInserter(packets));
                });
  std::cerr << "# trace packets = " << packets->size() << '\n';

  // std::string out{};
  // google::protobuf::TextFormat::PrintToString(trace, &out);
  // std::cout << out;

  // const std::filesystem::path output_path{
  //    "/Users/lelandjansen/Desktop/trace.perfetto"};
  // std::ofstream out_file{};
  // out_file.open(output_path);
  trace.SerializeToOstream(&std::cout);

  std::cerr << "done\n";

  return EXIT_SUCCESS;
}
