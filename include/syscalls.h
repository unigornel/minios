/**
 * syscalls.h
 *
 * Various system calls needed by the Go runtime.
 */
#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <mini-os/types.h>

extern int const sys_argc;
extern char *const sys_argv[];

struct sys_now_t {
	uint64_t sec;
	uint32_t nsec;
};

int32_t sys_write(uint64_t fd, void *p, int32_t n);
uint64_t sys_nanotime(void);
void sys_now(struct sys_now_t *now);
uint64_t sys_thread_id(void);
void sys_usleep(uint32_t usec);
void sys_nanosleep(uint64_t nsec);

void _unimplemented_syscall(void);

#endif /* _SYSCALL_H */
