#include <mini-os/types.h>
#include <mini-os/mm.h>
#include <mini-os/crash.h>

// Stubs to link with go
// All pthread stubs go here
int pthread_create(void *thread, const void *attr, void *(*f)(void *), void *arg) {
    CRASH("pthread_create is not implemented");
    ASSERT(0, "fack");
    return 1;
}

int pthread_mutex_lock(void *lock) {
    CRASH("pthread_mutex_lock is not implemented");
    return 1;
}

int pthread_mutex_unlock(void *lock) {
    CRASH("pthread_mutex_unlock is not implemented");
    return 1;
}

int pthread_cond_wait(void *cond, void *mutex) {
    CRASH("pthread_cond_wait is not implemented");
    return 1;
}

int pthread_cond_broadcast(void *cond) {
    CRASH("pthread_cond_broadcast is not implemented");
    return 1;
}

int pthread_attr_init(void *attr) {
    return 0;
}

int pthread_attr_getstacksize(void *attr, size_t *stacksize) {
    *stacksize = STACK_SIZE;
    return 0;
}

int pthread_attr_destroy(void *attr) {
    return 0;
}

int pthread_sigmask(int how, void *set, void *oldset) {
    CRASH("pthread_sigmask is not implemented");
    return 1;
}

// Other stubs
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
    CRASH("signaltstack is not implemented");
    return -1;
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

void _unimplemented_syscall(void) {
    CRASH("_unimplemented_syscall was called")
}
