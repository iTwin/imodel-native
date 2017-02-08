/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbRepositoryConnection.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/Client/WSRepositoryClient.h>
#include <WebServices/Client/WSChangeset.h>
#include <DgnDbServer/Client/DgnDbServerError.h>
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/RepositoryInfo.h>
#include <DgnDbServer/Client/FileInfo.h>
#include <WebServices/Azure/AzureBlobStorageClient.h>
#include <WebServices/Azure/EventServiceClient.h>
#include <WebServices/Azure/AzureServiceBusSASDTO.h>
#include <DgnDbServer/Client/DgnDbServerEventSubscription.h>
#include <DgnDbServer/Client/Events/DgnDbServerEventParser.h>
#include <BeHttp/AuthenticationHandler.h>
#include <DgnDbServer/Client/DgnDbServerBriefcaseInfo.h>
#include "DgnDbServerRevisionInfo.h"

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_HTTP
using namespace std;

DEFINE_POINTER_SUFFIX_TYPEDEFS(DgnDbRepositoryConnection);
typedef std::shared_ptr<struct DgnDbRepositoryConnection>               DgnDbRepositoryConnectionPtr;

struct DgnDbCodeLockSetResultInfo;
struct DgnDbCodeTemplate;
struct DgnDbCodeTemplateSetResultInfo;
typedef bset<DgnDbCodeTemplate> DgnDbCodeTemplateSet;

typedef std::function<void(DgnDbServerEventPtr)> DgnDbServerEventCallback;
typedef std::shared_ptr<DgnDbServerEventCallback> DgnDbServerEventCallbackPtr;
typedef bmap<DgnDbServerEventCallbackPtr, DgnDbServerEventTypeSet> DgnDbServerEventMap;
typedef std::shared_ptr<struct DgnDbServerEventManagerContext> DgnDbServerEventManagerContextPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(DgnDbServerEventManager);
typedef std::shared_ptr<struct DgnDbServerEventManager> DgnDbServerEventManagerPtr;
typedef std::shared_ptr<struct DgnDbServerPreDownloadManager> DgnDbServerPreDownloadManagerPtr;
typedef std::shared_ptr<struct DgnDbCodeLockSetResultInfo> DgnDbCodeLockSetResultInfoPtr;
typedef std::function<void(const WSObjectsReader::Instance& value, DgnDbCodeLockSetResultInfoPtr codesLocksResult)> DgnDbCodeLocksSetAddFunction;

DEFINE_TASK_TYPEDEFS(DgnDbRepositoryConnectionPtr, DgnDbRepositoryConnection);
DEFINE_TASK_TYPEDEFS(FileInfoPtr, DgnDbServerFile);
DEFINE_TASK_TYPEDEFS(bvector<FileInfoPtr>, DgnDbServerFiles);
DEFINE_TASK_TYPEDEFS(AzureServiceBusSASDTOPtr, AzureServiceBusSASDTO);
DEFINE_TASK_TYPEDEFS(Utf8String, DgnDbServerString);
DEFINE_TASK_TYPEDEFS(uint64_t, DgnDbServerUInt64);
DEFINE_TASK_TYPEDEFS(DgnDbCodeLockSetResultInfo, DgnDbServerCodeLockSet);
DEFINE_TASK_TYPEDEFS(DgnDbCodeTemplateSetResultInfo, DgnDbServerCodeTemplateSet);
DEFINE_TASK_TYPEDEFS(Http::Response, DgnDbServerEventReponse);

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
        void Insert(const DgnDbCodeLockSetResultInfo& codeLockResultInfo);
        void Insert(const DgnCodeSet& codes, const DgnCodeInfoSet& codeStates, const DgnLockSet& locks, const DgnLockInfoSet& lockStates);

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
//! DgnDbCodeTemplate instance
//@bsiclass                                    Algirdas.Mikoliunas              08/2016
//=======================================================================================
struct DgnDbCodeTemplate
    {
    enum Type : uint8_t
        {
        Maximum       = 0,  //!< Get maximum available code mathing given pattern
        NextAvailable = 1,  //!< Get next available code value given pattern, starting index and increment by
        };

private:
    CodeSpecId m_codeSpecId;
    Utf8String m_scope;
    Utf8String m_valuePattern;
    Utf8String m_value;

public:
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetValue() const { return m_value; }
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetValuePattern() const { return m_valuePattern; }
    DGNDBSERVERCLIENT_EXPORT CodeSpecId GetCodeSpecId() const { return m_codeSpecId; }
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetScope() const { return m_scope; }

    //! Determine if two DgnDbTemplates are equivalent
    bool operator==(DgnDbCodeTemplate const& other) const { return m_codeSpecId == other.m_codeSpecId && m_value == other.m_value && m_scope == other.m_scope && m_valuePattern == other.m_valuePattern;}
    //! Determine if two DgnDbTemplates are not equivalent
    bool operator!=(DgnDbCodeTemplate const& other) const { return !(*this == other); }
    //! Perform ordered comparison, e.g. for inclusion in associative containers
    DGNDBSERVERCLIENT_EXPORT bool operator<(DgnDbCodeTemplate const& rhs) const;
    
    //! Creates DgnDbCodeTemplate instance.
    DGNDBSERVERCLIENT_EXPORT DgnDbCodeTemplate() : m_value(""), m_scope(""), m_valuePattern("") {};
    DGNDBSERVERCLIENT_EXPORT DgnDbCodeTemplate(CodeSpecId codeSpecId, Utf8StringCR scope, Utf8StringCR valuePattern) : m_codeSpecId(codeSpecId), m_valuePattern(valuePattern), m_scope(scope), m_value("") {};
    DGNDBSERVERCLIENT_EXPORT DgnDbCodeTemplate(CodeSpecId codeSpecId, Utf8StringCR scope, Utf8StringCR value, Utf8StringCR valuePattern) : m_codeSpecId(codeSpecId), m_value(value), m_scope(scope), m_valuePattern(valuePattern) {};
    };

