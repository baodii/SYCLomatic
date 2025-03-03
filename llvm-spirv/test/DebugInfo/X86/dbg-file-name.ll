; RUN: llvm-as < %s -o %t.bc
; RUN: llvm-spirv %t.bc -o %t.spv
; RUN: llvm-spirv -r %t.spv -o - | llvm-dis -o %t.ll
; RUN: llc -mtriple x86_64-apple-darwin10.0.0  < %t.ll | FileCheck %s

; RUN: llvm-spirv %t.bc -o %t.spv --spirv-debug-info-version=nonsemantic-shader-100
; RUN: llvm-spirv -r %t.spv -o - | llvm-dis -o %t.ll
; RUN: llc -mtriple x86_64-apple-darwin10.0.0  < %t.ll | FileCheck %s

; RUN: llvm-spirv %t.bc -o %t.spv --spirv-debug-info-version=nonsemantic-shader-200
; RUN: llvm-spirv -r %t.spv -o - | llvm-dis -o %t.ll
; RUN: llc -mtriple x86_64-apple-darwin10.0.0  < %t.ll | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

; Verify that the file name is relative to the directory.
; rdar://problem/8884898
; CHECK: file	1 "/Users/manav/one/two" "simple.c"

declare i32 @printf(i8*, ...) nounwind

define i32 @main() nounwind !dbg !6 {
  ret i32 0
}

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!12}

!1 = !DIFile(filename: "simple.c", directory: "/Users/manav/one/two")
!2 = distinct !DICompileUnit(language: DW_LANG_C89, producer: "LLVM build 00", isOptimized: true, emissionKind: FullDebug, file: !10, enums: !11, retainedTypes: !11)
!5 = !DIBasicType(tag: DW_TAG_base_type, name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!6 = distinct !DISubprogram(name: "main", linkageName: "main", line: 9, isLocal: false, isDefinition: true, virtualIndex: 6, flags: DIFlagPrototyped, isOptimized: false, unit: !2, file: !10, scope: !1, type: !7)
!7 = !DISubroutineType(types: !8)
!8 = !{!5}
!10 = !DIFile(filename: "simple.c", directory: "/Users/manav/one/two")
!11 = !{}
!12 = !{i32 1, !"Debug Info Version", i32 3}
