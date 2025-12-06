
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#include <windows.h>
#include <process.h>

#include "event.h"
#include "log.h"
#include "mutex.h"
#include "threadpool.h"

typedef struct threadpool_job_s {
    void *user_data;
    threadpool_job_cb callback;
    struct threadpool_job_s *next;
} threadpool_job_s;

struct threadpool_s;

typedef struct threadpool_thread_s {
    uintptr_t handle;
    struct threadpool_thread_s *next;
} threadpool_thread_s;

typedef struct threadpool_s {
    bool stop;
    int32_t min_threads;
    int32_t num_threads;
    int32_t max_threads;
    int32_t busy_threads;
    void *wakeup_cond;
    void *lazy_cond;
    int32_t queue_count;
    void *queue_lock;
    threadpool_job_s *queue_first;
    threadpool_job_s *queue_last;
    threadpool_thread_s *threads;
} threadpool_s;

static threadpool_job_s *threadpool_job_create(void *user_data, threadpool_job_cb callback) {
    threadpool_job_s *job = (threadpool_job_s *)calloc(1, sizeof(threadpool_job_s));
    if (!job)
        return NULL;
    job->user_data = user_data;
    job->callback = callback;
    job->next = NULL;
    return job;
}

static bool threadpool_job_delete(threadpool_job_s **job) {
    if (!job)
        return false;
    free(*job);
    *job = NULL;
    return true;
}

static void threadpool_enqueue_job(threadpool_s *threadpool, threadpool_job_s *job) {
    log_debug("threadpool - job 0x%" PRIxPTR " - enqueue", (intptr_t)job);

    // Add job to the end of the queue
    if (!threadpool->queue_last) {
        threadpool->queue_first = job;
        threadpool->queue_last = job;
    } else {
        threadpool->queue_last->next = job;
        threadpool->queue_last = job;
    }
    threadpool->queue_count++;
}

static threadpool_job_s *threadpool_dequeue_job(threadpool_s *threadpool) {
    if (!threadpool->queue_first)
        return NULL;

    // Remove the first job from the queue
    threadpool_job_s *job = threadpool->queue_first;
    threadpool->queue_first = threadpool->queue_first->next;
    if (!threadpool->queue_first)
        threadpool->queue_last = NULL;
    threadpool->queue_count--;

    log_debug("threadpool - job 0x%" PRIxPTR " - dequeue", (intptr_t)job);
    return job;
}

static void __cdecl threadpool_do_work(void *arg) {
    threadpool_s *threadpool = (threadpool_s *)arg;

    log_debug("threadpool - worker 0x%" PRIx32 " - started", GetCurrentThreadId());

    while (true) {
        mutex_lock(threadpool->queue_lock);
        log_debug("threadpool - worker 0x%" PRIx32 " - waiting for job", GetCurrentThreadId());

        // Sleep until there is work to do
        while (!threadpool->stop && !threadpool->queue_first) {
            mutex_unlock(threadpool->queue_lock);
            // queue_lock will be unlocked during sleep and locked during awake
            bool wakeup = event_wait(threadpool->wakeup_cond, 250);
            mutex_lock(threadpool->queue_lock);
            if (!wakeup)
                continue;
        }

        if (threadpool->stop)
            break;

        // Get the next job to do
        threadpool_job_s *job = threadpool_dequeue_job(threadpool);

        // Increment count of busy threads
        threadpool->busy_threads++;
        mutex_unlock(threadpool->queue_lock);

        // Do the job
        if (job) {
            log_debug("threadpool - worker 0x%" PRIx32 " - processing job 0x%" PRIxPTR, GetCurrentThreadId(),
                      (intptr_t)job);
            job->callback(job->user_data);
            log_debug("threadpool - worker 0x%" PRIx32 " - job complete 0x%" PRIxPTR, GetCurrentThreadId(),
                      (intptr_t)job);
            threadpool_job_delete(&job);
        }

        mutex_lock(threadpool->queue_lock);

        // Decrement count of busy threads
        threadpool->busy_threads--;

        // If no busy threads then signal threadpool_wait that we are lazy
        if (threadpool->busy_threads == 0 && !threadpool->queue_first)
            event_set(threadpool->lazy_cond);

        mutex_unlock(threadpool->queue_lock);
    }

    log_debug("threadpool - worker 0x%" PRIx32 " - stopped", GetCurrentThreadId());

    event_set(threadpool->lazy_cond);
    mutex_unlock(threadpool->queue_lock);
}

