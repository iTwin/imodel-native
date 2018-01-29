 /*--------------------------------------------------------------------------------------+
 |
 |     $Source: Tests/iModelHubClient/FakeServer/MockIMHubHttpHandler.h $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/


#pragma once
#include "../Common.h"
#include <Bentley/Tasks/LimitingTaskQueue.h>
#include <Bentley/Tasks/WorkerThreadPool.h>
#include <BeHttp/IHttpHandler.h>
#include "RequestHandler.h"

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
USING_NAMESPACE_BENTLEY_HTTP

struct MockIMSHttpHandler : public Http::IHttpHandler
    {
    public:
        typedef std::function<Http::Response(Http::RequestCR)> OnResponseCallback;
    private:
        OnResponseCallback m_onAnyRequestCallback;

    public:
       
        MockIMSHttpHandler();
        virtual ~MockIMSHttpHandler() {}
        virtual Tasks::AsyncTaskPtr<Http::Response> _PerformRequest(Http::RequestCR request) override;
    };

END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
