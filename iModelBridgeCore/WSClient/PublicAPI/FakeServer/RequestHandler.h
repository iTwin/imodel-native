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
        FAKESERVER_EXPORT RequestHandler();
        FAKESERVER_EXPORT ~RequestHandler();
        //
        FAKESERVER_EXPORT Response BuddiRequest(Request req);
        FAKESERVER_EXPORT Response ImsTokenRequest (Request req);
        FAKESERVER_EXPORT Response PluginRequest(Request req);
        ////
        FAKESERVER_EXPORT Response PerformGetRequest(Request req);
        FAKESERVER_EXPORT Response PerformOtherRequest(Request req);
        
        //////
        FAKESERVER_EXPORT Response CreateiModelInstance(Request req);
        FAKESERVER_EXPORT Response CreateSeedFileInstance(Request req);
        FAKESERVER_EXPORT Response UploadSeedFile(Request req);
        FAKESERVER_EXPORT Response CreateiModel(Request req);
        FAKESERVER_EXPORT Response GetBriefcaseId(Request req);
        FAKESERVER_EXPORT Response DownloadiModel(Request req);
        FAKESERVER_EXPORT Response UploadNewSeedFile(Request req);
        FAKESERVER_EXPORT Response CreateFileInstance(Request req);
        FAKESERVER_EXPORT Response GetiModels(Request req);
        FAKESERVER_EXPORT Response DeleteiModels(Request req);
        FAKESERVER_EXPORT Response Push(Request req);
        FAKESERVER_EXPORT Response Pull(Request req);
        FAKESERVER_EXPORT Response GetChangeSetInfo(Request req);

        FAKESERVER_EXPORT static DbResult Initialize(BeFileName temporaryDir, BeSQLiteLib::LogErrors logSqliteErrors = BeSQLiteLib::LogErrors::No);
        FAKESERVER_EXPORT void CheckDb();
        FAKESERVER_EXPORT void Insert(bvector<Utf8String> insertStr);


    };
