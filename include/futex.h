/**
 * futex.h
 *
 * Minimal Linux-like futex implementation.
 */
#ifndef _FUTEX_H_
#define _FUTEX_H_

#include <mini-os/types.h>

void init_futex(void);

//sysnb sys_futex_wait runtime.futex_wait(addr unsafe.Pointer, val uint32, ns int64) int32
int32_t sys_futex_wait(uint32_t *addr, uint32_t val, int64_t ns);

//sysnb sys_futex_wake runtime.futex_wake(addr unsafe.Pointer, cnt uint32) int32
int32_t sys_futex_wake(uint32_t *addr, uint32_t count);

#endif /* _FUTEX_H_ */
