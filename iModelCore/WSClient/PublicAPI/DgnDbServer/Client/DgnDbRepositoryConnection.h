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

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
typedef std::shared_ptr<struct DgnDbRepositoryConnection> DgnDbRepositoryConnectionPtr;

typedef DgnClientFx::Utils::AsyncResult<void, DgnDbServerError> DgnDbResult;
typedef DgnClientFx::Utils::AsyncResult<DgnDbRepositoryConnectionPtr, DgnDbServerError> DgnDbRepositoryConnectionResult;
typedef DgnClientFx::Utils::AsyncResult<RepositoryInfoPtr, DgnDbServerError> DgnDbRepositoryResult;
typedef DgnClientFx::Utils::AsyncResult<Dgn::DgnRevisionPtr, DgnDbServerError> DgnDbRevisionResult;
typedef DgnClientFx::Utils::AsyncResult<bvector<Dgn::DgnRevisionPtr>, DgnDbServerError> DgnDbRevisionsResult;
typedef DgnClientFx::Utils::AsyncResult<uint64_t, DgnDbServerError> DgnDbUInt64Result;
typedef DgnClientFx::Utils::AsyncResult<uint32_t, DgnDbServerError> DgnDbUInt32Result;
typedef DgnClientFx::Utils::AsyncResult<Dgn::LockLevel, DgnDbServerError> DgnDbLockLevelResult;
typedef DgnClientFx::Utils::AsyncResult<Dgn::DgnLockSet, DgnDbServerError> DgnDbLockSetResult;
typedef DgnClientFx::Utils::AsyncResult<Dgn::LockRequest::Response, DgnDbServerError> DgnLockResponseResult;
typedef DgnClientFx::Utils::AsyncResult<Dgn::DgnLockOwnership, DgnDbServerError> DgnDbOwnershipResult;

//=======================================================================================
//! Connection to a repository on server.
//! This class performs all of the operations related to a single repository on the server.
//@bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct DgnDbRepositoryConnection
{
//__PUBLISH_SECTION_END__
private:
    RepositoryInfoPtr m_repositoryInfo;
    WebServices::IWSRepositoryClientPtr m_wsRepositoryClient;

    friend struct DgnDbClient;
    friend struct DgnDbBriefcase;
    friend struct DgnDbLocks;

    //! Update repository info from the server.
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbResult> UpdateRepositoryInfo(DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);

    //! Aquire a new briefcase id for this repository.
    DgnClientFx::Utils::AsyncTaskPtr<WebServices::WSCreateObjectResult> AcquireBriefcaseId(DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);

    //! Download a copy of the master file from the repository and write the briefcaseId into it.
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbResult> DownloadBriefcaseFile(BeFileName localFile, const BeSQLite::BeBriefcaseId& briefcaseId, DgnClientFx::Utils::HttpRequest::ProgressCallbackCR callback = nullptr, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);

    //! Download the file for this revision from server.
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbResult> DownloadRevisionFile(Dgn::DgnRevisionPtr revision, DgnClientFx::Utils::HttpRequest::ProgressCallbackCR callback = nullptr, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);

    //! Push this revision file to server.
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbResult> Push(Dgn::DgnRevisionPtr revision, uint32_t repositoryId, DgnClientFx::Utils::HttpRequest::ProgressCallbackCR callback = nullptr, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);

    //! Download all revision files after revisionId
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbRevisionsResult> Pull(Utf8StringCR revisionId, DgnClientFx::Utils::HttpRequest::ProgressCallbackCR callback = nullptr, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);

    //! Get all revision information based on a query.
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbRevisionsResult> RevisionsFromQuery(const WebServices::WSQuery& query, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);

    //! Get the index from a revisionId.
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbUInt64Result> GetRevisionIndex(Utf8StringCR revisionId, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);

    DgnClientFx::Utils::AsyncTaskPtr<DgnDbResult> QueryLocksHeld(bool& held, Dgn::LockRequestCR locksRequest, const BeSQLite::BeBriefcaseId& briefcaseId, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);
    DgnClientFx::Utils::AsyncTaskPtr<DgnLockResponseResult> AcquireLocks(JsonValueCR locksRequest, const BeSQLite::BeBriefcaseId& briefcaseId, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbResult> RelinquishLocks(const BeSQLite::BeBriefcaseId& briefcaseId, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbOwnershipResult> QueryOwnership(Dgn::LockableId lockId, const BeSQLite::BeBriefcaseId& briefcaseId, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbLockLevelResult> QueryLockLevel(Dgn::LockableId lockId, const BeSQLite::BeBriefcaseId& briefcaseId, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbLockSetResult> QueryLocks(const BeSQLite::BeBriefcaseId& briefcaseId, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);
    DgnClientFx::Utils::AsyncTaskPtr<DgnDbResult> ReleaseLocks(JsonValueCR locksRequest, const BeSQLite::BeBriefcaseId& briefcaseId, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);

    DgnDbRepositoryConnection(RepositoryInfoPtr repository, WebServices::CredentialsCR credentials, WebServices::ClientInfoPtr clientInfo);
public:
    //! Create an instance of the connection to a repository on the server.
    //! @param[in] repository Repository information used to connect to a specific repository on the server.
    //! @param[in] credentials Credentials used to authenticate on the repository.
    //! @param[in] clientInfo Application information sent to server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the created connection instance as the result.
    //! @note OpenBriefcase in DgnDbClient is used to create an instance of a DgnDbRepositoryConnection.
    static DgnClientFx::Utils::AsyncTaskPtr<DgnDbRepositoryConnectionResult> Create(RepositoryInfoPtr repository, DgnClientFx::Utils::CredentialsCR credentials, WebServices::ClientInfoPtr clientInfo, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);

    //__PUBLISH_SECTION_START__
    //! Get a revision for the specific revision id.
    //! @param[in] revisionId Id of the revision to retrieve.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the revision information as the result.
    DGNDBSERVERCLIENT_EXPORT DgnClientFx::Utils::AsyncTaskPtr<DgnDbRevisionResult> GetRevisionById(Utf8StringCR revisionId, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);

    //! Get all of the revisions after the specific revision id.
    //! @param[in] revisionId Id of the parent revision for the first revision in the resulting collection. If empty gets all revisions on server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of revision information as the result.
    DGNDBSERVERCLIENT_EXPORT DgnClientFx::Utils::AsyncTaskPtr<DgnDbRevisionsResult> GetRevisionsFromId(Utf8StringCR revisionId, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);

    //! Download the revision files.
    //! @param[in] revisions Set of revisions to download.
    //! @param[in] callback Download callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of revision information as the result.
    //! @note This is used to download the files in order to revert or inspect them. To update a briefcase DgnDbBriefcase methods should be used.
    DGNDBSERVERCLIENT_EXPORT DgnClientFx::Utils::AsyncTaskPtr<DgnDbResult> DownloadRevisions(const bvector<Dgn::DgnRevisionPtr>& revisions, DgnClientFx::Utils::HttpRequest::ProgressCallbackCR callback = nullptr, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);

    //! Verify the access to the revision on the server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that results in error if connection or authentication fails and success otherwise.
    DGNDBSERVERCLIENT_EXPORT DgnClientFx::Utils::AsyncTaskPtr<DgnDbResult> VerifyConnection(DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);

    DGNDBSERVERCLIENT_EXPORT RepositoryInfoCR GetRepositoryInfo(); //!< Returns repository information for this connection.
};
END_BENTLEY_DGNDBSERVER_NAMESPACE
