/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbBriefcase.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/RepositoryInfo.h>
#include <DgnDbServer/Client/DgnDbRepositoryConnection.h>
#include <DgnPlatform/DgnDb.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE

typedef std::shared_ptr<struct DgnDbBriefcase> DgnDbBriefcasePtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(DgnDbBriefcase);

DEFINE_TASK_TYPEDEFS(bool, DgnDbServerBool);
DEFINE_TASK_TYPEDEFS(Utf8String, DgnDbServerEventString);
DEFINE_TASK_TYPEDEFS(bvector<DgnDbServerRevisionPtr>, DgnDbServerRevisionMerge);

//=======================================================================================
// @bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct DgnDbBriefcase
{
//__PUBLISH_SECTION_END__
private:
    static const int              s_maxDelayTime = 5000;

    DgnDbRepositoryConnectionPtr  m_repositoryConnection;
    Dgn::DgnDbPtr                 m_db;
     
    DgnDbServerEventCallbackPtr            m_pullMergeAndPushCallback;
    DgnDbServerEvent::DgnDbServerEventType m_lastPullMergeAndPushEvent = DgnDbServerEvent::DgnDbServerEventType::UnknownEventType;

    DgnDbBriefcase(Dgn::DgnDbPtr db, DgnDbRepositoryConnectionPtr connection);

    DgnDbServerRevisionsTaskPtr PullMergeAndPushInternal(Utf8CP description, bool relinquishCodesLocks, Http::Request::ProgressCallbackCR downloadCallback = nullptr, Http::Request::ProgressCallbackCR uploadCallback = nullptr,
                                                          ICancellationTokenPtr cancellationToken = nullptr) const;
    DgnDbServerRevisionsTaskPtr PullMergeAndPushRepeated(Utf8CP description, bool relinquishCodesLocks, Http::Request::ProgressCallbackCR downloadCallback = nullptr, Http::Request::ProgressCallbackCR uploadCallback = nullptr,
                                                          ICancellationTokenPtr cancellationToken = nullptr, int attemptsCount = 1, int attempt = 1, int delay = 0);
    void CheckCreatingRevision() const;
    void WaitForRevisionEvent() const;
    void SubscribeForRevisionEvents();
    void UnsubscribeRevisionEvents();

public:
    //! Create an instance of a briefcase from previously downloaded briefcase file.
    //! @param[in] db Briefcase file. See DgnDbClient::AquireBriefcase.
    //! @param[in] connection Connection to a repository on server.
    //! @return Returns shared pointer of the created instance.
    //! @note This method is called by DgnDbClient. See DgnDbClient::OpenBriefcase.
    static DgnDbBriefcasePtr Create(Dgn::DgnDbPtr db, DgnDbRepositoryConnectionPtr connection);
    
    //! Gets used DgnDbRepositoryConnection
    //! @returns DgnDbRepositoryConnection
    DGNDBSERVERCLIENT_EXPORT DgnDbRepositoryConnectionPtr GetRepositoryConnectionPtr(); //!< Connection to a repository on server.

//__PUBLISH_SECTION_START__
public:
    //! Pull incomming revisions.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that returns success or an error and list of pulled revisions.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerRevisionsTaskPtr Pull(Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;


    //! Merge revisions.
    //! @param[in] revisions Revisions to merge.
    //! @return Asynchronous task that returns success or an error.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr Merge(bvector<DgnDbServerRevisionPtr> const& revisions) const;

    //! Send the outgoing revisions.
    //! @param[in] description Revision description.
    //! @param[in] relinquishCodesLocks Delete all currently held locks and codes.
    //! @param[in] uploadCallback Upload progress callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that returns success or an error and pushed revision.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr Push(Utf8CP description = nullptr, bool relinquishCodesLocks = false,
        Http::Request::ProgressCallbackCR uploadCallback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Pull and merge incomming revisions.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Blocking task that returns success or an error and list of pulled and merged revisions.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerRevisionMergeTaskPtr PullAndMerge(Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Pull and merge incomming revisions and then send the outgoing revisions.
    //! @param[in] description Revision description.
    //! @param[in] relinquishCodesLocks Delete all currently held locks and codes.
    //! @param[in] downloadCallback Download progress callback.
    //! @param[in] uploadCallback Upload progress callback.
    //! @param[in] cancellationToken
    //! @param[in] attemptsCount Maximum count of retries if fail.
    //! @return Blocking task that returns success or an error and list of pulled and merged revisions.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerRevisionMergeTaskPtr PullMergeAndPush(Utf8CP description = nullptr, bool relinquishCodesLocks = false, Http::Request::ProgressCallbackCR downloadCallback = nullptr,
                                                                              Http::Request::ProgressCallbackCR uploadCallback = nullptr,
                                                                              ICancellationTokenPtr cancellationToken = nullptr, int attemptsCount = 1);

    //! Returns true if briefcase is up to date and there are no revisions pending. This will end up sending request to the server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that returns success or an error.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerBoolTaskPtr IsBriefcaseUpToDate(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Return true if able to subscribe to given event types
    //! @param[in] eventTypes Event types callback function must be called for
    //! @param[in] callback   Callback method that is called after one of eventTypes event occurs
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr  SubscribeEventsCallback (DgnDbServerEventTypeSet* eventTypes, DgnDbServerEventCallbackPtr callback) const;

    //! Stops catching events and calling callback
    //! @param[in] callback   Callback that should be stopped calling
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr  UnsubscribeEventsCallback (DgnDbServerEventCallbackPtr callback) const;

    DGNDBSERVERCLIENT_EXPORT Dgn::DgnDbR GetDgnDb() const; //!< Briefcase file.
    DGNDBSERVERCLIENT_EXPORT DgnDbRepositoryConnectionCR GetRepositoryConnection() const; //!< Connection to a repository on server.
    DGNDBSERVERCLIENT_EXPORT BeSQLite::BeBriefcaseId GetBriefcaseId() const; //!< Briefcase Id.
    DGNDBSERVERCLIENT_EXPORT Utf8String GetLastRevisionPulled() const; //!< Last revision that was pulled by this briefcase.

};

END_BENTLEY_DGNDBSERVER_NAMESPACE