static void threadpool_create_thread_on_demand(threadpool_s *threadpool) {
    // Create new thread and add it to the list of threads
    uintptr_t handle = _beginthread(threadpool_do_work, 0, threadpool);
    if (handle == (uintptr_t)-1)
        return;

    threadpool_thread_s *thread = (threadpool_thread_s *)calloc(1, sizeof(threadpool_thread_s));
    thread->handle = handle;
    thread->next = threadpool->threads;

    threadpool->threads = thread;
    threadpool->num_threads++;
}

bool threadpool_enqueue(void *ctx, void *user_data, threadpool_job_cb callback) {
    threadpool_s *threadpool = (threadpool_s *)ctx;

    // Create new job
    threadpool_job_s *job = threadpool_job_create(user_data, callback);
    if (!job)
        return false;

    mutex_lock(threadpool->queue_lock);

    // Add job to the job queue
    threadpool_enqueue_job(threadpool, job);

    // Create new thread if all threads are busy
    if (threadpool->busy_threads == threadpool->num_threads && threadpool->num_threads < threadpool->max_threads)
        threadpool_create_thread_on_demand(threadpool);

    mutex_unlock(threadpool->queue_lock);

    // Wake up waiting threads
    event_set(threadpool->wakeup_cond);
    return true;
}

static void threadpool_stop_threads(threadpool_s *threadpool) {
    mutex_lock(threadpool->queue_lock);
    // Stop threads from doing anymore work
    threadpool->stop = true;
    mutex_unlock(threadpool->queue_lock);

    // Wake up all threads to check stop flag
    event_set(threadpool->wakeup_cond);
}

static void threadpool_delete_threads(threadpool_s *threadpool) {
    threadpool_thread_s *thread = NULL;

    // Delete threads from list of threads
    while (threadpool->threads) {
        thread = threadpool->threads;
        threadpool->threads = threadpool->threads->next;

        // Wait for thread to exit
        while (true) {
            // Signal wake up condition to wake up threads to stop
            event_set(threadpool->wakeup_cond);
            DWORD wait = WaitForSingleObject((HANDLE)thread->handle, 250);
            if (wait != WAIT_TIMEOUT)
                break;
        }

        free(thread);
    }
    threadpool->num_threads = 0;
}

static void threadpool_delete_jobs(threadpool_s *threadpool) {
    threadpool_job_s *job = NULL;

    // Delete job from the queue
    while (threadpool->queue_first) {
        job = threadpool->queue_first;
        threadpool->queue_first = threadpool->queue_first->next;
        threadpool_job_delete(&job);
    }
}

void threadpool_wait(void *ctx) {
    threadpool_s *threadpool = (threadpool_s *)ctx;
    if (!threadpool)
        return;

    mutex_lock(threadpool->queue_lock);
    while (true) {
        if ((!threadpool->stop && (threadpool->busy_threads != 0 || threadpool->queue_count != 0)) ||
            (threadpool->stop && threadpool->num_threads != 0)) {
            mutex_unlock(threadpool->queue_lock);
            // Wait for signal that indicates there is no more work to do
            event_wait(threadpool->lazy_cond, 250);
            mutex_lock(threadpool->queue_lock);
        } else {
            break;
        }
    }
    mutex_unlock(threadpool->queue_lock);
}

void *threadpool_create(int32_t min_threads, int32_t max_threads) {
    threadpool_s *threadpool = (threadpool_s *)calloc(1, sizeof(threadpool_s));
    if (!threadpool)
        return NULL;

    threadpool->queue_lock = mutex_create();
    if (!threadpool->queue_lock) {
        free(threadpool);
        return NULL;
    }

    threadpool->wakeup_cond = event_create();
    if (!threadpool->wakeup_cond) {
        mutex_delete(&threadpool->queue_lock);
        free(threadpool);
        return NULL;
    }

    threadpool->lazy_cond = event_create();
    if (!threadpool->lazy_cond) {
        event_delete(&threadpool->wakeup_cond);
        mutex_delete(&threadpool->queue_lock);
        free(threadpool);
        return NULL;
    }

    threadpool->min_threads = min_threads;
    threadpool->max_threads = max_threads;

    return threadpool;
}

bool threadpool_delete(void **ctx) {
    if (!ctx)
        return false;
    threadpool_s *threadpool = (threadpool_s *)*ctx;
    if (!threadpool)
        return false;

    threadpool_stop_threads(threadpool);
    threadpool_delete_threads(threadpool);
    threadpool_delete_jobs(threadpool);

    mutex_delete(&threadpool->queue_lock);

    event_delete(&threadpool->wakeup_cond);
    event_delete(&threadpool->lazy_cond);

    free(threadpool);
    *ctx = NULL;
    return true;
}
