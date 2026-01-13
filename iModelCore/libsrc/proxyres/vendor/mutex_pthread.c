#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include <pthread.h>

typedef struct mutex_s {
    pthread_mutex_t handle;
} mutex_s;

bool mutex_lock(void *ctx) {
    mutex_s *mutex = (mutex_s *)ctx;
    if (!mutex || pthread_mutex_lock(&mutex->handle))
        return false;
    return true;
}

bool mutex_unlock(void *ctx) {
    mutex_s *mutex = (mutex_s *)ctx;
    if (!mutex || pthread_mutex_unlock(&mutex->handle))
        return false;
    return true;
}

void *mutex_create(void) {
    mutex_s *mutex = (mutex_s *)calloc(1, sizeof(mutex_s));
    if (!mutex)
        return NULL;
    if (pthread_mutex_init(&mutex->handle, NULL)) {
        free(mutex);
        return NULL;
    }
    return mutex;
}
bool mutex_delete(void **ctx) {
    if (!ctx)
        return false;
    mutex_s *mutex = (mutex_s *)*ctx;
    if (!mutex)
        return false;
    if (pthread_mutex_destroy(&mutex->handle)) {
        free(mutex);
        *ctx = NULL;
        return false;
    }
    free(mutex);
    *ctx = NULL;
    return true;
}