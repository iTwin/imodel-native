#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <gtest/gtest.h>

#include "threadpool.h"

TEST(threadpool, create) {
    void *pool = threadpool_create(1, 1);
    ASSERT_NE(pool, nullptr);
    EXPECT_TRUE(threadpool_delete(&pool));
    ASSERT_EQ(pool, nullptr);
}

static void threadpool_run_one_worker(void *arg) {
    bool *job_was_run = (bool *)arg;
    *job_was_run = true;
}

TEST(threadpool, run_one) {
    bool job_was_run = false;
    void *pool = threadpool_create(1, 4);
    ASSERT_NE(pool, nullptr);
    EXPECT_TRUE(threadpool_enqueue(pool, &job_was_run, threadpool_run_one_worker));
    threadpool_wait(pool);
    EXPECT_TRUE(job_was_run);
    EXPECT_TRUE(threadpool_delete(&pool));
    ASSERT_EQ(pool, nullptr);
}

static void threadpool_run_many_worker(void *arg) {
    bool *job_was_run = (bool *)arg;
    *job_was_run = true;
}

TEST(threadpool, run_many) {
    bool job_was_run[100] = {false};
    void *pool = threadpool_create(1, 4);
    ASSERT_NE(pool, nullptr);
    for (int32_t i = 0; i < sizeof(job_was_run); i++)
        EXPECT_TRUE(threadpool_enqueue(pool, &job_was_run[i], threadpool_run_many_worker));
    threadpool_wait(pool);
    for (int32_t i = 0; i < sizeof(job_was_run); i++)
        EXPECT_TRUE(job_was_run[i]);
    EXPECT_TRUE(threadpool_delete(&pool));
    ASSERT_EQ(pool, nullptr);
}
