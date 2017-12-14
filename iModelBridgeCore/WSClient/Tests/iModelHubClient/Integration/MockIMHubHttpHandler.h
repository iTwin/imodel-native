///*--------------------------------------------------------------------------------------+
//|
//|     $Source: Tests/iModelHubClient/Integration/MockIMHubHttpHandler.h $
//|
//|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//|
//+--------------------------------------------------------------------------------------*/


#pragma once

#include "../BackDoor/PublicAPI/BackDoor/WebServices/iModelHub/BackDoor.h"
#include <Bentley/Tasks/LimitingTaskQueue.h>
#include <Bentley/Tasks/WorkerThreadPool.h>
#include <BeHttp/IHttpHandler.h>
#include "RequestHandler.h"

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
USING_NAMESPACE_BENTLEY_HTTP

struct MockIMSHttpHandler : public IHttpHandler
    {
    public:
        typedef std::function<Http::Response(Http::RequestCR)> OnResponseCallback;
    private:
        OnResponseCallback m_onAnyRequestCallback;

    public:
        
        MockIMSHttpHandler();
        virtual ~MockIMSHttpHandler() {}
        virtual Tasks::AsyncTaskPtr<Response> _PerformRequest(RequestCR request) override;
    };

END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
