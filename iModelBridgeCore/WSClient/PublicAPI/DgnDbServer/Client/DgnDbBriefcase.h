/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbBriefcase.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/RepositoryInfo.h>
#include <DgnDbServer/Client/DgnDbRepositoryConnection.h>
#include <DgnPlatform/DgnCore/DgnDb.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
typedef std::shared_ptr<struct DgnDbBriefcase> DgnDbBriefcasePtr;
struct DgnDbBriefcase
{
//__PUBLISH_SECTION_END__
private:
    uint32_t m_briefcaseId;
    Utf8String m_lastRevisionId;
    DgnDbRepositoryConnectionPtr m_remoteRepository;
    Dgn::DgnDbPtr m_db;

    friend struct DgnDbClient;

    DgnDbBriefcase(Dgn::DgnDbPtr db, DgnDbRepositoryConnectionPtr remoteRepository, uint32_t m_briefcaseId);
//__PUBLISH_SECTION_START__
public:
    static DgnDbBriefcasePtr Create(Dgn::DgnDbPtr db, DgnDbRepositoryConnectionPtr connection, uint32_t m_briefcaseId);
    DgnDbServerTaskPtr Sync(WebServices::HttpRequest::ProgressCallbackCR callback = nullptr, WebServices::ICancellationTokenPtr cancellationToken = nullptr);
    DgnDbServerTaskPtr SyncAndPush(WebServices::HttpRequest::ProgressCallbackCR callback = nullptr, WebServices::ICancellationTokenPtr cancellationToken = nullptr);

    Dgn::DgnDbR GetDgnDb();
    uint32_t GetBriefcaseId();
    Utf8StringCR GetLastRevisionId();
    DgnDbRepositoryConnectionPtr GetRemoteRepository();
};
END_BENTLEY_DGNDBSERVER_NAMESPACE
