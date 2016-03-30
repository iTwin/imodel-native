/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/DefaultHttpHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <BeHttp/DefaultHttpHandler.h>
#include <Bentley/Tasks/AsyncTasksManager.h>

#if defined (HTTP_LIB_CASABLANCA)
#include "Casablanca/CasablancaHttpHandler.h"
#elif defined (HTTP_LIB_CURL)
#include "Curl/ThreadCurlHttpHandler.h"
#endif

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

std::once_flag DefaultHttpHandler::s_initFlag;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static IHttpHandlerPtr createDefaultHandler ()
    {
#if defined (HTTP_LIB_CASABLANCA)
    return std::make_shared<CasablancaHttpHandler> ();
#elif defined (HTTP_LIB_CURL)
    return std::make_shared<ThreadCurlHttpHandler> ();
#endif
    }

static IHttpHandlerPtr s_instance = createDefaultHandler();

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IHttpHandlerPtr DefaultHttpHandler::GetInstance ()
    {
    std::call_once(s_initFlag, []
        {
        AsyncTasksManager::RegisterOnCompletedListener([]
            {
            s_instance.reset();
            });
        });

    return s_instance;
    }

