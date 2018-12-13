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
        FAKESERVER_EXPORT Response FileCreationConfirmation(Request req);
        FAKESERVER_EXPORT Response GetInitializationState(Request req);
        FAKESERVER_EXPORT Response GetBriefcaseInfo(Request req);
        FAKESERVER_EXPORT Response CreateBriefcaseInstance(Request req);
        FAKESERVER_EXPORT Response DeleteBriefcaseInstance(Request req);
        FAKESERVER_EXPORT Response DownloadiModel(Request req);
        FAKESERVER_EXPORT Response UploadNewSeedFile(Request req);
        FAKESERVER_EXPORT Response CreateFileInstance(Request req);
        FAKESERVER_EXPORT Response GetiModels(Request req);
        FAKESERVER_EXPORT Response DeleteiModels(Request req);
        FAKESERVER_EXPORT Response PushChangeSetMetadata(Request req);
        FAKESERVER_EXPORT Response GetChangeSetInfo(Request req);

        FAKESERVER_EXPORT Response PushAcquiredLocks(Request req);
        FAKESERVER_EXPORT Response LocksInfo(Request req);
        FAKESERVER_EXPORT Response MultiLocksInfo(Request req);
        FAKESERVER_EXPORT Response CodesInfo(Request req);
        FAKESERVER_EXPORT BeFileName GetDbPath();
        FAKESERVER_EXPORT BeFileName GetServerPath() {return BeFileName(serverPath);}

        FAKESERVER_EXPORT void CheckDb();
        FAKESERVER_EXPORT void CreateTables(BentleyM0200::BeSQLite::Db *m_db);
        FAKESERVER_EXPORT void DeleteTables(Utf8String tableName);
        FAKESERVER_EXPORT void Insert(bvector<Utf8String> insertStr);

    };
