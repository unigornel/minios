/**
 * mmap.h
 *
 * Minimal mmap implementation needed by the Go runtime.
 */
#ifndef _MMAP_H_
#define _MMAP_H_

#include <mini-os/types.h>

#define PROT_NONE       0x0
#define PROT_READ       0x1
#define PROT_WRITE      0x2
#define PROT_EXEC       0x4

#define MAP_ANON        0x1000
#define MAP_PRIVATE     0x2

void *sys_mmap(void *addr, size_t length, int32_t prot, int32_t flags, int32_t fd, int32_t offset);

#endif /* _MMAP_H_ */
