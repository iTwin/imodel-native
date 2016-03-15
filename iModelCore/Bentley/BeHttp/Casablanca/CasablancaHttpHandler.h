/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/Casablanca/CasablancaHttpHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <BeHttp/HttpRequest.h>
#include <BeHttp/IHttpHandler.h>
#include <Bentley/Tasks/WorkerThreadPool.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct CasablancaHttpHandler : public IHttpHandler
    {
private:    
    std::shared_ptr<WorkerThreadPool> m_webThreadPool;

public:
    CasablancaHttpHandler ();
    virtual ~CasablancaHttpHandler ();

    virtual Tasks::AsyncTaskPtr<HttpResponse> PerformRequest (HttpRequestCR request) override;
    };

END_BENTLEY_HTTP_NAMESPACE
