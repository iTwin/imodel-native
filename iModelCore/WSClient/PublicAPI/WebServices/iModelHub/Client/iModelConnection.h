/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Client/iModelConnection.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/Client/WSRepositoryClient.h>
#include <WebServices/Client/WSChangeset.h>
#include <WebServices/iModelHub/Client/Error.h>
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/iModelInfo.h>
#include <WebServices/iModelHub/Client/FileInfo.h>
#include <WebServices/Azure/AzureBlobStorageClient.h>
#include <WebServices/Azure/EventServiceClient.h>
#include <WebServices/Azure/AzureServiceBusSASDTO.h>
#include <WebServices/iModelHub/Events/EventSubscription.h>
#include <WebServices/iModelHub/Events/EventParser.h>
#include <BeHttp/AuthenticationHandler.h>
#include <WebServices/iModelHub/Client/BriefcaseInfo.h>
#include <WebServices/iModelHub/Client/ChangeSetInfo.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_HTTP

DEFINE_POINTER_SUFFIX_TYPEDEFS(iModelConnection);
typedef RefCountedPtr<struct iModelConnection> iModelConnectionPtr;

struct CodeLockSetResultInfo;
struct CodeTemplate;
struct CodeTemplateSetResultInfo;

typedef std::function<void(EventPtr)> EventCallback;
typedef std::shared_ptr<EventCallback> EventCallbackPtr;
typedef bmap<EventCallbackPtr, EventTypeSet> EventMap;
typedef RefCountedPtr<struct EventManagerContext> EventManagerContextPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(EventManager);
typedef RefCountedPtr<struct EventManager> EventManagerPtr;
typedef RefCountedPtr<struct PredownloadManager> PredownloadManagerPtr;
typedef RefCountedPtr<struct CodeLockSetResultInfo> CodeLockSetResultInfoPtr;
typedef std::function<void(const WSObjectsReader::Instance& value, CodeLockSetResultInfoPtr codesLocksResult)> CodeLocksSetAddFunction;

DEFINE_TASK_TYPEDEFS(iModelConnectionPtr, iModelConnection);
DEFINE_TASK_TYPEDEFS(FileInfoPtr, File);
DEFINE_TASK_TYPEDEFS(bvector<FileInfoPtr>, Files);
DEFINE_TASK_TYPEDEFS(AzureServiceBusSASDTOPtr, AzureServiceBusSASDTO);
DEFINE_TASK_TYPEDEFS(Utf8String, DgnDbServerString);
DEFINE_TASK_TYPEDEFS(uint64_t, DgnDbServerUInt64);
DEFINE_TASK_TYPEDEFS(CodeLockSetResultInfo, CodeLockSet);
DEFINE_TASK_TYPEDEFS(CodeTemplate, CodeTemplate);
DEFINE_TASK_TYPEDEFS(Http::Response, EventReponse);

//=======================================================================================
//! CodeSet and DgnDbLockSet results.
//@bsiclass                                    Algirdas.Mikoliunas              06/2016
//=======================================================================================
struct CodeLockSetResultInfo : RefCountedBase
{
private:
    DgnCodeSet      m_codes;
    DgnCodeInfoSet  m_codeStates;
    DgnLockSet      m_locks;
    DgnLockInfoSet  m_lockStates;

public:
    CodeLockSetResultInfo() {};
    void AddCode(const DgnCode dgnCode, DgnCodeState dgnCodeState, BeSQLite::BeBriefcaseId briefcaseId);
    void AddLock(const DgnLock dgnLock, BeSQLite::BeBriefcaseId briefcaseId, Utf8StringCR changeSetId);
    void Insert(const CodeLockSetResultInfo& codeLockResultInfo);
    void Insert(const DgnCodeSet& codes, const DgnCodeInfoSet& codeStates, const DgnLockSet& locks, const DgnLockInfoSet& lockStates);

    //! Returns the set of locks.
    DgnCodeSet const& GetCodes() const {return m_codes;}
    //! Returns lock state information.
    DgnCodeInfoSet const& GetCodeStates() const {return m_codeStates;}
    //! Returns the set of locks.
    DgnLockSet const& GetLocks() const {return m_locks;}
    //! Returns lock state information.
    DgnLockInfoSet const& GetLockStates() const {return m_lockStates;}
};

