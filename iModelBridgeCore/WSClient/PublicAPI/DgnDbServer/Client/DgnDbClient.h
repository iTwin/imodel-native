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
#include <DgnDbServer/Client/FileInfo.h>
#include <DgnDbServer/Client/RepositoryInfo.h>
#include <DgnDbServer/Client/DgnDbRepositoryConnection.h>
#include <DgnDbServer/Client/DgnDbBriefcase.h>
#include <DgnDbServer/Client/DgnDbRepositoryManager.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGNPLATFORM

typedef std::shared_ptr<struct DgnDbClient> DgnDbClientPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(DgnDbClient);
struct DgnDbServerBriefcaseInfo;
DEFINE_TASK_TYPEDEFS(RepositoryInfoPtr, DgnDbServerRepository);
DEFINE_TASK_TYPEDEFS(bvector<RepositoryInfoPtr>, DgnDbServerRepositories);
DEFINE_TASK_TYPEDEFS(DgnDbBriefcasePtr, DgnDbServerBriefcase);
DEFINE_TASK_TYPEDEFS(DgnDbServerBriefcaseInfo, DgnDbServerBriefcaseInfo);

typedef std::function<BeFileName(BeFileName, BeSQLite::BeBriefcaseId, RepositoryInfoCR, FileInfoCR)> BriefcaseFileNameCallback;

//=======================================================================================
//! DgnDbServerBriefcaseInfo results.
//@bsiclass                                       Andrius.Zonys                  06/2016
//=======================================================================================
struct DgnDbServerBriefcaseInfo
{
//__PUBLISH_SECTION_END__
private:
    BeFileName  m_filePath;
    bool        m_isReadOnly;

public:
    DgnDbServerBriefcaseInfo() {};
    DgnDbServerBriefcaseInfo(BeFileName filePath, bool isReadOnly);

//__PUBLISH_SECTION_START__
public:
    //! Returns the briefcase file path.
    DGNDBSERVERCLIENT_EXPORT BeFileName FilePath() const;

    //! Returns true if briefcase can't be modified.
    DGNDBSERVERCLIENT_EXPORT bool       IsReadOnly() const;
};

