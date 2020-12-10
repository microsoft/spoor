; Copyright (c) Microsoft Corporation.
; Licensed under the MIT License.

define i32 @_Z9Fibonaccii(i32 %0) {
  call void @_spoor_runtime_LogFunctionEntry(i64 8341858565178261504)
  %2 = icmp slt i32 %0, 2
  br i1 %2, label %9, label %3
3:
  %4 = add nsw i32 %0, -1
  %5 = tail call i32 @_Z9Fibonaccii(i32 %4)
  %6 = add nsw i32 %0, -2
  %7 = tail call i32 @_Z9Fibonaccii(i32 %6)
  %8 = add nsw i32 %7, %5
  call void @_spoor_runtime_LogFunctionExit(i64 8341858565178261504)
  ret i32 %8
9:
  call void @_spoor_runtime_LogFunctionExit(i64 8341858565178261504)
  ret i32 %0
}

define i32 @main() {
  call void @_spoor_runtime_InitializeRuntime()
  call void @_spoor_runtime_LogFunctionEntry(i64 8341858565178261505)
  %1 = tail call i32 @_Z9Fibonaccii(i32 7)
  call void @_spoor_runtime_LogFunctionExit(i64 8341858565178261505)
  call void @_spoor_runtime_DeinitializeRuntime()
  ret i32 %1
}

declare void @_spoor_runtime_InitializeRuntime()
declare void @_spoor_runtime_DeinitializeRuntime()
declare void @_spoor_runtime_EnableRuntime()
declare void @_spoor_runtime_LogFunctionEntry(i64)
declare void @_spoor_runtime_LogFunctionExit(i64)
