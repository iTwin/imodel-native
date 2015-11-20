/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbRepositoryConnection.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/Client/WSRepositoryClient.h>
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/RepositoryInfo.h>
#include <DgnDbServer/Client/RevisionInfo.h>
#include <DgnDbServer/Client/FileInfo.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
typedef std::shared_ptr<struct DgnDbRepositoryConnection> DgnDbRepositoryConnectionPtr;

typedef DgnClientFx::Utils::AsyncResult<void, DgnDbServerError> DgnDbResult;
typedef DgnClientFx::Utils::AsyncResult<Dgn::DgnRevisionPtr, DgnDbServerError> DgnDbRevisionResult;
typedef DgnClientFx::Utils::AsyncResult<bvector<Dgn::DgnRevisionPtr>, DgnDbServerError> DgnDbRevisionsResult;

struct DgnDbRepositoryConnection
{
//__PUBLISH_SECTION_END__
private:
    RepositoryInfoPtr m_repositoryInfo;
    WebServices::IWSRepositoryClientPtr m_wsRepositoryClient;

    friend struct DgnDbClient;
    friend struct DgnDbBriefcase;
    DgnClientFx::Utils::AsyncTaskPtr<WebServices::WSObjectsResult> AcquireBriefcaseId(DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbResult> DownloadBriefcaseFile(BeFileName localFile, DgnClientFx::Utils::HttpRequest::ProgressCallbackCR callback = nullptr, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbResult> DownloadRevisionFile(Dgn::DgnRevisionPtr revision, DgnClientFx::Utils::HttpRequest::ProgressCallbackCR callback = nullptr, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbResult> Push(Dgn::DgnRevisionPtr revision, DgnClientFx::Utils::HttpRequest::ProgressCallbackCR callback = nullptr, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbRevisionsResult> Pull(Utf8StringCR revisionId, DgnClientFx::Utils::HttpRequest::ProgressCallbackCR callback = nullptr, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbRevisionsResult> RevisionsFromQuery(const WebServices::WSQuery& query, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbResult> GetRevisionIndex(uint64_t& index, Utf8StringCR revisionId, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);
protected:
    DgnDbRepositoryConnection(RepositoryInfoPtr repository, Utf8StringCR host, WebServices::CredentialsCR credentials, WebServices::ClientInfoPtr clientInfo);
//__PUBLISH_SECTION_START__
public:
    DGNDBSERVERCLIENT_EXPORT static DgnDbRepositoryConnectionPtr Create(RepositoryInfoPtr repository, Utf8StringCR host, WebServices::CredentialsCR credentials, WebServices::ClientInfoPtr clientInfo);
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbRevisionResult> GetRevisionById(Utf8StringCR revisionId, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbRevisionsResult> GetRevisionsFromId(Utf8StringCR revisionId, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);
    DGNDBSERVERCLIENT_EXPORT DgnClientFx::Utils::AsyncTaskPtr<DgnDbResult> VerifyConnection(DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);
    DGNDBSERVERCLIENT_EXPORT RepositoryInfoCR GetRepositoryInfo();
};
END_BENTLEY_DGNDBSERVER_NAMESPACE
