#include <mini-os/types.h>
#include <mini-os/mm.h>
#include <mini-os/crash.h>
#include <mini-os/sched.h>
#include <mini-os/xmalloc.h>
#include <mini-os/lib.h>

#define PTHREAD_NAME_MAX_LEN 64

typedef struct {
    char name[PTHREAD_NAME_MAX_LEN];
    void *(*f)(void *);
    void *arg;
} pthread_t;

unsigned int pthread_counter = 0;

static void wrap_thread(void *ctx) {
    pthread_t *thread;

    thread = (pthread_t *)ctx;
    (thread->f)(thread->arg);

    free(thread);
}

int pthread_create(void *a1, const void *attr, void *(*f)(void *), void *arg) {
    pthread_t *thread;
    struct thread *os_thread;

    thread = malloc(sizeof(*thread));
    snprintf(thread->name, PTHREAD_NAME_MAX_LEN, "pthread-%u", pthread_counter);
    thread->f = f;
    thread->arg = arg;
    pthread_counter++;

    os_thread = create_thread(thread->name, wrap_thread, thread);
    os_thread->fs = alloc_page() + PAGE_SIZE;

    return 0;
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
    return 0;
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

void _unimplemented_syscall(void) {
    CRASH("_unimplemented_syscall was called")
}
