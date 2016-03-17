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
#include <WebServices/Client/WSChangeset.h>
#include <DgnDbServer/Client/DgnDbServerError.h>
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/RepositoryInfo.h>
#include <DgnDbServer/Client/DgnDbServerRevision.h>
#include <WebServices/Azure/AzureBlobStorageClient.h>
#include <DgnClientFx/Utils/Http/AuthenticationHandler.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS
USING_NAMESPACE_BENTLEY_WEBSERVICES

typedef std::shared_ptr<struct DgnDbRepositoryConnection>               DgnDbRepositoryConnectionPtr;
typedef struct DgnDbRepositoryConnection const&                         DgnDbRepositoryConnectionCR;

struct DgnDbLockSetResultInfo;

typedef AsyncResult<DgnDbRepositoryConnectionPtr, DgnDbServerError>     DgnDbRepositoryConnectionResult;
typedef AsyncResult<RepositoryInfoPtr, DgnDbServerError>                DgnDbRepositoryResult;
typedef AsyncResult<DgnRevisionPtr, DgnDbServerError>                   DgnDbRevisionResult;
typedef AsyncResult<bvector<DgnRevisionPtr>, DgnDbServerError>          DgnDbRevisionsResult;
typedef AsyncResult<uint64_t, DgnDbServerError>                         DgnDbUInt64Result;
typedef AsyncResult<uint32_t, DgnDbServerError>                         DgnDbUInt32Result;
typedef AsyncResult<DgnDbLockSetResultInfo, DgnDbServerError>           DgnDbLockSetResult;
typedef AsyncResult<DgnDbServerRevisionPtr, DgnDbServerError>           DgnDbServerRevisionResult;
typedef AsyncResult<bvector<DgnDbServerRevisionPtr>, DgnDbServerError>  DgnDbServerRevisionsResult;

//=======================================================================================
//! DgnDbLockSet results.
//@bsiclass                                      Eligijus.Mauragas              01/2016
//=======================================================================================
struct DgnDbLockSetResultInfo
{
//__PUBLISH_SECTION_END__
private:
    DgnLockSet      m_locks;
    DgnLockInfoSet  m_lockStates;

public:
    DgnDbLockSetResultInfo () {};
    void AddLock (const DgnLock dgnLock, BeSQLite::BeBriefcaseId briefcaseId, Utf8StringCR repositoryId);

//__PUBLISH_SECTION_START__
public:
    //! Returns the set of locks.
    DGNDBSERVERCLIENT_EXPORT const DgnLockSet& GetLocks () const;

    //! Returns lock state information.
    DGNDBSERVERCLIENT_EXPORT const DgnLockInfoSet& GetLockStates () const;
};


