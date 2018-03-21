/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Configuration/UrlProviderTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <WebServicesTestsHelper.h>

#include <WebServices/Configuration/UrlProvider.h>
#include <MobileDgn/Utils/Http/ProxyHttpHandler.h>

struct StubTaskScheduler : ITaskScheduler
    {
    void Push(std::shared_ptr<AsyncTask> task, AsyncTask::Priority priority)
        {
        ADD_FAILURE();
        task->Execute();
        };
    void Push(std::shared_ptr<AsyncTask> task, std::shared_ptr<AsyncTask> parentTask, AsyncTask::Priority priority)
        {
        ADD_FAILURE();
        if (parentTask)
            parentTask->AddSubTask(task);
        task->Execute();
        };
    std::shared_ptr<AsyncTask> WaitAndPop() { ADD_FAILURE(); return nullptr; };
    std::shared_ptr<AsyncTask> TryPop() { ADD_FAILURE(); return nullptr; };
    int GetQueueTaskCount() const { ADD_FAILURE(); return 0; };
    bool HasRunningTasks() const { ADD_FAILURE(); return false; };
    AsyncTaskPtr<void> OnEmpty() const { ADD_FAILURE(); return CreateCompletedAsyncTask(); };
    };

class UrlProviderTests : public WSClientBaseTest
    {
    void Reset()
        {
        AsyncTasksManager::SetDefaultScheduler(nullptr);
        UrlProvider::Uninitialize();
        }
    void SetUp() { Reset(); }
    void TearDown() { Reset(); }
    };

TEST_F(UrlProviderTests, Get_WithCustomDefaultSheduler_DoesNotUseDefaultShedulerToAvoidDeadlocks)
    {
    auto sheduler = std::make_shared<StubTaskScheduler>();
    AsyncTasksManager::SetDefaultScheduler(sheduler);

    auto thread = WorkerThread::Create();

    StubLocalState localState;
    UrlProvider::Initialize(UrlProvider::Environment::Release, UrlProvider::DefaultTimeout, &localState, nullptr, nullptr, thread);

    EXPECT_NE("", UrlProvider::Urls::ConnectedContext.Get());
    EXPECT_NE("", UrlProvider::Urls::ConnectedContext.Get());

    UrlProvider::Initialize(UrlProvider::Environment::Release, 0, &localState, nullptr, nullptr, thread);

    EXPECT_NE("", UrlProvider::Urls::ConnectedContext.Get());
    EXPECT_NE("", UrlProvider::Urls::ConnectedContext.Get());

    AsyncTasksManager::SetDefaultScheduler(nullptr);

    // Wait for background tasks to complete
    thread->OnEmpty()->Wait();
    }