//=======================================================================================
//! DgnDbCodeTemplate results.
//@bsiclass                                    Algirdas.Mikoliunas              08/2016
//=======================================================================================
struct DgnDbCodeTemplateSetResultInfo
    {
//__PUBLISH_SECTION_END__
    private:
        DgnDbCodeTemplateSet m_codeTemplates;

    public:
        DgnDbCodeTemplateSetResultInfo() {};
        void AddCodeTemplate(const DgnDbCodeTemplate codeTemplate);

//__PUBLISH_SECTION_START__
    public:
        //! Returns the set of code templates.
        DGNDBSERVERCLIENT_EXPORT const DgnDbCodeTemplateSet& GetTemplates() const;
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
    RepositoryInfo             m_repositoryInfo;

    IWSRepositoryClientPtr     m_wsRepositoryClient;
    IAzureBlobStorageClientPtr m_azureClient;

    EventServiceClient*                m_eventServiceClient = nullptr;
    DgnDbServerEventSubscriptionPtr    m_eventSubscription;
    AzureServiceBusSASDTOPtr           m_eventSAS;
    DgnDbServerEventManagerPtr         m_eventManagerPtr;

    bool m_subscribedForPreDownload = false;
    static DgnDbServerPreDownloadManagerPtr s_preDownloadManager;

    friend struct DgnDbClient;
    friend struct DgnDbBriefcase;
    friend struct DgnDbRepositoryManager;
    friend struct DgnDbServerPreDownloadManager;

    void SubscribeRevisionsDownload();
    DgnDbRepositoryConnection (RepositoryInfoCR repository, CredentialsCR credentials, ClientInfoPtr clientInfo, IHttpHandlerPtr customHandler);

    //! Sets AzureBlobStorageClient. 
    void SetAzureClient(IAzureBlobStorageClientPtr azureClient);

    //! Sets EventServiceClient.
    bool SetEventServiceClient(DgnDbServerEventTypeSet* eventTypes = nullptr, ICancellationTokenPtr cancellationToken = nullptr);

    //! Sets the EventSASToken in the EventServiceClient
    bool SetEventSASToken(ICancellationTokenPtr cancellationToken = nullptr);

    //! Sets the EventSubscription in the EventServiceClient
    bool SetEventSubscription(DgnDbServerEventTypeSet* eventTypes, ICancellationTokenPtr cancellationToken = nullptr);

    //! Create a new briefcase instance for this repository.
    AsyncTaskPtr<WSCreateObjectResult> CreateBriefcaseInstance (ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Write the briefcaseId into the file.
    DgnDbServerStatusResult WriteBriefcaseIdIntoFile (BeFileName filePath, BeSQLite::BeBriefcaseId briefcaseId) const;

    //! Creates a new file instance on the server. 
    DgnDbServerFileTaskPtr   CreateNewServerFile(FileInfoCR fileInfo, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Updates existing file instance on the server. 
    DgnDbServerStatusTaskPtr UpdateServerFile(FileInfoCR fileInfo, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Performs a file upload to on-premise server. 
    DgnDbServerStatusTaskPtr OnPremiseFileUpload(BeFileNameCR filePath, ObjectIdCR objectId, Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Performs a file upload to azure blob storage.
    DgnDbServerStatusTaskPtr AzureFileUpload(BeFileNameCR filePath, Utf8StringCR url , Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Uploads a BIM file to the server.
    DgnDbServerStatusTaskPtr UploadServerFile(BeFileNameCR filePath, FileInfoCR fileInfo, Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Finalizes the file upload.
    DgnDbServerStatusTaskPtr InitializeServerFile(FileInfoCR fileInfo, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Internal master files query.
    DgnDbServerFilesTaskPtr MasterFilesQuery(WSQuery query, ICancellationTokenPtr cancellationToken = nullptr) const;

    // Wait while bim file is initialized
    void WaitForInitializedBIMFile(BeGuid fileGuid, DgnDbServerFileResultPtr finalResult) const;

    //! Queries briefcase file instance from this repository.
    DgnDbServerFileTaskPtr GetBriefcaseFileInfo(BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken) const;

    //! Download a copy of the master file from the repository and initialize it as briefcase
    DgnDbServerStatusResult DownloadBriefcaseFile (BeFileName localFile, BeSQLite::BeBriefcaseId briefcaseId, Utf8StringCR url,
        Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download the file for this revision from server.
    DgnDbServerStatusTaskPtr DownloadRevisionFileInternal(Utf8StringCR revisionId, Utf8StringCR revisionUrl, BeFileNameCR revisionFileName,
        Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const;

    //! Download the file for this revision from server.
    DgnRevisionTaskPtr DownloadRevisionFile (DgnDbServerRevisionInfoPtr revision, Http::Request::ProgressCallbackCR callback = nullptr,
                                                 ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Push this revision file to server.
    DgnDbServerStatusTaskPtr Push(DgnRevisionPtr revision, Dgn::DgnDbCR dgndb, bool relinquishCodesLocks, Http::Request::ProgressCallbackCR callback = nullptr,
                                 ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get all revision information based on a query (repeated).
    DgnDbServerRevisionsInfoTaskPtr RevisionsFromQuery (const WSQuery& query, ICancellationTokenPtr cancellationToken = nullptr) const;
    
    //! Get all revision information based on a query.
    DgnDbServerRevisionsInfoTaskPtr RevisionsFromQueryInternal(const WSQuery& query, ICancellationTokenPtr cancellationToken = nullptr) const;

    // This pointer needs to change to be generic
    DgnDbServerEventSubscriptionTaskPtr SendEventChangesetRequest(std::shared_ptr<WSChangeset> changeset, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Gets the Event SAS Token from EventServiceClient
	AzureServiceBusSASDTOTaskPtr GetEventServiceSASToken(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get EventSubscription with the given Event Types
    DgnDbServerEventSubscriptionTaskPtr GetEventServiceSubscriptionId(DgnDbServerEventTypeSet* eventTypes = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Update the EventSubscription to the given EventTypes
    DgnDbServerEventSubscriptionTaskPtr UpdateEventServiceSubscriptionId(DgnDbServerEventTypeSet* eventTypes = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get Responses from the EventServiceClient
    DgnDbServerEventReponseTaskPtr GetEventServiceResponse(int numOfRetries, bool longpolling = true);

    //Returns birefcases information for given query. Query should have its filter already set.
    DgnDbServerBriefcasesInfoTaskPtr QueryBriefcaseInfoInternal(WSQuery const& query, ICancellationTokenPtr cancellationToken) const;

    //Returns all codes by code id
    DgnDbServerStatusTaskPtr QueryCodesInternal
        (
        const DgnCodeSet& codes, 
        const BeSQLite::BeBriefcaseId* briefcaseId, 
        DgnDbCodeLockSetResultInfoPtr codesLocksOut,
        ICancellationTokenPtr cancellationToken
        ) const;

    //Returns all codes by briefcase id
    DgnDbServerStatusTaskPtr QueryCodesInternal
    (
        const BeSQLite::BeBriefcaseId*  briefcaseId,
        DgnDbCodeLockSetResultInfoPtr codesLocksOut,
        ICancellationTokenPtr cancellationToken
    ) const;

    //Returns all locks by lock id
    DgnDbServerStatusTaskPtr QueryLocksInternal
        (
        const LockableIdSet& locks,
        const BeSQLite::BeBriefcaseId*  briefcaseId,
        DgnDbCodeLockSetResultInfoPtr codesLocksOut,
        ICancellationTokenPtr cancellationToken
        ) const;

    //Returns all locks by briefcase id
    DgnDbServerStatusTaskPtr QueryLocksInternal
        (
        const BeSQLite::BeBriefcaseId*  briefcaseId,
        DgnDbCodeLockSetResultInfoPtr codesLocksOut,
        ICancellationTokenPtr cancellationToken
        ) const;

    //! Returns all available codes and locks for given briefcase id.
    DgnDbServerCodeLockSetTaskPtr QueryCodesLocksInternal
        (
        DgnCodeSet const* codes,
        LockableIdSet const* locks, 
        const BeSQLite::BeBriefcaseId* briefcaseId, 
        ICancellationTokenPtr cancellationToken
        ) const;

    //! Returns all available codes and locks by executing given query.
    DgnDbServerStatusTaskPtr QueryCodesLocksInternal
        (
        WSQuery query,
        DgnDbCodeLockSetResultInfoPtr codesLocksOut,
        DgnDbCodeLocksSetAddFunction addFunction,
        ICancellationTokenPtr cancellationToken
        ) const;

    //! Sends a request from changeset.
    DgnDbServerStatusTaskPtr SendChangesetRequest(std::shared_ptr<WSChangeset> changeset, IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::All,
                                                  ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Sends a request from changeset.
    DgnDbServerStatusTaskPtr SendChangesetRequestInternal(std::shared_ptr<WSChangeset> changeset, IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::All,
        ICancellationTokenPtr cancellationToken = nullptr, IWSRepositoryClient::RequestOptionsPtr requestOptions = nullptr) const;

    //! Initializes the revision.
    DgnDbServerStatusTaskPtr InitializeRevision(Dgn::DgnRevisionPtr revision, Dgn::DgnDbCR dgndb, JsonValueR pushJson, ObjectId revisionObjectId, bool relinquishCodesLocks,
                                              Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const;

    //! Acquire the requested set of locks.
    DgnDbServerStatusTaskPtr AcquireCodesLocksInternal(LockRequestCR locks, DgnCodeSet codes, BeSQLite::BeBriefcaseId briefcaseId,
        BeGuidCR masterFileId, Utf8StringCR lastRevisionId, IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::All,
        ICancellationTokenPtr cancellationToken = nullptr) const;

    DgnDbServerStatusTaskPtr QueryUnavailableCodesInternal(const BeBriefcaseId briefcaseId, const uint64_t lastRevisionIndex,
                                                           DgnDbCodeLockSetResultInfoPtr codesLocksOut, ICancellationTokenPtr cancellationToken) const;

    DgnDbServerStatusTaskPtr QueryUnavailableLocksInternal(const BeBriefcaseId briefcaseId, const uint64_t lastRevisionIndex,
                                                           DgnDbCodeLockSetResultInfoPtr codesLocksOut, ICancellationTokenPtr cancellationToken) const;

public:
    virtual ~DgnDbRepositoryConnection();

    //! Create an instance of the connection to a repository on the server.
    //! @param[in] repository Repository information used to connect to a specific repository on the server.
    //! @param[in] credentials Credentials used to authenticate on the repository.
    //! @param[in] clientInfo Application information sent to server.
    //! @param[in] customHandler Http handler for connect authentication.
    //! @return Asynchronous task that has the created connection instance as the result.
    //! @note DgnDbClient is the class that creates this connection. See DgnDbClient::OpenBriefcase.
    static DgnDbRepositoryConnectionResult Create(RepositoryInfoCR repository, CredentialsCR credentials, ClientInfoPtr clientInfo,
                                                   IHttpHandlerPtr customHandler = nullptr);

    //! Checks whether master file with specified fileId is active.
    //! @param[in] fileId Db guid of the master file.
    //! @param[in] briefcaseId Briefcase id.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that returns error if there is no active master file with specified id.
    DgnDbServerStatusTaskPtr ValidateBriefcase (BeGuidCR fileId, BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Acquire the requested set of locks.
    //! @param[in] locks Set of locks to acquire
    //! @param[in] codes Set of codes to acquire
    //! @param[in] briefcaseId
    //! @param[in] masterFileId
    //! @param[in] lastRevisionId Last pulled revision id
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr AcquireCodesLocks(LockRequestCR locks, DgnCodeSet codes, BeSQLite::BeBriefcaseId briefcaseId,
        BeGuidCR masterFileId, Utf8StringCR lastRevisionId, IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::All, 
        ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Attempt to acquire the requested set of locks.
    //! @param[in] locks Set of locks to check
    //! @param[in] codes Set of codes to check
    //! @param[in] briefcaseId
    //! @param[in] masterFileId
    //! @param[in] lastRevisionId Last pulled revision id
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr QueryCodesLocksAvailability(LockRequestCR locks, DgnCodeSet codes, BeSQLite::BeBriefcaseId briefcaseId,
        BeGuidCR masterFileId, Utf8StringCR lastRevisionId, IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::All,
        ICancellationTokenPtr cancellationToken = nullptr) const;
    
    //!< Gets RepositoryClient.
    //! @return Returns repository client
    DGNDBSERVERCLIENT_EXPORT IWSRepositoryClientPtr GetRepositoryClient();
    //! Sets RepositoryClient.
    //! @param[in] client
    DGNDBSERVERCLIENT_EXPORT void  SetRepositoryClient(IWSRepositoryClientPtr client);

    //! Update the Event Subscription
    //! @param[in] eventTypes
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr   SubscribeToEvents(DgnDbServerEventTypeSet* eventTypes = nullptr, ICancellationTokenPtr cancellationToken = nullptr);

    //! Receive Events from EventService
    //! @param[in] longPolling
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT DgnDbServerEventTaskPtr     GetEvent(bool longPolling = false, ICancellationTokenPtr cancellationToken = nullptr);

    //! Cancel Events from EventService
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr    UnsubscribeToEvents();

//__PUBLISH_SECTION_START__
public:
    //! Release certain codes and locks.
    //! @param[in] locks Set of locks to release
    //! @param[in] codes Set of codes to release
    //! @param[in] briefcaseId
    //! @param[in] masterFileId
    //! @param[in] options
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr DemoteCodesLocks (const DgnLockSet& locks, DgnCodeSet const& codes, BeSQLite::BeBriefcaseId briefcaseId,
        BeGuidCR masterFileId, IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::All, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Delete all currently held codes abd locks by specific briefcase.
    //! @param[in] briefcaseId
    //! @param[in] options
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr RelinquishCodesLocks (BeSQLite::BeBriefcaseId briefcaseId,
        IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::All, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Lock repository for master file replacement.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the status of acquiring repository lock as result.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr LockRepository(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Replace a master file on the server.
    //! @param[in] filePath The path to the BIM file to upload.
    //! @param[in] fileInfo Details of the file.
    //! @param[in] waitForInitialized Wait for new file to be initialized
    //! @param[in] callback
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the uploaded file information as the result.
    //! @note Part of master file replacement. Needs repository to be locked before calling. See LockRepository.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerFileTaskPtr UploadNewMasterFile(BeFileNameCR filePath, FileInfoCR fileInfo, bool waitForInitialized = true, Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Cancels master file creation.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that is successful if file creation was canceled.
    //! @note This function should be used after DgnDbRepositoryConnection::UploadNewMasterFile or DgnDbClient::CreateNewRepository has failed.
    //! This method does not unlock the repository and allows the same user to attempt master file replacement again.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr CancelMasterFileCreation(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Unlock repository.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the status of releasing repository lock as result.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr UnlockRepository(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all master files available in the server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of file information as the result.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerFilesTaskPtr GetMasterFiles(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all master files with specified file id available in the server.
    //! @param[in] fileId DbGuid of the queried master file 
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of file information as the result.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerFileTaskPtr GetMasterFileById(BeGuidCR fileId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download a copy of the master file from the repository
    //! @param[in] localFile Location where the downloaded file should be placed.
    //! @param[in] fileInfo Master file information retrieved from a query to server.
    //! @param[in] callback
    //! @param[in] cancellationToken
    //! @return Asynchronous task that results in an error if the download failed.
    //! @note Should use GetMasterFiles or GetMasterFilesById to get FileInfo instance.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr DownloadMasterFile(BeFileName localFile, FileInfoCR fileInfo, Http::Request::ProgressCallbackCR callback = nullptr,
                                                                        ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Acquire briefcase id without downloading the briefcase file.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the new briefcase info as result.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerBriefcaseInfoTaskPtr AcquireNewBriefcase(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all revisions available in the server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of revision information as the result.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerRevisionsInfoTaskPtr GetAllRevisions (ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get a revision for the specific revision id.
    //! @param[in] revisionId Id of the revision to retrieve.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the revision information as the result.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerRevisionInfoTaskPtr GetRevisionById (Utf8StringCR revisionId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get all of the revisions after the specific revision id.
    //! @param[in] revisionId Id of the parent revision for the first revision in the resulting collection. If empty gets all revisions on server.
    //! @param[in] fileId Id of the master file revisions belong to.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of revision information as the result.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerRevisionsInfoTaskPtr GetRevisionsAfterId (Utf8StringCR revisionId, BeGuidCR fileId = BeGuid(false), ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download the revision files.
    //! @param[in] revisions Set of revisions to download.
    //! @param[in] callback Download callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of revisions metadata as the result.
    //! @note This is used to download the files in order to revert or inspect them. To update a briefcase DgnDbBriefcase methods should be used.
    DGNDBSERVERCLIENT_EXPORT DgnRevisionsTaskPtr DownloadRevisions (const bvector<DgnDbServerRevisionInfoPtr>& revisions, Http::Request::ProgressCallbackCR callback = nullptr,
                                                                       ICancellationTokenPtr cancellationToken = nullptr) const;
    
    //! Download all revision files after revisionId
    //! @param[in] revisionId Id of the parent revision for the first revision in the resulting collection. If empty gets all revisions on server.
    //! @param[in] fileId Db guid of the master file.
    //! @param[in] callback Download callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of downloaded revisions metadata as the result.
    //! @note This is used to download the files in order to revert or inspect them. To update a briefcase DgnDbBriefcase methods should be used.
    DGNDBSERVERCLIENT_EXPORT DgnRevisionsTaskPtr DownloadRevisionsAfterId (Utf8StringCR revisionId, BeGuidCR fileId = BeGuid(false), Http::Request::ProgressCallbackCR callback = nullptr,
                                                                                  ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Verify the access to the revision on the server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that results in error if connection or authentication fails and success otherwise.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr VerifyConnection (ICancellationTokenPtr cancellationToken = nullptr) const;

    //!< Returns repository information for this connection.
    DGNDBSERVERCLIENT_EXPORT RepositoryInfoCR GetRepositoryInfo () const;

    //! Returns all briefcases info.
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT DgnDbServerBriefcasesInfoTaskPtr QueryAllBriefcasesInfo(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns briefcase info about specific briefcase.
    //! @param[in] briefcaseId for queried briefcase
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT DgnDbServerBriefcaseInfoTaskPtr QueryBriefcaseInfo(BeSQLite::BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns info about selected briefcases.
    //! @param[in] briefcasesIds for which to return briefcases info
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT DgnDbServerBriefcasesInfoTaskPtr QueryBriefcasesInfo(bvector<BeSQLite::BeBriefcaseId>& briefcasesIds, ICancellationTokenPtr cancellationToken = nullptr) const;

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
    
    //! Returns maximum used code value by the given pattern.
    //! @param[in] codeTemplates
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT DgnDbServerCodeTemplateSetTaskPtr QueryCodeMaximumIndex(DgnDbCodeTemplateSet codeTemplates, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns next available code by the given pattern, index start and increment by value.
    //! @param[in] codeTemplates
    //! @param[in] startIndex
    //! @param[in] incrementBy
    //! @param[in] cancellationToken
    DGNDBSERVERCLIENT_EXPORT DgnDbServerCodeTemplateSetTaskPtr QueryCodeNextAvailable(DgnDbCodeTemplateSet codeTemplates, int startIndex=0, int incrementBy=1, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Checks if RepositoryConnection is subscribed to EventService
    DGNDBSERVERCLIENT_EXPORT bool IsSubscribedToEvents() const;

    //! Subscribe callback for the events
    //! @param[in] eventTypes
    //! @param[in] callback
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr SubscribeEventsCallback(DgnDbServerEventTypeSet* eventTypes, DgnDbServerEventCallbackPtr callback);

    //! Unsubscribe callback for events
    //! @param[in] callback
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr UnsubscribeEventsCallback(DgnDbServerEventCallbackPtr callback);


};
END_BENTLEY_DGNDBSERVER_NAMESPACE
