#ifndef __ARCH_SCHED_H__
#define __ARCH_SCHED_H__

#include "arch_limits.h"

void __arch_switch_threads(unsigned long *prevctx, unsigned long *nextctx);

#define arch_switch_threads(prev,next) __arch_switch_threads(&(prev)->sp, &(next)->sp)

#endif /* __ARCH_SCHED_H__ */
