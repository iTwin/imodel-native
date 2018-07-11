/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Client/Briefcase.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/iModelInfo.h>
#include <WebServices/iModelHub/Client/iModelConnection.h>
#include <DgnPlatform/DgnDb.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

typedef RefCountedPtr<struct Briefcase> BriefcasePtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(Briefcase);

DEFINE_TASK_TYPEDEFS(bool, Bool);
DEFINE_TASK_TYPEDEFS(Utf8String, EventString);

//=======================================================================================
// @bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct Briefcase : RefCountedBase
{
private:
    friend struct Client;

    static const int    s_maxDelayTime = 5000;
    iModelConnectionPtr m_imodelConnection;
    Dgn::DgnDbPtr       m_db;
    EventCallbackPtr    m_pullMergeAndPushCallback;
    Event::EventType    m_lastPullMergeAndPushEvent = Event::EventType::UnknownEventType;
    bool                m_eventsAvailable;

    Briefcase(Dgn::DgnDbPtr db, iModelConnectionPtr connection);
    //! Create an instance of a briefcase from previously downloaded briefcase file.
    static BriefcasePtr Create(Dgn::DgnDbPtr db, iModelConnectionPtr connection) { return new Briefcase(db, connection); }

    ChangeSetsTaskPtr PullMergeAndPushInternal(Utf8CP description, bool relinquishCodesLocks, 
                                               Http::Request::ProgressCallbackCR downloadCallback = nullptr, 
                                               Http::Request::ProgressCallbackCR uploadCallback = nullptr,
                                               IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::None,
                                               ICancellationTokenPtr cancellationToken = nullptr, ConflictsInfoPtr conflictsInfo = nullptr,
                                               CodeCallbackFunction* codesCallback = nullptr) const;
    ChangeSetsTaskPtr PullMergeAndPushRepeated(Utf8CP description, bool relinquishCodesLocks, 
                                               Http::Request::ProgressCallbackCR downloadCallback = nullptr, 
                                               Http::Request::ProgressCallbackCR uploadCallback = nullptr,
                                               IBriefcaseManager::ResponseOptions options = IBriefcaseManager::ResponseOptions::None,
                                               ICancellationTokenPtr cancellationToken = nullptr, int attemptsCount = 1, int attempt = 1, 
                                               int delay = 0, ConflictsInfoPtr conflictsInfo = nullptr,
                                               CodeCallbackFunction* codesCallback = nullptr);

    RevisionStatus AddRemoveChangeSetsFromDgnDb(ChangeSets changeSets, ICancellationTokenPtr cancellationToken = nullptr) const;
    RevisionStatus MergeChangeSets(ChangeSets::iterator begin, ChangeSets::iterator end, RevisionManagerR changeSetManager, ICancellationTokenPtr cancellationToken) const;
    RevisionStatus ReverseChangeSets(ChangeSets::reverse_iterator rbegin, ChangeSets::reverse_iterator rend, RevisionManagerR changeSetManager, ICancellationTokenPtr cancellationToken) const;

    void CheckCreatingChangeSet(ICancellationTokenPtr cancellationToken = nullptr) const;
    void WaitForChangeSetEvent(ICancellationTokenPtr cancellationToken = nullptr) const;
    void SubscribeForChangeSetEvents();
    void UnsubscribeChangeSetEvents();

public:
    //!< Briefcase file.
    Dgn::DgnDbR GetDgnDb() const {return *m_db;}

    //!< Briefcase Id.
    BeSQLite::BeBriefcaseId GetBriefcaseId() const {return GetDgnDb().GetBriefcaseId();}

    //!< Last changeSet that was pulled by this briefcase.
    Utf8String GetLastChangeSetPulled() const {return GetDgnDb().Revisions().HasReversedRevisions() ? 
        GetDgnDb().Revisions().GetReversedRevisionId() : GetDgnDb().Revisions().GetParentRevisionId();}

    //!< Connection to a iModel on server.
    iModelConnectionCR GetiModelConnection() const {return *m_imodelConnection;}

    //! Gets used iModelConnection
    //! @returns iModelConnection
    //! @private
    iModelConnectionPtr GetiModelConnectionPtr() const {return m_imodelConnection;}

    //! Pull incomming ChangeSets.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that returns success or an error and list of pulled ChangeSets.
    IMODELHUBCLIENT_EXPORT ChangeSetsTaskPtr Pull(Http::Request::ProgressCallbackCR callback = nullptr,
        ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Merge changeSets.
    //! @param[in] changeSets ChangeSets to merge.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that returns success or an error.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr Merge(ChangeSets const& changeSets, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Send the outgoing ChangeSets.
    //! @param[in] description ChangeSet description.
    //! @param[in] relinquishCodesLocks Delete all currently held locks and codes.
    //! @param[in] uploadCallback Upload progress callback.
    //! @param[in] options
    //! @param[in] cancellationToken
    //! @param[in] conflictsInfo Retruns codes/locks conflicts that occured during push.
    //! @param[in] codesCallback Callback for external codes. First argument is assigned codes, second is discarded codes.
    //! @return Asynchronous task that returns success or an error and pushed ChangeSet.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr Push(Utf8CP description, bool relinquishCodesLocks,
                                              Http::Request::ProgressCallbackCR uploadCallback, 
                                              IBriefcaseManager::ResponseOptions options,
                                              ICancellationTokenPtr cancellationToken = nullptr,
                                              ConflictsInfoPtr conflictsInfo = nullptr,
                                              CodeCallbackFunction* codesCallback = nullptr) const;

    //! Send the outgoing ChangeSets.
    //! @param[in] description ChangeSet description.
    //! @param[in] relinquishCodesLocks Delete all currently held locks and codes.
    //! @param[in] uploadCallback Upload progress callback.
    //! @param[in] cancellationToken
    //! @param[in] conflictsInfo Retruns codes/locks conflicts that occured during push.
    //! @param[in] codesCallback Callback for external codes. First argument is assigned codes, second is discarded codes.
    //! @return Asynchronous task that returns success or an error and pushed ChangeSet.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr Push(Utf8CP description = nullptr, bool relinquishCodesLocks = false,
                                              Http::Request::ProgressCallbackCR uploadCallback = nullptr,
                                              ICancellationTokenPtr cancellationToken = nullptr, 
                                              ConflictsInfoPtr conflictsInfo = nullptr,
                                              CodeCallbackFunction* codesCallback = nullptr) const;

    //! Pull and merge incomming changeSets.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Blocking task that returns success or an error and list of pulled and merged changeSets.
    IMODELHUBCLIENT_EXPORT ChangeSetsTaskPtr PullAndMerge(Http::Request::ProgressCallbackCR callback = nullptr,
                                                          ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Pull and merge incomming ChangeSets and then send the outgoing ChangeSets.
    //! @param[in] description ChangeSet description.
    //! @param[in] relinquishCodesLocks Delete all currently held locks and codes.
    //! @param[in] downloadCallback Download progress callback.
    //! @param[in] uploadCallback Upload progress callback.
    //! @param[in] cancellationToken
    //! @param[in] attemptsCount Maximum count of retries if fail.
    //! @param[in] conflictsInfo Retruns codes/locks conflicts that occured during push.
    //! @param[in] codesCallback Callback for external codes. First argument is assigned codes, second is discarded codes.
    //! @return Blocking task that returns success or an error and list of pulled and merged ChangeSet.
    IMODELHUBCLIENT_EXPORT ChangeSetsTaskPtr PullMergeAndPush(Utf8CP description = nullptr, bool relinquishCodesLocks = false,
                                                              Http::Request::ProgressCallbackCR downloadCallback = nullptr,
                                                              Http::Request::ProgressCallbackCR uploadCallback = nullptr,
                                                              ICancellationTokenPtr cancellationToken = nullptr,
                                                              int attemptsCount = 1, 
                                                              ConflictsInfoPtr conflictsInfo = nullptr,
                                                              CodeCallbackFunction* codesCallback = nullptr);

    //! Pull and merge incomming ChangeSets and then send the outgoing ChangeSets.
    //! @param[in] description ChangeSet description.
    //! @param[in] relinquishCodesLocks Delete all currently held locks and codes.
    //! @param[in] downloadCallback Download progress callback.
    //! @param[in] uploadCallback Upload progress callback.
    //! @param[in] options
    //! @param[in] cancellationToken
    //! @param[in] attemptsCount Maximum count of retries if fail.
    //! @param[in] conflictsInfo Retruns codes/locks conflicts that occured during push.
    //! @param[in] codesCallback Callback for external codes. First argument is assigned codes, second is discarded codes.
    //! @return Blocking task that returns success or an error and list of pulled and merged ChangeSet.
    IMODELHUBCLIENT_EXPORT ChangeSetsTaskPtr PullMergeAndPush(Utf8CP description, bool relinquishCodesLocks,
                                                              Http::Request::ProgressCallbackCR downloadCallback,
                                                              Http::Request::ProgressCallbackCR uploadCallback,
                                                              IBriefcaseManager::ResponseOptions options,
                                                              ICancellationTokenPtr cancellationToken = nullptr, 
                                                              int attemptsCount = 1, 
                                                              ConflictsInfoPtr conflictsInfo = nullptr,
                                                              CodeCallbackFunction* codesCallback =  nullptr);

    //! Returns true if briefcase is up to date and there are no ChangeSets pending. This will end up sending request to the server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that returns success or an error.
    IMODELHUBCLIENT_EXPORT BoolTaskPtr IsBriefcaseUpToDate(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Return true if able to subscribe to given event types
    //! @param[in] eventTypes Event types callback function must be called for
    //! @param[in] callback   Callback method that is called after one of eventTypes event occurs
    IMODELHUBCLIENT_EXPORT StatusTaskPtr  SubscribeEventsCallback(EventTypeSet* eventTypes, EventCallbackPtr callback) const;

    //! Stops catching events and calling callback
    //! @param[in] callback   Callback that should be stopped calling
    IMODELHUBCLIENT_EXPORT StatusTaskPtr  UnsubscribeEventsCallback(EventCallbackPtr callback) const;

    //! Updates briefcase to specified Version
    //! @param[in] versionId
    //! @param[in] callback
    //! @param[in] cancellationToken
    //! @return Asynchronous task that returns success or an error.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr UpdateToVersion(Utf8String versionId, Http::Request::ProgressCallbackCR callback = nullptr, 
                                                         ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Updates briefcase to specified ChangeSet
    //! @param[in] changeSetId
    //! @param[in] callback
    //! @param[in] cancellationToken
    //! @return Asynchronous task that returns success or an error.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr UpdateToChangeSet(Utf8String changeSetId, Http::Request::ProgressCallbackCR callback = nullptr, 
                                                           ICancellationTokenPtr cancellationToken = nullptr) const;
};

END_BENTLEY_IMODELHUB_NAMESPACE
