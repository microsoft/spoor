; Copyright (c) Microsoft Corporation.
; Licensed under the MIT License.

define i32 @_Z9Fibonaccii(i32 %0) {
  call void @_spoor_runtime_LogFunctionEntry(i64 -5636857539040641024)
  %2 = icmp slt i32 %0, 2
  br i1 %2, label %9, label %3
3:
  %4 = add nsw i32 %0, -1
  %5 = tail call i32 @_Z9Fibonaccii(i32 %4)
  %6 = add nsw i32 %0, -2
  %7 = tail call i32 @_Z9Fibonaccii(i32 %6)
  %8 = add nsw i32 %7, %5
  call void @_spoor_runtime_LogFunctionExit(i64 -5636857539040641024)
  ret i32 %8
9:
  call void @_spoor_runtime_LogFunctionExit(i64 -5636857539040641024)
  ret i32 %0
}

define i32 @main() {
  call void @_spoor_runtime_Initialize()
  call void @_spoor_runtime_Enable()
  call void @_spoor_runtime_LogFunctionEntry(i64 -5636857539040641023)
  %1 = tail call i32 @_Z9Fibonaccii(i32 7)
  call void @_spoor_runtime_LogFunctionExit(i64 -5636857539040641023)
  call void @_spoor_runtime_Deinitialize()
  ret i32 %1
}

declare void @_spoor_runtime_Initialize()
declare void @_spoor_runtime_Deinitialize()
declare void @_spoor_runtime_Enable()
declare void @_spoor_runtime_LogFunctionEntry(i64)
declare void @_spoor_runtime_LogFunctionExit(i64)