//=======================================================================================
//! CodeTemplate instance
//@bsiclass                                    Algirdas.Mikoliunas              08/2016
//=======================================================================================
struct CodeTemplate
{
private:
    CodeSpecId m_codeSpecId;
    Utf8String m_scope;
    Utf8String m_valuePattern;
    Utf8String m_value;

public:
    enum Type : uint8_t
        {
        Maximum = 0,  //!< Get maximum available code mathing given pattern
        NextAvailable = 1,  //!< Get next available code value given pattern, starting index and increment by
        };

    Utf8StringCR GetValue() const {return m_value;}
    Utf8StringCR GetValuePattern() const {return m_valuePattern;}
    CodeSpecId GetCodeSpecId() const {return m_codeSpecId;}
    Utf8StringCR GetScope() const {return m_scope;}

    //! Determine if two DgnDbTemplates are equivalent
    bool operator==(CodeTemplate const& other) const {return m_codeSpecId == other.m_codeSpecId && m_value == other.m_value && m_scope == other.m_scope && m_valuePattern == other.m_valuePattern;}
    //! Determine if two DgnDbTemplates are not equivalent
    bool operator!=(CodeTemplate const& other) const {return !(*this == other);}
    //! Perform ordered comparison, e.g. for inclusion in associative containers
    IMODELHUBCLIENT_EXPORT bool operator<(CodeTemplate const& rhs) const;
    
    //! Creates CodeTemplate instance.
    CodeTemplate() : m_value(""), m_scope(""), m_valuePattern("") {}
    CodeTemplate(CodeSpecId codeSpecId, Utf8StringCR scope, Utf8StringCR valuePattern) : m_codeSpecId(codeSpecId), m_scope(scope), m_valuePattern(valuePattern), m_value("") {}
    CodeTemplate(CodeSpecId codeSpecId, Utf8StringCR scope, Utf8StringCR value, Utf8StringCR valuePattern) : m_codeSpecId(codeSpecId), m_scope(scope), m_valuePattern(valuePattern), m_value(value) {}
};

