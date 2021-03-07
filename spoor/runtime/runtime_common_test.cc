// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <array>
#include <string>

#include "gsl/gsl"
#include "gtest/gtest.h"
#include "spoor/runtime/runtime.h"

namespace {

TEST(Runtime, ReleaseTraceFilePaths) {  // NOLINT
  constexpr _spoor_runtime_TraceFiles empty_trace_files{
      .file_paths_size = 0, .file_path_sizes = nullptr, .file_paths = nullptr};
  constexpr auto path_a_size{5};
  constexpr auto path_b_size{10};
  constexpr auto paths_size{2};
  // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
  gsl::span<char> path_a{static_cast<char*>(malloc(sizeof(char) * path_a_size)),
                         path_a_size};
  // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
  gsl::span<char> path_b{static_cast<char*>(malloc(sizeof(char) * path_b_size)),
                         path_b_size};
  gsl::span<_spoor_runtime_SizeType> path_sizes{
      static_cast<_spoor_runtime_SizeType*>(
          // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
          malloc(sizeof(_spoor_runtime_SizeType) * paths_size)),
      paths_size};
  gsl::span<char*> paths{
      // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
      static_cast<char**>(malloc(sizeof(char*) * paths_size)), paths_size};
  paths[0] = path_a.data();
  paths[1] = path_b.data();
  _spoor_runtime_TraceFiles trace_files{.file_paths_size = 2,
                                        .file_path_sizes = path_sizes.data(),
                                        .file_paths = paths.data()};
  ASSERT_NE(trace_files, empty_trace_files);
  _spoor_runtime_ReleaseTraceFilePaths(&trace_files);
  ASSERT_EQ(trace_files, empty_trace_files);
  _spoor_runtime_ReleaseTraceFilePaths(&trace_files);
  ASSERT_EQ(trace_files, empty_trace_files);
}

TEST(Runtime, TraceFilesEquality) {  // NOLINT
  std::string path_a{"/path/a"};
  std::string path_b{"/path/b"};
  std::string path_c{"/path/to/c"};
  std::string path_d{};
  std::string path_e{"/path/a"};
  ASSERT_EQ(path_a, path_e);
  std::array<_spoor_runtime_SizeType, 4> file_path_sizes_a{
      {path_a.size(), path_b.size(), path_c.size(), path_d.size()}};
  std::array<char*, 4> file_paths_a{
      {path_a.data(), path_b.data(), path_c.data(), path_d.data()}};
  ASSERT_NE(file_paths_a.size(), 0);
  ASSERT_EQ(file_path_sizes_a.size(), file_paths_a.size());
  std::array<_spoor_runtime_SizeType, 1> file_path_sizes_b{{path_e.size()}};
  std::array<char*, 1> file_paths_b{{path_e.data()}};
  _spoor_runtime_TraceFiles trace_files_a{
      .file_paths_size = file_paths_a.size(),
      .file_path_sizes = file_path_sizes_a.data(),
      .file_paths = file_paths_a.data()};
  _spoor_runtime_TraceFiles trace_files_b{
      .file_paths_size = file_paths_a.size(),
      .file_path_sizes = file_path_sizes_a.data(),
      .file_paths = file_paths_a.data()};
  _spoor_runtime_TraceFiles trace_files_c{
      .file_paths_size = 1,
      .file_path_sizes = file_path_sizes_a.data(),
      .file_paths = file_paths_a.data()};
  _spoor_runtime_TraceFiles trace_files_d{
      .file_paths_size = file_paths_b.size(),
      .file_path_sizes = file_path_sizes_b.data(),
      .file_paths = file_paths_b.data()};
  _spoor_runtime_TraceFiles malformed_trace_files_e{
      .file_paths_size = file_paths_a.size(),
      .file_path_sizes = nullptr,
      .file_paths = nullptr};
  _spoor_runtime_TraceFiles malformed_trace_files_f{
      .file_paths_size = file_paths_a.size(),
      .file_path_sizes = nullptr,
      .file_paths = file_paths_a.data()};
  // Note: ASAN will fail when testing a struct whose `file_paths_size` is
  // greater than the size of `file_paths_size` or `file_paths`.
  ASSERT_TRUE(_spoor_runtime_TraceFilesEqual(nullptr, nullptr));
  ASSERT_FALSE(_spoor_runtime_TraceFilesEqual(&trace_files_a, nullptr));
  ASSERT_FALSE(_spoor_runtime_TraceFilesEqual(nullptr, &trace_files_a));
  ASSERT_EQ(trace_files_a, trace_files_a);
  ASSERT_EQ(trace_files_a, trace_files_b);
  ASSERT_NE(trace_files_a, trace_files_c);
  ASSERT_NE(trace_files_a, trace_files_d);
  ASSERT_EQ(trace_files_c, trace_files_d);
  ASSERT_EQ(malformed_trace_files_e, malformed_trace_files_e);
  ASSERT_NE(trace_files_a, malformed_trace_files_e);
  ASSERT_NE(trace_files_a, malformed_trace_files_f);
  ASSERT_NE(malformed_trace_files_e, malformed_trace_files_f);
}

TEST(Runtime, DeletedFilesInfoEquality) {  // NOLINT
  constexpr _spoor_runtime_DeletedFilesInfo info_a{.deleted_files = 1,
                                                   .deleted_bytes = 2};
  constexpr _spoor_runtime_DeletedFilesInfo info_b{.deleted_files = 1,
                                                   .deleted_bytes = 2};
  constexpr _spoor_runtime_DeletedFilesInfo info_c{.deleted_files = 0,
                                                   .deleted_bytes = 0};
  ASSERT_TRUE(_spoor_runtime_DeletedFilesInfoEqual(nullptr, nullptr));
  ASSERT_FALSE(_spoor_runtime_DeletedFilesInfoEqual(&info_a, nullptr));
  ASSERT_FALSE(_spoor_runtime_DeletedFilesInfoEqual(nullptr, &info_a));
  ASSERT_EQ(info_a, info_a);
  ASSERT_EQ(info_a, info_b);
  ASSERT_NE(info_a, info_c);
}

TEST(Runtime, ConfigEquality) {  // NOLINT
  constexpr std::string_view path_a{"/path/to/file"};
  constexpr _spoor_runtime_Config config_a{
      .trace_file_path_size = path_a.size(),
      .trace_file_path = path_a.data(),
      .session_id = 1,
      .thread_event_buffer_capacity = 2,
      .max_reserved_event_buffer_slice_capacity = 3,
      .max_dynamic_event_buffer_slice_capacity = 4,
      .reserved_event_pool_capacity = 5,
      .dynamic_event_pool_capacity = 6,
      .dynamic_event_slice_borrow_cas_attempts = 7,
      .event_buffer_retention_duration_nanoseconds = 8,
      .max_flush_buffer_to_file_attempts = 9,
      .flush_all_events = true};
  constexpr _spoor_runtime_Config config_b{
      .trace_file_path_size = path_a.size(),
      .trace_file_path = path_a.data(),
      .session_id = 1,
      .thread_event_buffer_capacity = 2,
      .max_reserved_event_buffer_slice_capacity = 3,
      .max_dynamic_event_buffer_slice_capacity = 4,
      .reserved_event_pool_capacity = 5,
      .dynamic_event_pool_capacity = 6,
      .dynamic_event_slice_borrow_cas_attempts = 7,
      .event_buffer_retention_duration_nanoseconds = 8,
      .max_flush_buffer_to_file_attempts = 9,
      .flush_all_events = true};
  constexpr std::string_view path_c{"/path/to/file"};
  ASSERT_EQ(path_a, path_c);
  constexpr _spoor_runtime_Config config_c{
      .trace_file_path_size = path_c.size(),
      .trace_file_path = path_c.data(),
      .session_id = 1,
      .thread_event_buffer_capacity = 2,
      .max_reserved_event_buffer_slice_capacity = 3,
      .max_dynamic_event_buffer_slice_capacity = 4,
      .reserved_event_pool_capacity = 5,
      .dynamic_event_pool_capacity = 6,
      .dynamic_event_slice_borrow_cas_attempts = 7,
      .event_buffer_retention_duration_nanoseconds = 8,
      .max_flush_buffer_to_file_attempts = 9,
      .flush_all_events = true};
  constexpr std::string_view path_d{"xxxxxxxxxxxxx"};
  ASSERT_EQ(path_a.size(), path_d.size());
  constexpr _spoor_runtime_Config config_d{
      .trace_file_path_size = path_d.size(),
      .trace_file_path = path_d.data(),
      .session_id = 1,
      .thread_event_buffer_capacity = 2,
      .max_reserved_event_buffer_slice_capacity = 3,
      .max_dynamic_event_buffer_slice_capacity = 4,
      .reserved_event_pool_capacity = 5,
      .dynamic_event_pool_capacity = 6,
      .dynamic_event_slice_borrow_cas_attempts = 7,
      .event_buffer_retention_duration_nanoseconds = 8,
      .max_flush_buffer_to_file_attempts = 9,
      .flush_all_events = true};
  constexpr _spoor_runtime_Config config_e{
      .trace_file_path_size = path_a.size(),
      .trace_file_path = path_a.data(),
      .session_id = 0,
      .thread_event_buffer_capacity = 0,
      .max_reserved_event_buffer_slice_capacity = 0,
      .max_dynamic_event_buffer_slice_capacity = 0,
      .reserved_event_pool_capacity = 0,
      .dynamic_event_pool_capacity = 0,
      .dynamic_event_slice_borrow_cas_attempts = 0,
      .event_buffer_retention_duration_nanoseconds = 0,
      .max_flush_buffer_to_file_attempts = 0,
      .flush_all_events = false};
  constexpr _spoor_runtime_Config malformed_config_f{
      .trace_file_path_size = 1,
      .trace_file_path = nullptr,
      .session_id = 1,
      .thread_event_buffer_capacity = 2,
      .max_reserved_event_buffer_slice_capacity = 3,
      .max_dynamic_event_buffer_slice_capacity = 4,
      .reserved_event_pool_capacity = 5,
      .dynamic_event_pool_capacity = 6,
      .dynamic_event_slice_borrow_cas_attempts = 7,
      .event_buffer_retention_duration_nanoseconds = 8,
      .max_flush_buffer_to_file_attempts = 9,
      .flush_all_events = true};
  constexpr _spoor_runtime_Config malformed_config_g{
      .trace_file_path_size = 1,
      .trace_file_path = path_a.data(),
      .session_id = 1,
      .thread_event_buffer_capacity = 2,
      .max_reserved_event_buffer_slice_capacity = 3,
      .max_dynamic_event_buffer_slice_capacity = 4,
      .reserved_event_pool_capacity = 5,
      .dynamic_event_pool_capacity = 6,
      .dynamic_event_slice_borrow_cas_attempts = 7,
      .event_buffer_retention_duration_nanoseconds = 8,
      .max_flush_buffer_to_file_attempts = 9,
      .flush_all_events = true};
  ASSERT_TRUE(_spoor_runtime_ConfigEqual(nullptr, nullptr));
  ASSERT_FALSE(_spoor_runtime_ConfigEqual(&config_a, nullptr));
  ASSERT_FALSE(_spoor_runtime_ConfigEqual(nullptr, &config_a));
  ASSERT_EQ(config_a, config_a);
  ASSERT_EQ(config_a, config_b);
  ASSERT_EQ(config_a, config_c);
  ASSERT_NE(config_a, config_d);
  ASSERT_NE(config_a, config_e);
  ASSERT_NE(config_a, malformed_config_f);
  ASSERT_NE(config_a, malformed_config_g);
}

}  // namespace
