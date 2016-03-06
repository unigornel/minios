/**
 * go_mm.c
 *
 * Best fit page allocator suited for the Go runtime
 */
#include <mini-os/types.h>
#include <mini-os/mm.h>
#include <mini-os/console.h>
#include <mini-os/lib.h>

#define round_pgdown(p) ((p) & PAGE_MASK)
#define round_pgup(p) (((p) + (PAGE_SIZE-1)) & PAGE_MASK)

static struct {
    uint64_t *bitmap;
    uint64_t bitmap_size;
    unsigned long min_phys;
    unsigned long max_phys;
    unsigned long num_pages;
} mm;

static void init_page_allocator(unsigned long min, unsigned long max);
static void bitmap_free(unsigned long first_page, unsigned long num_pages);
static void bitmap_set(unsigned long first_page, unsigned long num_pages);
static unsigned long bitmap_num_free(unsigned long first_page, unsigned long max_pages);

void init_mm(void)
{
    unsigned long start_pfn, max_pfn;

    printk("go_mm: initializing\n");

    arch_init_mm(&start_pfn, &max_pfn);

    printk("go_mm: initializing best fit page allocator for %lx-%lx\n",
        (unsigned long)pfn_to_virt(start_pfn),
        (unsigned long)pfn_to_virt(max_pfn));
    init_page_allocator(PFN_PHYS(start_pfn), PFN_PHYS(max_pfn));
    printk("go_mm: done\n");

    arch_init_p2m(max_pfn);
    arch_init_demand_mapping_area(max_pfn);
}

void fini_mm(void)
{
}

unsigned long alloc_pages_aligned(int order, int zero_bits) {
    unsigned long i, c, l, virt, align_mask;

    c = 1UL << order;
    i = 0;

    align_mask = ~((1 << zero_bits) - 1);

    while(i + c < mm.num_pages) {
        virt = (unsigned long)pfn_to_virt(i);
        if(virt != (virt & align_mask)) {
            i++;
            continue;
        }

        l = bitmap_num_free(i, c);
        if(l == c) {
            bitmap_set(i, c);
            return virt;
        }
        i += l + 1;
    }

    printk("Cannot allocate %lu pages\n", c);
    return 0;
}

void free_pages(void *pointer, int order) {
    unsigned long num_pages, first_page;

    num_pages = 1UL << order;
    first_page = virt_to_pfn(pointer);
    bitmap_free(first_page, num_pages);
}

static void init_page_allocator(unsigned long min, unsigned long max)
{
    mm.min_phys = round_pgup(min);
    mm.max_phys = round_pgdown(max);

    mm.bitmap_size = (mm.max_phys + 1) >> (PAGE_SHIFT + 3);
    mm.bitmap_size = round_pgup(mm.bitmap_size);
    mm.bitmap = (uint64_t *)to_virt(mm.min_phys);
    mm.min_phys += mm.bitmap_size;
    memset(mm.bitmap, ~0, mm.bitmap_size);

    mm.num_pages = (mm.max_phys - mm.min_phys) >> PAGE_SHIFT;
    bitmap_free(PHYS_PFN(mm.min_phys), mm.num_pages);

    printk("go_mm: page allocator manages %lu free pages\n", mm.num_pages);
}

static void bitmap_free(unsigned long first_page, unsigned long num_pages)
{
    unsigned long curr_idx, bit_idx;
    uint64_t bit_mask;

    while(num_pages > 0) {
        curr_idx = first_page >> 6;
        bit_idx = first_page & 63;
        bit_mask = 1UL << bit_idx;
        mm.bitmap[curr_idx] &= ~bit_mask;
        first_page++;
        num_pages--;
    }
}

static void bitmap_set(unsigned long first_page, unsigned long num_pages)
{
    unsigned long curr_idx, bit_idx;
    uint64_t bit_mask;

    while(num_pages > 0) {
        curr_idx = first_page >> 6;
        bit_idx = first_page & 63;
        bit_mask = 1UL << bit_idx;
        mm.bitmap[curr_idx] |= bit_mask;
        first_page++;
        num_pages--;
    }
}

/**
 * bitmap_num_free checks whether `first_page` is the start of a list of
 * `max_pages` free pages.
 *
 * The function returns the number of free pages that follow `first_page` plus
 * one if `first_page` is free. If this number is higher than `max_pages`,
 * `max_pages` is returned.
 *
 * If `first_page` is not free or `max_pages` is zero, zero is returned.
 *
 * This function does not check whether `first_page + num_pages > num_pages` and
 * thus can cause an overflow.
 */
static unsigned long bitmap_num_free(unsigned long first_page, unsigned long max_pages)
{
    unsigned long curr_idx, bit_idx, count, i;
    uint64_t bit_mask;

    count = 0;
    i = first_page;
    while(max_pages > 0) {
        curr_idx = i >> 6;
        bit_idx = i & 63;
        bit_mask = 1UL << bit_idx;
        if(mm.bitmap[curr_idx] & bit_mask) {
            break;
        } else {
            count++;
        }
        max_pages--;
        i++;
    }

    return count;
}
