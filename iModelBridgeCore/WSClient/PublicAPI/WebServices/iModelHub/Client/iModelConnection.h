/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Client/iModelConnection.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
#include <WebServices/iModelHub/Client/UserInfoManager.h>
#include <WebServices/iModelHub/Client/VersionsManager.h>
#include <WebServices/iModelHub/Client/ChangeSetCacheManager.h>
#include <WebServices/iModelHub/Client/StatisticsManager.h>
#include <WebServices/iModelHub/Client/ThumbnailsManager.h>
#include <WebServices/iModelHub/Client/ChunkedWSChangeset.h>
#include <WebServices/iModelHub/Client/ConflictsInfo.h>
#include <WebServices/iModelHub/Client/GlobalRequestOptions.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(iModelConnection);
typedef RefCountedPtr<struct iModelConnection> iModelConnectionPtr;

struct CodeLockSetResultInfo;
struct CodeSequence;
struct CodeSequenceSetResultInfo;

typedef std::function<void(EventPtr)> EventCallback;
typedef std::shared_ptr<EventCallback> EventCallbackPtr;
typedef bmap<EventCallbackPtr, EventTypeSet> EventMap;
typedef RefCountedPtr<struct EventCallbackManagerContext> EventCallbackManagerContextPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(EventCallbackManager);
typedef RefCountedPtr<struct EventCallbackManager> EventCallbackManagerPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(EventManager);
typedef RefCountedPtr<struct EventManager> EventManagerPtr;
typedef RefCountedPtr<struct PredownloadManager> PredownloadManagerPtr;
typedef RefCountedPtr<struct CodeLockSetResultInfo> CodeLockSetResultInfoPtr;
typedef std::function<void(const WSObjectsReader::Instance& value, CodeLockSetResultInfoPtr codesLocksResult)> CodeLocksSetAddFunction;
typedef std::function<Tasks::AsyncTaskPtr<void>(Dgn::DgnCodeSet const&, Dgn::DgnCodeSet const&)> CodeCallbackFunction;
DEFINE_POINTER_SUFFIX_TYPEDEFS(CodeSequence);
DEFINE_POINTER_SUFFIX_TYPEDEFS(ChangeSetCacheManager);

DEFINE_TASK_TYPEDEFS(iModelConnectionPtr, iModelConnection);
DEFINE_TASK_TYPEDEFS(FileInfoPtr, File);
DEFINE_TASK_TYPEDEFS(bvector<FileInfoPtr>, Files);
DEFINE_TASK_TYPEDEFS(AzureServiceBusSASDTOPtr, AzureServiceBusSASDTO);
DEFINE_TASK_TYPEDEFS(Utf8String, DgnDbServerString);
DEFINE_TASK_TYPEDEFS(uint64_t, DgnDbServerUInt64);
DEFINE_TASK_TYPEDEFS(CodeLockSetResultInfo, CodeLockSet);
DEFINE_TASK_TYPEDEFS(CodeSequence, CodeSequence);
DEFINE_TASK_TYPEDEFS(Http::Response, EventReponse);
DEFINE_TASK_TYPEDEFS(Dgn::DgnCodeInfoSet, CodeInfoSet);
DEFINE_TASK_TYPEDEFS(Dgn::DgnLockInfoSet, LockInfoSet);

//=======================================================================================
//! CodeSet and DgnDbLockSet results.
//@bsiclass                                    Algirdas.Mikoliunas              06/2016
//=======================================================================================
struct CodeLockSetResultInfo : RefCountedBase
{
private:
    Dgn::DgnCodeSet      m_codes;
    Dgn::DgnCodeInfoSet  m_codeStates;
    Dgn::DgnLockSet      m_locks;
    Dgn::DgnLockInfoSet  m_lockStates;

public:
    CodeLockSetResultInfo() {};
    void AddCode(const Dgn::DgnCode dgnCode, Dgn::DgnCodeState dgnCodeState, BeSQLite::BeBriefcaseId briefcaseId);
    void AddLock(const Dgn::DgnLock dgnLock, BeSQLite::BeBriefcaseId briefcaseId, Utf8StringCR changeSetId);
    void Insert(const CodeLockSetResultInfo& codeLockResultInfo);
    void Insert(const Dgn::DgnCodeSet& codes, const Dgn::DgnCodeInfoSet& codeStates, const Dgn::DgnLockSet& locks, 
                const Dgn::DgnLockInfoSet& lockStates);

