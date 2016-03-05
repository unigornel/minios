/**
 * syscalls.h
 *
 * Various system calls needed by the Go runtime.
 */
#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <mini-os/types.h>

void *sys_alloc(size_t length);

#endif /* _SYSCALL_H */
