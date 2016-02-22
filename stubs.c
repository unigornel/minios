#include <types.h>

// Stubs to link with go
// All pthread stubs go here
int pthread_create(void *thread, const void *attr, void *(*f)(void *), void *arg) {
    return 1;
}

int pthread_mutex_lock(void *lock) {
    return 1;
}

int pthread_mutex_unlock(void *lock) {
    return 1;
}

int pthread_cond_wait(void *cond, void *mutex) {
    return 1;
}

int pthread_cond_broadcast(void *cond) {
    return 1;
}

int pthread_attr_init(void *attr) {
    return 1;
}

int pthread_attr_getstacksize(void *attr, void *stacksize) {
    return 1;
}

int pthread_attr_destroy(void *attr) {
    return 1;
}

int pthread_sigmask(int how, void *set, void *oldset) {
    return 1;
}

// Other stubs
void *stderr;

char *strerror(int errno) {
    return "";
}

int __fprintf_chk(void *fh, int flag, const char *format, ...) {
    return -1;
}

void abort(void) {
    char *null = (char *)0;
    *null = 0;
}

int sigaltstack(const void *ss, void *oss) {
    return -1;
}

int sigfillset(void *set) {
    return -1;
}

int setenv(const void *name, const char *value, int ow) {
    return -1;
}

int unsetenv(const void *name) {
    return 0;
}

int fprintf(void *fh, const char *format, ...) {
    return -1;
}

size_t fwrite(const void *data, size_t size, size_t n, void *fh) {
    return 0;
}
