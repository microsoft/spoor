define i32 @_Z9Fibonaccii(i32 %0) {
  call void @__spoor_runtime_LogFunctionEntry(i64 0)
  %2 = icmp slt i32 %0, 2
  br i1 %2, label %9, label %3
3:
  %4 = add nsw i32 %0, -1
  %5 = tail call i32 @_Z9Fibonaccii(i32 %4)
  %6 = add nsw i32 %0, -2
  %7 = tail call i32 @_Z9Fibonaccii(i32 %6)
  %8 = add nsw i32 %7, %5
  call void @__spoor_runtime_LogFunctionExit(i64 0)
  ret i32 %8
9:
  call void @__spoor_runtime_LogFunctionExit(i64 0)
  ret i32 %0
}

define i32 @main() {
  call void @__spoor_runtime_InitializeRuntime()
  call void @__spoor_runtime_EnableRuntime()
  call void @__spoor_runtime_LogFunctionEntry(i64 1)
  %1 = tail call i32 @_Z9Fibonaccii(i32 7)
  call void @__spoor_runtime_LogFunctionExit(i64 1)
  call void @__spoor_runtime_DeinitializeRuntime()
  ret i32 %1
}

declare void @__spoor_runtime_InitializeRuntime()
declare void @__spoor_runtime_DeinitializeRuntime()
declare void @__spoor_runtime_EnableRuntime()
declare void @__spoor_runtime_LogFunctionEntry(i64)
declare void @__spoor_runtime_LogFunctionExit(i64)