    //! Returns the set of codes.
    Dgn::DgnCodeSet const& GetCodes() const {return m_codes;}
    //! Returns code state information.
    Dgn::DgnCodeInfoSet const& GetCodeStates() const {return m_codeStates;}
    //! Returns the set of locks.
    Dgn::DgnLockSet const& GetLocks() const {return m_locks;}
    //! Returns lock state information.
    Dgn::DgnLockInfoSet const& GetLockStates() const {return m_lockStates;}
};

//=======================================================================================
//! CodeSequence instance
//@bsiclass                                    Algirdas.Mikoliunas              08/2016
//=======================================================================================
struct CodeSequence
{
private:
    Dgn::CodeSpecId m_codeSpecId;
    Utf8String m_scope;
    Utf8String m_valuePattern;
    Utf8String m_value;

public:
    enum Type : uint8_t
        {
        Maximum = 0,  //!< Get maximum available code matching given pattern
        NextAvailable = 1,  //!< Get next available code value given pattern, starting index and increment by
        };

    Utf8StringCR GetValue() const {return m_value;}
    Utf8StringCR GetValuePattern() const {return m_valuePattern;}
    Dgn::CodeSpecId GetCodeSpecId() const {return m_codeSpecId;}
    Utf8StringCR GetScope() const {return m_scope;}

    //! Determine if two CodeSequences are equivalent
    bool operator==(CodeSequence const& other) const {return m_codeSpecId == other.m_codeSpecId && 
        m_value == other.m_value && m_scope == other.m_scope && m_valuePattern == other.m_valuePattern;}
    //! Determine if two CodeSequences are not equivalent
    bool operator!=(CodeSequence const& other) const {return !(*this == other);}
    //! Perform ordered comparison, e.g. for inclusion in associative containers
    IMODELHUBCLIENT_EXPORT bool operator<(CodeSequence const& rhs) const;
    
    //! Creates CodeSequence instance.
    CodeSequence() : m_value(""), m_scope(""), m_valuePattern("") {}
    CodeSequence(Dgn::CodeSpecId codeSpecId, Utf8StringCR scope, Utf8StringCR valuePattern) : m_codeSpecId(codeSpecId), m_scope(scope), 
        m_valuePattern(valuePattern), m_value("") {}
    CodeSequence(Dgn::CodeSpecId codeSpecId, Utf8StringCR scope, Utf8StringCR value, Utf8StringCR valuePattern) : m_codeSpecId(codeSpecId), 
        m_scope(scope), m_valuePattern(valuePattern), m_value(value) {}
};

//=======================================================================================
//@bsistruct                                      Algirdas.Mikoliunas          03/2018
//=======================================================================================
enum class ConflictStrategy : uint8_t
{
    Default = 0, //!< No changes applied if conflict occurred
    Continue = 1 << 0, //!< If conflict occurred, valid changes are applied and conflicts are returned for the user
};

//=======================================================================================
//@bsistruct                                      Algirdas.Mikoliunas          03/2018
//=======================================================================================
struct ChangesetResponseOptions
{
    Dgn::IBriefcaseManager::ResponseOptions m_options;
    ConflictStrategy m_conflictStrategy;

public:
    ChangesetResponseOptions(Dgn::IBriefcaseManager::ResponseOptions options, ConflictStrategy confclitStrategy = ConflictStrategy::Default):
        m_options(options), m_conflictStrategy(confclitStrategy) {}

    Dgn::IBriefcaseManager::ResponseOptions GetBriefcaseResponseOptions() const { return m_options; };
    ConflictStrategy GetConflictStrategy() const { return m_conflictStrategy; };
};

