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

void _unimplemented_syscall(void);

//sysnb sys_write runtime.write(fd uint64, p unsafe.Pointer, n int32) int32
int32_t sys_write(uint64_t fd, void *p, int32_t n);

//sys sys_read runtime.read(fd int32, p unsafe.Pointer, n int32) int32
int32_t sys_read(int32_t fd, void *p, int32_t n);

//sysnb sys_nanotime runtime.nanotime() uint64
uint64_t sys_nanotime(void);

//sysasm sys_now time.now 16
//LEAQ  0(SP), DI
//LEAQ  sys_now(SB), AX
//CALL  AX
//MOVQ  0(SP), AX
//MOVL  8(SP), DX
//MOVQ  AX, sec+0(FP)
//MOVL  DX, nsec+8(FP)
//RET
void sys_now(struct sys_now_t *now);

//sysnb sys_thread_id runtime.thread_id() uint64
uint64_t sys_thread_id(void);

//sysnb sys_usleep runtime.usleep(usec uint32)
void sys_usleep(uint32_t usec);

//sysnb sys_nanosleep runtime.nanosleep(nsec uint64)
void sys_nanosleep(uint64_t nsec);

//sysnb _unimplemented_syscall runtime.lwp_create(context unsafe.Pointer, flags uintptr, lwpid unsafe.Pointer) int32
//sysnb _unimplemented_syscall runtime.lwp_tramp()
//sysnb _unimplemented_syscall runtime.osyield()
//sysnb _unimplemented_syscall runtime.exit(code int32)
//sysnb _unimplemented_syscall runtime.exit1(code int32)
//sysnb _unimplemented_syscall runtime.open(name *byte, mode int32, perm int32) int32
//sysnb _unimplemented_syscall runtime.closefd(fd int32) int32
//sysnb _unimplemented_syscall runtime.raise(sig int32)
//sysnb _unimplemented_syscall runtime.raiseproc(sig int32)
//sysnb _unimplemented_syscall runtime.setitimer(mode int32, new *itimerval, old *itimerval)
//sysnb _unimplemented_syscall runtime.getcontext(ctxt unsafe.Pointer)
//sysnb _unimplemented_syscall runtime.sigprocmask(mode int32, new *sigset, old *sigset)
//sysnb _unimplemented_syscall runtime.sigaction(sig int32, new *sigset, old *sigset)
//sysnb _unimplemented_syscall runtime.munmap(addr unsafe.Pointer, n uintptr)
//sysnb _unimplemented_syscall runtime.madvise(addr unsafe.Pointer, n uintptr, flags int32)
//sysnb _unimplemented_syscall runtime.sigaltstack(new *sigaltstackt, old *sigaltstackt)
//sysnb _unimplemented_syscall runtime.settls()
//sysnb _unimplemented_syscall runtime.sysctl(mib *uint32, miblen uint32, out *byte, size *uintptr, dst *byte, ndst uintptr) int32
//sysnb _unimplemented_syscall runtime.kqueue() int32
//sysnb _unimplemented_syscall runtime.kevent(kq int32, ch *kqueuet, nch int32, ev *kqueuet, nev int32, ts *timespec) int32
//sysnb _unimplemented_syscall runtime.closeonexec(fd int32)

#endif /* _SYSCALL_H */
