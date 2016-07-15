/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/Casablanca/CasablancaHttpHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CasablancaHttpHandler.h"

#include <Bentley/BeTimeUtilities.h>

#include "../SimplePackagedAsyncTask.h"
#include "CasablancaHttpRequest.h"
#include "CasablancaTaskRunner.h"

#include <BeHttp/HttpClient.h>

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CasablancaHttpHandler::CasablancaHttpHandler ()
    {
    m_webThreadPool = Tasks::WorkerThreadPool::Create
        (
        1,
        "Curl Web",
        std::shared_ptr<Tasks::AsyncTaskRunnerFactory<CasablancaTaskRunner>> (new Tasks::AsyncTaskRunnerFactory<CasablancaTaskRunner> ())
        );
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CasablancaHttpHandler::~CasablancaHttpHandler ()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<Response> CasablancaHttpHandler::_PerformRequest (RequestCR request)
    {
    auto casablancaRequest = std::make_shared<CasablancaHttpRequest> (request);
    auto task = std::make_shared<SimplePackagedAsyncTask<std::shared_ptr<CasablancaHttpRequest>, Response>> (casablancaRequest);
        
    m_webThreadPool->Push(task);

    return task;
    }
