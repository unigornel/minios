#ifndef _STUBS_H_
#define _STUBS_H_

#include <mini-os/types.h>

#define PTHREAD_NAME_MAX_LEN 64

typedef struct {
    char name[PTHREAD_NAME_MAX_LEN];
    void *(*f)(void *);
    void *arg;
    void *tls;
} pthread_t;

int pthread_create(pthread_t *t, const void *attr, void *(*f)(void *), void *arg);
int pthread_mutex_lock(void *lock);
int pthread_mutex_unlock(void *lock);
int pthread_cond_wait(void *cond, void *mutex);
int pthread_cond_broadcast(void *cond);
int pthread_attr_init(void *attr);
int pthread_attr_getstacksize(void *attr, size_t *stacksize);
int pthread_attr_destroy(void *attr);
int pthread_sigmask(int how, void *set, void *oldset);

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
void _unimplemented_syscall(void);

#endif /* _STUBS_H_ */
