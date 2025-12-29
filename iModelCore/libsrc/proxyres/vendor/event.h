#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Sets an event to signalled state.
bool event_set(void *ctx);

// Waits for an event to be signalled.
bool event_wait(void *ctx, int32_t timeout_ms);

// Creates an event.
void *event_create(void);

// Delete an event.
bool event_delete(void **ctx);

#ifdef __cplusplus
}
#endif
