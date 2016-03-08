/**
 * mmap.c
 *
 * Minimal mmap implementation needed by the Go runtime.
 */
#include <mini-os/mmap.h>

#include <mini-os/mm.h>
#include <mini-os/crash.h>

#define MAX_RESERVE (100UL << (10 + 10)) // 100 MByte

static struct {
    unsigned long address;
    unsigned long pages;
    unsigned long max_address;
} reserved;

static void *reserve(void *addr, size_t length);
static void *alloc_reserved(void *addr, size_t length);

void *sys_mmap(void *addr, size_t length, int32_t prot, int32_t flags, int32_t fd, int32_t offset)
{
    unsigned long num_pages;

    printk("mmap(addr=%p,len=0x%lx,prot=0x%x, flags=0x%x)\n", addr, length, prot, flags);

    if(flags != (MAP_ANON | MAP_PRIVATE)) {
        CRASH("expected flags to be MAP_ANON | MAP_PRIVATE, got 0x%d", flags);
    }

    /* Reserve memory */
    if(addr != NULL && prot == PROT_NONE) {
        return reserve(addr, length);
    }

    /* Allocate from reserved memory */
    if(prot != (PROT_READ | PROT_WRITE)) {
        CRASH("expected protection to be PROT_READ | PROT_WRITE, got 0x%d", prot);
    }

    if(addr != NULL) {
        return alloc_reserved(addr, length);
    }

    /* Allocate unreserved memory */
    num_pages = length >> PAGE_SHIFT;
    num_pages += ((length & PAGE_SIZE) > 0);

    return (void *)alloc_num_pages_aligned(num_pages, 0);
}

static void *reserve(void *addr, size_t length)
{
    unsigned long num_pages;

    if(reserved.address != 0) {
        CRASH("memory was already reserved");
    }

    if(length > MAX_RESERVE) {
        length = MAX_RESERVE;
    }

    num_pages = length >> PAGE_SHIFT;
    num_pages += ((length & PAGE_SIZE) > 0);

    reserved.address = alloc_num_pages_aligned(num_pages, 0);
    reserved.pages = num_pages;
    reserved.max_address = reserved.address + (num_pages << PAGE_SHIFT);

    return (void *)reserved.address;
}

static void *alloc_reserved(void *addr, size_t length)
{
    unsigned long p;

    p = (unsigned long)addr;
    ASSERT(reserved.address != 0, "allocating from uninitialized reserved memory");
    ASSERT(p >= reserved.address, "allocating below reserved memory");
    ASSERT(p + length < reserved.max_address, "allocating above reserved memory");

    return addr;
}

