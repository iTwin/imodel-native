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
#include <WebServices/Azure/EventServiceClient.h>
#include <DgnDbServer/Client/DgnDbServerEventSubscription.h>
#include <DgnDbServer/Client/DgnDbServerEventSAS.h>
#include <DgnDbServer/Client/Events/DgnDbServerEventParser.h>
#include <BeHttp/AuthenticationHandler.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES
using namespace std;

DEFINE_POINTER_SUFFIX_TYPEDEFS(DgnDbRepositoryConnection);
typedef std::shared_ptr<struct DgnDbRepositoryConnection>               DgnDbRepositoryConnectionPtr;
typedef struct DgnDbRepositoryConnection const&                         DgnDbRepositoryConnectionCR;

struct DgnDbCodeLockSetResultInfo;
DEFINE_TASK_TYPEDEFS(DgnDbRepositoryConnectionPtr, DgnDbRepositoryConnection);
DEFINE_TASK_TYPEDEFS(DgnDbServerRevisionPtr, DgnDbServerRevision);
DEFINE_TASK_TYPEDEFS(bvector<DgnDbServerRevisionPtr>, DgnDbServerRevisions);
DEFINE_TASK_TYPEDEFS(uint64_t, DgnDbServerUInt64);
DEFINE_TASK_TYPEDEFS(DgnDbCodeLockSetResultInfo, DgnDbServerCodeLockSet);
DEFINE_TASK_TYPEDEFS(void, DgnDbServerCancelEvent);


