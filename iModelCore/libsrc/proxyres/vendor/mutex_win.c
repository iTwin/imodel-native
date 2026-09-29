#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include <windows.h>

typedef struct mutex_s {
    CRITICAL_SECTION handle;
} mutex_s;

bool mutex_lock(void *ctx) {
    mutex_s *mutex = (mutex_s *)ctx;
    if (!mutex)
        return false;
    EnterCriticalSection(&mutex->handle);
    return true;
}

bool mutex_unlock(void *ctx) {
    mutex_s *mutex = (mutex_s *)ctx;
    if (!mutex)
        return false;
    LeaveCriticalSection(&mutex->handle);
    return true;
}

void *mutex_create(void) {
    mutex_s *mutex = (mutex_s *)calloc(1, sizeof(mutex_s));
    if (!mutex)
        return NULL;
    InitializeCriticalSection(&mutex->handle);
    return mutex;
}

bool mutex_delete(void **ctx) {
    if (!ctx)
        return false;
    mutex_s *mutex = (mutex_s *)*ctx;
    if (!mutex)
        return false;
    DeleteCriticalSection(&mutex->handle);
    free(mutex);
    *ctx = NULL;
    return true;
}