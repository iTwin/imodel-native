 /*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
 //__PUBLISH_SECTION_START__
#include "FakeServer.h"
#include <Bentley/Tasks/LimitingTaskQueue.h>
#include <Bentley/Tasks/WorkerThreadPool.h>
#include <BeHttp/IHttpHandler.h>
#include "RequestHandler.h"

USING_NAMESPACE_BENTLEY_HTTP

typedef std::shared_ptr<struct MockIMSHttpHandler> MockIMSHttpHandlerPtr;
struct MockIMSHttpHandler : public Http::IHttpHandler
    {
    public:
        typedef std::function<Http::Response(Http::RequestCR)> OnResponseCallback;
    private:
        OnResponseCallback m_onAnyRequestCallback;

    public:
        FAKESERVER_EXPORT MockIMSHttpHandler() {}
        virtual ~MockIMSHttpHandler() {}
        FAKESERVER_EXPORT virtual Tasks::AsyncTaskPtr<Http::Response> _PerformRequest(Http::RequestCR request) override;
    };

