/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbClient.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <WebServices/Client/WSClient.h>
#include <DgnDbServer/Client/RepositoryInfo.h>
#include <DgnDbServer/Client/DgnDbRepositoryConnection.h>
#include <DgnDbServer/Client/DgnDbBriefcase.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
typedef std::shared_ptr<struct DgnDbClient> DgnDbClientPtr;
typedef MobileDgn::Utils::AsyncResult<bvector<RepositoryInfoPtr>, DgnDbServerError> DgnDbRepositoriesResult;
typedef MobileDgn::Utils::AsyncResult<RepositoryInfoPtr, DgnDbServerError> DgnDbRepositoryResult;
typedef MobileDgn::Utils::AsyncResult<DgnDbBriefcasePtr, DgnDbServerError> DgnDbBriefcaseResult;
typedef MobileDgn::Utils::AsyncResult<BeFileName, DgnDbServerError> DgnDbFileNameResult;
struct DgnDbClient
{
//__PUBLISH_SECTION_END__
private:
    Utf8String m_host;
    MobileDgn::Utils::Credentials m_credentials;
    WebServices::ClientInfoPtr m_clientInfo;

    DgnDbRepositoryConnectionPtr ConnectToRepository(RepositoryInfoPtr repository);
    DgnDbClient(Utf8StringCR host, MobileDgn::Utils::CredentialsCR credentials, WebServices::ClientInfoPtr clientInfo);
//__PUBLISH_SECTION_START__
public:
    DGNDBSERVERCLIENT_EXPORT static DgnDbClientPtr Create(Utf8StringCR host, MobileDgn::Utils::CredentialsCR credentials, WebServices::ClientInfoPtr clientInfo);
    DGNDBSERVERCLIENT_EXPORT MobileDgn::Utils::AsyncTaskPtr<DgnDbRepositoriesResult> GetRepositories(MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr);
    DGNDBSERVERCLIENT_EXPORT MobileDgn::Utils::AsyncTaskPtr<DgnDbRepositoryResult> CreateNewRepository(Utf8StringCR repositoryId, Utf8StringCR description, Utf8StringCR localPath, bool publish = true, MobileDgn::Utils::HttpRequest::ProgressCallbackCR callback = nullptr, MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr);
    DGNDBSERVERCLIENT_EXPORT MobileDgn::Utils::AsyncTaskPtr<DgnDbBriefcaseResult> OpenBriefcase(Dgn::DgnDbPtr db, bool doSync = false, MobileDgn::Utils::HttpRequest::ProgressCallbackCR callback = nullptr, MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr);
    DGNDBSERVERCLIENT_EXPORT static MobileDgn::Utils::AsyncTaskPtr<DgnDbBriefcaseResult> OpenBriefcase(Dgn::DgnDbPtr db, MobileDgn::Utils::CredentialsCR credentials, WebServices::ClientInfoPtr clientInfo, bool doSync = false, MobileDgn::Utils::HttpRequest::ProgressCallbackCR callback = nullptr, MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr);
    DGNDBSERVERCLIENT_EXPORT MobileDgn::Utils::AsyncTaskPtr<DgnDbFileNameResult> AquireBriefcase(RepositoryInfoPtr repository, BeFileNameCR localPath, bool doSync = true, MobileDgn::Utils::HttpRequest::ProgressCallbackCR callback = nullptr, MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr);
};
END_BENTLEY_DGNDBSERVER_NAMESPACE
