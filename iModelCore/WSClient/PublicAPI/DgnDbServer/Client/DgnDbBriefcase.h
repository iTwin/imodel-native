/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbBriefcase.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

//=======================================================================================
//! A class that represents 
// @bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct DgnDbBriefcase
{
//__PUBLISH_SECTION_END__
private:
    DgnDbRepositoryConnectionPtr m_repositoryConnection;
    Dgn::DgnDbPtr m_db;
    DgnDbBriefcase(Dgn::DgnDbPtr db, DgnDbRepositoryConnectionPtr connection);
//__PUBLISH_SECTION_START__
public:
    //! Create an instance of a briefcase from previously downloaded briefcase file.
    //! @param[in] db Briefcase file.
    //! @param[in] connection Connection to a repository on server.
    //! @return Returns shared pointer of the created instance.
    //! @note OpenBriefcase in DgnDbClient is used to create an instance of a DgnDbBriefcase.
    static DgnDbBriefcasePtr Create(Dgn::DgnDbPtr db, DgnDbRepositoryConnectionPtr connection);

    //! Pull and merge incomming revisions.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that returns success or an error.
    DGNDBSERVERCLIENT_EXPORT DgnClientFx::Utils::AsyncTaskPtr<DgnDbResult> PullAndMerge(DgnClientFx::Utils::HttpRequest::ProgressCallbackCR callback = nullptr, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);

    //! Pull and merge incomming revisions and then send the outgoing revisions.
    //! @param[in] downloadCallback Download progress callback.
    //! @param[in] uploadCallback Upload progress callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that returns success or an error.
    DGNDBSERVERCLIENT_EXPORT DgnClientFx::Utils::AsyncTaskPtr<DgnDbResult> PullMergeAndPush(DgnClientFx::Utils::HttpRequest::ProgressCallbackCR downloadCallback = nullptr, DgnClientFx::Utils::HttpRequest::ProgressCallbackCR uploadCallback = nullptr, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken = nullptr);

    DGNDBSERVERCLIENT_EXPORT Dgn::DgnDbR GetDgnDb(); //!< Briefcase file.
    DGNDBSERVERCLIENT_EXPORT DgnDbRepositoryConnectionPtr GetRepositoryConnection(); //!< Connection to a repository on server.
};
END_BENTLEY_DGNDBSERVER_NAMESPACE