//=======================================================================================
//! Connection to a repository on server.
//! This class performs all of the operations related to a single repository on the server.
//@bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct DgnDbRepositoryConnection
{
//__PUBLISH_SECTION_END__
private:
    RepositoryInfo                          m_repositoryInfo;
    WebServices::IWSRepositoryClientPtr     m_wsRepositoryClient;
    WebServices::IAzureBlobStorageClientPtr m_azureClient;

    friend struct DgnDbClient;
    friend struct DgnDbBriefcase;
    friend struct DgnDbRepositoryManager;

    DgnDbRepositoryConnection (RepositoryInfoCR repository, WebServices::CredentialsCR credentials, WebServices::ClientInfoPtr clientInfo, AuthenticationHandlerPtr authenticationHandler);

    //! Sets AzureBlobStorageClient. 
    void SetAzureClient(WebServices::IAzureBlobStorageClientPtr azureClient);

    //! Update repository info from the server.
    AsyncTaskPtr<DgnDbServerResult> UpdateRepositoryInfo (ICancellationTokenPtr cancellationToken = nullptr);

    //! Aquire a new briefcase id for this repository.
    AsyncTaskPtr<WebServices::WSCreateObjectResult> AcquireBriefcaseId (ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Write the briefcaseId into the file.
    DgnDbServerResult WriteBriefcaseIdIntoFile (BeFileName filePath, BeSQLite::BeBriefcaseId briefcaseId) const;

    //! Download a copy of the master file from the repository
    AsyncTaskPtr<DgnDbServerResult> DownloadBriefcaseFile (BeFileName localFile, BeSQLite::BeBriefcaseId briefcaseId, Utf8StringCR url,
                                                     HttpRequest::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download the file for this revision from server.
    AsyncTaskPtr<DgnDbServerResult> DownloadRevisionFile (DgnDbServerRevisionPtr revision, HttpRequest::ProgressCallbackCR callback = nullptr,
                                                    ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Push this revision file to server.
    AsyncTaskPtr<DgnDbServerResult> Push (DgnRevisionPtr revision, BeSQLite::BeBriefcaseId briefcaseId, HttpRequest::ProgressCallbackCR callback = nullptr,
                                    ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download all revision files after revisionId
    AsyncTaskPtr<DgnDbServerRevisionsResult> Pull (Utf8StringCR revisionId, HttpRequest::ProgressCallbackCR callback = nullptr,
                                                   ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get all revision information based on a query.
    AsyncTaskPtr<DgnDbServerRevisionsResult> RevisionsFromQuery (const WebServices::WSQuery& query, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get the index from a revisionId.
    AsyncTaskPtr<DgnDbUInt64Result> GetRevisionIndex (Utf8StringCR revisionId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all available locks for given lock ids and briefcase id.
    AsyncTaskPtr<DgnDbLockSetResult> QueryLocksInternal (LockableIdSet const* ids, const BeSQLite::BeBriefcaseId* briefcaseId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Sends a request from changeset.
    AsyncTaskPtr<DgnDbServerResult> SendChangesetRequest(std::shared_ptr<WSChangeset> changeset, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Initializes the revision.
    AsyncTaskPtr<DgnDbServerResult> InitializeRevision(Dgn::DgnRevisionPtr revision, BeSQLite::BeBriefcaseId briefcaseId, JsonValueR pushJson, ObjectId revisionObjectId,
                                                       HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const;

public:
    //! Create an instance of the connection to a repository on the server.
    //! @param[in] repository Repository information used to connect to a specific repository on the server.
    //! @param[in] credentials Credentials used to authenticate on the repository.
    //! @param[in] clientInfo Application information sent to server.
    //! @param[in] cancellationToken
    //! @param[in] authenticationHandler Http handler for connect authentication.
    //! @return Asynchronous task that has the created connection instance as the result.
    //! @note DgnDbClient is the class that creates this connection. See DgnDbClient::OpenBriefcase.
    static AsyncTaskPtr<DgnDbRepositoryConnectionResult> Create (RepositoryInfoCR repository, CredentialsCR credentials, WebServices::ClientInfoPtr clientInfo,
                                                                 ICancellationTokenPtr cancellationToken = nullptr, AuthenticationHandlerPtr authenticationHandler = nullptr);

    //! Aquire the requested set of locks.
    //! @param[in] locks Set of locks to acquire
    //! @param[in] briefcaseId
    //! @param[in] lastRevisionId Last pulled revision id
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnDbServerResult> AcquireLocks (LockRequestCR locks, BeSQLite::BeBriefcaseId briefcaseId, Utf8StringCR lastRevisionId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Release certain locks.
    //! @param[in] locks Set of locks to release
    //! @param[in] briefcaseId
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnDbServerResult> DemoteLocks (const DgnLockSet& locks, BeSQLite::BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Delete all currently held locks by specific briefcase.
    //! @param[in] briefcaseId
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnDbServerResult> RelinquishLocks (BeSQLite::BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken = nullptr) const;

//__PUBLISH_SECTION_START__
public:
    //! Returns all revisions available in the server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of revision information as the result.
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnDbServerRevisionsResult> GetAllRevisions (ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get a revision for the specific revision id.
    //! @param[in] revisionId Id of the revision to retrieve.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the revision information as the result.
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnDbServerRevisionResult> GetRevisionById (Utf8StringCR revisionId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get all of the revisions after the specific revision id.
    //! @param[in] revisionId Id of the parent revision for the first revision in the resulting collection. If empty gets all revisions on server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of revision information as the result.
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnDbServerRevisionsResult> GetRevisionsAfterId (Utf8StringCR revisionId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download the revision files.
    //! @param[in] revisions Set of revisions to download.
    //! @param[in] callback Download callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of revision information as the result.
    //! @note This is used to download the files in order to revert or inspect them. To update a briefcase DgnDbBriefcase methods should be used.
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnDbServerResult> DownloadRevisions (const bvector<DgnDbServerRevisionPtr>& revisions, HttpRequest::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Verify the access to the revision on the server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that results in error if connection or authentication fails and success otherwise.
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnDbServerResult> VerifyConnection (ICancellationTokenPtr cancellationToken = nullptr) const;

    //!< Returns repository information for this connection.
    DGNDBSERVERCLIENT_EXPORT RepositoryInfoCR GetRepositoryInfo () const;

    //! Returns all available locks for given briefcase id.
    //! @param[in] briefcaseId
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnDbLockSetResult> QueryLocks (BeSQLite::BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all available locks for given lock ids and briefcase id.
    //! @param[in] ids lock ids to query
    //! @param[in] briefcaseId
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnDbLockSetResult> QueryLocksById (LockableIdSet const& ids, BeSQLite::BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all available locks for given lock ids and for any briefcase.
    //! @param[in] ids lock ids to query
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnDbLockSetResult> QueryLocksById (LockableIdSet const& ids, ICancellationTokenPtr cancellationToken = nullptr) const;
};
END_BENTLEY_DGNDBSERVER_NAMESPACE
