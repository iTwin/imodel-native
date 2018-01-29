#include "MockIMHubHttpHandler.h"
#include <BeHttp/ProxyHttpHandler.h>

USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_TASKS
USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MockIMSHttpHandler::MockIMSHttpHandler()
    {

    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<Response> MockIMSHttpHandler::_PerformRequest(RequestCR request)
    {
    if (request.GetMethod().EqualsI("GET")) 
        {
        RequestHandler reqHandler;
        auto taskGet = std::make_shared<PackagedAsyncTask<Http::Response>>([&] 
            {
            return reqHandler.PerformGetRequest(request);
            });
        taskGet->Execute();
        return taskGet;
        }
    auto task = std::make_shared<PackagedAsyncTask<Http::Response>>([&] 
        {
        RequestHandler reqHandler;
        return reqHandler.PerformOtherRequest(request);
        });
    task->Execute();
    return task;
    }
