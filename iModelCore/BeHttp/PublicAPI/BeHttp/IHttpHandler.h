/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeHttp/Http.h>
#include <Bentley/Tasks/AsyncTask.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

typedef std::shared_ptr<struct IHttpHandler> IHttpHandlerPtr;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct IHttpHandler
    {
public:
    virtual ~IHttpHandler() {}

    //! Perform HttpRequest and receive HttpResponse
    virtual Tasks::AsyncTaskPtr<Response> _PerformRequest(RequestCR request) = 0;
    };

END_BENTLEY_HTTP_NAMESPACE
