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

USING_NAMESPACE_BENTLEY_TASKS

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

    virtual AsyncTaskPtr<Response> _PerformRequest (RequestCR request) override;
    };

END_BENTLEY_HTTP_NAMESPACE
