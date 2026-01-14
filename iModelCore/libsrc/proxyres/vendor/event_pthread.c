#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include <pthread.h>

#include "event.h"

typedef struct event_s {
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    bool signalled;
} event_s;

bool event_set(void *ctx) {
    event_s *event = (event_s *)ctx;
    if (!event)
        return false;
    pthread_mutex_lock(&event->mutex);
    int32_t err = pthread_cond_signal(&event->cond);
    if (err == 0)
        event->signalled = true;
    pthread_mutex_unlock(&event->mutex);
    return err == 0;
}

bool event_wait(void *ctx, int32_t timeout_ms) {
    event_s *event = (event_s *)ctx;
    int32_t err = 0;

    if (!event)
        return false;

    pthread_mutex_lock(&event->mutex);
    if (!event->signalled) {
        if (timeout_ms < 0) {
            err = pthread_cond_wait(&event->cond, &event->mutex);
        } else {
            struct timespec ts;

            ts.tv_sec = timeout_ms / 1000;
            ts.tv_nsec = (timeout_ms % 1000) * 1000000;

            err = pthread_cond_timedwait(&event->cond, &event->mutex, &ts);
        }
        if (err == 0)
            event->signalled = true;
    }
    pthread_mutex_unlock(&event->mutex);
    return err == 0;
}

void *event_create(void) {
    event_s *event = (event_s *)calloc(1, sizeof(event_s));
    if (!event)
        return NULL;
    if (pthread_cond_init(&event->cond, NULL)) {
        free(event);
        return NULL;
    }
    if (pthread_mutex_init(&event->mutex, NULL)) {
        pthread_cond_destroy(&event->cond);
        free(event);
        return NULL;
    }
    return event;
}

bool event_delete(void **ctx) {
    if (!ctx)
        return false;
    event_s *event = (event_s *)*ctx;
    if (!event)
        return false;
    pthread_cond_destroy(&event->cond);
    pthread_mutex_destroy(&event->mutex);
    free(event);
    *ctx = NULL;
    return true;
}
