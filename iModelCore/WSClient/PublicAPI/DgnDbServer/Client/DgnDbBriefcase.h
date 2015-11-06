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
#include <DgnPlatform/DgnDb.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
typedef std::shared_ptr<struct DgnDbBriefcase> DgnDbBriefcasePtr;
struct DgnDbBriefcase
{
//__PUBLISH_SECTION_END__
private:
    BeSQLite::BeBriefcaseId m_briefcaseId;
    Utf8String m_lastRevisionId;
    DgnDbRepositoryConnectionPtr m_repositoryConnection;
    Dgn::DgnDbPtr m_db;
    DgnDbBriefcase(Dgn::DgnDbPtr db, DgnDbRepositoryConnectionPtr connection);
//__PUBLISH_SECTION_START__
public:
    DGNDBSERVERCLIENT_EXPORT static DgnDbBriefcasePtr Create(Dgn::DgnDbPtr db, DgnDbRepositoryConnectionPtr connection);
    DGNDBSERVERCLIENT_EXPORT DgnClientFx::Utils::AsyncTaskPtr<DgnDbResult> Sync(DgnClientFx::Utils::HttpRequest::ProgressCallbackCR callback = nullptr, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);
    DGNDBSERVERCLIENT_EXPORT DgnClientFx::Utils::AsyncTaskPtr<DgnDbResult> SyncAndPush(DgnClientFx::Utils::HttpRequest::ProgressCallbackCR callback = nullptr, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);

    DGNDBSERVERCLIENT_EXPORT Dgn::DgnDbR GetDgnDb();
    DGNDBSERVERCLIENT_EXPORT const BeSQLite::BeBriefcaseId& GetBriefcaseId();
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetLastRevisionId();
    DGNDBSERVERCLIENT_EXPORT DgnDbRepositoryConnectionPtr GetRepositoryConnection();
};
END_BENTLEY_DGNDBSERVER_NAMESPACE
