/**
 * gdt.c
 *
 * We need a GDT to implement TLS for the Go runtime.
 */

#include <mini-os/gdt.h>

#include <mini-os/hypervisor.h>
#include <mini-os/mm.h>
#include <mini-os/types.h>
#include <mini-os/lib.h>
#include <mini-os/crash.h>

/**
 * seg_desc_t represents one entry in the GDT.
 */
struct _segment_descriptor {
    unsigned lolimit:16;    /* low bits of maximum segment size */
    unsigned lobase:24;     /* low bits of segment starting address */
    unsigned accessed:1;    /* segment accessed */
    unsigned rw:1;          /* readable for code, writable for data */
    unsigned dc:1;          /* direction for data, conforming for code */
    unsigned ex:1;          /* executable for code */
    unsigned system:1;      /* system critical descriptor */
    unsigned dpl:2;         /* descriptor privilege level */
    unsigned present:1;     /* present bit, must be 1 */
    unsigned hilimit:4;     /* high bits of maximum segment size */
    unsigned zero:2;        /* always zero */
    unsigned size:1;        /* size of p-mode segment */
    unsigned gran:1;        /* granularity of segment 1b for 0, 4K for 1 */
    unsigned hibase:8;      /* high bits of maximum segment size */
} __attribute__((__packed__));

typedef struct _segment_descriptor seg_desc_t;

typedef enum {
    seg_desc_type_rwa, /* read write accessed */
    seg_desc_type_era, /* executable read accessed */
} seg_desc_type_t;

/**
 * NUM_GDT_ENTRIES is the maximum number of entries in the GDT.
 *
 * We limit the GDT size to one page.
 */
#define NUM_GDT_ENTRIES (PAGE_SIZE/sizeof(seg_desc_t))

/* First descriptor should be the NULL descriptor, so start at index 1 */
#define KERNEL_FS ((1 << 3) | 1)
#define SEG_DESC_FS (KERNEL_FS >> 3)

/**
 * gdt contains the full GDT.
 *
 * The GDT is limited to one page and is aligned on a page size boundary.
 */
static seg_desc_t gdt[NUM_GDT_ENTRIES] __attribute__((aligned(PAGE_SIZE)));

/**
 * fs_pa contains the physical adddess to the fs segment descriptor
 */
static uint64_t fs_pa;

/**
 * fs is the descriptor for the fs segment
 */
static union {
    seg_desc_t desc;
    uint64_t raw;
} fs;


void seg_desc_fill(seg_desc_t *sd, seg_desc_type_t type)
{
    sd->lobase = 0;
    sd->hibase = 0;
    sd->lolimit = 0;
    sd->hilimit = 0;

    sd->dpl = 1;
    sd->present = 1;
    sd->zero = 0;
    sd->size = 1;
    sd->gran = 1;

    switch (type) {
    case seg_desc_type_rwa:
        sd->accessed = 1;
        sd->rw = 1;
        sd->dc = 0;
        sd->ex = 0;
        sd->system = 1;
        break;

    case seg_desc_type_era: /* fallthrough */
    default:
        sd->accessed = 1;
        sd->rw = 1;
        sd->dc = 0;
        sd->ex = 1;
        sd->system = 1;
        break;
    }
}

void switch_fs(unsigned long p)
{
    ASSERT((p & 0xFFFFFFFF) == p, "FS base is limited to 32 bits");

    fs.desc.lobase = p & 0xFFFFFF;
    fs.desc.hibase = (p >> 24) & 0xFF;
    if (HYPERVISOR_update_descriptor(fs_pa, fs.raw)) {
        printk("Cannot update FS descriptor\n");
        *(char *)0 = 0;
    }

    __asm__ __volatile__("mov %0, %%fs" :: "r"(KERNEL_FS));
}

unsigned long get_fs(void)
{
    unsigned long p = 0;
    p |= (unsigned long)gdt[SEG_DESC_FS].hibase << 24;
    p |= (unsigned long)gdt[SEG_DESC_FS].lobase;
    return p;
}

void init_gdt(void)
{
    pte_t pte;
    unsigned long frames[1];

    seg_desc_fill(&gdt[SEG_DESC_FS], seg_desc_type_rwa);

    pte = __pte((virt_to_mach(&gdt)) | L1_PROT_RO);
    if (HYPERVISOR_update_va_mapping((unsigned long)&gdt, pte, UVMF_INVLPG)) {
        printk("Cannot make GDT read-only\n");
        *(char *)0 = 0;
    }

    frames[0] = virt_to_mfn(&gdt);
    if (HYPERVISOR_set_gdt(frames, NUM_GDT_ENTRIES)) {
        printk("Cannot set the GDT\n");
        *(char *)0 = 0;
    }

    memcpy(&fs, &gdt[SEG_DESC_FS], sizeof(fs));
    fs_pa = virt_to_mach(&gdt[SEG_DESC_FS]);
}