//=======================================================================================
//! Client of DgnDbServer.
//! This class provides the interface for the operations with repository files.
//! Client class does not maintain a connection to server itself and just provides
//! the required connection information to other DgnDbServer Client classes.
// @bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct DgnDbClient
{
//__PUBLISH_SECTION_END__
private:
    DgnDbRepositoryManagerPtr   m_repositoryManager;
    Utf8String                  m_serverUrl;
    Credentials                 m_credentials;
    Utf8String                  m_projectId;
    ClientInfoPtr               m_clientInfo;
    AuthenticationHandlerPtr    m_authenticationHandler;

    DgnDbClient(ClientInfoPtr clientInfo, AuthenticationHandlerPtr authenticationHandler);

    DgnDbServerRepositoryTaskPtr InitializeRepository(IWSRepositoryClientPtr client, Utf8StringCR repositoryId, Json::Value repositoryCreationJson,
                                                                 ObjectId repositoryObjectId, Http::Request::ProgressCallbackCR callback = nullptr,
                                                                 ICancellationTokenPtr cancellationToken = nullptr) const;
    DgnDbServerStatusTaskPtr DownloadBriefcase(DgnDbRepositoryConnectionPtr connection, BeFileName filePath, BeSQLite::BeBriefcaseId briefcaseId,
                                               FileInfoCR fileInfo, bool doSync = true, Http::Request::ProgressCallbackCR callback = nullptr,
                                               ICancellationTokenPtr cancellationToken = nullptr) const;
    DgnDbServerRepositoryTaskPtr CreateRepositoryInstance(Utf8StringCR repositoryName, Utf8StringCR description, bool published,
                                                      ICancellationTokenPtr cancellationToken) const;

//__PUBLISH_SECTION_START__
public:
    DGNDBSERVERCLIENT_EXPORT static BriefcaseFileNameCallback                       DefaultFileNameCallback;

    //! Set up the DgnDbServer Client library. Initialization is required for most of the library functions to work.
    //! Needs to be called from the work thread where DgnPlatform is initialized.
    DGNDBSERVERCLIENT_EXPORT static void Initialize();

    //! Create an instance of the client.
    //! @param[in] clientInfo Application information sent to server.
    //! @param[in] authenticationHandler Http handler for connect authentication.
    //! @return Returns a shared pointer to the created instance.
    DGNDBSERVERCLIENT_EXPORT static DgnDbClientPtr Create(ClientInfoPtr clientInfo, AuthenticationHandlerPtr authenticationHandler = nullptr);

    //! Address of the server.
    DGNDBSERVERCLIENT_EXPORT void SetServerURL (Utf8StringCR serverUrl);

    //! Credentials used to authenticate to the on-premise server.
    DGNDBSERVERCLIENT_EXPORT void SetCredentials(CredentialsCR credentials);

    //! ProjectId to bind repositories to. Required for IMS authentication and should be empty for Basic.
    DGNDBSERVERCLIENT_EXPORT void SetProject(Utf8StringCR projectId);

    //! Creates a connection to a repository. Use this method if you need to access repository information without acquirying a briefcase.
    //! If you already have a briefcase, please use DgnDbBriefcase.GetRepositoryConnection()
    DGNDBSERVERCLIENT_EXPORT DgnDbRepositoryConnectionTaskPtr ConnectToRepository(RepositoryInfoCR repository, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Creates a connection to a repository. Use this method if you need to access repository information without acquirying a briefcase.
    //! If you already have a briefcase, please use DgnDbBriefcase.GetRepositoryConnection()
    DGNDBSERVERCLIENT_EXPORT DgnDbRepositoryConnectionTaskPtr  ConnectToRepository(Utf8StringCR repositoryId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get list of available repostiories for this client.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has a collection of repository information as the result. See RepositoryInfo.
    //! @note Does not return unpublished, uninitialized repositories or repositories that the user does not have authorization to access.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerRepositoriesTaskPtr GetRepositories(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Create a new repository on the server.
    //! @param[in] db A DgnDb file to upload as a master file for the repository.
    //! @param[in] publish Sets the respository as published (unpublished repositories cannot be accessed).
    //! @param[in] callback Progress callback for the file upload.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has created repository information as the result. See RepositoryInfo.
    //! @note This method uses name and description properties from dgn_Proj namespace as repository name and description. If name property is not set, it will use the filename instead.
    //! @note Returned repository Id might be different from the user supplied repository name.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerRepositoryTaskPtr CreateNewRepository(DgnDbCR db, bool publish = true, Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Create a new repository on the server.
    //! @param[in] db A DgnDb file to upload as a master file for the repository.
    //! @param[in] repositoryName Explicit repository name.
    //! @param[in] description Explicit description of the repository.
    //! @param[in] publish Sets the respository as published (unpublished repositories cannot be accessed).
    //! @param[in] callback Progress callback for the file upload.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has created repository information as the result. See RepositoryInfo.
    //! @note CreateNewRepository without repositoryName and descriptons arguments should be used instead, to resolve name and description from the dgndb file.
    //! @note Returned repository Id might be different from the user supplied repository name.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerRepositoryTaskPtr CreateNewRepository(DgnDbCR db, Utf8StringCR repositoryName, Utf8StringCR description, bool publish = true,
                                                                                     Http::Request::ProgressCallbackCR  callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download a briefcase of a repository from the server.
    //! @param[in] repositoryInfo Information of repository to be acquired. This value should be returned by the server. See DgnDbClient::GetRepositories and DgnDbClient::CreateNewRepository.
    //! @param[in] localFileName Path to a local file including filename and extension, where briefcase should be saved.
    //! @param[in] doSync If set to true, it will download all of the revisions that have not been merged on server and merge locally.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has downloaded file name and is read-only flag as the result.
    //! @note If localFileName is an existing file, this method will fail. If it is an existing directory, file name retrieved from server will be appended.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerBriefcaseInfoTaskPtr AcquireBriefcase(RepositoryInfoCR repositoryInfo, BeFileNameCR localFileName, bool doSync = true,
                                                                                     Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download a briefcase of a repository from the server using callback to provide the file path.
    //! @param[in] repositoryInfo Information of repository to be acquired. This value should be returned by the server. See DgnDbClient::GetRepositories and DgnDbClient::CreateNewRepository.
    //! @param[in] baseDirectory Path to a directory that will be used in callback function.
    //! @param[in] doSync If set to true, it will download all of the revisions that have not been merged on server and merge locally.
    //! @param[in] fileNameCallback Callback function, that takes baseDirectory, briefcase Id and repository info as arguments and returns full filename.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has downloaded file name and is read-only flag as the result.
    //! @note Default callback will save repository at baseDirectory\\repositoryId\\briefcaseId\\fileName
    DGNDBSERVERCLIENT_EXPORT DgnDbServerBriefcaseInfoTaskPtr AcquireBriefcaseToDir(RepositoryInfoCR repositoryInfo, BeFileNameCR baseDirectory, bool doSync = true,
                                                                                     BriefcaseFileNameCallback const& fileNameCallback = DefaultFileNameCallback, 
                                                                                     Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;
   
    //! Create a DgnDbBriefcase instance from a previously downloaded DgnDb file.
    //! @param[in] db Previously downloaded briefcase file. See DgnDbClient::AcquireBriefcase.
    //! @param[in] doSync If set to true, it will download all of the incomming revisions and merge locally.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has briefcase instance as result. See DgnDbBriefcase.
    //! @note This method ignores the server url set in client and uses server url read from the briefcase file.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerBriefcaseTaskPtr OpenBriefcase(DgnDbPtr db, bool doSync = false, Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Delete a repository from server
    //! @param[in] repositoryInfo Information of repository to be deleted. This value should be returned by the server. See DgnDbClient::GetRepositories and DgnDbClient::CreateNewRepository.
    //! @param[in] cancellationToken Cancellation is not going to prevent repository deletion, if the request is already sent.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr DeleteRepository(RepositoryInfoCR repositoryInfo, ICancellationTokenPtr cancellationToken = nullptr);

    //! Returns DgnDbServer RepositoryManager.
    DGNDBSERVERCLIENT_EXPORT IRepositoryManager* GetRepositoryManagerP();
};

END_BENTLEY_DGNDBSERVER_NAMESPACE
