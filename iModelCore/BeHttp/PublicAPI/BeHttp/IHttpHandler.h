/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeHttp/IHttpHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeHttp/Http.h>
#include <Bentley/Tasks/AsyncTask.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

struct Request;
struct HttpResponse;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct IHttpHandler> IHttpHandlerPtr;

struct EXPORT_VTABLE_ATTRIBUTE IHttpHandler
    {
public:
    BEHTTP_EXPORT virtual ~IHttpHandler();

    //! Perform HttpRequest and receive HttpResponse
    virtual Tasks::AsyncTaskPtr<Response> _PerformRequest(RequestCR request) = 0;
    };

END_BENTLEY_HTTP_NAMESPACE