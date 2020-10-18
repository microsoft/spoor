; Source program: fibonacci.swift
; 1: func fibonacci(_ n: Int) -> Int {
; 2:   if (n < 2) {
; 3:     return n;
; 4:   }
; 5:   return fibonacci(n - 1) + fibonacci(n - 2)
; 6: }
; 7:
; 8: fibonacci(7)

; ModuleID = '<swift-imported-modules>'
source_filename = "<swift-imported-modules>"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.15.0"

@"\01l_entry_point" = private constant { i32 } { i32 trunc (i64 sub (i64 ptrtoint (i32 (i32, i8**)* @main to i64), i64 ptrtoint ({ i32 }* @"\01l_entry_point" to i64)) to i32) }, section "__TEXT, __swift5_entry, regular, no_dead_strip", align 4
@"_swift_FORCE_LOAD_$_swiftCompatibility51_$_fibonacci" = weak_odr hidden constant void ()* @"_swift_FORCE_LOAD_$_swiftCompatibility51"
@__swift_reflection_version = linkonce_odr hidden constant i16 3
@llvm.used = appending global [4 x i8*] [i8* bitcast ({ i32 }* @"\01l_entry_point" to i8*), i8* bitcast (i16* @__swift_reflection_version to i8*), i8* bitcast (void ()** @"_swift_FORCE_LOAD_$_swiftCompatibility51_$_fibonacci" to i8*), i8* bitcast (i32 (i32, i8**)* @main to i8*)], section "llvm.metadata"
; Function Attrs: nounwind
define i32 @main(i32 %0, i8** nocapture readnone %1) #0 !dbg !26 {
entry:
  %2 = tail call swiftcc i64 @"$s9fibonacciAAyS2iF"(i64 7), !dbg !38
  ret i32 0, !dbg !38
}
; Function Attrs: nounwind
define hidden swiftcc i64 @"$s9fibonacciAAyS2iF"(i64 %0) local_unnamed_addr #0 !dbg !40 {
entry:
  call void @llvm.dbg.value(metadata i64 %0, metadata !44, metadata !DIExpression()), !dbg !46
  %1 = icmp slt i64 %0, 2, !dbg !47
  br i1 %1, label %10, label %2, !dbg !47

2:                                                ; preds = %entry
  %3 = add nsw i64 %0, -1, !dbg !50
  %4 = tail call swiftcc i64 @"$s9fibonacciAAyS2iF"(i64 %3), !dbg !51
  %5 = add nsw i64 %0, -2, !dbg !52
  %6 = tail call swiftcc i64 @"$s9fibonacciAAyS2iF"(i64 %5), !dbg !53
  %7 = tail call { i64, i1 } @llvm.sadd.with.overflow.i64(i64 %4, i64 %6), !dbg !54
  %8 = extractvalue { i64, i1 } %7, 0, !dbg !54
  %9 = extractvalue { i64, i1 } %7, 1, !dbg !54
  br i1 %9, label %12, label %10, !dbg !54, !prof !55, !misexpect !56

10:                                               ; preds = %2, %entry
  %11 = phi i64 [ %0, %entry ], [ %8, %2 ], !dbg !57
  ret i64 %11, !dbg !57

12:                                               ; preds = %2
  tail call void asm sideeffect "", "n"(i32 0) #3, !dbg !54
  tail call void @llvm.trap(), !dbg !58
  unreachable, !dbg !58
}
; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #1
; Function Attrs: nounwind readnone speculatable willreturn
declare { i64, i1 } @llvm.sadd.with.overflow.i64(i64, i64) #1
; Function Attrs: cold noreturn nounwind
declare void @llvm.trap() #2
declare extern_weak void @"_swift_FORCE_LOAD_$_swiftCompatibility51"()

attributes #0 = { nounwind "frame-pointer"="all" "probe-stack"="__chkstk_darwin" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" }
attributes #1 = { nounwind readnone speculatable willreturn }
attributes #2 = { cold noreturn nounwind }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4, !5, !6, !7, !8, !9, !10}
!llvm.dbg.cu = !{!11, !19}
!swift.module.flags = !{!21}
!llvm.linker.options = !{!22, !23, !24}
!llvm.asan.globals = !{!25}

