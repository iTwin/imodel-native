/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <FakeServer/MockIMHubHttpHandler.h>
#include <BeHttp/ProxyHttpHandler.h>

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Tasks::AsyncTaskPtr<Http::Response> MockIMSHttpHandler::_PerformRequest(RequestCR request)
    {
    if (request.GetMethod().EqualsI("GET")) 
        {
        RequestHandler reqHandler;
        auto taskGet = std::make_shared<Tasks::PackagedAsyncTask<Http::Response>>([&] 
            {
            return reqHandler.PerformGetRequest(request);
            });
        taskGet->Execute();
        return taskGet;
        }
    auto task = std::make_shared<Tasks::PackagedAsyncTask<Http::Response>>([&] 
        {
        RequestHandler reqHandler;
        return reqHandler.PerformOtherRequest(request);
        });
    task->Execute();
    return task;
    }
