/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbRepositoryConnection.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/Client/WSRepositoryClient.h>
#include <DgnDbServer/Client/DgnDbServerError.h>
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/RepositoryInfo.h>
#include <DgnDbServer/Client/DgnDbServerRevision.h>
#include <WebServices/Azure/AzureBlobStorageClient.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS
typedef std::shared_ptr<struct DgnDbRepositoryConnection> DgnDbRepositoryConnectionPtr;

typedef AsyncResult<void, DgnDbServerError> DgnDbResult;
typedef AsyncResult<DgnDbRepositoryConnectionPtr, DgnDbServerError> DgnDbRepositoryConnectionResult;
typedef AsyncResult<RepositoryInfoPtr, DgnDbServerError> DgnDbRepositoryResult;
typedef AsyncResult<DgnRevisionPtr, DgnDbServerError> DgnDbRevisionResult;
typedef AsyncResult<bvector<DgnRevisionPtr>, DgnDbServerError> DgnDbRevisionsResult;
typedef AsyncResult<uint64_t, DgnDbServerError> DgnDbUInt64Result;
typedef AsyncResult<uint32_t, DgnDbServerError> DgnDbUInt32Result;
typedef AsyncResult<LockLevel, DgnDbServerError> DgnDbLockLevelResult;
typedef AsyncResult<DgnLockSet, DgnDbServerError> DgnDbLockSetResult;
typedef AsyncResult<LockRequest::Response, DgnDbServerError> DgnLockResponseResult;
typedef AsyncResult<DgnLockOwnership, DgnDbServerError> DgnDbOwnershipResult;
typedef AsyncResult<DgnDbServerRevisionPtr, DgnDbServerError> DgnDbServerRevisionResult;
typedef AsyncResult<bvector<DgnDbServerRevisionPtr>, DgnDbServerError> DgnDbServerRevisionsResult;

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
    WebServices::IAzureBlobStorageClientPtr m_azureClient;

    friend struct DgnDbClient;
    friend struct DgnDbBriefcase;
    friend struct DgnDbLocks;

    //! Returns AzureBlobStorageClient. Creates if doesn't exist.
    WebServices::IAzureBlobStorageClientPtr GetAzureClient();

    //! Update repository info from the server.
    AsyncTaskPtr<DgnDbResult> UpdateRepositoryInfo(ICancellationTokenPtr cancellationToken = nullptr);

    //! Aquire a new briefcase id for this repository.
    AsyncTaskPtr<WebServices::WSCreateObjectResult> AcquireBriefcaseId(ICancellationTokenPtr cancellationToken = nullptr);

    //! Write the briefcaseId into the file.
    DgnDbResult WriteBriefcaseIdIntoFile(BeFileName filePath, const BeSQLite::BeBriefcaseId& briefcaseId);

    //! Download a copy of the master file from the repository
    AsyncTaskPtr<DgnDbResult> DownloadBriefcaseFile(BeFileName localFile, const BeSQLite::BeBriefcaseId& briefcaseId, Utf8StringCR url,
    HttpRequest::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr);

    //! Download the file for this revision from server.
    AsyncTaskPtr<DgnDbResult> DownloadRevisionFile(DgnDbServerRevisionPtr revision, HttpRequest::ProgressCallbackCR callback = nullptr,
    ICancellationTokenPtr cancellationToken = nullptr);

    //! Push this revision file to server.
    AsyncTaskPtr<DgnDbResult> Push(DgnRevisionPtr revision, uint32_t repositoryId, HttpRequest::ProgressCallbackCR callback = nullptr,
    ICancellationTokenPtr cancellationToken = nullptr);

    //! Download all revision files after revisionId
    AsyncTaskPtr<DgnDbServerRevisionsResult> Pull(Utf8StringCR revisionId, HttpRequest::ProgressCallbackCR callback = nullptr,
    ICancellationTokenPtr cancellationToken = nullptr);

    //! Get all revision information based on a query.
    AsyncTaskPtr<DgnDbServerRevisionsResult> RevisionsFromQuery(const WebServices::WSQuery& query, ICancellationTokenPtr cancellationToken = nullptr);

    //! Get the index from a revisionId.
    AsyncTaskPtr<DgnDbUInt64Result> GetRevisionIndex(Utf8StringCR revisionId, ICancellationTokenPtr cancellationToken = nullptr);

    //! Check if the requested set of locks is already held.
    AsyncTaskPtr<DgnDbResult> QueryLocksHeld(bool& held, LockRequestCR locksRequest, const BeSQLite::BeBriefcaseId& briefcaseId,
    ICancellationTokenPtr cancellationToken = nullptr);

    //! Query ownership information of a specific lock.
    AsyncTaskPtr<DgnDbOwnershipResult> QueryOwnership(LockableId lockId, ICancellationTokenPtr cancellationToken = nullptr);

    AsyncTaskPtr<DgnDbLockLevelResult> QueryLockLevel(LockableId lockId, const BeSQLite::BeBriefcaseId& briefcaseId,
    ICancellationTokenPtr cancellationToken = nullptr);

    DgnDbRepositoryConnection(RepositoryInfoPtr repository, WebServices::CredentialsCR credentials, WebServices::ClientInfoPtr clientInfo);

