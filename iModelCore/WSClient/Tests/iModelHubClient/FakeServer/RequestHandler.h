/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/FakeServer/RequestHandler.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../Common.h"
#include "FakeServer.h"

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
struct RequestHandler
    {
    WCharCP serverPath;
    public:
        RequestHandler();
        ~RequestHandler();
        Http::Response PerformGetRequest(Http::Request req);
        Http::Response PerformOtherRequest(Http::Request req);
        Http::Response CreateiModel(Http::Request req);

    private:

    };
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
