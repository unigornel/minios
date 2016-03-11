/*
 * gdt.h
 *
 * We need a GDT to implement TLS for the Go runtime.
 */
#ifndef _GDT_H_
#define _GDT_H_

void init_gdt(void);
void switch_fs(unsigned long p);
unsigned long get_fs(void);

#endif /* GDT_H_ */
