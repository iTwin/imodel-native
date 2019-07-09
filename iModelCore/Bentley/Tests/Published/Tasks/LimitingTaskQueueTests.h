/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <Bentley/Tasks/AsyncTasksManager.h>

USING_NAMESPACE_BENTLEY_TASKS

class LimitingTaskQueueTests : public ::testing::Test
    {
    void SetUp()
        {
        AsyncTasksManager::SetDefaultScheduler(nullptr);
        }
    void TearDown()
        {
        AsyncTasksManager::SetDefaultScheduler(nullptr);
        }
    };
