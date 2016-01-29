/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbClient.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <WebServices/Client/WSClient.h>
#include <DgnDbServer/Client/RepositoryInfo.h>
#include <DgnDbServer/Client/DgnDbRepositoryConnection.h>
#include <DgnDbServer/Client/DgnDbBriefcase.h>
#include <DgnDbServer/Client/DgnDbLocks.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS
typedef std::shared_ptr<struct DgnDbClient> DgnDbClientPtr;
typedef AsyncResult<bvector<RepositoryInfoPtr>, DgnDbServerError> DgnDbRepositoriesResult;
typedef AsyncResult<RepositoryInfoPtr, DgnDbServerError> DgnDbRepositoryResult;
typedef AsyncResult<DgnDbBriefcasePtr, DgnDbServerError> DgnDbBriefcaseResult;
typedef AsyncResult<BeFileName, DgnDbServerError> DgnDbFileNameResult;

//=======================================================================================
//! Client of DgnDbServer.
//! This class provides the interface for the operations with repository files.
// @bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct DgnDbClient
{
//__PUBLISH_SECTION_END__
private:
#ifdef NEEDSWORK_LOCKS
    DgnDbLocksPtr m_locks;
#endif // NEEDSWORK_LOCKS
    Utf8String m_serverUrl;
    Credentials m_credentials;
    WebServices::ClientInfoPtr m_clientInfo;

    AsyncTaskPtr<DgnDbRepositoryConnectionResult> ConnectToRepository(RepositoryInfoPtr repository, ICancellationTokenPtr cancellationToken = nullptr);
    DgnDbClient(WebServices::ClientInfoPtr clientInfo);
    AsyncTaskPtr<DgnDbRepositoryResult> InitializeRepository(WebServices::IWSRepositoryClientPtr client, Utf8StringCR repositoryId, Json::Value repositoryCreationJson,
                                                             WebServices::ObjectId repositoryObjectId, HttpRequest::ProgressCallbackCR callback = nullptr,
                                                             ICancellationTokenPtr cancellationToken = nullptr);
//__PUBLISH_SECTION_START__
public:
    //! Set up the DgnDbServer Client library. Needs to be called from the work thread where DgnPlatform is initialized.
    DGNDBSERVERCLIENT_EXPORT static void Initialize();

    //! Create an instance of the client.
    //! @param[in] clientInfo Application information sent to server.
    //! @return Returns a shared pointer to the created instance.
    DGNDBSERVERCLIENT_EXPORT static DgnDbClientPtr Create(WebServices::ClientInfoPtr clientInfo);

    DGNDBSERVERCLIENT_EXPORT void SetServerURL(Utf8StringCR serverUrl); //!< Address of the server.

    DGNDBSERVERCLIENT_EXPORT void SetCredentials(CredentialsCR credentials); //!< Credentials used to authenticate on the server.

    //! Get list of available repostiories for this client.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has a collection of repository information as the result.
    //! @note Does not return unpublished, uninitialized repositories or repositories that the user does not have authorization to access.
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnDbRepositoriesResult> GetRepositories(ICancellationTokenPtr cancellationToken = nullptr);

    //! Create a new repository on the server.
    //! @param[in] db A DgnDb file to upload as a master file for the repository.
    //! @param[in] publish Sets the respository as published (unpublished repositories cannot be accessed).
    //! @param[in] callback Progress callback for the file upload.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has created repository information as the result.
    //! @note This method uses dgn_Proj Name property in the DgnDb file as as a repository id and will fail if it is not set.
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnDbRepositoryResult> CreateNewRepository(Dgn::DgnDbPtr db, bool publish = true,
    HttpRequest::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr);

    //! Create a new repository on the server.
    //! @param[in] db A DgnDb file to upload as a master file for the repository.
    //! @param[in] repositoryId Repository name on server.
    //! @param[in] description Short description of the repository.
    //! @param[in] publish Sets the respository as published (unpublished repositories cannot be accessed).
    //! @param[in] callback Progress callback for the file upload.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has created repository information as the result.
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnDbRepositoryResult> CreateNewRepository(Dgn::DgnDbPtr db, Utf8StringCR repositoryId, Utf8StringCR description,
    bool publish = true, HttpRequest::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr);

    //! Download a briefcase of a repository from the server.
    //! @param[in] repositoryId Id of the repository whose briefcase should be aquired.
    //! @param[in] localPath Path where the file will be downloaded.
    //! @param[in] doSync If true will download all of the revisions that have not been merged on server and merge locally.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has downloaded file name as the result.
    //! @note If localPath is an existing directory, the resulting file name will be localPath/RepositoryIdBriefcaseId/FileName
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnDbFileNameResult> AquireBriefcase(Utf8StringCR repositoryId, BeFileNameCR localPath, bool doSync = true,
    HttpRequest::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr);

    //! Create a DgnDbBriefcase instance from a previously downloaded DgnDb file.
    //! @param[in] db Previously downloaded briefcase file.
    //! @param[in] doSync If true will download all of the revisions that have not been merged on server and merge locally.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @see AquireBriefcase
    //! @note This method uses serverUrl from local values in the DgnDb that was written during AquireBriefcase instead of the value set in client.
    DGNDBSERVERCLIENT_EXPORT AsyncTaskPtr<DgnDbBriefcaseResult> OpenBriefcase(Dgn::DgnDbPtr db, bool doSync = false,
    HttpRequest::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr);

#ifdef NEEDSWORK_LOCKS
    DGNDBSERVERCLIENT_EXPORT Dgn::ILocksServer* GetLocksServerP();
#endif // NEEDSWORK_LOCKS
};
END_BENTLEY_DGNDBSERVER_NAMESPACE
