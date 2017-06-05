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

DEFINE_POINTER_SUFFIX_TYPEDEFS(iModelConnection);
typedef RefCountedPtr<struct iModelConnection> iModelConnectionPtr;

struct iModelConnectionImpl;
struct CodeLockSetResultInfo;
struct CodeSequence;
struct CodeSequenceSetResultInfo;

typedef std::function<void(EventPtr)> EventCallback;
typedef std::shared_ptr<EventCallback> EventCallbackPtr;
typedef bmap<EventCallbackPtr, EventTypeSet> EventMap;
typedef RefCountedPtr<struct EventManagerContext> EventManagerContextPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(EventManager);
typedef RefCountedPtr<struct EventManager> EventManagerPtr;
typedef RefCountedPtr<struct PredownloadManager> PredownloadManagerPtr;
typedef RefCountedPtr<struct CodeLockSetResultInfo> CodeLockSetResultInfoPtr;
typedef std::function<void(const WSObjectsReader::Instance& value, CodeLockSetResultInfoPtr codesLocksResult)> CodeLocksSetAddFunction;
DEFINE_POINTER_SUFFIX_TYPEDEFS(CodeSequence);

DEFINE_TASK_TYPEDEFS(iModelConnectionPtr, iModelConnection);
DEFINE_TASK_TYPEDEFS(FileInfoPtr, File);
DEFINE_TASK_TYPEDEFS(bvector<FileInfoPtr>, Files);
DEFINE_TASK_TYPEDEFS(AzureServiceBusSASDTOPtr, AzureServiceBusSASDTO);
DEFINE_TASK_TYPEDEFS(Utf8String, DgnDbServerString);
DEFINE_TASK_TYPEDEFS(uint64_t, DgnDbServerUInt64);
DEFINE_TASK_TYPEDEFS(CodeLockSetResultInfo, CodeLockSet);
DEFINE_TASK_TYPEDEFS(CodeSequence, CodeSequence);
DEFINE_TASK_TYPEDEFS(Http::Response, EventReponse);

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
    void Insert(const Dgn::DgnCodeSet& codes, const Dgn::DgnCodeInfoSet& codeStates, const Dgn::DgnLockSet& locks, const Dgn::DgnLockInfoSet& lockStates);

    //! Returns the set of locks.
    Dgn::DgnCodeSet const& GetCodes() const {return m_codes;}
    //! Returns lock state information.
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
        Maximum = 0,  //!< Get maximum available code mathing given pattern
        NextAvailable = 1,  //!< Get next available code value given pattern, starting index and increment by
        };

    Utf8StringCR GetValue() const {return m_value;}
    Utf8StringCR GetValuePattern() const {return m_valuePattern;}
    Dgn::CodeSpecId GetCodeSpecId() const {return m_codeSpecId;}
    Utf8StringCR GetScope() const {return m_scope;}

    //! Determine if two DgnDbTemplates are equivalent
    bool operator==(CodeSequence const& other) const {return m_codeSpecId == other.m_codeSpecId && m_value == other.m_value && m_scope == other.m_scope && m_valuePattern == other.m_valuePattern;}
    //! Determine if two DgnDbTemplates are not equivalent
    bool operator!=(CodeSequence const& other) const {return !(*this == other);}
    //! Perform ordered comparison, e.g. for inclusion in associative containers
    IMODELHUBCLIENT_EXPORT bool operator<(CodeSequence const& rhs) const;
    
    //! Creates CodeSequence instance.
    CodeSequence() : m_value(""), m_scope(""), m_valuePattern("") {}
    CodeSequence(Dgn::CodeSpecId codeSpecId, Utf8StringCR scope, Utf8StringCR valuePattern) : m_codeSpecId(codeSpecId), m_scope(scope), m_valuePattern(valuePattern), m_value("") {}
    CodeSequence(Dgn::CodeSpecId codeSpecId, Utf8StringCR scope, Utf8StringCR value, Utf8StringCR valuePattern) : m_codeSpecId(codeSpecId), m_scope(scope), m_valuePattern(valuePattern), m_value(value) {}
};

