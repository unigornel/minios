/**
 * runtime.c
 *
 * This file contains some implementations and stubs of functions needed by
 * the Go runtime.
 *  - strerror, fprintf, fwrite nad abort
 *  - setenv, unsetenv
 *  - signaling
 */
#include <mini-os/runtime.h>

#include <mini-os/crash.h>

void *stderr;

char *strerror(int errno) {
    return "";
}

int __fprintf_chk(void *fh, int flag, const char *format, ...) {
    CRASH("__fprintf_chk is not implemented");
    return -1;
}

void abort(void) {
    CRASH("abort was called");
}

int sigaltstack(const void *ss, void *oss) {
    return 0;
}

int sigfillset(void *set) {
    return 0;
}

int setenv(const void *name, const char *value, int ow) {
    CRASH("setenv is not implemented");
    return -1;
}

int unsetenv(const void *name) {
    CRASH("unsetenv is not implemented");
    return 0;
}

int fprintf(void *fh, const char *format, ...) {
    CRASH("fprintf is not implemented");
    return -1;
}

size_t fwrite(const void *data, size_t size, size_t n, void *fh) {
    return 0;
}
