/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbClient.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
#include <DgnDbServer/Client/DgnDbRepositoryAdmin.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_DGN

typedef std::shared_ptr<struct DgnDbClient> DgnDbClientPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(DgnDbClient);
DEFINE_TASK_TYPEDEFS(bvector<RepositoryInfoPtr>, DgnDbServerRepositories);
DEFINE_TASK_TYPEDEFS(DgnDbBriefcasePtr, DgnDbServerBriefcase);
DEFINE_TASK_TYPEDEFS(DgnDbRepositoryManagerPtr, DgnDbRepositoryManager);

typedef std::function<BeFileName(BeFileName, BeSQLite::BeBriefcaseId, RepositoryInfoCR, FileInfoCR)> BriefcaseFileNameCallback;

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
    Utf8String                  m_serverUrl;
    Credentials                 m_credentials;
    Utf8String                  m_projectId;
    ClientInfoPtr               m_clientInfo;
    IHttpHandlerPtr             m_customHandler;
    DgnDbRepositoryAdmin        m_repositoryAdmin;

    DgnDbClient(ClientInfoPtr clientInfo, IHttpHandlerPtr customHandler);

    DgnDbServerRepositoryTaskPtr InitializeRepository(IWSRepositoryClientPtr client, Utf8StringCR repositoryId, Json::Value repositoryCreationJson,
                                                                 ObjectId repositoryObjectId, Http::Request::ProgressCallbackCR callback = nullptr,
                                                                 ICancellationTokenPtr cancellationToken = nullptr) const;
    DgnDbServerStatusResult DownloadBriefcase(DgnDbRepositoryConnectionPtr connection, BeFileName filePath, BeSQLite::BeBriefcaseId briefcaseId,
                                                FileInfoCR fileInfo, bool doSync = true, Http::Request::ProgressCallbackCR callback = nullptr,
                                                ICancellationTokenPtr cancellationToken = nullptr) const;
    DgnDbServerRepositoryTaskPtr CreateRepositoryInstance(Utf8StringCR repositoryName, Utf8StringCR description,
                                                      ICancellationTokenPtr cancellationToken) const;
    DgnDbRepositoryConnectionResult CreateRepositoryConnection(RepositoryInfoCR repositoryInfo) const;
    DgnDbServerRepositoryTaskPtr GetRepositoryFromQuery(WSQueryCR query, ICancellationTokenPtr cancellationToken = nullptr) const;

public:
    //! Set custom handler.
    //! @param[in] customHandler
    DGNDBSERVERCLIENT_EXPORT void SetHttpHandler(IHttpHandlerPtr customHandler);

    //! Get custom handler.
    //! @return Returns HttpHandler
    DGNDBSERVERCLIENT_EXPORT IHttpHandlerPtr GetHttpHandler();

