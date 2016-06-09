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

USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

typedef std::shared_ptr<struct DgnDbBriefcase> DgnDbBriefcasePtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(DgnDbBriefcase);

DEFINE_TASK_TYPEDEFS(bool, DgnDbServerBool);
DEFINE_TASK_TYPEDEFS(Utf8String, DgnDbServerEventValue);
DEFINE_TASK_TYPEDEFS(bvector<DgnDbServerRevisionPtr>, DgnDbServerRevisionMerge);

//=======================================================================================
//! A class that represents 
// @bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct DgnDbBriefcase
{
//__PUBLISH_SECTION_END__
private:
    static const int              s_maxDelayTime = 5000;

    DgnDbRepositoryConnectionPtr  m_repositoryConnection;
    Dgn::DgnDbPtr                 m_db;

    DgnDbBriefcase (Dgn::DgnDbPtr db, DgnDbRepositoryConnectionPtr connection);

    DgnDbServerRevisionsTaskPtr PullMergeAndPushInternal (HttpRequest::ProgressCallbackCR downloadCallback = nullptr, HttpRequest::ProgressCallbackCR uploadCallback = nullptr,
                                                          ICancellationTokenPtr cancellationToken = nullptr) const;
    DgnDbServerRevisionsTaskPtr PullMergeAndPushRepeated (HttpRequest::ProgressCallbackCR downloadCallback = nullptr, HttpRequest::ProgressCallbackCR uploadCallback = nullptr,
                                                          ICancellationTokenPtr cancellationToken = nullptr, int attemptsCount = 1, int attempt = 1, int delay = 0) const;

public:
    //! Create an instance of a briefcase from previously downloaded briefcase file.
    //! @param[in] db Briefcase file. See DgnDbClient::AquireBriefcase.
    //! @param[in] connection Connection to a repository on server.
    //! @return Returns shared pointer of the created instance.
    //! @note This method is called by DgnDbClient. See DgnDbClient::OpenBriefcase.
    static DgnDbBriefcasePtr                                    Create                  (Dgn::DgnDbPtr db, DgnDbRepositoryConnectionPtr connection);

//__PUBLISH_SECTION_START__
public:
    //! Pull and merge incomming revisions.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that returns success or an error and list of pulled and merged revisions.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerRevisionMergeTaskPtr    PullAndMerge            (HttpRequest::ProgressCallbackCR callback = nullptr,
                                                                                         ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Pull and merge incomming revisions and then send the outgoing revisions.
    //! @param[in] downloadCallback Download progress callback.
    //! @param[in] uploadCallback Upload progress callback.
    //! @param[in] cancellationToken
    //! @param[in] attemptsCount Maximum count of retries if fail.
    //! @return Asynchronous task that returns success or an error and list of pulled and merged revisions.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerRevisionMergeTaskPtr    PullMergeAndPush        (HttpRequest::ProgressCallbackCR downloadCallback = nullptr,
                                                                                         HttpRequest::ProgressCallbackCR uploadCallback = nullptr,
                                                                                         ICancellationTokenPtr cancellationToken = nullptr, int attemptsCount = 1) const;

    //! Returns true if briefcase is up to date and there are no revisions pending. This will end up sending request to the server.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that returns success or an error.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerBoolTaskPtr             IsBriefcaseUpToDate     (ICancellationTokenPtr cancellationToken = nullptr) const;

	/*DGNDBSERVERCLIENT_EXPORT EventServiceReceiveTaskPtr         GetEvents				(bool longPolling = false, ICancellationTokenPtr cancellationToken = nullptr) const;*/

    /*DGNDBSERVERCLIENT_EXPORT IDgnDbServerEventTaskPtr          GetEvent(bool longPolling = false, ICancellationTokenPtr cancellationToken = nullptr) const;*/
    DGNDBSERVERCLIENT_EXPORT DgnDbServerEventValueTaskPtr      GetEvent(bool longPolling = true, ICancellationTokenPtr cancellationToken = nullptr) const;
    DGNDBSERVERCLIENT_EXPORT IDgnDbServerEventsTaskPtr         GetEvents(bool longPolling = true, ICancellationTokenPtr cancellationToken = nullptr) const;

    DGNDBSERVERCLIENT_EXPORT Dgn::DgnDbR                        GetDgnDb                () const; //!< Briefcase file.
    DGNDBSERVERCLIENT_EXPORT DgnDbRepositoryConnectionCR        GetRepositoryConnection () const; //!< Connection to a repository on server.
    DGNDBSERVERCLIENT_EXPORT BeSQLite::BeBriefcaseId            GetBriefcaseId          () const; //!< Briefcase Id.
    DGNDBSERVERCLIENT_EXPORT Utf8String                         GetLastRevisionPulled   () const; //!< Last revision that was pulled by this briefcase.

};

END_BENTLEY_DGNDBSERVER_NAMESPACE