public:
    //! Create an instance of the connection to a repository on the server.
    //! @param[in] repository Repository information used to connect to a specific repository on the server.
    //! @param[in] credentials Credentials used to authenticate on the repository.
    //! @param[in] clientInfo Application information sent to server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the created connection instance as the result.
    //! @note OpenBriefcase in DgnDbClient is used to create an instance of a DgnDbRepositoryConnection.
    static AsyncTaskPtr<DgnDbRepositoryConnectionResult> Create(RepositoryInfoPtr repository, CredentialsCR credentials, WebServices::ClientInfoPtr clientInfo,
    ICancellationTokenPtr cancellationToken = nullptr);

//__PUBLISH_SECTION_START__

    //! Get a revision for the specific revision id.
    //! @param[in] revisionId Id of the revision to retrieve.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the revision information as the result.
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnDbServerRevisionResult> GetRevisionById(Utf8StringCR revisionId, ICancellationTokenPtr cancellationToken = nullptr);

    //! Get all of the revisions after the specific revision id.
    //! @param[in] revisionId Id of the parent revision for the first revision in the resulting collection. If empty gets all revisions on server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of revision information as the result.
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnDbServerRevisionsResult> GetRevisionsAfterId(Utf8StringCR revisionId, ICancellationTokenPtr cancellationToken = nullptr);

    //! Download the revision files.
    //! @param[in] revisions Set of revisions to download.
    //! @param[in] callback Download callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of revision information as the result.
    //! @note This is used to download the files in order to revert or inspect them. To update a briefcase DgnDbBriefcase methods should be used.
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnDbResult> DownloadRevisions(const bvector<DgnDbServerRevisionPtr>& revisions,
    HttpRequest::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr);

    //! Verify the access to the revision on the server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that results in error if connection or authentication fails and success otherwise.
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnDbResult> VerifyConnection(ICancellationTokenPtr cancellationToken = nullptr);

    //!< Returns repository information for this connection.
    DGNDBSERVERCLIENT_EXPORT RepositoryInfoCR GetRepositoryInfo();

    //! Returns all available locks for given briefcase id.
    //! @param[in] briefcaseId
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnDbLockSetResult> QueryLocks (const BeSQLite::BeBriefcaseId& briefcaseId, ICancellationTokenPtr cancellationToken = nullptr);

    //! Aquire the requested set of locks.
    //! @param[in] locksRequest Set of locks to acquire
    //! @param[in] briefcaseId
    //! @param[in] lastRevisionId Last pulled revision id
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnLockResponseResult> AcquireLocks (JsonValueCR locksRequest, const BeSQLite::BeBriefcaseId& briefcaseId,
        Utf8StringCR lastRevisionId, ICancellationTokenPtr cancellationToken = nullptr);

    //! Release certain locks.
    //! @param[in] locksRequest Set of locks to release
    //! @param[in] briefcaseId
    //! @param[in] releasedWithRevisionId Revision that was pushed just before those locks are released
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnDbResult> ReleaseLocks (JsonValueCR locksRequest, const BeSQLite::BeBriefcaseId& briefcaseId, Utf8StringCR releasedWithRevisionId,
        ICancellationTokenPtr cancellationToken = nullptr);

    //! Delete all currently held locks by specific briefcase.
    //! @param[in] briefcaseId
    //! @param[in] releasedWithRevisionId Revision that was pushed just before those locks are released
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnDbResult> RelinquishLocks (const BeSQLite::BeBriefcaseId& briefcaseId, Utf8StringCR releasedWithRevisionId, ICancellationTokenPtr cancellationToken = nullptr);

};
END_BENTLEY_DGNDBSERVER_NAMESPACE
