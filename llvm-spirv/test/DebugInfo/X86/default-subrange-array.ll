; RUN: llvm-as < %s -o %t.bc
; RUN: llvm-spirv %t.bc -o %t.spv
; RUN: llvm-spirv -r %t.spv -o - | llvm-dis -o %t.ll
; RUN: llc -mtriple=x86_64-apple-darwin -O0 -filetype=obj -dwarf-version 4 \
; RUN:     -o - < %s | llvm-dwarfdump -v - --debug-info \
; RUN:     | FileCheck %s -check-prefixes=CHECK,DWARF4
; RUN: llc -mtriple=x86_64-apple-darwin -O0 -filetype=obj -dwarf-version 5 \
; RUN:     -o - < %s | llvm-dwarfdump -v - --debug-info \
; RUN:     | FileCheck %s -check-prefixes=CHECK,DWARF5

; RUN: llvm-spirv %t.bc -o %t.spv --spirv-debug-info-version=nonsemantic-shader-100
; RUN: llvm-spirv -r %t.spv -o - | llvm-dis -o %t.ll
; RUN: llc -mtriple=x86_64-apple-darwin -O0 -filetype=obj -dwarf-version 4 \
; RUN:     -o - < %s | llvm-dwarfdump -v - --debug-info \
; RUN:     | FileCheck %s -check-prefixes=CHECK,DWARF4
; RUN: llc -mtriple=x86_64-apple-darwin -O0 -filetype=obj -dwarf-version 5 \
; RUN:     -o - < %s | llvm-dwarfdump -v - --debug-info \
; RUN:     | FileCheck %s -check-prefixes=CHECK,DWARF5

; RUN: llvm-spirv %t.bc -o %t.spv --spirv-debug-info-version=nonsemantic-shader-200
; RUN: llvm-spirv -r %t.spv -o - | llvm-dis -o %t.ll
; RUN: llc -mtriple=x86_64-apple-darwin -O0 -filetype=obj -dwarf-version 4 \
; RUN:     -o - < %s | llvm-dwarfdump -v - --debug-info \
; RUN:     | FileCheck %s -check-prefixes=CHECK,DWARF4
; RUN: llc -mtriple=x86_64-apple-darwin -O0 -filetype=obj -dwarf-version 5 \
; RUN:     -o - < %s | llvm-dwarfdump -v - --debug-info \
; RUN:     | FileCheck %s -check-prefixes=CHECK,DWARF5

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

; Check that we can omit default array lower-bounds.
; DW_LANG_C_plus_plus_11 is new in DWARF v5, so if we use that with
; DWARF v4, we should get the DW_AT_lower_bound attribute.

source_filename = "test/DebugInfo/X86/default-subrange-array.ll"

%class.A = type { [42 x i32] }

@a = global %class.A zeroinitializer, align 4, !dbg !0

; CHECK:       DW_TAG_class_type
; CHECK:         DW_TAG_member
; CHECK-NEXT:      DW_AT_name {{.*}} "x"
; CHECK-NEXT:      DW_AT_type [DW_FORM_ref4] {{.*}} => {[[ARRAY:0x[0-9a-f]+]]}

; CHECK: [[ARRAY]]: DW_TAG_array_type
; CHECK-NEXT:         DW_AT_type
; CHECK:            DW_TAG_subrange_type
; CHECK-NEXT:         DW_AT_type
; DWARF4-NEXT:        DW_AT_lower_bound [DW_FORM_sdata] (0)
; CHECK-NEXT:         DW_AT_count [DW_FORM_data1]       (0x2a)
; DWARF5-NOT:         DW_AT_lower_bound


!llvm.dbg.cu = !{!14}
!llvm.module.flags = !{!17}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = !DIGlobalVariable(name: "a", scope: null, file: !2, line: 1, type: !3, isLocal: false, isDefinition: true)
!2 = !DIFile(filename: "t.cpp", directory: "/Volumes/Sandbox/llvm")
!3 = !DICompositeType(tag: DW_TAG_class_type, name: "A", file: !2, line: 1, align: 32, elements: !4)
!4 = !{!5, !10}
!5 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !3, file: !2, line: 1, baseType: !6, flags: DIFlagPrivate)
!6 = !DICompositeType(tag: DW_TAG_array_type, baseType: !7, align: 32, elements: !8)
!7 = !DIBasicType(name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!8 = !{!9}
!9 = !DISubrange(count: 42, lowerBound: 0)
!10 = !DISubprogram(name: "A", scope: !3, file: !2, line: 1, type: !11, isLocal: false, isDefinition: false, scopeLine: 1, virtualIndex: 6, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false)
!11 = !DISubroutineType(types: !12)
!12 = !{null, !13}
!13 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3, size: 64, align: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!14 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_11, file: !2, producer: "clang version 3.3 (trunk 169136)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !15, retainedTypes: !15, globals: !16, imports: !15)
!15 = !{}
!16 = !{!0}
!17 = !{i32 1, !"Debug Info Version", i32 3}

