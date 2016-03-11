/**
 * futex.h
 *
 * Minimal Linux-like futex implementation.
 */
#ifndef _FUTEX_H_
#define _FUTEX_H_

#include <mini-os/types.h>

void init_futex(void);

int32_t sys_futex_wait(uint32_t *addr, uint32_t val, int64_t ns);
int32_t sys_futex_wake(uint32_t *addr, uint32_t count);

#endif /* _FUTEX_H_ */