//=======================================================================================
//! Connection to a iModel on server.
//! This class performs all of the operations related to a single iModel on the server.
//@bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct iModelConnection : RefCountedBase
{
private:
    iModelInfo             m_iModelInfo;

    IWSRepositoryClientPtr     m_wsRepositoryClient;
    IAzureBlobStorageClientPtr m_azureClient;

    EventServiceClient*                m_eventServiceClient = nullptr;
    EventSubscriptionPtr    m_eventSubscription;
    AzureServiceBusSASDTOPtr           m_eventSAS;
    EventManagerPtr         m_eventManagerPtr;

    bool m_subscribedForPreDownload = false;
    static PredownloadManagerPtr s_preDownloadManager;

    friend struct Client;
    friend struct Briefcase;
    friend struct iModelManager;
    friend struct PredownloadManager;

    void SubscribeChangeSetsDownload();
    iModelConnection (iModelInfoCR iModel, CredentialsCR credentials, ClientInfoPtr clientInfo, IHttpHandlerPtr customHandler);

    //! Sets AzureBlobStorageClient. 
    void SetAzureClient(IAzureBlobStorageClientPtr azureClient);

    //! Sets EventServiceClient.
    bool SetEventServiceClient(EventTypeSet* eventTypes = nullptr, ICancellationTokenPtr cancellationToken = nullptr);

    //! Sets the EventSASToken in the EventServiceClient
    bool SetEventSASToken(ICancellationTokenPtr cancellationToken = nullptr);

    //! Sets the EventSubscription in the EventServiceClient
    bool SetEventSubscription(EventTypeSet* eventTypes, ICancellationTokenPtr cancellationToken = nullptr);

    //! Create a new briefcase instance for this iModel.
    AsyncTaskPtr<WSCreateObjectResult> CreateBriefcaseInstance (ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Write the briefcaseId into the file.
    StatusResult WriteBriefcaseIdIntoFile (BeFileName filePath, BeSQLite::BeBriefcaseId briefcaseId) const;

    //! Creates a new file instance on the server. 
    FileTaskPtr CreateNewServerFile(FileInfoCR fileInfo, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Updates existing file instance on the server. 
    StatusTaskPtr UpdateServerFile(FileInfoCR fileInfo, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Performs a file upload to on-premise server. 
    StatusTaskPtr OnPremiseFileUpload(BeFileNameCR filePath, ObjectIdCR objectId, Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Performs a file upload to azure blob storage.
    StatusTaskPtr AzureFileUpload(BeFileNameCR filePath, FileAccessKeyPtr url , Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Uploads a BIM file to the server.
    StatusTaskPtr UploadServerFile(BeFileNameCR filePath, FileInfoCR fileInfo, Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Finalizes the file upload.
    StatusTaskPtr InitializeServerFile(FileInfoCR fileInfo, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Internal master files query.
    FilesTaskPtr MasterFilesQuery(WSQuery query, ICancellationTokenPtr cancellationToken = nullptr) const;

    // Wait while bim file is initialized
    void WaitForInitializedBIMFile(BeGuid fileGuid, FileResultPtr finalResult) const;

    //! Queries briefcase file instance from this iModel.
    FileTaskPtr GetBriefcaseFileInfo(BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken) const;

    //! Download a copy of the master file from the iModel and initialize it as briefcase
    StatusResult DownloadBriefcaseFile (BeFileName localFile, BeSQLite::BeBriefcaseId briefcaseId,
        Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    FileAccessKeyTaskPtr QueryFileAccessKey(ObjectId objectId, ICancellationTokenPtr cancellationToken) const;
    StatusTaskPtr DownloadFileInternal 
        (
        BeFileName localFile,
        ObjectIdCR fileId, 
        FileAccessKeyPtr fileAccessKey, 
        Http::Request::ProgressCallbackCR callback, 
        ICancellationTokenPtr cancellationToken
        ) const;

    //! Download the file for this change set from server.
    DgnRevisionTaskPtr DownloadChangeSetFile (ChangeSetInfoPtr changeSet, Http::Request::ProgressCallbackCR callback = nullptr,
                                                 ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Push this ChangeSet file to server.
    StatusTaskPtr Push(DgnRevisionPtr changeSet, Dgn::DgnDbCR dgndb, bool relinquishCodesLocks, Http::Request::ProgressCallbackCR callback = nullptr,
                                 ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get all ChangeSet information based on a query (repeated).
    ChangeSetsInfoTaskPtr ChangeSetsFromQuery (WSQuery const& query, bool parseFileAccessKey, ICancellationTokenPtr cancellationToken = nullptr) const;
    
    //! Get all ChangeSet information based on a query.
    ChangeSetsInfoTaskPtr ChangeSetsFromQueryInternal(WSQuery const& query, bool parseFileAccessKey, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get all of the changeSets after the specific ChangeSetId.
    ChangeSetsInfoTaskPtr GetChangeSetsInternal(WSQuery const& query, bool parseFileAccessKey, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get all of the changeSets.
    ChangeSetsInfoTaskPtr GetAllChangeSetsInternal(bool loadAccessKey, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get all of the changeSets after the specific ChangeSetId.
    ChangeSetsInfoTaskPtr GetChangeSetsAfterIdInternal(Utf8StringCR changeSetId, BeGuidCR fileId = BeGuid(false), bool loadAccessKey = false, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Gets single ChangeSet by Id
    ChangeSetInfoTaskPtr GetChangeSetByIdInternal(Utf8StringCR changeSetId, bool loadAccessKey, ICancellationTokenPtr cancellationToken) const;

    //! Download the ChangeSet files.
    DgnRevisionsTaskPtr DownloadChangeSetsInternal(bvector<ChangeSetInfoPtr> const& changeSets, Http::Request::ProgressCallbackCR callback = nullptr,
        ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download the ChangeSet files.
    DgnRevisionsTaskPtr DownloadChangeSets(std::deque<ObjectId>& changeSetIds, Http::Request::ProgressCallbackCR callback = nullptr,
        ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download a copy of the file from the iModel.
    StatusTaskPtr DownloadFile(BeFileName localFile, ObjectIdCR fileId, Http::Request::ProgressCallbackCR callback = nullptr,
        ICancellationTokenPtr cancellationToken = nullptr) const;

    // This pointer needs to change to be generic
    EventSubscriptionTaskPtr SendEventChangesetRequest(std::shared_ptr<WSChangeset> changeset, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Gets the Event SAS Token from EventServiceClient
	AzureServiceBusSASDTOTaskPtr GetEventServiceSASToken(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get EventSubscription with the given Event Types
    EventSubscriptionTaskPtr GetEventServiceSubscriptionId(EventTypeSet* eventTypes = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Update the EventSubscription to the given EventTypes
    EventSubscriptionTaskPtr UpdateEventServiceSubscriptionId(EventTypeSet* eventTypes = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get Responses from the EventServiceClient
    EventReponseTaskPtr GetEventServiceResponse(int numOfRetries, bool longpolling = true);

    //Returns birefcases information for given query. Query should have its filter already set.
    BriefcasesInfoTaskPtr QueryBriefcaseInfoInternal(WSQuery const& query, ICancellationTokenPtr cancellationToken) const;

    //Returns all codes by code id
    StatusTaskPtr QueryCodesInternal
        (
        DgnCodeSet const& codes,
        BeSQLite::BeBriefcaseId const* briefcaseId,
        CodeLockSetResultInfoPtr codesLocksOut,
        ICancellationTokenPtr cancellationToken
        ) const;

    //Returns all codes by briefcase id
    StatusTaskPtr QueryCodesInternal
    (
        BeSQLite::BeBriefcaseId const*  briefcaseId,
        CodeLockSetResultInfoPtr codesLocksOut,
        ICancellationTokenPtr cancellationToken
    ) const;

    //Returns all locks by lock id
    StatusTaskPtr QueryLocksInternal
        (
        LockableIdSet const& locks,
        BeSQLite::BeBriefcaseId const*  briefcaseId,
        CodeLockSetResultInfoPtr codesLocksOut,
        ICancellationTokenPtr cancellationToken
        ) const;

    //Returns all locks by briefcase id
    StatusTaskPtr QueryLocksInternal
        (
        BeSQLite::BeBriefcaseId const*  briefcaseId,
        CodeLockSetResultInfoPtr codesLocksOut,
        ICancellationTokenPtr cancellationToken
        ) const;

    //! Returns all available codes and locks for given briefcase id.
    CodeLockSetTaskPtr QueryCodesLocksInternal
        (
        DgnCodeSet const* codes,
        LockableIdSet const* locks, 
        BeSQLite::BeBriefcaseId const* briefcaseId,
        ICancellationTokenPtr cancellationToken
        ) const;

    //! Returns all available codes and locks by executing given query.
    StatusTaskPtr QueryCodesLocksInternal
        (
        WSQuery query,
        CodeLockSetResultInfoPtr codesLocksOut,
        CodeLocksSetAddFunction addFunction,
        ICancellationTokenPtr cancellationToken
        ) const;

    //! Sends a request from changeset.
    StatusTaskPtr SendChangesetRequest(std::shared_ptr<WSChangeset> changeset, IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::All,
                                                  ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Sends a request from changeset.
    StatusTaskPtr SendChangesetRequestInternal(std::shared_ptr<WSChangeset> changeset, IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::All,
        ICancellationTokenPtr cancellationToken = nullptr, IWSRepositoryClient::RequestOptionsPtr requestOptions = nullptr) const;

    //! Initializes the changeSet.
    StatusTaskPtr InitializeChangeSet(Dgn::DgnRevisionPtr changeSet, Dgn::DgnDbCR dgndb, JsonValueR pushJson, ObjectId changeSetObjectId, bool relinquishCodesLocks,
                                              Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const;

    //! Acquire the requested set of locks.
    StatusTaskPtr AcquireCodesLocksInternal(LockRequestCR locks, DgnCodeSet codes, BeSQLite::BeBriefcaseId briefcaseId,
        BeGuidCR masterFileId, Utf8StringCR lastChangeSetId, IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::All,
        ICancellationTokenPtr cancellationToken = nullptr) const;

    StatusTaskPtr QueryUnavailableCodesInternal(BeBriefcaseId const briefcaseId, CodeLockSetResultInfoPtr codesLocksOut,
		ICancellationTokenPtr cancellationToken) const;

    StatusTaskPtr QueryUnavailableLocksInternal(BeBriefcaseId const briefcaseId, uint64_t const lastChangeSetIndex,
                                                           CodeLockSetResultInfoPtr codesLocksOut, ICancellationTokenPtr cancellationToken) const;

    WSQuery CreateChangeSetsAfterIdQuery (Utf8StringCR changeSetId, BeGuidCR fileId) const;
    WSQuery CreateChangeSetsByIdQuery(std::deque<ObjectId>& changeSetIds) const;

public:
    virtual ~iModelConnection();

    //!< Gets RepositoryClient.
    //! @return Returns repository client
    IWSRepositoryClientPtr GetRepositoryClient() const {return m_wsRepositoryClient;}

    //! Sets RepositoryClient.
    //! @param[in] client
    void  SetRepositoryClient(IWSRepositoryClientPtr client) {m_wsRepositoryClient.swap(client);}

    //!< Returns iModel information for this connection.
    iModelInfoCR GetiModelInfo() const {return m_iModelInfo;}

    //! Create an instance of the connection to a iModel on the server.
    //! @param[in] iModel iModel information used to connect to a specific iModel on the server.
    //! @param[in] credentials Credentials used to authenticate on the iModel.
    //! @param[in] clientInfo Application information sent to server.
    //! @param[in] customHandler Http handler for connect authentication.
    //! @return Asynchronous task that has the created connection instance as the result.
    //! @note Client is the class that creates this connection. See Client::OpenBriefcase.
    static iModelConnectionResult Create(iModelInfoCR iModel, CredentialsCR credentials, ClientInfoPtr clientInfo,
                                                   IHttpHandlerPtr customHandler = nullptr);

    //! Checks whether master file with specified fileId is active.
    //! @param[in] fileId Db guid of the master file.
    //! @param[in] briefcaseId Briefcase id.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that returns error if there is no active master file with specified id.
    StatusTaskPtr ValidateBriefcase(BeGuidCR fileId, BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Acquire the requested set of locks.
    //! @param[in] locks Set of locks to acquire
    //! @param[in] codes Set of codes to acquire
    //! @param[in] briefcaseId
    //! @param[in] masterFileId
    //! @param[in] lastChangeSetId Last pulled ChangeSetId
    //! @param[in] options
    //! @param[in] cancellationToken
    //! @param[in] options
    IMODELHUBCLIENT_EXPORT StatusTaskPtr AcquireCodesLocks(LockRequestCR locks, DgnCodeSet codes, BeSQLite::BeBriefcaseId briefcaseId,
        BeGuidCR masterFileId, Utf8StringCR lastChangeSetId, IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::All, 
        ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Attempt to acquire the requested set of locks.
    //! @param[in] locks Set of locks to check
    //! @param[in] codes Set of codes to check
    //! @param[in] briefcaseId
    //! @param[in] masterFileId
    //! @param[in] lastChangeSetId Last pulled ChangeSetId
    //! @param[in] options
    //! @param[in] cancellationToken
    //! @param[in] options
    IMODELHUBCLIENT_EXPORT StatusTaskPtr QueryCodesLocksAvailability(LockRequestCR locks, DgnCodeSet codes, BeSQLite::BeBriefcaseId briefcaseId,
        BeGuidCR masterFileId, Utf8StringCR lastChangeSetId, IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::All,
        ICancellationTokenPtr cancellationToken = nullptr) const;
    
    //! Update the Event Subscription
    //! @param[in] eventTypes
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT StatusTaskPtr   SubscribeToEvents(EventTypeSet* eventTypes = nullptr, ICancellationTokenPtr cancellationToken = nullptr);

    //! Receive Events from EventService
    //! @param[in] longPolling
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT EventTaskPtr     GetEvent(bool longPolling = false, ICancellationTokenPtr cancellationToken = nullptr);

    //! Cancel Events from EventService
    IMODELHUBCLIENT_EXPORT StatusTaskPtr    UnsubscribeToEvents();

    //! Release certain codes and locks.
    //! @param[in] locks Set of locks to release
    //! @param[in] codes Set of codes to release
    //! @param[in] briefcaseId
    //! @param[in] masterFileId
    //! @param[in] options
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT StatusTaskPtr DemoteCodesLocks(DgnLockSet const& locks, DgnCodeSet const& codes, BeSQLite::BeBriefcaseId briefcaseId,
        BeGuidCR masterFileId, IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::All, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Delete all currently held codes abd locks by specific briefcase.
    //! @param[in] briefcaseId
    //! @param[in] options
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT StatusTaskPtr RelinquishCodesLocks(BeSQLite::BeBriefcaseId briefcaseId,
        IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::All, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Lock iModel for master file replacement.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the status of acquiring iModel lock as result.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr LockiModel(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Replace a master file on the server.
    //! @param[in] filePath The path to the BIM file to upload.
    //! @param[in] fileInfo Details of the file.
    //! @param[in] waitForInitialized Wait for new file to be initialized
    //! @param[in] callback
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the uploaded file information as the result.
    //! @note Part of master file replacement. Needs iModel to be locked before calling. See LockiModel.
    IMODELHUBCLIENT_EXPORT FileTaskPtr UploadNewMasterFile(BeFileNameCR filePath, FileInfoCR fileInfo, bool waitForInitialized = true, Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Cancels master file creation.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that is successful if file creation was canceled.
    //! @note This function should be used after iModelConnection::UploadNewMasterFile or Client::CreateNewiModel has failed.
    //! This method does not unlock the iModel and allows the same user to attempt master file replacement again.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr CancelMasterFileCreation(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Unlock iModel.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the status of releasing iModel lock as result.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr UnlockiModel(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all master files available in the server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of file information as the result.
    IMODELHUBCLIENT_EXPORT FilesTaskPtr GetMasterFiles(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all master files with specified file id available in the server.
    //! @param[in] fileId DbGuid of the queried master file 
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of file information as the result.
    IMODELHUBCLIENT_EXPORT FileTaskPtr GetMasterFileById(BeGuidCR fileId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download a copy of the master file from the iModel
    //! @param[in] localFile Location where the downloaded file should be placed.
    //! @param[in] fileId File id.
    //! @param[in] callback
    //! @param[in] cancellationToken
    //! @return Asynchronous task that results in an error if the download failed.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr DownloadMasterFile(BeFileName localFile, Utf8StringCR fileId, Http::Request::ProgressCallbackCR callback = nullptr,
        ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Acquire briefcase id without downloading the briefcase file.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the new briefcase info as result.
    IMODELHUBCLIENT_EXPORT BriefcaseInfoTaskPtr AcquireNewBriefcase(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all changeSets available in the server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of ChangeSet information as the result.
    IMODELHUBCLIENT_EXPORT ChangeSetsInfoTaskPtr GetAllChangeSets(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get a ChangeSet for the specific ChangeSetId.
    //! @param[in] changeSetId Id of the ChangeSet to retrieve.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the ChangeSet information as the result.
    IMODELHUBCLIENT_EXPORT ChangeSetInfoTaskPtr GetChangeSetById(Utf8StringCR changeSetId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get all of the changeSets after the specific ChangeSetId.
    //! @param[in] changeSetId Id of the parent ChangeSet for the first ChangeSet in the resulting collection. If empty gets all changeSets on server.
    //! @param[in] fileId Id of the master file changeSets belong to.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of ChangeSet information as the result.
    IMODELHUBCLIENT_EXPORT ChangeSetsInfoTaskPtr GetChangeSetsAfterId(Utf8StringCR changeSetId, BeGuidCR fileId = BeGuid(false), ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download the ChangeSet files.
    //! @param[in] changeSets Set of changeSets to download.
    //! @param[in] callback Download callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of changeSets metadata as the result.
    //! @note This is used to download the files in order to revert or inspect them. To update a briefcase Briefcase methods should be used.
    IMODELHUBCLIENT_EXPORT DgnRevisionsTaskPtr DownloadChangeSets(bvector<ChangeSetInfoPtr> const& changeSets, Http::Request::ProgressCallbackCR callback = nullptr,
                                                                       ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download the ChangeSet files.
    //! @param[in] changeSetIds Set of ChangeSet ids to download.
    //! @param[in] callback Download callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of changeSets metadata as the result.
    //! @note This is used to download the files in order to revert or inspect them. To update a briefcase Briefcase methods should be used.
    IMODELHUBCLIENT_EXPORT DgnRevisionsTaskPtr DownloadChangeSets(bvector<Utf8String> const& changeSetIds, Http::Request::ProgressCallbackCR callback = nullptr,
        ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download all ChangeSet files after ChangeSetId
    //! @param[in] changeSetId Id of the parent ChangeSet for the first ChangeSet in the resulting collection. If empty gets all changeSets on server.
    //! @param[in] fileId Db guid of the master file.
    //! @param[in] callback Download callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of downloaded changeSets metadata as the result.
    //! @note This is used to download the files in order to revert or inspect them. To update a briefcase Briefcase methods should be used.
    IMODELHUBCLIENT_EXPORT DgnRevisionsTaskPtr DownloadChangeSetsAfterId(Utf8StringCR changeSetId, BeGuidCR fileId = BeGuid(false), Http::Request::ProgressCallbackCR callback = nullptr,
                                                                                  ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Verify the access to the change set on the server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that results in error if connection or authentication fails and success otherwise.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr VerifyConnection(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all briefcases info.
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT BriefcasesInfoTaskPtr QueryAllBriefcasesInfo(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns briefcase info about specific briefcase.
    //! @param[in] briefcaseId for queried briefcase
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT BriefcaseInfoTaskPtr QueryBriefcaseInfo(BeSQLite::BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns info about selected briefcases.
    //! @param[in] briefcasesIds for which to return briefcases info
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT BriefcasesInfoTaskPtr QueryBriefcasesInfo(bvector<BeSQLite::BeBriefcaseId>& briefcasesIds, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all codes and locks by ids.
    //! @param[in] codes
    //! @param[in] locks
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT CodeLockSetTaskPtr QueryCodesLocksById(DgnCodeSet const& codes, LockableIdSet const& locks, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all codes and locks by ids and briefcase id.
    //! @param[in] codes
    //! @param[in] locks
    //! @param[in] briefcaseId
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT CodeLockSetTaskPtr QueryCodesLocksById(DgnCodeSet const& codes, LockableIdSet const& locks, BeSQLite::BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all codes and locks by briefcase id.
    //! @param[in] briefcaseId
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT CodeLockSetTaskPtr QueryCodesLocks(BeSQLite::BeBriefcaseId const briefcaseId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all codes and locks held by other briefcases.
    //! @param[in] briefcaseId
    //! @param[in] lastChangeSetId
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT CodeLockSetTaskPtr QueryUnavailableCodesLocks(BeSQLite::BeBriefcaseId const briefcaseId, Utf8StringCR lastChangeSetId, ICancellationTokenPtr cancellationToken = nullptr) const;
    
    //! Returns maximum used code value by the given pattern.
    //! @param[in] codeSpec
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT CodeTemplateTaskPtr QueryCodeMaximumIndex(CodeSpecCR codeSpec, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns next available code by the given pattern, index start and increment by value.
    //! @param[in] codeSpec
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT CodeTemplateTaskPtr QueryCodeNextAvailable(CodeSpecCR codeSpec, ICancellationTokenPtr cancellationToken=nullptr) const;

    //! Checks if iModelConnection is subscribed to EventService
    IMODELHUBCLIENT_EXPORT bool IsSubscribedToEvents() const;

    //! Subscribe callback for the events
    //! @param[in] eventTypes
    //! @param[in] callback
    IMODELHUBCLIENT_EXPORT StatusTaskPtr SubscribeEventsCallback(EventTypeSet* eventTypes, EventCallbackPtr callback);

    //! Unsubscribe callback for events
    //! @param[in] callback
    IMODELHUBCLIENT_EXPORT StatusTaskPtr UnsubscribeEventsCallback(EventCallbackPtr callback);
};
END_BENTLEY_IMODELHUB_NAMESPACE
