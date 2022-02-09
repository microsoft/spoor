// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <array>
#include <string>

#include "gtest/gtest.h"
#include "spoor/runtime/runtime.h"

namespace {

TEST(Runtime, DeletedFilesInfoEquality) {  // NOLINT
  constexpr spoor::runtime::DeletedFilesInfo info_a{
      .deleted_files = 1,
      .deleted_bytes = 2,
  };
  constexpr spoor::runtime::DeletedFilesInfo info_b{
      .deleted_files = 1,
      .deleted_bytes = 2,
  };
  constexpr spoor::runtime::DeletedFilesInfo info_c{
      .deleted_files = 0,
      .deleted_bytes = 0,
  };
  ASSERT_EQ(info_a, info_a);
  ASSERT_EQ(info_a, info_b);
  ASSERT_NE(info_a, info_c);
}

TEST(Runtime, ConfigEquality) {  // NOLINT
  constexpr std::string_view path_a{"/path/to/file"};
  constexpr std::string_view path_c{"/path/to/file"};
  constexpr std::string_view path_d{"xxxxxxxxxxxxx"};
  ASSERT_EQ(path_a, path_c);
  ASSERT_EQ(path_a.size(), path_d.size());

  const spoor::runtime::Config config_a{
      .trace_file_path = path_a,
      .session_id = 1,
      .thread_event_buffer_capacity = 2,
      .max_reserved_event_buffer_slice_capacity = 3,
      .max_dynamic_event_buffer_slice_capacity = 4,
      .reserved_event_pool_capacity = 5,
      .dynamic_event_pool_capacity = 6,
      .dynamic_event_slice_borrow_cas_attempts = 7,
      .event_buffer_retention_duration_nanoseconds = 8,
      .max_flush_buffer_to_file_attempts = 9,
      .flush_all_events = true,
  };
  const spoor::runtime::Config config_b{
      .trace_file_path = path_a,
      .session_id = 1,
      .thread_event_buffer_capacity = 2,
      .max_reserved_event_buffer_slice_capacity = 3,
      .max_dynamic_event_buffer_slice_capacity = 4,
      .reserved_event_pool_capacity = 5,
      .dynamic_event_pool_capacity = 6,
      .dynamic_event_slice_borrow_cas_attempts = 7,
      .event_buffer_retention_duration_nanoseconds = 8,
      .max_flush_buffer_to_file_attempts = 9,
      .flush_all_events = true,
  };
  const spoor::runtime::Config config_c{
      .trace_file_path = path_c,
      .session_id = 1,
      .thread_event_buffer_capacity = 2,
      .max_reserved_event_buffer_slice_capacity = 3,
      .max_dynamic_event_buffer_slice_capacity = 4,
      .reserved_event_pool_capacity = 5,
      .dynamic_event_pool_capacity = 6,
      .dynamic_event_slice_borrow_cas_attempts = 7,
      .event_buffer_retention_duration_nanoseconds = 8,
      .max_flush_buffer_to_file_attempts = 9,
      .flush_all_events = true,
  };
  const spoor::runtime::Config config_d{
      .trace_file_path = path_d,
      .session_id = 1,
      .thread_event_buffer_capacity = 2,
      .max_reserved_event_buffer_slice_capacity = 3,
      .max_dynamic_event_buffer_slice_capacity = 4,
      .reserved_event_pool_capacity = 5,
      .dynamic_event_pool_capacity = 6,
      .dynamic_event_slice_borrow_cas_attempts = 7,
      .event_buffer_retention_duration_nanoseconds = 8,
      .max_flush_buffer_to_file_attempts = 9,
      .flush_all_events = true,
  };
  const spoor::runtime::Config config_e{
      .trace_file_path = path_a,
      .session_id = 0,
      .thread_event_buffer_capacity = 0,
      .max_reserved_event_buffer_slice_capacity = 0,
      .max_dynamic_event_buffer_slice_capacity = 0,
      .reserved_event_pool_capacity = 0,
      .dynamic_event_pool_capacity = 0,
      .dynamic_event_slice_borrow_cas_attempts = 0,
      .event_buffer_retention_duration_nanoseconds = 0,
      .max_flush_buffer_to_file_attempts = 0,
      .flush_all_events = false,
  };
  ASSERT_EQ(config_a, config_a);
  ASSERT_EQ(config_a, config_b);
  ASSERT_EQ(config_a, config_c);
  ASSERT_NE(config_a, config_d);
  ASSERT_NE(config_a, config_e);
}

}  // namespace
