### Tools
## Clang C - clang++

Since clang++ is built on top of LLVM, we can use it to emit LLVM IR:

```shell
$ clang++ -S -emit-llvm ./src/test.cpp
$ cat ./test.ll

; ModuleID = './src/min.cpp'
source_filename = "./src/min.cpp"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128"
target triple = "arm64-apple-macosx14.0.0"

; Function Attrs: noinline norecurse nounwind optnone ssp uwtable(sync)
define i32 @main() #0 {
  %1 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  ret i32 42
}

attributes #0 = { noinline norecurse nounwind optnone ssp uwtable(sync) "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "probe-stack"="__chkstk_darwin" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+crc,+crypto,+dotprod,+fp-armv8,+fp16fml,+fullfp16,+lse,+neon,+ras,+rcpc,+rdm,+sha2,+sha3,+sm4,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8.5a,+v8a,+zcm,+zcz" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 14, i32 0]}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 8, !"PIC Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 7, !"frame-pointer", i32 1}
!5 = !{!"Apple clang version 15.0.0 (clang-1500.0.40.1)"}
```
There's a lot of metadata, the most minimal representation of the executable code can be:

```
define i32 @main() #0 {
  ret i32 42
}
```

We can also compile IR with clang++ directly:
```shell
clang++ ./test.ll -o ./min
./min
```

## Bitcode Assembler - llvm-as

Converts .ll to .bc. LLVM Bitecode assembler produces binary representation of the LLVM IR.

```shel
$ llvm-as test.ll
$ file test.bc
test.bc: LLVM IR bitcode
```

## LLVM Disassembler - llvm-dis

Converts back form llvm .bc to .ll.

```shell
$ llvm-dis ./test.bc -o test2.ll
$ cat test2.ll
; ModuleID = './test.bc'
source_filename = "test.ll"

define i32 @main() {
  ret i32 42
}
```

## Generating native assembly code

```shell
$ clang++ -S ./test.ll
$ cat ./test.s 
        .section        __TEXT,__text,regular,pure_instructions
        .build_version macos, 14, 0
        .globl  _main                           ; -- Begin function main
        .p2align        2
_main:                                  ; @main
        .cfi_startproc
; %bb.0:
        mov     w0, #42
        ret
        .cfi_endproc
                                        ; -- End function
.subsections_via_symbols
```
