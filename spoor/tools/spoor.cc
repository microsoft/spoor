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
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "city_hash/city.h"
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

auto TransformSpoorFunctionInfoMapToPerfettoSourceLocation(
    const std::pair<FunctionId, std::vector<FunctionInfo>>& function_info)
    -> SourceLocation {
  const auto make_file_name =
      [](const FunctionInfo& function_info, const bool append_line) -> std::optional<std::string> {
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
    if (path.empty()) return {};
    return path.string();
  };
  const auto make_function_name =
      [](const FunctionInfo& function_info) -> std::optional<std::string> {
    if (function_info.has_demangled_name()) {
      return function_info.demangled_name();
    }
    if (function_info.has_linkage_name()) {
      return function_info.linkage_name();
    }
    return {};
  };

  const auto& [function_id, function_infos] = function_info;
  SourceLocation source_location{};
  // source_location.set_iid(function_id);
  if (function_infos.empty()) return source_location;

  std::vector<std::string> file_names{};
  file_names.reserve(function_infos.size());
  const auto append_line = function_infos.size() != 1;
  std::transform(std::cbegin(function_infos), std::cend(function_infos),
                 std::back_inserter(file_names),
                 [&](const auto& function_info) {
                   return make_file_name(function_info, append_line).value_or("unknown");
                 });
  const auto file_name = absl::StrJoin(file_names, "; ");
  source_location.set_file_name(file_name);

  std::vector<std::string> function_names{};
  function_names.reserve(function_infos.size());
  std::transform(std::cbegin(function_infos), std::cend(function_infos),
                 std::back_inserter(function_names),
                 [&](const auto& function_info) {
                   return make_function_name(function_info).value_or("unknown");
                 });
  const auto function_name = absl::StrJoin(function_names, "; ");
  source_location.set_function_name(function_name);

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
        trace_packet.set_trusted_packet_sequence_id(42);  // TODO
        auto* track_descriptor = trace_packet.mutable_track_descriptor();
        track_descriptor->set_uuid(42);
        auto* thread_descriptor = track_descriptor->mutable_thread();

        if (std::numeric_limits<int32>::max() < trace_file.process_id) {
          const auto* ptr =
              reinterpret_cast<const char*>(&trace_file.process_id);
          const auto process_id_hash =
              CityHash32(ptr, sizeof(trace_file.process_id));
          thread_descriptor->set_pid(static_cast<int32>(process_id_hash));
        } else {
          thread_descriptor->set_pid(
              gsl::narrow_cast<int32>(trace_file.process_id));
        }

        if (std::numeric_limits<int32>::max() < trace_file.thread_id) {
          const auto* ptr =
              reinterpret_cast<const char*>(&trace_file.thread_id);
          const auto thread_id_hash =
              CityHash32(ptr, sizeof(trace_file.thread_id));
          thread_descriptor->set_tid(static_cast<int32>(thread_id_hash));
        } else {
          thread_descriptor->set_tid(
              gsl::narrow_cast<int32>(trace_file.thread_id));
        }

        trace_packet.set_timestamp(spoor_event.steady_clock_timestamp);
        /// TODO user-defined steady clock
        // trace_packet.set_timestamp_clock_id(42);

        auto* track_event = trace_packet.mutable_track_event();
        switch (static_cast<SpoorEvent::Event::Type>(spoor_event.type)) {
          case SpoorEvent::Type::kFunctionEntry: {
            track_event->set_type(TrackEvent::TYPE_SLICE_BEGIN);
            // track_event->set_source_location_iid(spoor_event.payload_1);
            break;
          }
          case SpoorEvent::Type::kFunctionExit: {
            track_event->set_type(TrackEvent::TYPE_SLICE_END);
            // track_event->set_source_location_iid(spoor_event.payload_1);
            break;
          }
        }

        auto it = function_info_map.find(spoor_event.payload_1);
        if (it != std::cend(function_info_map)) {
          auto* track_source_location = track_event->mutable_source_location();
          auto source_location = 
              TransformSpoorFunctionInfoMapToPerfettoSourceLocation(*it);
          track_event->set_name(source_location.function_name());
          *track_source_location = std::move(source_location);
        }

        return trace_packet;
      });
  return trace_packets;
}

