/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/Bentley/Tasks/AsyncTaskFollyAdapter.h $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
 //__PUBLISH_SECTION_START__

#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/ThreadlessTaskScheduler.h>

// This is only helper code, so Bentley lib does not depend on BeFolly.
// BeFolly needs to be added as dependency to consuming code.
#include <folly/BeFolly.h>

BEGIN_BENTLEY_TASKS_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct AsyncTaskFollyAdapter
    {
    //! Convert AsyncTaskPtr to folly::Future with value
    template<class T>
    static folly::Future<T> ToFolly(AsyncTaskPtr<T> asyncTask)
        {
        auto follyPromise = std::make_shared<folly::Promise<T>>();

        asyncTask->Then(std::make_shared<ThreadlessTaskScheduler>(), [=] (T value)
            {
            follyPromise->setValue(std::move(value));
            });

        return follyPromise->getFuture().via(&BeFolly::ThreadPool::GetCpuPool());
        }

    //! Convert AsyncTaskPtr to folly::Future with no value
    static folly::Future<folly::Unit> ToFolly(AsyncTaskPtr<void> asyncTask)
        {
        auto follyPromise = std::make_shared<folly::Promise<folly::Unit>>();

        asyncTask->Then(std::make_shared<ThreadlessTaskScheduler>(), [=]
            {
            follyPromise->setValue(folly::Unit());
            });

        return follyPromise->getFuture().via(&BeFolly::ThreadPool::GetCpuPool());
        }

    //! Convert folly::Future to AsyncTaskPtr with value. Future object should not throw any exceptions.
    template<class T>
    static AsyncTaskPtr<T> FromFolly(folly::Future<T>& future)
        {
        auto valuePtr = std::make_shared<T>();
        auto task = std::make_shared<PackagedAsyncTask<T>>([=]
            {
            return *valuePtr;
            });

        auto parentTask = AsyncTasksManager::GetCurrentThreadAsyncTask();
        if (parentTask)
            parentTask->AddSubTask(task);

        future.onError([=] (folly::exception_wrapper ew)
            {
            BeAssert(false && "Received folly::Future exception, cannot convert to AsyncTask, returning default value.");
            return T();
            }).then([=] (T value)
            {
            *valuePtr = value;
            std::make_shared<ThreadlessTaskScheduler>()->Push(task);
            });

        return task;
        }

    //! Convert folly::Future to AsyncTaskPtr with no value. Future object should not throw any exceptions.
    static AsyncTaskPtr<void> FromFolly(folly::Future<folly::Unit>& future)
        {
        auto task = std::make_shared<PackagedAsyncTask<void>>([] {});

        auto parentTask = AsyncTasksManager::GetCurrentThreadAsyncTask();
        if (parentTask)
            parentTask->AddSubTask(task);

        future.onError([=] (folly::exception_wrapper ew)
            {
            BeAssert(false && "Received folly::Future exception, cannot convert to AsyncTask, returning.");
            }).then([=]
            {
            std::make_shared<ThreadlessTaskScheduler>()->Push(task);
            });

        return task;
        }
    };

END_BENTLEY_TASKS_NAMESPACE
