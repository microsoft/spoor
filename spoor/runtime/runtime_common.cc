// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/runtime.h"

namespace spoor::runtime {

auto operator==(const DeletedFilesInfo& lhs, const DeletedFilesInfo& rhs)
    -> bool {
  return lhs.deleted_files == rhs.deleted_files &&
         lhs.deleted_bytes == rhs.deleted_bytes;
}

auto operator==(const Config& lhs, const Config& rhs) -> bool {
  return lhs.trace_file_path == rhs.trace_file_path &&
         lhs.session_id == rhs.session_id &&
         lhs.thread_event_buffer_capacity == rhs.thread_event_buffer_capacity &&
         lhs.max_reserved_event_buffer_slice_capacity ==
             rhs.max_reserved_event_buffer_slice_capacity &&
         lhs.max_dynamic_event_buffer_slice_capacity ==
             rhs.max_dynamic_event_buffer_slice_capacity &&
         lhs.reserved_event_pool_capacity == rhs.reserved_event_pool_capacity &&
         lhs.dynamic_event_pool_capacity == rhs.dynamic_event_pool_capacity &&
         lhs.dynamic_event_slice_borrow_cas_attempts ==
             rhs.dynamic_event_slice_borrow_cas_attempts &&
         lhs.event_buffer_retention_duration_nanoseconds ==
             rhs.event_buffer_retention_duration_nanoseconds &&
         lhs.max_flush_buffer_to_file_attempts ==
             rhs.max_flush_buffer_to_file_attempts &&
         lhs.flush_all_events == rhs.flush_all_events;
}

}  // namespace spoor::runtime

auto _spoor_runtime_Initialize() -> void { spoor::runtime::Initialize(); }

auto _spoor_runtime_Deinitialize() -> void { spoor::runtime::Deinitialize(); }

auto _spoor_runtime_Enable() -> void { spoor::runtime::Enable(); }

auto _spoor_runtime_Disable() -> void { spoor::runtime::Disable(); }