//__PUBLISH_SECTION_START__
public:
    DGNDBSERVERCLIENT_EXPORT static BriefcaseFileNameCallback                       DefaultFileNameCallback;

    //! Create new user. Use just for basic authorisation
    //! @param[in] credentials of the new user
    //! @param[in] cancellationToken
    //! @return Returns a Asynchronous task, which indicates succes or error of the method
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr CreateBasicUser(Credentials credentials, ICancellationTokenPtr cancellationToken = nullptr);

    //! Delete a user. Use just for basic authorisation
    //! @param[in] credentials of the new user
    //! @param[in] cancellationToken
    //! @return Returns a Asynchronous task, which indicates succes or error of the method
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr RemoveBasicUser(Credentials credentials, ICancellationTokenPtr cancellationToken = nullptr);

    //! Create an instance of the client.
    //! @param[in] clientInfo Application information sent to server.
    //! @param[in] customHandler Http handler for connect authentication.
    //! @return Returns a shared pointer to the created instance.
    DGNDBSERVERCLIENT_EXPORT static DgnDbClientPtr Create(ClientInfoPtr clientInfo, IHttpHandlerPtr customHandler = nullptr);

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
    DGNDBSERVERCLIENT_EXPORT DgnDbRepositoryConnectionTaskPtr ConnectToRepository(Utf8StringCR repositoryId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get list of available repostiories for this client.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has a collection of repository information as the result. See RepositoryInfo.
    //! @note Does not return uninitialized repositories or repositories that the user does not have authorization to access.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerRepositoriesTaskPtr GetRepositories(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Gets repository with the specified name.
    //! @param[in] repositoryName
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has repository information as the result. See RepositoryInfo.
    //! @note Does not return uninitialized repositories or repositories that the user does not have authorization to access.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerRepositoryTaskPtr GetRepositoryByName(Utf8StringCR repositoryName, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Gets repository with the specified id.
    //! @param[in] repositoryId
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has repository information as the result. See RepositoryInfo.
    //! @note Does not return uninitialized repositories or repositories that the user does not have authorization to access.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerRepositoryTaskPtr GetRepositoryById(Utf8StringCR repositoryId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Create a new repository on the server.
    //! @param[in] db A DgnDb file to upload as a master file for the repository.
    //! @param[in] waitForInitialized Wait for initialized repository.
    //! @param[in] callback Progress callback for the file upload.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has created repository information as the result. See RepositoryInfo.
    //! @note This method uses name and description properties from dgn_Proj namespace as repository name and description. If name property is not set, it will use the filename instead.
    //! @note Returned repository Id might be different from the user supplied repository name.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerRepositoryTaskPtr CreateNewRepository(DgnDbCR db, bool waitForInitialized = true, Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Create a new repository on the server.
    //! @param[in] db A DgnDb file to upload as a master file for the repository.
    //! @param[in] repositoryName Explicit repository name.
    //! @param[in] description Explicit description of the repository.
    //! @param[in] waitForInitialized Wait for initialized repository.
    //! @param[in] callback Progress callback for the file upload.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has created repository information as the result. See RepositoryInfo.
    //! @note CreateNewRepository without repositoryName and descriptons arguments should be used instead, to resolve name and description from the dgndb file.
    //! @note Returned repository Id might be different from the user supplied repository name.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerRepositoryTaskPtr CreateNewRepository(DgnDbCR db, Utf8StringCR repositoryName, Utf8StringCR description, bool waitForInitialized = true,
                                                                                     Http::Request::ProgressCallbackCR  callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Delete a repository from server
    //! @param[in] repositoryInfo Information of repository to be deleted. This value should be returned by the server. See DgnDbClient::GetRepositories and DgnDbClient::CreateNewRepository.
    //! @param[in] cancellationToken Cancellation is not going to prevent repository deletion, if the request is already sent.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr DeleteRepository(RepositoryInfoCR repositoryInfo, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download a briefcase of a repository from the server.
    //! @param[in] repositoryInfo Information of repository to be acquired. This value should be returned by the server. See DgnDbClient::GetRepositories and DgnDbClient::CreateNewRepository.
    //! @param[in] localFileName Path to a local file including filename and extension, where briefcase should be saved.
    //! @param[in] doSync If set to true, it will download all of the revisions that have not been merged on server and merge locally.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has briefcase info as the result.
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
    //! @return Asynchronous task that has briefcase info as the result.
    //! @note Default callback will save repository at baseDirectory\\repositoryId\\briefcaseId\\fileName
    DGNDBSERVERCLIENT_EXPORT DgnDbServerBriefcaseInfoTaskPtr AcquireBriefcaseToDir(RepositoryInfoCR repositoryInfo, BeFileNameCR baseDirectory, bool doSync = true,
                                                                                     BriefcaseFileNameCallback const& fileNameCallback = DefaultFileNameCallback, 
                                                                                     Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;
   
    //! Abandon a briefcase. It will abandon a briefcase and release all locks and codes associated to it. Make sure you delete briefcase BIM file after calling this.
    //! @param[in] repositoryInfo Information of repository to connect to. This value should be returned by the server. See DgnDbClient::GetRepositories and DgnDbClient::CreateNewRepository.
    //! @param[in] briefcaseId id that should be abandoned.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that returns error if abandoning briefcase fails.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr AbandonBriefcase(RepositoryInfoCR repositoryInfo, BeSQLite::BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Create a DgnDbBriefcase instance from a previously downloaded DgnDb file.
    //! @param[in] db Previously downloaded briefcase file. See DgnDbClient::AcquireBriefcase.
    //! @param[in] doSync If set to true, it will download all of the incomming revisions and merge locally.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has briefcase instance as result. See DgnDbBriefcase.
    //! @note This method ignores the server url set in client and uses server url read from the briefcase file.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerBriefcaseTaskPtr OpenBriefcase(DgnDbPtr db, bool doSync = false, Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Redownload briefcase file
    //! @param[in] db Previously downloaded briefcase file. See DgnDbClient::AcquireBriefcase.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that returns error if download has failed. See DgnDbBriefcase.
    //! @note Should be used if briefcase file has became invalid.
    DGNDBSERVERCLIENT_EXPORT DgnDbServerStatusTaskPtr RecoverBriefcase(DgnDbPtr db, Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns repository admin that caches DgnDbRepositoryManager instances.
    //! @return Returns pointer to DgnDbClient managed repository admin.
    DGNDBSERVERCLIENT_EXPORT DgnPlatformLib::Host::RepositoryAdmin* GetRepositoryAdmin();

    //! Creates repository manager that is not managed by DgnDbClient.
    //! @param[in] repositoryInfo
    //! @param[in] fileInfo
    //! @param[in] briefcaseInfo
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has briefcase manager as result. See DgnDbRepositoryManager.
    //! @note Should use DgnDbRepositoryAdmin provided by DgnDbClient::GetRepositoryAdmin.
    DGNDBSERVERCLIENT_EXPORT DgnDbRepositoryManagerTaskPtr CreateRepositoryManager(RepositoryInfoCR repositoryInfo, FileInfoCR fileInfo, DgnDbServerBriefcaseInfoCR briefcaseInfo,
        ICancellationTokenPtr cancellationToken = nullptr);
};

END_BENTLEY_DGNDBSERVER_NAMESPACE
