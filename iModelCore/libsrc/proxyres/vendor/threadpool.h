#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define THREADPOOL_DEFAULT_MIN_THREADS 1
#define THREADPOOL_DEFAULT_MAX_THREADS 3

typedef void (*threadpool_job_cb)(void *user_data);

// Add a job to the thread pool.
bool threadpool_enqueue(void *ctx, void *user_data, threadpool_job_cb callback);
// Wait for thread pool to finish all jobs.
void threadpool_wait(void *ctx);

// Create a thread pool instance.
void *threadpool_create(int32_t min_threads, int32_t max_threads);

// Deletes a thread pool instance.
bool threadpool_delete(void **ctx);

#ifdef __cplusplus
}
#endif
