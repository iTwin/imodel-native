/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/FakeServer/RequestHandler.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "FakeServer.h"
#include <BeHttp/HttpRequest.h>
#include <BeHttp/HttpResponse.h>

USING_NAMESPACE_BENTLEY_HTTP

class RequestHandler
    {
    Utf8String serverPath; 
    public:
        RequestHandler();
        ~RequestHandler();
        //
        Response BuddiRequest(Request req);
        Response ImsTokenRequest (Request req);
        Response PluginRequest(Request req);
        ////
        Response PerformGetRequest(Request req);
        Response PerformOtherRequest(Request req);
        
        //////
        Response CreateiModel(Request req);
        Response GetBriefcaseId(Request req);
        Response DownloadiModel(Request req);
        Response UploadNewSeedFile(Request req);
        Response CreateFileInstance(Request req);
        Response GetiModels(Request req);
        Response DeleteiModels(Request req);
        
        static DbResult Initialize(BeFileName temporaryDir, BeSQLiteLib::LogErrors logSqliteErrors = BeSQLiteLib::LogErrors::No);
        void CheckDb();
        void Insert(bvector<Utf8String> insertStr);


    };
