#!/bin/bash

I=../include
HEADER_FILES="$I/syscalls.h $I/mmap.h $I/futex.h"
GOROOT=../../go

set -ex
./mksys.py                                                            $HEADER_FILES > $GOROOT/src/runtime/sys_unigornel_amd64.s
./mksys.py                             --prefix sys --package syscall $HEADER_FILES > $GOROOT/src/syscall/zasm_unigornel_amd64.s
./mksys.py --godecl --convert-pointers --prefix sys --package syscall $HEADER_FILES > $GOROOT/src/syscall/zdecl_unigornel_amd64.go
