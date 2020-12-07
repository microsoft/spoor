; Copyright (c) Microsoft Corporation.
; Licensed under the MIT License.

; Source program: fibonacci.cc
; 1: auto Fibonacci(int n) -> int {
; 2:   if (n < 2) return n;
; 3:   return Fibonacci(n - 1) + Fibonacci(n - 2);
; 4: }
; 5: 
; 6: auto main() -> int { return Fibonacci(7); }

; ModuleID = 'fibonacci.cc'
source_filename = "fibonacci.cc"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.15.0"

; Function Attrs: noinline optnone ssp uwtable
define i32 @_Z9Fibonaccii(i32 %0) #0 !dbg !8 {
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store i32 %0, i32* %3, align 4
  call void @llvm.dbg.declare(metadata i32* %3, metadata !12, metadata !DIExpression()), !dbg !13
  %4 = load i32, i32* %3, align 4, !dbg !14
  %5 = icmp slt i32 %4, 2, !dbg !16
  br i1 %5, label %6, label %8, !dbg !17

6:                                                ; preds = %1
  %7 = load i32, i32* %3, align 4, !dbg !18
  store i32 %7, i32* %2, align 4, !dbg !19
  br label %16, !dbg !19

8:                                                ; preds = %1
  %9 = load i32, i32* %3, align 4, !dbg !20
  %10 = sub nsw i32 %9, 1, !dbg !21
  %11 = call i32 @_Z9Fibonaccii(i32 %10), !dbg !22
  %12 = load i32, i32* %3, align 4, !dbg !23
  %13 = sub nsw i32 %12, 2, !dbg !24
  %14 = call i32 @_Z9Fibonaccii(i32 %13), !dbg !25
  %15 = add nsw i32 %11, %14, !dbg !26
  store i32 %15, i32* %2, align 4, !dbg !27
  br label %16, !dbg !27

16:                                               ; preds = %8, %6
  %17 = load i32, i32* %2, align 4, !dbg !28
  ret i32 %17, !dbg !28
}

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline norecurse optnone ssp uwtable
define i32 @main() #2 !dbg !29 {
  %1 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  %2 = call i32 @_Z9Fibonaccii(i32 7), !dbg !32
  ret i32 %2, !dbg !33
}

attributes #0 = { noinline optnone ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable willreturn }
attributes #2 = { noinline norecurse optnone ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.linker.options = !{}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang version 10.0.1 ", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "fibonacci.cc", directory: "/path/to/file")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 10.0.1 "}
!8 = distinct !DISubprogram(name: "Fibonacci", linkageName: "_Z9Fibonaccii", scope: !1, file: !1, line: 1, type: !9, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{!11, !11}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DILocalVariable(name: "n", arg: 1, scope: !8, file: !1, line: 1, type: !11)
!13 = !DILocation(line: 1, column: 20, scope: !8)
!14 = !DILocation(line: 2, column: 7, scope: !15)
!15 = distinct !DILexicalBlock(scope: !8, file: !1, line: 2, column: 7)
!16 = !DILocation(line: 2, column: 9, scope: !15)
!17 = !DILocation(line: 2, column: 7, scope: !8)
!18 = !DILocation(line: 2, column: 21, scope: !15)
!19 = !DILocation(line: 2, column: 14, scope: !15)
!20 = !DILocation(line: 3, column: 20, scope: !8)
!21 = !DILocation(line: 3, column: 22, scope: !8)
!22 = !DILocation(line: 3, column: 10, scope: !8)
!23 = !DILocation(line: 3, column: 39, scope: !8)
!24 = !DILocation(line: 3, column: 41, scope: !8)
!25 = !DILocation(line: 3, column: 29, scope: !8)
!26 = !DILocation(line: 3, column: 27, scope: !8)
!27 = !DILocation(line: 3, column: 3, scope: !8)
!28 = !DILocation(line: 4, column: 1, scope: !8)
!29 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 6, type: !30, scopeLine: 6, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!30 = !DISubroutineType(types: !31)
!31 = !{!11}
!32 = !DILocation(line: 6, column: 29, scope: !29)
!33 = !DILocation(line: 6, column: 22, scope: !29)
