#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <windows.h>

#include "log.h"
#include "mutex.h"
#include "threadpool.h"
#include "util.h"

typedef struct threadpool_job_s {
    PTP_WORK handle;
    void *user_data;
    threadpool_job_cb callback;
    struct threadpool_s *pool;
    struct threadpool_job_s *next;
    struct threadpool_job_s *prev;
} threadpool_job_s;

typedef struct threadpool_s {
    PTP_POOL handle;
    TP_CALLBACK_ENVIRON cb_environ;
    void *queue_lock;
    int32_t queue_count;
    threadpool_job_s *queue_first;
    threadpool_job_s *queue_last;
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

static bool threadpool_add_job(threadpool_s *threadpool, threadpool_job_s *job) {
    log_debug("threadpool - job 0x%" PRIxPTR " - add", (intptr_t)job);

    // Add job to the end of the queue
    if (!threadpool->queue_last) {
        threadpool->queue_first = job;
        threadpool->queue_last = job;
    } else {
        job->prev = threadpool->queue_last;
        threadpool->queue_last->next = job;
        threadpool->queue_last = job;
    }
    threadpool->queue_count++;
    return true;
}

static void threadpool_remove_job(threadpool_s *threadpool, threadpool_job_s *job) {
    // Remove the job from the queue
    if (job == threadpool->queue_first) {
        threadpool->queue_first = job->next;
        if (threadpool->queue_first)
            threadpool->queue_first->prev = NULL;
    } else {
        job->prev->next = job->next;
        if (job->next)
            job->next->prev = job->prev;
    }
    if (job == threadpool->queue_last)
        threadpool->queue_last = job->prev;
    threadpool->queue_count--;

    log_debug("threadpool - job 0x%" PRIxPTR " - remove", (intptr_t)job);
}

VOID CALLBACK threadpool_job_callback(PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_WORK work) {
    UNUSED(instance);

    threadpool_job_s *job = (threadpool_job_s *)context;
    if (!job)
        return;

    // Do the job
    log_debug("threadpool - worker 0x%" PRIxPTR " - processing job 0x%" PRIxPTR, (intptr_t)work, (intptr_t)job);
    job->callback(job->user_data);
    log_debug("threadpool - worker 0x%" PRIxPTR " - job complete 0x%" PRIxPTR, (intptr_t)work, (intptr_t)job);

    // Remove job from job queue
    threadpool_s *threadpool = job->pool;
    mutex_lock(threadpool->queue_lock);
    threadpool_remove_job(threadpool, job);
    mutex_unlock(threadpool->queue_lock);
}

bool threadpool_enqueue(void *ctx, void *user_data, threadpool_job_cb callback) {
    threadpool_s *threadpool = (threadpool_s *)ctx;

    threadpool_job_s *job = threadpool_job_create(user_data, callback);
    if (!job)
        return false;

    job->pool = threadpool;
    job->handle = CreateThreadpoolWork(threadpool_job_callback, job, &threadpool->cb_environ);
    if (!job->handle) {
        threadpool_job_delete(&job);
        return false;
    }

    mutex_lock(threadpool->queue_lock);
    threadpool_add_job(threadpool, job);
    mutex_unlock(threadpool->queue_lock);

    SubmitThreadpoolWork(job->handle);
    return true;
}

void threadpool_wait(void *ctx) {
    threadpool_s *threadpool = (threadpool_s *)ctx;
    threadpool_job_s *job = NULL;
    PTP_WORK work_handle = NULL;

    if (!threadpool)
        return;

    while (true) {
        mutex_lock(threadpool->queue_lock);
        job = threadpool->queue_first;
        if (job)
            work_handle = job->handle;
        mutex_unlock(threadpool->queue_lock);
        if (!job)
            break;
        WaitForThreadpoolWorkCallbacks(work_handle, FALSE);
    }
}

void *threadpool_create(int32_t min_threads, int32_t max_threads) {
    threadpool_s *threadpool = (threadpool_s *)calloc(1, sizeof(threadpool_s));
    if (!threadpool)
        return NULL;

    InitializeThreadpoolEnvironment(&threadpool->cb_environ);

    threadpool->handle = CreateThreadpool(NULL);
    if (!threadpool->handle) {
        DestroyThreadpoolEnvironment(&threadpool->cb_environ);
        free(threadpool);
        return NULL;
    }

    threadpool->queue_lock = mutex_create();
    if (!threadpool->queue_lock) {
        CloseThreadpool(threadpool->handle);
        DestroyThreadpoolEnvironment(&threadpool->cb_environ);
        free(threadpool);
        return NULL;
    }

    SetThreadpoolThreadMinimum(threadpool->handle, min_threads);
    SetThreadpoolThreadMaximum(threadpool->handle, max_threads);

    SetThreadpoolCallbackPool(&threadpool->cb_environ, threadpool->handle);
    return threadpool;
}

bool threadpool_delete(void **ctx) {
    if (!ctx)
        return false;
    threadpool_s *threadpool = (threadpool_s *)*ctx;
    if (!threadpool)
        return false;
    if (threadpool->handle)
        CloseThreadpool(threadpool->handle);
    mutex_delete(&threadpool->queue_lock);
    DestroyThreadpoolEnvironment(&threadpool->cb_environ);
    free(threadpool);
    *ctx = NULL;
    return true;
}