// auto SourceLocationsFromSpoorFunctionMap(const std::filesystem::path& path)
//     -> SourceLocationMap {
//   spoor::InstrumentedFunctionMap instrumented_function_map{};
//   const auto success = instrumented_function_map.ParseFromIstream(&file);
//   if (!success) {
//     std::cerr << "Failed to parse " << path << '\n';
//     return {};
//   }
//
//   const auto& function_map = instrumented_function_map.function_map();
//   SourceLocationMap source_location_map{};
//   source_location_map.reserve(function_map.size());
//   std::transform(
//       std::cbegin(function_map), std::cend(function_map),
//       std::inserter(source_location_map, std::end(source_location_map)),
//       [](const auto& key_value) -> SourceLocationMap::value_type {
//         const auto& [function_id, function_info] = key_value;
//         SourceLocation source_location{};
//         source_location.set_iid(function_id);
//         source_location.set_file_name(function_info.file_name());
//         if (function_info.demangled_name().empty()) {
//           source_location.set_function_name(function_info.demangled_name());
//         } else {
//           source_location.set_function_name(function_info.linkage_name());
//         }
//         source_location.set_line_number(function_info.line());
//         return {function_id, std::move(source_location)};
//       });
//   return source_location_map;
//   // for (const auto& [function_id, function_info] : function_map) {
//   //   SourceLocation source_location{};
//   //   source_location.set_iid(function_id);
//   //   source_location.set_file_name(function_info.file_name());
//   //   if (function_info.demangled_name().empty()) {
//   //     source_location.set_function_name(function_info.demangled_name());
//   //   } else {
//   //     source_location.set_function_name(function_info.linkage_name());
//   //   }
//   //   source_location.set_line_number(function_info.line());
//   //   source_locations.emplace_back(source_location);
//   // }
//   // return source_locations;
// }
//
// auto ParseTrace(const std::filesystem::path& path) ->
// std::vector<TracePacket> {
//   std::ifstream file{};
//   file.open(path);
//   if (!file.is_open()) {
//     std::cerr << "Failed to open " << path << '\n';
//     return {};
//   }
//
//   // FileReader file_reader{};
//   TraceFileReader trace_file_reader{
//       {/*.file_reader = &file_reader, */ .initial_buffer_capacity = 0}};
//   const auto trace_file_result = trace_file_reader.Read(path, true);
//   if (trace_file_result.IsErr()) {
//     switch (trace_file_result.Err()) {
//       case spoor::runtime::trace::TraceReader::Error::kFailedToOpenFile: {
//         std::cout << "failed to open file\n";
//         break;
//       }
//       case spoor::runtime::trace::TraceReader::Error::kMalformedFile: {
//         std::cout << "malformed file\n";
//         break;
//       }
//       case spoor::runtime::trace::TraceReader::Error::kMismatchedMagicNumber:
//       {
//         std::cout << "mismatched magic number\n";
//         break;
//       }
//       case spoor::runtime::trace::TraceReader::Error::kUnknownVersion: {
//         std::cout << "unknown version\n";
//         break;
//       }
//       case spoor::runtime::trace::TraceReader::Error::kUncompressError: {
//         std::cout << "uncopress error\n";
//         break;
//       }
//     }
//     std::cerr << "Failed to parse " << path << '\n';
//     return {};
//   }
//   const auto& trace_file = trace_file_result.Ok();
//
//   std::vector<TracePacket> trace_packets{};
//   trace_packets.reserve(trace_file.events.size());
//   for (const auto spoor_event : trace_file.events) {  // TODO transform
//     TracePacket trace_packet{};
//     trace_packet.set_trusted_packet_sequence_id(42);  // TODO
//     auto* track_descriptor = trace_packet.mutable_track_descriptor();
//     track_descriptor->set_uuid(42);
//     auto* thread_descriptor = track_descriptor->mutable_thread();
//     thread_descriptor->set_pid(trace_file.process_id);  // TODO narrowing
//     thread_descriptor->set_tid(trace_file.thread_id);   // TODO narrowing
//
//     trace_packet.set_timestamp(spoor_event.steady_clock_timestamp);
//     // trace_packet.set_timestamp_clock_id(42);  // TODO user-defined for
//     steady
//     // clock
//
//     auto* track_event = trace_packet.mutable_track_event();
//     switch (static_cast<SpoorEvent::Event::Type>(spoor_event.type)) {
//       case SpoorEvent::Type::kFunctionEntry: {
//         track_event->set_type(TrackEvent::TYPE_SLICE_BEGIN);
//         track_event->set_source_location_iid(spoor_event.payload_1);
//         break;
//       }
//       case SpoorEvent::Type::kFunctionExit: {
//         track_event->set_type(TrackEvent::TYPE_SLICE_END);
//         track_event->set_source_location_iid(spoor_event.payload_1);
//         break;
//       }
//     }
//     trace_packets.emplace_back(trace_packet);
//   }
//   return trace_packets;
// }

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

  // TracePacket interned_data_trace_packet{};
  // interned_data_trace_packet.set_trusted_packet_sequence_id(42);  // TODO
  // auto* interned_data = interned_data_trace_packet.mutable_interned_data();
  // auto* source_locations = interned_data->mutable_source_locations();
  // source_locations->Reserve(function_info_map.size());
  // std::for_each(
  //     std::cbegin(function_info_map), std::cend(function_info_map),
  //     [&called_function_ids, source_locations](const auto& key_value) {
  //       const auto& [function_id, _] = key_value;
  //       if (!called_function_ids.contains(function_id)) return;
  //       source_locations->Add(
  //           spoor::tools::TransformSpoorFunctionInfoMapToPerfettoSourceLocation(
  //               key_value));
  //     });
  // // std::transform(
  // //     std::cbegin(function_info_map), std::cend(function_info_map),
  // //     RepeatedFieldBackInserter(source_locations),
  // //     spoor::tools::TransformSpoorFunctionInfoMapToPerfettoSourceLocation);
  // std::cerr << "# source locations = " << source_locations->size() << '\n';

  Trace trace{};
  auto* packets = trace.mutable_packet();
  packets->Reserve(/*1 + */events_size);
  // packets->Add(std::move(interned_data_trace_packet));
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

  std::string out{};
  google::protobuf::TextFormat::PrintToString(trace, &out);
  std::cout << out;

  const std::filesystem::path output_path{
      "/Users/lelandjansen/Desktop/trace.perfetto"};
  std::ofstream out_file{};
  out_file.open(output_path);
  trace.SerializeToOstream(&out_file);

  std::cerr << "done\n";

  return EXIT_SUCCESS;

  /*

  // struct Reducer : std::binary_function<SourceLocationMap, SourceLocationMap,
  //                                       SourceLocationMap> {
  //   auto operator()(SourceLocationMap x, const SourceLocationMap& y) const
  //       -> SourceLocationMap {
  //     x.reserve(x.size() + y.size());
  //     std::copy(std::cbegin(y), std::cend(y), std::inserter(x, std::end(x)));
  //     return x;
  //   }
  // };
  // SourceLocationMap r{};
  //

  // std::cerr << "Generating source location maps\n";
  // const SourceLocationMap source_location_map = std::transform_reduce(
  //     // std::execution::par,
  //     std::cbegin(function_map_files), std::cend(function_map_files),
  //     SourceLocationMap{},
  //     [](const SourceLocationMap& x, SourceLocationMap y) ->
  //     SourceLocationMap {
  //       std::cerr << "reducing " << x.size() << ", " << y.size() << '\n';
  //       y.reserve(x.size() + y.size());
  //       std::copy(std::cbegin(x), std::cend(x), std::inserter(y,
  //       std::end(y))); return y;
  //     },
  //     spoor::tools::SourceLocationsFromSpoorFunctionMap);

  // TODO std::execution::par + mutex
  SourceLocationMap source_location_map{};
  std::for_each(
      std::cbegin(function_map_files), std::cend(function_map_files),
      [&source_location_map](const auto& path) {
        const auto map =
            spoor::tools::SourceLocationsFromSpoorFunctionMap(path);
        std::copy_if(
            std::cbegin(map), std::cend(map),
            std::inserter(source_location_map, std::end(source_location_map)),
            [&source_location_map](const auto& key_value) {
              const auto& [function_id, data] = key_value;
              auto iterator = source_location_map.find(function_id);
              if (iterator == std::cend(source_location_map)) return true;
              const auto& [_, new_data] = *iterator;
              if (!MessageDifferencer::Equals(data, new_data)) {
                std::cerr << "Conflicting function ID " << function_id << '\n';
              }
              return false;
            });
      });
  std::cout << "# source locations = " << source_location_map.size() << '\n';

  // TODO std::execution::par + mutex
  std::vector<TracePacket> trace_packets{};
  std::unordered_set<FunctionId> encountered_function_ids{};
  std::for_each(std::cbegin(spoor_trace_files), std::cend(spoor_trace_files),
                [&trace_packets, &encountered_function_ids](const auto& path) {
                  const auto packets = spoor::tools::ParseTrace(path);
                  std::copy(std::cbegin(packets), std::cend(packets),
                            std::back_inserter(trace_packets));
                  std::transform(
                      std::cbegin(packets), std::cend(packets),
                      std::inserter(encountered_function_ids,
                                    std::end(encountered_function_ids)),
                      [](const auto& packet) {
                        return packet.track_event().source_location_iid();
                      });
                });

  std::cout << "# events = " << trace_packets.size() << '\n';
  std::cout << "# encountered_functions = " << encountered_function_ids.size()
            << '\n';

  const auto missing_events =
      std::count_if(std::cbegin(encountered_function_ids),
                    std::cend(encountered_function_ids),
                    [&source_location_map](const auto function_id) {
                      return !source_location_map.contains(function_id);
                    });
  std::cout << "# missing events = " << missing_events << '\n';

  // for (auto argi{1}; argi < argc; ++argi) {
  //   const auto* const arg = argv[argi];
  //   // std::cerr << "Searching " << arg << '\n';

  //   std::filesystem::recursive_directory_iterator directory_iterator{
  //       arg, std::filesystem::directory_options::skip_permission_denied};
  //   for (const auto& directory_entry : directory_iterator) {
  //     if (!std::filesystem::is_regular_file(directory_entry)) continue;
  //     const auto& path = directory_entry.path();
  //     if (path.extension() == ".spoor_function_map") {  // TODO use constant
  //       const auto file_source_locations =
  //       spoor::tools::ParseFunctionMap(path);
  //       std::copy(std::cbegin(file_source_locations),
  //                 std::cend(file_source_locations),
  //                 RepeatedFieldBackInserter(source_locations));
  //     }
  //     if (path.extension() == ".spoor_trace") {  // TODO use constant
  //       const auto file_trace_packets = spoor::tools::ParseTrace(path);
  //       std::copy(std::cbegin(file_trace_packets),
  //                 std::cend(file_trace_packets),
  //                 std::back_inserter(trace_packets));
  //     }
  //   }
  // }

  // std::cout << "source locations = " << source_locations->size() << '\n';
  // std::cout << "trace packets = " << trace_packets.size() << '\n';
  // */

  // const std::filesystem::path output_path{
  //     "/Users/lelandjansen/Desktop/trace.perfetto"};
  // std::ofstream out_file{};
  // out_file.open(output_path);
  // trace.SerializeToOstream(&out_file);

  // std::cout << "Wrote files to " << output_path << '\n';
}