//=======================================================================================
//! DgnDbCodeSet and DgnDbLockSet results.
//@bsiclass                                    Algirdas.Mikoliunas              06/2016
//=======================================================================================
struct DgnDbCodeLockSetResultInfo
    {
    //__PUBLISH_SECTION_END__
    private:
        DgnCodeSet      m_codes;
        DgnCodeInfoSet  m_codeStates;
        DgnLockSet      m_locks;
        DgnLockInfoSet  m_lockStates;

    public:
        DgnDbCodeLockSetResultInfo() {};
        void AddCode(const DgnCode dgnCode, DgnCodeState dgnCodeState, BeSQLite::BeBriefcaseId briefcaseId, Utf8StringCR repositoryId);
        void AddLock(const DgnLock dgnLock, BeSQLite::BeBriefcaseId briefcaseId, Utf8StringCR repositoryId);

        //__PUBLISH_SECTION_START__
    public:
        //! Returns the set of locks.
        DGNDBSERVERCLIENT_EXPORT const DgnCodeSet& GetCodes() const;
        //! Returns lock state information.
        DGNDBSERVERCLIENT_EXPORT const DgnCodeInfoSet& GetCodeStates() const;
        //! Returns the set of locks.
        DGNDBSERVERCLIENT_EXPORT const DgnLockSet& GetLocks() const;
        //! Returns lock state information.
        DGNDBSERVERCLIENT_EXPORT const DgnLockInfoSet& GetLockStates() const;
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
    IWSRepositoryClientPtr     m_wsRepositoryClient;
    IAzureBlobStorageClientPtr m_azureClient;
    // TODO: Make non static
    static EventServiceClient*         m_eventServiceClient;
    DgnDbServerEventSubscriptionPtr m_eventSubscription;
    DgnDbServerEventSASPtr m_eventSAS;

    friend struct DgnDbClient;
    friend struct DgnDbBriefcase;
    friend struct DgnDbRepositoryManager;

    DgnDbRepositoryConnection (RepositoryInfoCR repository, CredentialsCR credentials, ClientInfoPtr clientInfo, AuthenticationHandlerPtr authenticationHandler);

    //! Sets AzureBlobStorageClient. 
    void SetAzureClient(IAzureBlobStorageClientPtr azureClient);

    //! Sets EventServiceClient.
    bool SetEventServiceClient(bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes = nullptr, ICancellationTokenPtr cancellationToken = nullptr);

    //! Sets the EventSASToken in the EventServiceClient
    bool SetEventSASToken(ICancellationTokenPtr cancellationToken = nullptr);

    //! Sets the EventSubscription in the EventServiceClient
    bool SetEventSubscription(bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes, ICancellationTokenPtr cancellationToken = nullptr);

    //! Update repository info from the server.
    DgnDbServerStatusTaskPtr UpdateRepositoryInfo (ICancellationTokenPtr cancellationToken = nullptr);

    //! Acquire a new briefcase id for this repository.
    AsyncTaskPtr<WSCreateObjectResult> AcquireBriefcaseId (ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Write the briefcaseId into the file.
    DgnDbServerStatusResult WriteBriefcaseIdIntoFile (BeFileName filePath, BeSQLite::BeBriefcaseId briefcaseId) const;

    //! Download a copy of the master file from the repository
    DgnDbServerStatusTaskPtr DownloadBriefcaseFile (BeFileName localFile, BeSQLite::BeBriefcaseId briefcaseId, Utf8StringCR url,
                                                     Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download the file for this revision from server.
    DgnDbServerStatusTaskPtr DownloadRevisionFile (DgnDbServerRevisionPtr revision, Http::Request::ProgressCallbackCR callback = nullptr,
                                                 ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Push this revision file to server.
    DgnDbServerStatusTaskPtr Push (DgnRevisionPtr revision, BeSQLite::BeBriefcaseId briefcaseId, Http::Request::ProgressCallbackCR callback = nullptr,
                                 ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download all revision files after revisionId
    DgnDbServerRevisionsTaskPtr Pull (Utf8StringCR revisionId, Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get all revision information based on a query.
    DgnDbServerRevisionsTaskPtr RevisionsFromQuery (const WSQuery& query, ICancellationTokenPtr cancellationToken = nullptr) const;

    // This pointer needs to change to be generic
    DgnDbServerEventSubscriptionTaskPtr SendEventChangesetRequest(std::shared_ptr<WSChangeset> changeset, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Gets the Event SAS Token from EventServiceClient
    DgnDbServerEventSASTaskPtr GetEventServiceSASToken(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get EventSubscription with the given Event Types
    DgnDbServerEventSubscriptionTaskPtr GetEventServiceSubscriptionId(bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Update the EventSubscription to the given EventTypes
    DgnDbServerEventSubscriptionTaskPtr UpdateEventServiceSubscriptionId(bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get Responses from the EventServiceClient
    Http::Response GetEventServiceResponse(bool longpolling = true, int numOfRetries = 3);

    //! Get the index from a revisionId.
    DgnDbServerUInt64TaskPtr GetRevisionIndex (Utf8StringCR revisionId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all available codes and locks for given briefcase id.
    DgnDbServerCodeLockSetTaskPtr QueryCodesLocksInternal(DgnCodeSet const* codes, LockableIdSet const* locks, const BeSQLite::BeBriefcaseId* briefcaseId, ICancellationTokenPtr cancellationToken) const;

    //! Sends a request from changeset.
    DgnDbServerStatusTaskPtr SendChangesetRequest(std::shared_ptr<WSChangeset> changeset, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Initializes the revision.
    DgnDbServerStatusTaskPtr InitializeRevision(Dgn::DgnRevisionPtr revision, BeSQLite::BeBriefcaseId briefcaseId, JsonValueR pushJson, ObjectId revisionObjectId,
                                              Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const;

public:
    //! Create an instance of the connection to a repository on the server.
    //! @param[in] repository Repository information used to connect to a specific repository on the server.
    //! @param[in] credentials Credentials used to authenticate on the repository.
    //! @param[in] clientInfo Application information sent to server.
    //! @param[in] cancellationToken
    //! @param[in] authenticationHandler Http handler for connect authentication.
    //! @return Asynchronous task that has the created connection instance as the result.
    //! @note DgnDbClient is the class that creates this connection. See DgnDbClient::OpenBriefcase.
    static DgnDbRepositoryConnectionTaskPtr Create (RepositoryInfoCR repository, CredentialsCR credentials, ClientInfoPtr clientInfo,
                                                    ICancellationTokenPtr cancellationToken = nullptr, AuthenticationHandlerPtr authenticationHandler = nullptr);

    //! Acquire the requested set of locks.
    //! @param[in] locks Set of locks to acquire
    //! @param[in] codes Set of codes to acquire
    //! @param[in] briefcaseId
    //! @param[in] lastRevisionId Last pulled revision id
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr AcquireCodesLocks(LockRequestCR locks, DgnCodeSet codes, BeSQLite::BeBriefcaseId briefcaseId,
         Utf8StringCR lastRevisionId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Attempt to acquire the requested set of locks.
    //! @param[in] locks Set of locks to check
    //! @param[in] codes Set of codes to check
    //! @param[in] briefcaseId
    //! @param[in] lastRevisionId Last pulled revision id
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr QueryCodesLocksAvailability(LockRequestCR locks, DgnCodeSet codes, BeSQLite::BeBriefcaseId briefcaseId,
        Utf8StringCR lastRevisionId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Release certain locks.
    //! @param[in] locks Set of locks to release
    //! @param[in] codes Set of codes to release
    //! @param[in] briefcaseId
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr DemoteCodesLocks (const DgnLockSet& locks, DgnCodeSet const& codes, BeSQLite::BeBriefcaseId briefcaseId,
                                                                 ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Delete all currently held locks by specific briefcase.
    //! @param[in] briefcaseId
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr RelinquishCodesLocks (BeSQLite::BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken = nullptr) const;

//__PUBLISH_SECTION_START__
public:
    //! Returns all revisions available in the server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of revision information as the result.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerRevisionsTaskPtr GetAllRevisions (ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get a revision for the specific revision id.
    //! @param[in] revisionId Id of the revision to retrieve.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the revision information as the result.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerRevisionTaskPtr GetRevisionById (Utf8StringCR revisionId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get all of the revisions after the specific revision id.
    //! @param[in] revisionId Id of the parent revision for the first revision in the resulting collection. If empty gets all revisions on server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of revision information as the result.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerRevisionsTaskPtr GetRevisionsAfterId (Utf8StringCR revisionId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download the revision files.
    //! @param[in] revisions Set of revisions to download.
    //! @param[in] callback Download callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of revision information as the result.
    //! @note This is used to download the files in order to revert or inspect them. To update a briefcase DgnDbBriefcase methods should be used.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr DownloadRevisions (const bvector<DgnDbServerRevisionPtr>& revisions, Http::Request::ProgressCallbackCR callback = nullptr,
                                                                       ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Verify the access to the revision on the server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that results in error if connection or authentication fails and success otherwise.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr VerifyConnection (ICancellationTokenPtr cancellationToken = nullptr) const;

    //!< Returns repository information for this connection.
    DGNDBSERVERCLIENT_EXPORT RepositoryInfoCR GetRepositoryInfo () const;

    //! Returns all codes and locks by ids.
    //! @param[in] codes
    //! @param[in] locks
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT DgnDbServerCodeLockSetTaskPtr QueryCodesLocksById(DgnCodeSet const& codes, LockableIdSet const& locks, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all codes and locks by ids and briefcase id.
    //! @param[in] codes
    //! @param[in] locks
    //! @param[in] briefcaseId
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT DgnDbServerCodeLockSetTaskPtr QueryCodesLocksById(DgnCodeSet const& codes, LockableIdSet const& locks, BeSQLite::BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all codes and locks by briefcase id.
    //! @param[in] briefcaseId
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT DgnDbServerCodeLockSetTaskPtr QueryCodesLocks(const BeSQLite::BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all codes and locks held by other briefcases.
    //! @param[in] briefcaseId
    //! @param[in] lastRevisionId
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT DgnDbServerCodeLockSetTaskPtr QueryUnavailableCodesLocks(const BeSQLite::BeBriefcaseId briefcaseId, Utf8StringCR lastRevisionId, ICancellationTokenPtr cancellationToken = nullptr) const;
    
    //! Update the Event Subscription
    //! @param[in] eventTypes
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT bool SubscribeToEvents(bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes = nullptr, ICancellationTokenPtr cancellationToken = nullptr);

    //! Receive Events from EventService
    //! @param[in] longPolling
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT DgnDbServerEventTaskPtr    GetEvent(bool longPolling = false, ICancellationTokenPtr cancellationToken = nullptr);

    //! Cancel Events from EventService
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT DgnDbServerCancelEventTaskPtr    UnsubscribeToEvents (ICancellationTokenPtr cancellationToken = nullptr);

};
END_BENTLEY_DGNDBSERVER_NAMESPACE
