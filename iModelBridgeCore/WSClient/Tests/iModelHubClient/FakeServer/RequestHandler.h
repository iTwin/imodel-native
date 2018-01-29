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
USING_NAMESPACE_BENTLEY_HTTP

class RequestHandler
    {
    Utf8String serverPath; 
    public:
        RequestHandler();
        ~RequestHandler();
        Response PerformGetRequest(Request req);
        Response PerformOtherRequest(Request req);
        Response CreateiModel(Request req);
        Response DownloadiModel(bvector<Utf8String> args);
        Response BuddiRequest(Request req);
        Response ImsTokenRequest (Request req);
        Response PluginRequest(Request req);
        static DbResult Initialize(BeFileName temporaryDir, BeSQLiteLib::LogErrors logSqliteErrors = BeSQLiteLib::LogErrors::No);
        void CheckDb();
        void Insert(bvector<Utf8String> insertStr);


    };
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
