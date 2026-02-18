#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include <windows.h>

#include "event.h"

typedef struct event_s {
    HANDLE handle;
} event_s;

bool event_wait(void *ctx, int32_t timeout_ms) {
    event_s *event = (event_s *)ctx;
    if (!event)
        return false;
    if (WaitForSingleObject(event->handle, timeout_ms ? timeout_ms : INFINITE) != WAIT_OBJECT_0)
        return false;
    return true;
}

bool event_set(void *ctx) {
    event_s *event = (event_s *)ctx;
    if (!event || !SetEvent(event->handle))
        return false;
    return true;
}

void *event_create(void) {
    event_s *event = (event_s *)calloc(1, sizeof(event_s));
    if (!event)
        return NULL;
    event->handle = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!event->handle) {
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
    CloseHandle(event->handle);
    free(event);
    *ctx = NULL;
    return true;
}
