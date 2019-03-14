/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Configuration/UrlProviderTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "UrlProviderTests.h"

#include <WebServices/Client/WSClient.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <BeHttp/ProxyHttpHandler.h>

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

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, Get_WithCustomDefaultSheduler_DoesNotUseDefaultShedulerToAvoidDeadlocks)
    {
    auto sheduler = std::make_shared<StubTaskScheduler>();
    AsyncTasksManager::SetDefaultScheduler(sheduler);

    auto thread = WorkerThread::Create();

    RuntimeJsonLocalState localState;
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

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
INSTANTIATE_TEST_CASE_P(,UrlProviderParamTests,
    ::testing::Combine(
        ::testing::Values(UrlProvider::Environment::Dev, UrlProvider::Environment::Qa, UrlProvider::Environment::Release, UrlProvider::Environment::Perf),
        ::testing::ValuesIn(UrlProvider::GetUrlRegistry())
    ),
    UrlProviderParamTests::PrintToStringParamName());

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(UrlProviderParamTests, VerifyClientConnection_Success)
    {
    RuntimeJsonLocalState localState;
    UrlProvider::Environment enviroment = ::testing::get<0>(GetParam());
    UrlProvider::Initialize(enviroment, UrlProvider::DefaultTimeout, &localState);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    auto urlDescriptor = ::testing::get<1>(GetParam());
    Utf8StringCR serverUrl = urlDescriptor->Get();
    if (serverUrl.empty())
        return;

    auto client = WSClient::Create(serverUrl, StubValidClientInfo(), proxy);
    EXPECT_TRUE(client->VerifyConnection()->GetResult().IsSuccess()) << "Failed url: " + serverUrl;
    }