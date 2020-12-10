; Copyright (c) Microsoft Corporation.
; Licensed under the MIT License.

; Source program: fibonacci.m
;  1: @interface Fibonacci
;  2: +(int)compute:(int)n;
;  3: @end
;  4:
;  5: @implementation Fibonacci
;  6: +(int)compute:(int)n {
;  7:   if (n < 2) return n;
;  8:   return [Fibonacci compute:n - 1] + [Fibonacci compute:n - 2];
;  9: }
; 10: @end
; 11:
; 12: int main() { return [Fibonacci compute:7]; }

; ModuleID = 'fibonacci.m'
source_filename = "fibonacci.m"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.15.0"

%struct._class_t = type { %struct._class_t*, %struct._class_t*, %struct._objc_cache*, i8* (i8*, i8*)**, %struct._class_ro_t* }
%struct._objc_cache = type opaque
%struct._class_ro_t = type { i32, i32, i32, i8*, i8*, %struct.__method_list_t*, %struct._objc_protocol_list*, %struct._ivar_list_t*, i8*, %struct._prop_list_t* }
%struct.__method_list_t = type { i32, i32, [0 x %struct._objc_method] }
%struct._objc_method = type { i8*, i8*, i8* }
%struct._objc_protocol_list = type { i64, [0 x %struct._protocol_t*] }
%struct._protocol_t = type { i8*, i8*, %struct._objc_protocol_list*, %struct.__method_list_t*, %struct.__method_list_t*, %struct.__method_list_t*, %struct.__method_list_t*, %struct._prop_list_t*, i32, i32, i8**, i8*, %struct._prop_list_t* }
%struct._ivar_list_t = type { i32, i32, [0 x %struct._ivar_t] }
%struct._ivar_t = type { i64*, i8*, i8*, i32, i32 }
%struct._prop_list_t = type { i32, i32, [0 x %struct._prop_t] }
%struct._prop_t = type { i8*, i8* }

@"OBJC_CLASS_$_Fibonacci" = global %struct._class_t { %struct._class_t* @"OBJC_METACLASS_$_Fibonacci", %struct._class_t* null, %struct._objc_cache* @_objc_empty_cache, i8* (i8*, i8*)** null, %struct._class_ro_t* @"_OBJC_CLASS_RO_$_Fibonacci" }, section "__DATA, __objc_data", align 8
@"OBJC_CLASSLIST_REFERENCES_$_" = internal global %struct._class_t* @"OBJC_CLASS_$_Fibonacci", section "__DATA,__objc_classrefs,regular,no_dead_strip", align 8
@OBJC_METH_VAR_NAME_ = private unnamed_addr constant [9 x i8] c"compute:\00", section "__TEXT,__objc_methname,cstring_literals", align 1
@OBJC_SELECTOR_REFERENCES_ = internal externally_initialized global i8* getelementptr inbounds ([9 x i8], [9 x i8]* @OBJC_METH_VAR_NAME_, i32 0, i32 0), section "__DATA,__objc_selrefs,literal_pointers,no_dead_strip", align 8
@_objc_empty_cache = external global %struct._objc_cache
@"OBJC_METACLASS_$_Fibonacci" = global %struct._class_t { %struct._class_t* @"OBJC_METACLASS_$_Fibonacci", %struct._class_t* @"OBJC_CLASS_$_Fibonacci", %struct._objc_cache* @_objc_empty_cache, i8* (i8*, i8*)** null, %struct._class_ro_t* @"_OBJC_METACLASS_RO_$_Fibonacci" }, section "__DATA, __objc_data", align 8
@OBJC_CLASS_NAME_ = private unnamed_addr constant [10 x i8] c"Fibonacci\00", section "__TEXT,__objc_classname,cstring_literals", align 1
@OBJC_METH_VAR_TYPE_ = private unnamed_addr constant [11 x i8] c"i20@0:8i16\00", section "__TEXT,__objc_methtype,cstring_literals", align 1
@"_OBJC_$_CLASS_METHODS_Fibonacci" = internal global { i32, i32, [1 x %struct._objc_method] } { i32 24, i32 1, [1 x %struct._objc_method] [%struct._objc_method { i8* getelementptr inbounds ([9 x i8], [9 x i8]* @OBJC_METH_VAR_NAME_, i32 0, i32 0), i8* getelementptr inbounds ([11 x i8], [11 x i8]* @OBJC_METH_VAR_TYPE_, i32 0, i32 0), i8* bitcast (i32 (i8*, i8*, i32)* @"\01+[Fibonacci compute:]" to i8*) }] }, section "__DATA, __objc_const", align 8
@"_OBJC_METACLASS_RO_$_Fibonacci" = internal global %struct._class_ro_t { i32 3, i32 40, i32 40, i8* null, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @OBJC_CLASS_NAME_, i32 0, i32 0), %struct.__method_list_t* bitcast ({ i32, i32, [1 x %struct._objc_method] }* @"_OBJC_$_CLASS_METHODS_Fibonacci" to %struct.__method_list_t*), %struct._objc_protocol_list* null, %struct._ivar_list_t* null, i8* null, %struct._prop_list_t* null }, section "__DATA, __objc_const", align 8
@"_OBJC_CLASS_RO_$_Fibonacci" = internal global %struct._class_ro_t { i32 2, i32 0, i32 0, i8* null, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @OBJC_CLASS_NAME_, i32 0, i32 0), %struct.__method_list_t* null, %struct._objc_protocol_list* null, %struct._ivar_list_t* null, i8* null, %struct._prop_list_t* null }, section "__DATA, __objc_const", align 8
@"OBJC_LABEL_CLASS_$" = private global [1 x i8*] [i8* bitcast (%struct._class_t* @"OBJC_CLASS_$_Fibonacci" to i8*)], section "__DATA,__objc_classlist,regular,no_dead_strip", align 8
@llvm.compiler.used = appending global [7 x i8*] [i8* bitcast (%struct._class_t** @"OBJC_CLASSLIST_REFERENCES_$_" to i8*), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @OBJC_METH_VAR_NAME_, i32 0, i32 0), i8* bitcast (i8** @OBJC_SELECTOR_REFERENCES_ to i8*), i8* getelementptr inbounds ([10 x i8], [10 x i8]* @OBJC_CLASS_NAME_, i32 0, i32 0), i8* getelementptr inbounds ([11 x i8], [11 x i8]* @OBJC_METH_VAR_TYPE_, i32 0, i32 0), i8* bitcast ({ i32, i32, [1 x %struct._objc_method] }* @"_OBJC_$_CLASS_METHODS_Fibonacci" to i8*), i8* bitcast ([1 x i8*]* @"OBJC_LABEL_CLASS_$" to i8*)], section "llvm.metadata"

