/**
 * pthread.h
 *
 * Minimal pthreads implementation for Mini-OS as needed by the Go runtime.
 */

#ifndef _PTHREAD_H_
#define _PTHREAD_H_

#include <mini-os/types.h>
#include <mini-os/waittypes.h>

#define PTHREAD_NAME_MAX_LEN 64

typedef struct {
    char name[PTHREAD_NAME_MAX_LEN];
    void *(*f)(void *);
    void *arg;
    void *tls;
} pthread_t;

int pthread_create(pthread_t *t, const void *attr, void *(*f)(void *), void *arg);
int pthread_create_name(pthread_t *t, const char *name, const void *attr, void *(*f)(void *), void *arg);
int pthread_mutex_lock(void *lock);
int pthread_mutex_unlock(void *lock);
int pthread_cond_wait(struct wait_queue_head *wq, int *condition);
int pthread_cond_broadcast(struct wait_queue_head *wq);
int pthread_attr_init(void *attr);
int pthread_attr_getstacksize(void *attr, size_t *stacksize);
int pthread_attr_destroy(void *attr);
int pthread_sigmask(int how, void *set, void *oldset);

#endif /* _PTHREAD_H_ */
