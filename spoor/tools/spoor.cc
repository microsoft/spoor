#include <algorithm>
#include <cstdlib>
#include <execution>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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

auto ReadInstrumentedFunctionMap(const std::filesystem::path& path)
    -> util::result::Result<InstrumentedFunctionMap, Error> {
  std::ifstream file{};
  file.open(path);
  if (!file.is_open()) return Error::kFailedToOpenFile;

  InstrumentedFunctionMap instrumented_function_map{};
  const auto success = instrumented_function_map.ParseFromIstream(&file);
  if (!success) return Error::kFailedToParse;

  return instrumented_function_map;
}

auto ReadTraceFile(const std::filesystem::path& path)
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

auto ReduceInstrumentedFunctionMapInto(
    gsl::not_null<
        std::unordered_map<FunctionId, std::unordered_set<FunctionInfo>>*>
        function_info_map) -> void {
}

auto SourceLocationsFromSpoorFunctionMap(const std::filesystem::path& path)
    -> util::SourceLocationMap {
  spoor::InstrumentedFunctionMap instrumented_function_map{};
  const auto success = instrumented_function_map.ParseFromIstream(&file);
  if (!success) {
    std::cerr << "Failed to parse " << path << '\n';
    return {};
  }

  const auto& function_map = instrumented_function_map.function_map();
  SourceLocationMap source_location_map{};
  source_location_map.reserve(function_map.size());
  std::transform(
      std::cbegin(function_map), std::cend(function_map),
      std::inserter(source_location_map, std::end(source_location_map)),
      [](const auto& key_value) -> SourceLocationMap::value_type {
        const auto& [function_id, function_info] = key_value;
        SourceLocation source_location{};
        source_location.set_iid(function_id);
        source_location.set_file_name(function_info.file_name());
        if (function_info.demangled_name().empty()) {
          source_location.set_function_name(function_info.demangled_name());
        } else {
          source_location.set_function_name(function_info.linkage_name());
        }
        source_location.set_line_number(function_info.line());
        return {function_id, std::move(source_location)};
      });
  return source_location_map;
  // for (const auto& [function_id, function_info] : function_map) {
  //   SourceLocation source_location{};
  //   source_location.set_iid(function_id);
  //   source_location.set_file_name(function_info.file_name());
  //   if (function_info.demangled_name().empty()) {
  //     source_location.set_function_name(function_info.demangled_name());
  //   } else {
  //     source_location.set_function_name(function_info.linkage_name());
  //   }
  //   source_location.set_line_number(function_info.line());
  //   source_locations.emplace_back(source_location);
  // }
  // return source_locations;
}

auto ParseTrace(const std::filesystem::path& path) -> std::vector<TracePacket> {
  std::ifstream file{};
  file.open(path);
  if (!file.is_open()) {
    std::cerr << "Failed to open " << path << '\n';
    return {};
  }

  // FileReader file_reader{};
  TraceFileReader trace_file_reader{
      {/*.file_reader = &file_reader, */ .initial_buffer_capacity = 0}};
  const auto trace_file_result = trace_file_reader.Read(path, true);
  if (trace_file_result.IsErr()) {
    switch (trace_file_result.Err()) {
      case spoor::runtime::trace::TraceReader::Error::kFailedToOpenFile: {
        std::cout << "failed to open file\n";
        break;
      }
      case spoor::runtime::trace::TraceReader::Error::kMalformedFile: {
        std::cout << "malformed file\n";
        break;
      }
      case spoor::runtime::trace::TraceReader::Error::kMismatchedMagicNumber: {
        std::cout << "mismatched magic number\n";
        break;
      }
      case spoor::runtime::trace::TraceReader::Error::kUnknownVersion: {
        std::cout << "unknown version\n";
        break;
      }
      case spoor::runtime::trace::TraceReader::Error::kUncompressError: {
        std::cout << "uncopress error\n";
        break;
      }
    }
    std::cerr << "Failed to parse " << path << '\n';
    return {};
  }
  const auto& trace_file = trace_file_result.Ok();

  std::vector<TracePacket> trace_packets{};
  trace_packets.reserve(trace_file.events.size());
  for (const auto spoor_event : trace_file.events) {  // TODO transform
    TracePacket trace_packet{};
    trace_packet.set_trusted_packet_sequence_id(42);  // TODO
    auto* track_descriptor = trace_packet.mutable_track_descriptor();
    track_descriptor->set_uuid(42);
    auto* thread_descriptor = track_descriptor->mutable_thread();
    thread_descriptor->set_pid(trace_file.process_id);  // TODO narrowing
    thread_descriptor->set_tid(trace_file.thread_id);   // TODO narrowing

    trace_packet.set_timestamp(spoor_event.steady_clock_timestamp);
    // trace_packet.set_timestamp_clock_id(42);  // TODO user-defined for steady
    // clock

    auto* track_event = trace_packet.mutable_track_event();
    switch (static_cast<SpoorEvent::Event::Type>(spoor_event.type)) {
      case SpoorEvent::Type::kFunctionEntry: {
        track_event->set_type(TrackEvent::TYPE_SLICE_BEGIN);
        track_event->set_source_location_iid(spoor_event.payload_1);
        break;
      }
      case SpoorEvent::Type::kFunctionExit: {
        track_event->set_type(TrackEvent::TYPE_SLICE_END);
        track_event->set_source_location_iid(spoor_event.payload_1);
        break;
      }
    }
    trace_packets.emplace_back(trace_packet);
  }
  return trace_packets;
}

}  // namespace spoor::tools

auto main(const int argc, const char** argv) -> int {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <search_paths>\n";
    return EXIT_FAILURE;
  }

  // TracePacket interned_data_trace_packet{};
  // interned_data_trace_packet.set_trusted_packet_sequence_id(42);  // TODO
  // auto* interned_data = interned_data_trace_packet.mutable_interned_data();
  // auto* source_locations = interned_data->mutable_source_locations();

  // std::vector<TracePacket> trace_packets{};

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
      if (path.extension() == ".spoor_function_map") {  // TODO use constant
        function_map_files.emplace_back(path);
      } else if (path.extension() == ".spoor_trace") {  // TODO use constant
        spoor_trace_files.emplace_back(path);
      }
    }
  }

  std::cerr << "# function map files = " << function_map_files.size() << '\n';
  std::cerr << "# spoor trace files = " << spoor_trace_files.size() << '\n';

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

  // Trace trace{};
  // auto* packets = trace.mutable_packet();
  // packets->Reserve(trace_packets.size() + 1);
  // packets->Add(std::move(interned_data_trace_packet));
  // std::copy(std::cbegin(trace_packets), std::cend(trace_packets),
  //           RepeatedFieldBackInserter(packets));

  // const std::filesystem::path output_path{
  //     "/Users/lelandjansen/Desktop/trace.perfetto"};
  // std::ofstream out_file{};
  // out_file.open(output_path);
  // trace.SerializeToOstream(&out_file);

  // std::cout << "Wrote files to " << output_path << '\n';
}