; Function Attrs: noinline optnone ssp uwtable
define internal i32 @"\01+[Fibonacci compute:]"(i8* %0, i8* %1, i32 %2) #0 !dbg !15 {
  %4 = alloca i32, align 4
  %5 = alloca i8*, align 8
  %6 = alloca i8*, align 8
  %7 = alloca i32, align 4
  store i8* %0, i8** %5, align 8
  call void @llvm.dbg.declare(metadata i8** %5, metadata !25, metadata !DIExpression()), !dbg !27
  store i8* %1, i8** %6, align 8
  call void @llvm.dbg.declare(metadata i8** %6, metadata !28, metadata !DIExpression()), !dbg !27
  store i32 %2, i32* %7, align 4
  call void @llvm.dbg.declare(metadata i32* %7, metadata !30, metadata !DIExpression()), !dbg !31
  %8 = load i32, i32* %7, align 4, !dbg !32
  %9 = icmp slt i32 %8, 2, !dbg !34
  br i1 %9, label %10, label %12, !dbg !35

10:                                               ; preds = %3
  %11 = load i32, i32* %7, align 4, !dbg !36
  store i32 %11, i32* %4, align 4, !dbg !37
  br label %26, !dbg !37

12:                                               ; preds = %3
  %13 = load %struct._class_t*, %struct._class_t** @"OBJC_CLASSLIST_REFERENCES_$_", align 8, !dbg !38
  %14 = load i32, i32* %7, align 4, !dbg !39
  %15 = sub nsw i32 %14, 1, !dbg !40
  %16 = load i8*, i8** @OBJC_SELECTOR_REFERENCES_, align 8, !dbg !38, !invariant.load !2
  %17 = bitcast %struct._class_t* %13 to i8*, !dbg !38
  %18 = call i32 bitcast (i8* (i8*, i8*, ...)* @objc_msgSend to i32 (i8*, i8*, i32)*)(i8* %17, i8* %16, i32 %15), !dbg !38
  %19 = load %struct._class_t*, %struct._class_t** @"OBJC_CLASSLIST_REFERENCES_$_", align 8, !dbg !41
  %20 = load i32, i32* %7, align 4, !dbg !42
  %21 = sub nsw i32 %20, 2, !dbg !43
  %22 = load i8*, i8** @OBJC_SELECTOR_REFERENCES_, align 8, !dbg !41, !invariant.load !2
  %23 = bitcast %struct._class_t* %19 to i8*, !dbg !41
  %24 = call i32 bitcast (i8* (i8*, i8*, ...)* @objc_msgSend to i32 (i8*, i8*, i32)*)(i8* %23, i8* %22, i32 %21), !dbg !41
  %25 = add nsw i32 %18, %24, !dbg !44
  store i32 %25, i32* %4, align 4, !dbg !45
  br label %26, !dbg !45

26:                                               ; preds = %12, %10
  %27 = load i32, i32* %4, align 4, !dbg !46
  ret i32 %27, !dbg !46
}

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nonlazybind
declare i8* @objc_msgSend(i8*, i8*, ...) #2

