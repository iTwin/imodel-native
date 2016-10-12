/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/DefaultHttpHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <BeHttp/DefaultHttpHandler.h>
#include <Bentley/Tasks/AsyncTasksManager.h>
#include "Curl/ThreadCurlHttpHandler.h"

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static IHttpHandlerPtr createDefaultHandler()
    {
    return std::make_shared<ThreadCurlHttpHandler> ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IHttpHandlerPtr DefaultHttpHandler::GetInstance()
    {
    static BeMutex s_mutex;
    static IHttpHandlerPtr s_instance = nullptr;
    BeMutexHolder lock(s_mutex);
    if (nullptr == s_instance)
        {
        s_instance = createDefaultHandler();
        AsyncTasksManager::RegisterOnCompletedListener([]
            {
            s_instance.reset();
            });
        }

    return s_instance;
    }