//=======================================================================================
//! Connection to a iModel on server.
//! This class performs all of the operations related to a single iModel on the server.
//@bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct iModelConnection : RefCountedBase
{
private:
    friend struct Client;
    friend struct Briefcase;
    friend struct iModelManager;
    friend struct PredownloadManager;
    friend struct EventCallbackManager;
    friend struct VersionsManager;
    friend struct ChangeSetCacheManager;
    friend struct EventManager;

    static PredownloadManagerPtr s_preDownloadManager;
    bool m_subscribedForPreDownload = false;

    iModelInfo                 m_iModelInfo;

    IWSRepositoryClientPtr     m_wsRepositoryClient;
    IAzureBlobStorageClientPtr m_azureClient;
    IHttpHandlerPtr            m_customHandler;
    GlobalRequestOptionsPtr    m_globalRequestOptionsPtr;

    EventManagerPtr            m_eventManagerPtr;
    EventCallbackManagerPtr    m_eventCallbackManagerPtr;
    UserInfoManager            m_userInfoManager;
    VersionsManager            m_versionsManager;
    ChangeSetCacheManager      m_changeSetCacheManager;
    StatisticsManager          m_statisticsManager;
    ThumbnailsManager          m_thumbnailsManager;

    iModelConnection(iModelInfoCR iModel, CredentialsCR credentials, ClientInfoPtr clientInfo, GlobalRequestOptionsPtr globalRequestOptions, IHttpHandlerPtr customHandler);

    StatusTaskPtr DownloadFileInternal(BeFileName localFile, ObjectIdCR fileId, FileAccessKeyPtr fileAccessKey, 
                                       Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const;

    //! Download a copy of the file from the iModel.
    StatusTaskPtr DownloadFile(BeFileName localFile, ObjectIdCR fileId, FileAccessKeyPtr fileAccessKey, Http::Request::ProgressCallbackCR callback = nullptr,
        ICancellationTokenPtr cancellationToken = nullptr) const;

    FileAccessKeyTaskPtr QueryFileAccessKey(ObjectId objectId, ICancellationTokenPtr cancellationToken) const;

    //! Creates a new file instance on the server. 
    FileTaskPtr CreateNewServerFile(FileInfoCR fileInfo, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Performs a file upload to azure blob storage.
    StatusTaskPtr AzureFileUpload(BeFileNameCR filePath, FileAccessKeyPtr url, Http::Request::ProgressCallbackCR callback = nullptr, 
                                  ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Updates existing file instance on the server. 
    StatusTaskPtr UpdateServerFile(FileInfoCR fileInfo, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Finalizes the file upload.
    StatusTaskPtr InitializeServerFile(FileInfoCR fileInfo, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Internal seed files query.
    FilesTaskPtr SeedFilesQuery(WSQuery query, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Write the briefcaseId into the file.
    StatusResult WriteBriefcaseIdIntoFile(BeFileName filePath, BeSQLite::BeBriefcaseId briefcaseId) const;

    //! Download a copy of the seed file from the iModel and initialize it as briefcase
    StatusResult DownloadBriefcaseFile(BeFileName localFile, BeSQLite::BeBriefcaseId briefcaseId, FileAccessKeyPtr fileAccessKey,
                                       Http::Request::ProgressCallbackCR callback = nullptr, 
                                       ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Acquire the requested set of locks.
    StatusTaskPtr AcquireCodesLocksInternal(LockRequestCR locks, Dgn::DgnCodeSet codes, BeSQLite::BeBriefcaseId briefcaseId,
                                            BeSQLite::BeGuidCR seedFileId, Utf8StringCR lastChangeSetId, 
                                            IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::All,
                                            ICancellationTokenPtr cancellationToken = nullptr) const;
    
    //! Delete all currently held codes and/or locks by specific briefcase.
    StatusTaskPtr RelinquishCodesLocksInternal(IBriefcaseManager::Resources resourcesToRelinquish, BeSQLite::BeBriefcaseId briefcaseId,
                                               IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::All, 
                                               ICancellationTokenPtr cancellationToken = nullptr) const;

    //Returns birefcases information for given query. Query should have its filter already set.
    BriefcasesInfoTaskPtr QueryBriefcaseInfoInternal(WSQuery const& query, ICancellationTokenPtr cancellationToken) const;
    
    //Returns all codes by code id
    StatusTaskPtr QueryCodesInternal
    (
    Dgn::DgnCodeSet const& codes,
    BeSQLite::BeBriefcaseId const briefcaseId,
    CodeLockSetResultInfoPtr codesLocksOut,
    ICancellationTokenPtr cancellationToken
    ) const;

    //Returns all codes by briefcase id
    StatusTaskPtr QueryCodesInternal
    (
    BeSQLite::BeBriefcaseId const briefcaseId,
    CodeLockSetResultInfoPtr codesLocksOut,
    ICancellationTokenPtr cancellationToken
    ) const;

    //Returns all locks by lock id
    StatusTaskPtr QueryLocksInternal
    (
    LockableIdSet const& locks,
    BeSQLite::BeBriefcaseId const briefcaseId,
    CodeLockSetResultInfoPtr codesLocksOut,
    ICancellationTokenPtr cancellationToken
    ) const;

    //Returns all locks by briefcase id
    StatusTaskPtr QueryLocksInternal
    (
    BeSQLite::BeBriefcaseId const briefcaseId,
    CodeLockSetResultInfoPtr codesLocksOut,
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

    //! Returns all codes and locks by ids.
    CodeLockSetTaskPtr QueryCodesLocksByIdInternal
    (
    Dgn::DgnCodeSet const& codes,
    LockableIdSet const& locks,
    BeSQLite::BeBriefcaseId briefcaseId,
    ICancellationTokenPtr cancellationToken = nullptr
    ) const;

    //! Returns all codes by ids and briefcase id.
    CodeInfoSetTaskPtr QueryCodesByIdsInternal
    (
    Dgn::DgnCodeSet const& codes,
    BeSQLite::BeBriefcaseId briefcaseId,
    ICancellationTokenPtr cancellationToken = nullptr
    ) const;

    //! Returns all locks by ids and briefcase id.
    LockInfoSetTaskPtr QueryLocksByIdsInternal
    (
    LockableIdSet const& locks,
    BeSQLite::BeBriefcaseId briefcaseId,
    ICancellationTokenPtr cancellationToken = nullptr
    ) const;

    StatusTaskPtr QueryUnavailableCodesInternal(BeSQLite::BeBriefcaseId const briefcaseId, CodeLockSetResultInfoPtr codesLocksOut,
                                                ICancellationTokenPtr cancellationToken) const;

    StatusTaskPtr QueryUnavailableLocksInternal(BeSQLite::BeBriefcaseId const briefcaseId, uint64_t const lastChangeSetIndex,
        CodeLockSetResultInfoPtr codesLocksOut, ICancellationTokenPtr cancellationToken) const;

    CodeSequenceTaskPtr QueryCodeMaximumIndexInternal(std::shared_ptr<WSChangeset> changeSet, 
                                                      ICancellationTokenPtr cancellationToken = nullptr) const;
    CodeSequenceTaskPtr QueryCodeNextAvailableInternal(std::shared_ptr<WSChangeset> changeSet, 
                                                       ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Create a new briefcase instance for this iModel.
    AsyncTaskPtr<WSCreateObjectResult> CreateBriefcaseInstance(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Subscribe callback for the events
    StatusTaskPtr SubscribeEventsCallback(EventTypeSet* eventTypes, EventCallbackPtr callback, iModelConnectionP imodelConnectionP);

    //! Gets single ChangeSet by Id
    ChangeSetInfoTaskPtr GetChangeSetByIdInternal(Utf8StringCR changeSetId, bool loadAccessKey, ICancellationTokenPtr cancellationToken) const;

    //! Get all ChangeSet information based on a query.
    ChangeSetsInfoTaskPtr ChangeSetsFromQueryInternal(WSQuery const& query, bool parseFileAccessKey, 
                                                      ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get all ChangeSet information based on a query (repeated).
    ChangeSetsInfoTaskPtr ChangeSetsFromQuery(WSQuery const& query, bool parseFileAccessKey, 
                                              ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get all of the changeSets after the specific ChangeSetId.
    ChangeSetsInfoTaskPtr GetChangeSetsInternal(WSQuery const& query, bool parseFileAccessKey, 
                                                ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get all of the changeSets.
    ChangeSetsInfoTaskPtr GetAllChangeSetsInternal(bool loadAccessKey, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get all of the changeSets after the specific ChangeSetId.
    ChangeSetsInfoTaskPtr GetChangeSetsAfterIdInternal(Utf8StringCR changeSetId, BeSQLite::BeGuidCR fileId = BeSQLite::BeGuid(false), 
                                                       bool loadAccessKey = false, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download the ChangeSet files.
    ChangeSetsTaskPtr DownloadChangeSetsInternal(bvector<ChangeSetInfoPtr> const& changeSets, Http::Request::ProgressCallbackCR callback = nullptr,
        ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download the ChangeSet files.
    ChangeSetsTaskPtr DownloadChangeSets(std::deque<ObjectId>& changeSetIds, Http::Request::ProgressCallbackCR callback = nullptr,
        ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download the file for this change set from server.
    ChangeSetTaskPtr DownloadChangeSetFile(ChangeSetInfoPtr changeSet, Http::Request::ProgressCallbackCR callback = nullptr,
        ICancellationTokenPtr cancellationToken = nullptr) const;

    WSQuery CreateChangeSetsAfterIdQuery(Utf8StringCR changeSetId, BeSQLite::BeGuidCR fileId) const;
    WSQuery CreateChangeSetsByIdQuery(std::deque<ObjectId>& changeSetIds) const;
    WSQuery CreateBetweenChangeSetsQuery(Utf8StringCR firstchangeSetId, Utf8StringCR secondChangeSetId, BeSQLite::BeGuidCR fileId) const;

    //! Sends a request from changeset.
    StatusTaskPtr SendChangesetRequest(
        std::shared_ptr<WSChangeset> changeset,
        ChangesetResponseOptions const options = ChangesetResponseOptions(Dgn::IBriefcaseManager::ResponseOptions::All, ConflictStrategy::Default),
        ICancellationTokenPtr cancellationToken = nullptr,
        IWSRepositoryClient::RequestOptionsPtr requestOptions = nullptr) const;

    //! Sends a request from changeset.
    StatusTaskPtr SendChangesetRequestWithRetry(
        std::shared_ptr<WSChangeset> changeset,
        ChangesetResponseOptions options = ChangesetResponseOptions(Dgn::IBriefcaseManager::ResponseOptions::All, ConflictStrategy::Default),
        ICancellationTokenPtr cancellationToken = nullptr) const;

    StatusTaskPtr SendChunkedChangesetRequestRecursive(
        ChunkedWSChangeset const& changesets,
        ChangesetResponseOptions options,
        ICancellationTokenPtr cancellationToken,
        IWSRepositoryClient::RequestOptionsPtr requestOptions,
        uint64_t changesetIndex,
        uint8_t attempt,
        uint8_t const attemptsLimit,
        StatusResultPtr finalResult, 
        ConflictsInfoPtr conflictsInfo) const;

    StatusTaskPtr SendChunkedChangesetRequest(
        ChunkedWSChangeset const& changesets,
        ChangesetResponseOptions options = ChangesetResponseOptions(Dgn::IBriefcaseManager::ResponseOptions::All, ConflictStrategy::Default),
        ICancellationTokenPtr cancellationToken = nullptr,
        IWSRepositoryClient::RequestOptionsPtr requestOptions = nullptr,
        uint8_t const attemptsLimit = 3,
        ConflictsInfoPtr conflictsInfo = nullptr) const;

    //! Initializes the changeSet.
    StatusTaskPtr InitializeChangeSet(
        Dgn::DgnRevisionPtr changeSet,
        Dgn::DgnDbR dgndb,
        JsonValueR pushJson,
        ObjectId changeSetObjectId,
        bool relinquishCodesLocks,
        IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::None,
        ICancellationTokenPtr cancellationToken = nullptr,
        ConflictsInfoPtr conflictsInfo = nullptr,
        CodeCallbackFunction* codesCallback = nullptr) const;

    StatusTaskPtr PushPendingCodesLocks(Dgn::DgnDbPtr dgndb, ICancellationTokenPtr cancellationToken = nullptr) const;

    // Wait while bim file is initialized
    void WaitForInitializedBIMFile(BeSQLite::BeGuid fileGuid, FileResultPtr finalResult, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Push this ChangeSet file to server.
    StatusTaskPtr Push(
        DgnRevisionPtr changeSet,
        Dgn::DgnDbR dgndb,
        bool relinquishCodesLocks, 
        Http::Request::ProgressCallbackCR callback = nullptr,
        IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::None,
        ICancellationTokenPtr cancellationToken = nullptr,
        ConflictsInfoPtr conflictsInfo = nullptr,
        CodeCallbackFunction* codesCallback = nullptr) const;

    static Json::Value CreateFileJson(FileInfoCR fileInfo);

    //! Create an instance of the connection to a iModel on the server.
    //! @param[in] iModel iModel information used to connect to a specific iModel on the server.
    //! @param[in] credentials Credentials used to authenticate on the iModel.
    //! @param[in] clientInfo Application information sent to server.
    //! @param[in] customHandler Http handler for connect authentication.
    //! @return Asynchronous task that has the created connection instance as the result.
    //! @note Client is the class that creates this connection. See Client::OpenBriefcase.
    static iModelConnectionResult Create(iModelInfoCR iModel, CredentialsCR credentials, ClientInfoPtr clientInfo, GlobalRequestOptionsPtr globalRequestOptions,
                                         IHttpHandlerPtr customHandler = nullptr);

    //! Checks whether seed file with specified fileId is active.
    //! @param[in] fileId Db guid of the seed file.
    //! @param[in] briefcaseId Briefcase id.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that returns error if there is no active seed file with specified id.
    StatusTaskPtr ValidateBriefcase(BeSQLite::BeGuidCR fileId, BeSQLite::BeBriefcaseId briefcaseId, 
                                    ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Acquire the requested set of locks.
    //! @param[in] locks Set of locks to acquire
    //! @param[in] codes Set of codes to acquire
    //! @param[in] briefcaseId
    //! @param[in] seedFileId
    //! @param[in] lastChangeSetId Last pulled ChangeSetId
    //! @param[in] options
    //! @param[in] cancellationToken
    StatusTaskPtr AcquireCodesLocks(LockRequestCR locks, Dgn::DgnCodeSet codes, BeSQLite::BeBriefcaseId briefcaseId, BeSQLite::BeGuidCR seedFileId, 
                                    Utf8StringCR lastChangeSetId, 
                                    IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::All, 
                                    ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Attempt to acquire the requested set of locks.
    //! @param[in] locks Set of locks to check
    //! @param[in] codes Set of codes to check
    //! @param[in] briefcaseId
    //! @param[in] seedFileId
    //! @param[in] lastChangeSetId Last pulled ChangeSetId
    //! @param[in] options
    //! @param[in] cancellationToken
    StatusTaskPtr QueryCodesLocksAvailability(LockRequestCR locks, Dgn::DgnCodeSet codes, BeSQLite::BeBriefcaseId briefcaseId,
                                              BeSQLite::BeGuidCR seedFileId, Utf8StringCR lastChangeSetId, 
                                              IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::All,
        ICancellationTokenPtr cancellationToken = nullptr) const;
    
    //! Update the Event Subscription
    //! @param[in] eventTypes
    //! @param[in] cancellationToken
    StatusTaskPtr   SubscribeToEvents(EventTypeSet* eventTypes = nullptr, ICancellationTokenPtr cancellationToken = nullptr);

    //! Cancel Events from EventService
    StatusTaskPtr    UnsubscribeToEvents();
public:
    virtual ~iModelConnection();

    //!< Returns iModel information for this connection.
    iModelInfoCR GetiModelInfo() const { return m_iModelInfo; }

    //! Gets UserInfoManager
    //! @return UserInfo manager
    IMODELHUBCLIENT_EXPORT UserInfoManagerCR GetUserInfoManager() const { return m_userInfoManager; }

    //! Gets StatisticsManager
    //! @return Statistics manager
    IMODELHUBCLIENT_EXPORT StatisticsManagerCR GetStatisticsManager() const { return m_statisticsManager; }

    //! Gets ThumbnailsManager
    //! @return Thumbnails manager
    IMODELHUBCLIENT_EXPORT ThumbnailsManagerCR GetThumbnailsManager() const { return m_thumbnailsManager; }

    //!< Gets RepositoryClient.
    //! @return Returns repository client
    //! @private
    IWSRepositoryClientPtr GetRepositoryClient() const { return m_wsRepositoryClient; }

    //! Get custom handler.
    //! @return Returns HttpHandler
    //! @private
    IHttpHandlerPtr GetHttpHandler() { return m_customHandler; }

    //! Sets RepositoryClient.
    //! @param[in] client
    //! @private
    void SetRepositoryClient(IWSRepositoryClientPtr client)
        {
        m_wsRepositoryClient = client;
        m_userInfoManager = UserInfoManager(client);
        m_versionsManager = VersionsManager(client, m_globalRequestOptionsPtr, this);
        m_changeSetCacheManager = ChangeSetCacheManager(this);
        m_statisticsManager = StatisticsManager(client);
        m_thumbnailsManager = ThumbnailsManager(client);
        }

    //! Gets AzureBlobStorageClient.
    //! @return Returns Azure blob storage client
    //! @private
    IAzureBlobStorageClientPtr GetAzureBlobStorageClient() const { return m_azureClient; }

    //! Sets AzureBlobStorageClient.
    //! @param[in] client
    //! @private
    void SetAzureBlobStorageClient(IAzureBlobStorageClientPtr client) { m_azureClient = client; }

    //! Gets VersionsManager
    //! @return Versions manager
    IMODELHUBCLIENT_EXPORT VersionsManagerCR GetVersionsManager() const { return m_versionsManager; }

    //! Gets Change set cache manager
    //! @return Change set cache manager
    IMODELHUBCLIENT_EXPORT ChangeSetCacheManagerCR GetChangeSetCacheManager() const { return m_changeSetCacheManager; }

    //! Receive Events from EventService
    //! @param[in] longPolling
    //! @param[in] cancellationToken
    //! @private
    IMODELHUBCLIENT_EXPORT EventTaskPtr     GetEvent(bool longPolling = false, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Release certain codes and locks.
    //! @param[in] locks Set of locks to release
    //! @param[in] codes Set of codes to release
    //! @param[in] briefcaseId
    //! @param[in] seedFileId
    //! @param[in] options
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT StatusTaskPtr DemoteCodesLocks(Dgn::DgnLockSet const& locks, Dgn::DgnCodeSet const& codes, BeSQLite::BeBriefcaseId briefcaseId,
        BeSQLite::BeGuidCR seedFileId, IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::All, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Delete all currently held codes and locks by specific briefcase.
    //! @param[in] briefcaseId
    //! @param[in] options
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT StatusTaskPtr RelinquishCodesLocks(BeSQLite::BeBriefcaseId briefcaseId,
        IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::All, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Lock iModel for seed file replacement.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the status of acquiring iModel lock as result.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr LockiModel(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Replace a seed file on the server.
    //! @param[in] filePath The path to the BIM file to upload.
    //! @param[in] fileInfo Details of the file.
    //! @param[in] waitForInitialized Wait for new file to be initialized
    //! @param[in] callback
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the uploaded file information as the result.
    //! @note Part of seed file replacement. Needs iModel to be locked before calling. See LockiModel.
    IMODELHUBCLIENT_EXPORT FileTaskPtr UploadNewSeedFile(BeFileNameCR filePath, FileInfoCR fileInfo, bool waitForInitialized = true, 
                                                         Http::Request::ProgressCallbackCR callback = nullptr, 
                                                         ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Cancels seed file creation.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that is successful if file creation was canceled.
    //! @note This function should be used after iModelConnection::UploadNewSeedFile or Client::CreateNewiModel has failed.
    //! This method does not unlock the iModel and allows the same user to attempt seed file replacement again.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr CancelSeedFileCreation(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Unlock iModel.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the status of releasing iModel lock as result.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr UnlockiModel(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all seed files available in the server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of file information as the result.
    IMODELHUBCLIENT_EXPORT FilesTaskPtr GetSeedFiles(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all seed files with specified file id available in the server.
    //! @param[in] fileId DbGuid of the queried seed file 
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of file information as the result.
    IMODELHUBCLIENT_EXPORT FileTaskPtr GetSeedFileById(BeSQLite::BeGuidCR fileId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns latest seed file puted on server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the file information as the result.
    IMODELHUBCLIENT_EXPORT FileTaskPtr GetLatestSeedFile(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download a copy of the seed file from the iModel
    //! @param[in] localFile Location where the downloaded file should be placed.
    //! @param[in] fileId File id.
    //! @param[in] callback
    //! @param[in] cancellationToken
    //! @return Asynchronous task that results in an error if the download failed.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr DownloadSeedFile(BeFileName localFile, Utf8StringCR fileId, 
                                                          Http::Request::ProgressCallbackCR callback = nullptr,
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
    //! @param[in] fileId Id of the seed file changeSets belong to.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of ChangeSet information as the result.
    IMODELHUBCLIENT_EXPORT ChangeSetsInfoTaskPtr GetChangeSetsAfterId(Utf8StringCR changeSetId, BeSQLite::BeGuidCR fileId = BeSQLite::BeGuid(false), 
                                                                      ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get ChangeSets between two specified ChangeSets.
    //! @param[in] firstChangeSetId If empty gets all changeSets before secondChangeSetId
    //! @param[in] secondChangeSetId If empty gets all changeSets before firstChangeSetId.
    //! @param[in] fileId Id of the seed file changeSets belong to.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of ChangeSet information as the result.
    IMODELHUBCLIENT_EXPORT ChangeSetsInfoTaskPtr GetChangeSetsBetween(Utf8StringCR firstChangeSetId, Utf8StringCR secondChangeSetId, 
                                                                      BeSQLite::BeGuidCR fileId = BeSQLite::BeGuid(false), 
                                                                      ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download the ChangeSet files.
    //! @param[in] changeSets Set of changeSets to download.
    //! @param[in] callback Download callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of changeSets metadata as the result.
    //! @note This is used to download the files in order to revert or inspect them. To update a briefcase Briefcase methods should be used.
    IMODELHUBCLIENT_EXPORT ChangeSetsTaskPtr DownloadChangeSets(bvector<ChangeSetInfoPtr> const& changeSets, 
                                                                Http::Request::ProgressCallbackCR callback = nullptr,
                                                                ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download the ChangeSet files.
    //! @param[in] changeSetIds Set of ChangeSet ids to download.
    //! @param[in] callback Download callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of changeSets metadata as the result.
    //! @note This is used to download the files in order to revert or inspect them. To update a briefcase Briefcase methods should be used.
    IMODELHUBCLIENT_EXPORT ChangeSetsTaskPtr DownloadChangeSets(bvector<Utf8String> const& changeSetIds, 
                                                                Http::Request::ProgressCallbackCR callback = nullptr, 
                                                                ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download all ChangeSet files after ChangeSetId
    //! @param[in] changeSetId Id of the parent ChangeSet for the first ChangeSet in the resulting collection. If empty gets all changeSets on server.
    //! @param[in] fileId Db guid of the seed file.
    //! @param[in] callback Download callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of downloaded changeSets metadata as the result.
    //! @note This is used to download the files in order to revert or inspect them. To update a briefcase Briefcase methods should be used.
    IMODELHUBCLIENT_EXPORT ChangeSetsTaskPtr DownloadChangeSetsAfterId(Utf8StringCR changeSetId, BeSQLite::BeGuidCR fileId = BeSQLite::BeGuid(false), 
                                                                       Http::Request::ProgressCallbackCR callback = nullptr, 
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
    IMODELHUBCLIENT_EXPORT BriefcaseInfoTaskPtr QueryBriefcaseInfo(BeSQLite::BeBriefcaseId briefcaseId, 
                                                                   ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns info about selected briefcases.
    //! @param[in] briefcasesIds for which to return briefcases info
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT BriefcasesInfoTaskPtr QueryBriefcasesInfo(bvector<BeSQLite::BeBriefcaseId>& briefcasesIds, 
                                                                     ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all codes and locks.
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT CodeLockSetTaskPtr QueryAllCodesLocks(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all codes and locks by ids.
    //! @param[in] codes
    //! @param[in] locks
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT CodeLockSetTaskPtr QueryCodesLocksById(Dgn::DgnCodeSet const& codes, LockableIdSet const& locks, 
                                                                  ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all codes and locks by ids and briefcase id.
    //! @param[in] codes
    //! @param[in] locks
    //! @param[in] briefcaseId
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT CodeLockSetTaskPtr QueryCodesLocksById(Dgn::DgnCodeSet const& codes, LockableIdSet const& locks, 
                                                                  BeSQLite::BeBriefcaseId briefcaseId, 
                                                                  ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all codes and locks by briefcase id.
    //! @param[in] briefcaseId
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT CodeLockSetTaskPtr QueryCodesLocks(BeSQLite::BeBriefcaseId const briefcaseId, 
                                                              ICancellationTokenPtr cancellationToken = nullptr) const;
    
    //! Returns all codes.
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT CodeInfoSetTaskPtr QueryAllCodes(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all codes by ids.
    //! @param[in] codes
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT CodeInfoSetTaskPtr QueryCodesByIds(Dgn::DgnCodeSet const& codes,
                                                              ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all codes by ids and briefcase id.
    //! @param[in] codes
    //! @param[in] briefcaseId
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT CodeInfoSetTaskPtr QueryCodesByIds(Dgn::DgnCodeSet const& codes,
                                                              BeSQLite::BeBriefcaseId briefcaseId,
                                                              ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all codes by briefcase id.
    //! @param[in] briefcaseId
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT CodeInfoSetTaskPtr QueryCodesByBriefcaseId(BeSQLite::BeBriefcaseId const briefcaseId,
                                                                      ICancellationTokenPtr cancellationToken = nullptr) const;

    
    //! Returns all locks.
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT LockInfoSetTaskPtr QueryAllLocks(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all locks by ids.
    //! @param[in] locks
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT LockInfoSetTaskPtr QueryLocksByIds(LockableIdSet const& locks,
                                                              ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all locks by ids and briefcase id.
    //! @param[in] locks
    //! @param[in] briefcaseId
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT LockInfoSetTaskPtr QueryLocksByIds(LockableIdSet const& locks,
                                                              BeSQLite::BeBriefcaseId briefcaseId,
                                                              ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all locks by briefcase id.
    //! @param[in] briefcaseId
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT LockInfoSetTaskPtr QueryLocksByBriefcaseId(BeSQLite::BeBriefcaseId const briefcaseId,
                                                                      ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all codes and locks held by other briefcases.
    //! @param[in] briefcaseId
    //! @param[in] lastChangeSetId
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT CodeLockSetTaskPtr QueryUnavailableCodesLocks(BeSQLite::BeBriefcaseId const briefcaseId, 
                                                                         Utf8StringCR lastChangeSetId, 
                                                                         ICancellationTokenPtr cancellationToken = nullptr) const;
    
    //! Returns maximum used code value by the given pattern.
    //! @param[in] codeSequence
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT CodeSequenceTaskPtr QueryCodeMaximumIndex(CodeSequenceCR codeSequence, 
                                                                     ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns next available code by the given pattern, index start and increment by value.
    //! @param[in] codeSequence
    //! @param[in] startIndex
    //! @param[in] incrementBy
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT CodeSequenceTaskPtr QueryCodeNextAvailable(CodeSequenceCR codeSequence, int startIndex = 0, 
                                                                      int incrementBy = 1, ICancellationTokenPtr cancellationToken = nullptr) const;

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