; Function Attrs: noinline optnone ssp uwtable
define i32 @main() #0 !dbg !47 {
  %1 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  %2 = load %struct._class_t*, %struct._class_t** @"OBJC_CLASSLIST_REFERENCES_$_", align 8, !dbg !50
  %3 = load i8*, i8** @OBJC_SELECTOR_REFERENCES_, align 8, !dbg !50, !invariant.load !2
  %4 = bitcast %struct._class_t* %2 to i8*, !dbg !50
  %5 = call i32 bitcast (i8* (i8*, i8*, ...)* @objc_msgSend to i32 (i8*, i8*, i32)*)(i8* %4, i8* %3, i32 7), !dbg !50
  ret i32 %5, !dbg !51
}

attributes #0 = { noinline optnone ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable willreturn }
attributes #2 = { nonlazybind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!5, !6, !7, !8, !9, !10, !11, !12, !13}
!llvm.ident = !{!14}

!0 = distinct !DICompileUnit(language: DW_LANG_ObjC, file: !1, producer: "clang version 11.0.0", isOptimized: false, runtimeVersion: 2, emissionKind: FullDebug, enums: !2, retainedTypes: !3, nameTableKind: None, sysroot: "/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk", sdk: "MacOSX10.15.sdk")
!1 = !DIFile(filename: "fibonacci.m", directory: "/path/to/file")
!2 = !{}
!3 = !{!4}
!4 = !DICompositeType(tag: DW_TAG_structure_type, name: "Fibonacci", scope: !1, file: !1, line: 1, flags: DIFlagObjcClassComplete, elements: !2, runtimeLang: DW_LANG_ObjC)
!5 = !{i32 1, !"Objective-C Version", i32 2}
!6 = !{i32 1, !"Objective-C Image Info Version", i32 0}
!7 = !{i32 1, !"Objective-C Image Info Section", !"__DATA,__objc_imageinfo,regular,no_dead_strip"}
!8 = !{i32 1, !"Objective-C Garbage Collection", i8 0}
!9 = !{i32 1, !"Objective-C Class Properties", i32 64}
!10 = !{i32 7, !"Dwarf Version", i32 4}
!11 = !{i32 2, !"Debug Info Version", i32 3}
!12 = !{i32 1, !"wchar_size", i32 4}
!13 = !{i32 7, !"PIC Level", i32 2}
!14 = !{!"clang version 11.0.0"}
!15 = distinct !DISubprogram(name: "+[Fibonacci compute:]", scope: !1, file: !1, line: 6, type: !16, scopeLine: 6, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !0, retainedNodes: !2)
!16 = !DISubroutineType(types: !17)
!17 = !{!18, !19, !22, !18}
!18 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!19 = !DIDerivedType(tag: DW_TAG_typedef, name: "Class", file: !1, baseType: !20, flags: DIFlagArtificial | DIFlagObjectPointer)
!20 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !21, size: 64)
!21 = !DICompositeType(tag: DW_TAG_structure_type, name: "objc_class", file: !1, flags: DIFlagFwdDecl)
!22 = !DIDerivedType(tag: DW_TAG_typedef, name: "SEL", file: !1, baseType: !23, flags: DIFlagArtificial)
!23 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !24, size: 64)
!24 = !DICompositeType(tag: DW_TAG_structure_type, name: "objc_selector", file: !1, flags: DIFlagFwdDecl)
!25 = !DILocalVariable(name: "self", arg: 1, scope: !15, type: !26, flags: DIFlagArtificial | DIFlagObjectPointer)
!26 = !DIDerivedType(tag: DW_TAG_typedef, name: "Class", file: !1, baseType: !20)
!27 = !DILocation(line: 0, scope: !15)
!28 = !DILocalVariable(name: "_cmd", arg: 2, scope: !15, type: !29, flags: DIFlagArtificial)
!29 = !DIDerivedType(tag: DW_TAG_typedef, name: "SEL", file: !1, baseType: !23)
!30 = !DILocalVariable(name: "n", arg: 3, scope: !15, file: !1, line: 6, type: !18)
!31 = !DILocation(line: 6, column: 20, scope: !15)
!32 = !DILocation(line: 7, column: 7, scope: !33)
!33 = distinct !DILexicalBlock(scope: !15, file: !1, line: 7, column: 7)
!34 = !DILocation(line: 7, column: 9, scope: !33)
!35 = !DILocation(line: 7, column: 7, scope: !15)
!36 = !DILocation(line: 7, column: 21, scope: !33)
!37 = !DILocation(line: 7, column: 14, scope: !33)
!38 = !DILocation(line: 8, column: 10, scope: !15)
!39 = !DILocation(line: 8, column: 29, scope: !15)
!40 = !DILocation(line: 8, column: 31, scope: !15)
!41 = !DILocation(line: 8, column: 38, scope: !15)
!42 = !DILocation(line: 8, column: 57, scope: !15)
!43 = !DILocation(line: 8, column: 59, scope: !15)
!44 = !DILocation(line: 8, column: 36, scope: !15)
!45 = !DILocation(line: 8, column: 3, scope: !15)
!46 = !DILocation(line: 9, column: 1, scope: !15)
!47 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 12, type: !48, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!48 = !DISubroutineType(types: !49)
!49 = !{!18}
!50 = !DILocation(line: 12, column: 21, scope: !47)
!51 = !DILocation(line: 12, column: 14, scope: !47)
