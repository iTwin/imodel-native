 /*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/FakeServer/MockIMHubHttpHandler.h $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
 //__PUBLISH_SECTION_START__
#include "FakeServer.h"
#include <Bentley/Tasks/LimitingTaskQueue.h>
#include <Bentley/Tasks/WorkerThreadPool.h>
#include <BeHttp/IHttpHandler.h>
#include "RequestHandler.h"

USING_NAMESPACE_BENTLEY_HTTP

struct MockIMSHttpHandler : public Http::IHttpHandler
    {
    public:
        typedef std::function<Http::Response(Http::RequestCR)> OnResponseCallback;
    private:
        OnResponseCallback m_onAnyRequestCallback;

    public:
       
        FAKESERVER_EXPORT MockIMSHttpHandler();
        virtual ~MockIMSHttpHandler() {}
        FAKESERVER_EXPORT virtual Tasks::AsyncTaskPtr<Http::Response> _PerformRequest(Http::RequestCR request) override;
    };