//=======================================================================================
//! Connection to a iModel on server.
//! This class performs all of the operations related to a single iModel on the server.
//@bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct iModelConnection : RefCountedBase
{
private:
    RefCountedPtr<iModelConnectionImpl> m_impl;

    friend struct Client;
    friend struct Briefcase;
    friend struct iModelManager;
    friend struct PredownloadManager;

    iModelConnection (iModelInfoCR iModel, CredentialsCR credentials, ClientInfoPtr clientInfo, IHttpHandlerPtr customHandler);

    void SubscribeChangeSetsDownload();

    //! Push this ChangeSet file to server.
    StatusTaskPtr Push(DgnRevisionPtr changeSet, Dgn::DgnDbCR dgndb, bool relinquishCodesLocks, Http::Request::ProgressCallbackCR callback = nullptr,
        ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Gets single ChangeSet by Id
    ChangeSetInfoTaskPtr GetChangeSetByIdInternal(Utf8StringCR changeSetId, bool loadAccessKey, ICancellationTokenPtr cancellationToken) const;

    //! Queries briefcase file instance from this iModel.
    FileTaskPtr GetBriefcaseFileInfo(BeSQLite::BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken) const;

    StatusTaskPtr DownloadFileInternal(BeFileName localFile, ObjectIdCR fileId, FileAccessKeyPtr fileAccessKey, Http::Request::ProgressCallbackCR callback,
        ICancellationTokenPtr cancellationToken) const;

    //! Download a copy of the master file from the iModel and initialize it as briefcase
    StatusResult DownloadBriefcaseFile(BeFileName localFile, BeSQLite::BeBriefcaseId briefcaseId,
        Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Create a new briefcase instance for this iModel.
    AsyncTaskPtr<WSCreateObjectResult> CreateBriefcaseInstance(ICancellationTokenPtr cancellationToken = nullptr) const;

public:
    virtual ~iModelConnection();

    //!< Gets RepositoryClient.
    //! @return Returns repository client
    IMODELHUBCLIENT_EXPORT IWSRepositoryClientPtr GetRepositoryClient() const;

    //! Sets RepositoryClient.
    //! @param[in] client
    IMODELHUBCLIENT_EXPORT void SetRepositoryClient(IWSRepositoryClientPtr client);

    //!< Returns iModel information for this connection.
    IMODELHUBCLIENT_EXPORT iModelInfoCR GetiModelInfo() const;

    //! Create an instance of the connection to a iModel on the server.
    //! @param[in] iModel iModel information used to connect to a specific iModel on the server.
    //! @param[in] credentials Credentials used to authenticate on the iModel.
    //! @param[in] clientInfo Application information sent to server.
    //! @param[in] customHandler Http handler for connect authentication.
    //! @return Asynchronous task that has the created connection instance as the result.
    //! @note Client is the class that creates this connection. See Client::OpenBriefcase.
    static iModelConnectionResult Create(iModelInfoCR iModel, CredentialsCR credentials, ClientInfoPtr clientInfo,
                                                   IHttpHandlerPtr customHandler = nullptr);

    //! Checks whether seed file with specified fileId is active.
    //! @param[in] fileId Db guid of the seed file.
    //! @param[in] briefcaseId Briefcase id.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that returns error if there is no active seed file with specified id.
    StatusTaskPtr ValidateBriefcase(BeSQLite::BeGuidCR fileId, BeSQLite::BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Acquire the requested set of locks.
    //! @param[in] locks Set of locks to acquire
    //! @param[in] codes Set of codes to acquire
    //! @param[in] briefcaseId
    //! @param[in] seedFileId
    //! @param[in] lastChangeSetId Last pulled ChangeSetId
    //! @param[in] options
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT StatusTaskPtr AcquireCodesLocks(LockRequestCR locks, Dgn::DgnCodeSet codes, BeSQLite::BeBriefcaseId briefcaseId,
        BeSQLite::BeGuidCR seedFileId, Utf8StringCR lastChangeSetId, IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::All, 
        ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Attempt to acquire the requested set of locks.
    //! @param[in] locks Set of locks to check
    //! @param[in] codes Set of codes to check
    //! @param[in] briefcaseId
    //! @param[in] seedFileId
    //! @param[in] lastChangeSetId Last pulled ChangeSetId
    //! @param[in] options
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT StatusTaskPtr QueryCodesLocksAvailability(LockRequestCR locks, Dgn::DgnCodeSet codes, BeSQLite::BeBriefcaseId briefcaseId,
        BeSQLite::BeGuidCR seedFileId, Utf8StringCR lastChangeSetId, IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::All,
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
    //! @param[in] seedFileId
    //! @param[in] options
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT StatusTaskPtr DemoteCodesLocks(Dgn::DgnLockSet const& locks, Dgn::DgnCodeSet const& codes, BeSQLite::BeBriefcaseId briefcaseId,
        BeSQLite::BeGuidCR seedFileId, IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::All, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Delete all currently held codes abd locks by specific briefcase.
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
    IMODELHUBCLIENT_EXPORT FileTaskPtr UploadNewSeedFile(BeFileNameCR filePath, FileInfoCR fileInfo, bool waitForInitialized = true, Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

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

    //! Download a copy of the seed file from the iModel
    //! @param[in] localFile Location where the downloaded file should be placed.
    //! @param[in] fileId File id.
    //! @param[in] callback
    //! @param[in] cancellationToken
    //! @return Asynchronous task that results in an error if the download failed.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr DownloadSeedFile(BeFileName localFile, Utf8StringCR fileId, Http::Request::ProgressCallbackCR callback = nullptr,
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
    IMODELHUBCLIENT_EXPORT ChangeSetsInfoTaskPtr GetChangeSetsAfterId(Utf8StringCR changeSetId, BeSQLite::BeGuidCR fileId = BeSQLite::BeGuid(false), ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download the ChangeSet files.
    //! @param[in] changeSets Set of changeSets to download.
    //! @param[in] callback Download callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of changeSets metadata as the result.
    //! @note This is used to download the files in order to revert or inspect them. To update a briefcase Briefcase methods should be used.
    IMODELHUBCLIENT_EXPORT ChangeSetsTaskPtr DownloadChangeSets(bvector<ChangeSetInfoPtr> const& changeSets, Http::Request::ProgressCallbackCR callback = nullptr,
                                                                       ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download the ChangeSet files.
    //! @param[in] changeSetIds Set of ChangeSet ids to download.
    //! @param[in] callback Download callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of changeSets metadata as the result.
    //! @note This is used to download the files in order to revert or inspect them. To update a briefcase Briefcase methods should be used.
    IMODELHUBCLIENT_EXPORT ChangeSetsTaskPtr DownloadChangeSets(bvector<Utf8String> const& changeSetIds, Http::Request::ProgressCallbackCR callback = nullptr,
        ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download all ChangeSet files after ChangeSetId
    //! @param[in] changeSetId Id of the parent ChangeSet for the first ChangeSet in the resulting collection. If empty gets all changeSets on server.
    //! @param[in] fileId Db guid of the seed file.
    //! @param[in] callback Download callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the collection of downloaded changeSets metadata as the result.
    //! @note This is used to download the files in order to revert or inspect them. To update a briefcase Briefcase methods should be used.
    IMODELHUBCLIENT_EXPORT ChangeSetsTaskPtr DownloadChangeSetsAfterId(Utf8StringCR changeSetId, BeSQLite::BeGuidCR fileId = BeSQLite::BeGuid(false), Http::Request::ProgressCallbackCR callback = nullptr,
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
    IMODELHUBCLIENT_EXPORT CodeLockSetTaskPtr QueryCodesLocksById(Dgn::DgnCodeSet const& codes, LockableIdSet const& locks, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns all codes and locks by ids and briefcase id.
    //! @param[in] codes
    //! @param[in] locks
    //! @param[in] briefcaseId
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT CodeLockSetTaskPtr QueryCodesLocksById(Dgn::DgnCodeSet const& codes, LockableIdSet const& locks, BeSQLite::BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken = nullptr) const;

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
    IMODELHUBCLIENT_EXPORT CodeSequenceTaskPtr QueryCodeMaximumIndex(CodeSpecCR codeSpec, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns next available code by the given pattern, index start and increment by value.
    //! @param[in] codeSpec
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT CodeSequenceTaskPtr QueryCodeNextAvailable(CodeSpecCR codeSpec, ICancellationTokenPtr cancellationToken=nullptr) const;

    //! Returns maximum used code value by the given pattern.
    //! @param[in] codeSequence
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT CodeSequenceTaskPtr QueryCodeMaximumIndex(CodeSequenceCR codeSequence, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns next available code by the given pattern, index start and increment by value.
    //! @param[in] codeSequence
    //! @param[in] startIndex
    //! @param[in] incrementBy
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT CodeSequenceTaskPtr QueryCodeNextAvailable(CodeSequenceCR codeSequence, int startIndex = 0, int incrementBy = 1, ICancellationTokenPtr cancellationToken = nullptr) const;

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
