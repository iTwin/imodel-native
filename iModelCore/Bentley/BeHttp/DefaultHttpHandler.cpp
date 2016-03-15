/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/DefaultHttpHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <BeHttp/DefaultHttpHandler.h>

#if defined (HTTP_LIB_CASABLANCA)
#include "Casablanca/CasablancaHttpHandler.h"
#elif defined (HTTP_LIB_CURL)
#include "Curl/ThreadCurlHttpHandler.h"
#endif

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IHttpHandlerPtr CreateDefaultHandler ()
    {
#if defined (HTTP_LIB_CASABLANCA)
    return std::make_shared<CasablancaHttpHandler> ();
#elif defined (HTTP_LIB_CURL)
    return std::make_shared<ThreadCurlHttpHandler> ();
#endif
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IHttpHandlerPtr DefaultHttpHandler::GetInstance ()
    {
    static IHttpHandlerPtr s_instance = CreateDefaultHandler ();
    return s_instance;
    }
