/**
 * runtime.h
 *
 * This file contains some implementations and stubs of functions needed by
 * the Go runtime.
 *  - strerror, fprintf, fwrite nad abort
 *  - setenv, unsetenv
 *  - signaling
 */

#ifndef _RUNTIME_H_
#define _RUNTIME_H_

#include <mini-os/types.h>

extern void *stderr;
char *strerror(int errno);
int __fprintf_chk(void *fh, int flag, const char *format, ...);
void abort(void);
int sigaltstack(const void *ss, void *oss);
int sigfillset(void *set);
int setenv(const void *name, const char *value, int ow);
int unsetenv(const void *name);
int fprintf(void *fh, const char *format, ...);
size_t fwrite(const void *data, size_t size, size_t n, void *fh);

#endif /* _RUNTIME_H_ */
