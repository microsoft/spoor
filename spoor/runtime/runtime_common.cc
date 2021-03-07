// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <cstring>

#include "gsl/gsl"
#include "spoor/runtime/runtime.h"

auto _spoor_runtime_ReleaseTraceFilePaths(
    _spoor_runtime_TraceFiles* trace_files) -> void {
  gsl::span file_paths{trace_files->file_paths,
                       static_cast<std::size_t>(trace_files->file_paths_size)};
  for (auto* path : file_paths) {
    // clang-format off NOLINTNEXTLINE(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory) clang-format on
    free(path);
  }
  // clang-format off NOLINTNEXTLINE(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory) clang-format on
  free(trace_files->file_paths);
  trace_files->file_paths = nullptr;
  // clang-format off NOLINTNEXTLINE(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory) clang-format on
  free(trace_files->file_path_sizes);
  trace_files->file_path_sizes = nullptr;
  trace_files->file_paths_size = 0;
}

auto _spoor_runtime_DeletedFilesInfoEqual(
    const _spoor_runtime_DeletedFilesInfo* lhs,
    const _spoor_runtime_DeletedFilesInfo* rhs) -> bool {
  if (lhs == rhs) return true;
  if (lhs == nullptr || rhs == nullptr) return false;
  return lhs->deleted_files == rhs->deleted_files &&
         lhs->deleted_bytes == rhs->deleted_bytes;
}

auto _spoor_runtime_TraceFilesEqual(const _spoor_runtime_TraceFiles* lhs,
                                    const _spoor_runtime_TraceFiles* rhs)
    -> bool {
  if (lhs == rhs) return true;
  if (lhs == nullptr || rhs == nullptr) return false;
  if (lhs->file_paths_size != rhs->file_paths_size) return false;
  if (lhs->file_paths_size == 0) return true;
  if (lhs->file_path_sizes == nullptr || lhs->file_paths == nullptr ||
      rhs->file_path_sizes == nullptr || rhs->file_paths == nullptr) {
    return false;
  }
  const gsl::span<_spoor_runtime_SizeType> lhs_file_path_sizes{
      lhs->file_path_sizes, lhs->file_paths_size};
  const gsl::span<_spoor_runtime_SizeType> rhs_file_path_sizes{
      rhs->file_path_sizes, rhs->file_paths_size};
  const gsl::span<char*> lhs_file_paths{lhs->file_paths, lhs->file_paths_size};
  const gsl::span<char*> rhs_file_paths{rhs->file_paths, rhs->file_paths_size};
  for (_spoor_runtime_SizeType file_path{0}; file_path < lhs_file_paths.size();
       ++file_path) {
    if (lhs_file_path_sizes[file_path] != rhs_file_path_sizes[file_path]) {
      return false;
    }
    const auto result =
        std::strncmp(lhs_file_paths[file_path], rhs_file_paths[file_path],
                     lhs_file_path_sizes[file_path]);
    if (result != 0) return false;
  }
  return true;
}

auto _spoor_runtime_ConfigEqual(const _spoor_runtime_Config* lhs,
                                const _spoor_runtime_Config* rhs) -> bool {
  if (lhs == rhs) return true;
  if (lhs == nullptr || rhs == nullptr) return false;
  if (lhs->trace_file_path_size != rhs->trace_file_path_size) return false;
  const auto result = std::strncmp(lhs->trace_file_path, rhs->trace_file_path,
                                   lhs->trace_file_path_size);
  if (result != 0) return false;
  return lhs->session_id == rhs->session_id &&
         lhs->thread_event_buffer_capacity ==
             rhs->thread_event_buffer_capacity &&
         lhs->max_reserved_event_buffer_slice_capacity ==
             rhs->max_reserved_event_buffer_slice_capacity &&
         lhs->max_dynamic_event_buffer_slice_capacity ==
             rhs->max_dynamic_event_buffer_slice_capacity &&
         lhs->reserved_event_pool_capacity ==
             rhs->reserved_event_pool_capacity &&
         lhs->dynamic_event_pool_capacity == rhs->dynamic_event_pool_capacity &&
         lhs->dynamic_event_slice_borrow_cas_attempts ==
             rhs->dynamic_event_slice_borrow_cas_attempts &&
         lhs->event_buffer_retention_duration_nanoseconds ==
             rhs->event_buffer_retention_duration_nanoseconds &&
         lhs->max_flush_buffer_to_file_attempts ==
             rhs->max_flush_buffer_to_file_attempts &&
         lhs->flush_all_events == rhs->flush_all_events;
}

auto operator==(const _spoor_runtime_DeletedFilesInfo& lhs,
                const _spoor_runtime_DeletedFilesInfo& rhs) -> bool {
  return _spoor_runtime_DeletedFilesInfoEqual(&lhs, &rhs);
}

auto operator==(const _spoor_runtime_TraceFiles& lhs,
                const _spoor_runtime_TraceFiles& rhs) -> bool {
  return _spoor_runtime_TraceFilesEqual(&lhs, &rhs);
}

auto operator==(const _spoor_runtime_Config& lhs,
                const _spoor_runtime_Config& rhs) -> bool {
  return _spoor_runtime_ConfigEqual(&lhs, &rhs);
}