!0 = !{i32 2, !"SDK Version", [3 x i32] [i32 10, i32 15, i32 6]}
!1 = !{i32 1, !"Objective-C Version", i32 2}
!2 = !{i32 1, !"Objective-C Image Info Version", i32 0}
!3 = !{i32 1, !"Objective-C Image Info Section", !"__DATA,__objc_imageinfo,regular,no_dead_strip"}
!4 = !{i32 4, !"Objective-C Garbage Collection", i32 84084480}
!5 = !{i32 1, !"Objective-C Class Properties", i32 64}
!6 = !{i32 7, !"Dwarf Version", i32 4}
!7 = !{i32 2, !"Debug Info Version", i32 3}
!8 = !{i32 1, !"wchar_size", i32 4}
!9 = !{i32 7, !"PIC Level", i32 2}
!10 = !{i32 1, !"Swift Version", i32 7}
!11 = distinct !DICompileUnit(language: DW_LANG_Swift, file: !12, producer: "Apple Swift version 5.3 (swiftlang-1200.0.29.2 clang-1200.0.30.1)", isOptimized: true, runtimeVersion: 5, emissionKind: FullDebug, enums: !13, imports: !14, sysroot: "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk", sdk: "MacOSX10.15.sdk")
!12 = !DIFile(filename: "fibonacci.swift", directory: "/Users/lelandjansen/Desktop")
!13 = !{}
!14 = !{!15, !17}
!15 = !DIImportedEntity(tag: DW_TAG_imported_module, scope: !12, entity: !16, file: !12)
!16 = !DIModule(scope: null, name: "fibonacci")
!17 = !DIImportedEntity(tag: DW_TAG_imported_module, scope: !12, entity: !18, file: !12)
!18 = !DIModule(scope: null, name: "Swift", includePath: "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk/usr/lib/swift/Swift.swiftmodule/x86_64.swiftinterface")
!19 = distinct !DICompileUnit(language: DW_LANG_ObjC, file: !20, producer: "Apple clang version 12.0.0 (clang-1200.0.30.1)", isOptimized: true, runtimeVersion: 2, emissionKind: FullDebug, enums: !13, nameTableKind: None, sysroot: "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk", sdk: "MacOSX10.15.sdk")
!20 = !DIFile(filename: "<swift-imported-modules>", directory: "/Users/lelandjansen/Desktop")
!21 = !{!"standard-library", i1 false}
!22 = !{!"-lswiftCore"}
!23 = !{!"-lobjc"}
!24 = !{!"-lswiftCompatibility51"}
!25 = distinct !{null, null, null, i1 false, i1 true}
!26 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !16, file: !12, line: 1, type: !27, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !11, retainedNodes: !13)
!27 = !DISubroutineType(types: !28)
!28 = !{!29, !29, !31}
!29 = !DICompositeType(tag: DW_TAG_structure_type, name: "Int32", scope: !18, file: !30, size: 32, elements: !13, runtimeLang: DW_LANG_Swift, identifier: "$ss5Int32VD")
!30 = !DIFile(filename: "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk/usr/lib/swift/Swift.swiftmodule/x86_64.swiftinterface", directory: "")
!31 = !DICompositeType(tag: DW_TAG_structure_type, scope: !18, file: !12, size: 64, elements: !32, runtimeLang: DW_LANG_Swift)
!32 = !{!33}
!33 = !DIDerivedType(tag: DW_TAG_member, scope: !18, file: !12, baseType: !34, size: 64)
!34 = !DICompositeType(tag: DW_TAG_structure_type, name: "UnsafeMutablePointer", scope: !18, file: !12, runtimeLang: DW_LANG_Swift, templateParams: !35, identifier: "$sSpySpys4Int8VGSgGD")
!35 = !{!36}
!36 = !DITemplateTypeParameter(type: !37)
!37 = !DICompositeType(tag: DW_TAG_structure_type, name: "$sSpys4Int8VGSgD", scope: !18, flags: DIFlagFwdDecl, runtimeLang: DW_LANG_Swift, identifier: "$sSpys4Int8VGSgD")
!38 = !DILocation(line: 8, column: 1, scope: !39)
!39 = distinct !DILexicalBlock(scope: !26, file: !12, line: 8, column: 1)
!40 = distinct !DISubprogram(name: "fibonacci", linkageName: "$s9fibonacciAAyS2iF", scope: !16, file: !12, line: 1, type: !41, scopeLine: 1, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !11, retainedNodes: !13)
!41 = !DISubroutineType(types: !42)
!42 = !{!43, !43}
!43 = !DICompositeType(tag: DW_TAG_structure_type, name: "Int", scope: !18, file: !30, size: 64, elements: !13, runtimeLang: DW_LANG_Swift, identifier: "$sSiD")
!44 = !DILocalVariable(name: "n", arg: 1, scope: !40, file: !12, line: 1, type: !45)
!45 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !43)
!46 = !DILocation(line: 1, column: 16, scope: !40)
!47 = !DILocation(line: 2, column: 9, scope: !48)
!48 = distinct !DILexicalBlock(scope: !49, file: !12, line: 2, column: 3)
!49 = distinct !DILexicalBlock(scope: !40, file: !12, line: 1, column: 33)
!50 = !DILocation(line: 5, column: 22, scope: !49)
!51 = !DILocation(line: 5, column: 10, scope: !49)
!52 = !DILocation(line: 5, column: 41, scope: !49)
!53 = !DILocation(line: 5, column: 29, scope: !49)
!54 = !DILocation(line: 5, column: 27, scope: !49)
!55 = !{!"branch_weights", i32 1, i32 2000}
!56 = !{!"misexpect", i64 1, i64 2000, i64 1}
!57 = !DILocation(line: 6, column: 1, scope: !49)
!58 = !DILocation(line: 0, scope: !59, inlinedAt: !54)
!59 = distinct !DISubprogram(name: "Swift runtime failure: arithmetic overflow", scope: !16, file: !12, type: !60, flags: DIFlagArtificial, spFlags: DISPFlagDefinition, unit: !11, retainedNodes: !13)
!60 = !DISubroutineType(types: null)
