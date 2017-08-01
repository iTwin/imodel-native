/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Client/Client.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/Client/WSClient.h>
#include <WebServices/iModelHub/Client/FileInfo.h>
#include <WebServices/iModelHub/Client/iModelInfo.h>
#include <WebServices/iModelHub/Client/iModelConnection.h>
#include <WebServices/iModelHub/Client/Briefcase.h>
#include <WebServices/iModelHub/Client/iModelManager.h>
#include <WebServices/iModelHub/Client/iModelAdmin.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

typedef RefCountedPtr<struct Client> ClientPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(Client);
DEFINE_TASK_TYPEDEFS(bvector<iModelInfoPtr>, iModels);
DEFINE_TASK_TYPEDEFS(BriefcasePtr, Briefcase);
DEFINE_TASK_TYPEDEFS(iModelManagerPtr, iModelManager);

typedef std::function<BeFileName(BeFileName, BeSQLite::BeBriefcaseId, iModelInfoCR, FileInfoCR)> BriefcaseFileNameCallback;

//=======================================================================================
//! Client of DgnDbServer.
//! This class provides the interface for the operations with iModel hubs.
//! Client class does not maintain a connection to services itself and just provides
//! the required connection information to other iModel Hub Client classes.
// @bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct Client : RefCountedBase
{
private:
    Utf8String                  m_serverUrl;
    Credentials                 m_credentials;
    Utf8String                  m_projectId;
    ClientInfoPtr               m_clientInfo;
    IHttpHandlerPtr             m_customHandler;
    iModelAdmin                 m_iModelAdmin;

    Client(ClientInfoPtr clientInfo, IHttpHandlerPtr customHandler) : 
        m_clientInfo(clientInfo), m_customHandler(customHandler), m_projectId(""), m_iModelAdmin(this) {}

    StatusResult DownloadBriefcase(iModelConnectionPtr connection, BeFileName filePath, BeSQLite::BeBriefcaseId briefcaseId,
                                                FileInfoCR fileInfo, bool doSync = true, Http::Request::ProgressCallbackCR callback = nullptr,
                                                ICancellationTokenPtr cancellationToken = nullptr) const;
    iModelTaskPtr CreateiModelInstance(Utf8StringCR iModelName, Utf8StringCR description,
                                                      ICancellationTokenPtr cancellationToken) const;
    iModelConnectionResult CreateiModelConnection(iModelInfoCR iModelInfo) const { return iModelConnection::Create(iModelInfo, m_credentials, m_clientInfo, m_customHandler); }
    IWSRepositoryClientPtr CreateProjectConnection() const;

public:
    //! Set custom handler.
    //! @param[in] customHandler
    //! @private
    void SetHttpHandler(IHttpHandlerPtr customHandler) {m_customHandler = customHandler;}

    //! Get custom handler.
    //! @return Returns HttpHandler
    //! @private
    IHttpHandlerPtr GetHttpHandler() {return m_customHandler;}

    //! Get the server URL
    //! @private
    Utf8StringCR GetServerUrl() const {return m_serverUrl;}

    //! Get the project ID
    //! @private
    Utf8StringCR GetProjectId() const {return m_projectId;}

    //! Get the client's credentials
    //! @private
    CredentialsCR GetCredentials() const {return m_credentials;}

    //! Address of the server.
    void SetServerURL(Utf8StringCR serverUrl) {m_serverUrl = serverUrl;}

    //! Credentials used to authenticate to the on-premise server.
    void SetCredentials(CredentialsCR credentials) {m_credentials = credentials;}

    //! ProjectId to bind iModels to. Required for IMS authentication and should be empty for Basic.
    void SetProject(Utf8StringCR projectId) {m_projectId = projectId;}

    //! Returns iModel admin that caches iModelManager instances.
    DgnPlatformLib::Host::RepositoryAdmin* GetiModelAdmin() { return dynamic_cast<DgnPlatformLib::Host::RepositoryAdmin*>(&m_iModelAdmin); }

    IMODELHUBCLIENT_EXPORT static BriefcaseFileNameCallback DefaultFileNameCallback;

    //! Create new user. Use just for basic authorisation
    //! @param[in] credentials of the new user
    //! @param[in] cancellationToken
    //! @return Returns a Asynchronous task, which indicates succes or error of the method
    IMODELHUBCLIENT_EXPORT StatusTaskPtr CreateBasicUser(Credentials credentials, ICancellationTokenPtr cancellationToken = nullptr);

    //! Delete a user. Use just for basic authorisation
    //! @param[in] credentials of the new user
    //! @param[in] cancellationToken
    //! @return Returns a Asynchronous task, which indicates succes or error of the method
    IMODELHUBCLIENT_EXPORT StatusTaskPtr RemoveBasicUser(Credentials credentials, ICancellationTokenPtr cancellationToken = nullptr);

    //! Create an instance of the client.
    //! @param[in] clientInfo Application information sent to server.
    //! @param[in] customHandler Http handler for connect authentication.
    //! @return Returns a shared pointer to the created instance.
    IMODELHUBCLIENT_EXPORT static ClientPtr Create(ClientInfoPtr clientInfo, IHttpHandlerPtr customHandler = nullptr);

    //! Creates a connection to a iModel. Use this method if you need to access iModel information without acquirying a briefcase.
    //! If you already have a briefcase, please use Briefcase.GetiModelConnection()
    IMODELHUBCLIENT_EXPORT iModelConnectionTaskPtr ConnectToiModel(iModelInfoCR iModel, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Creates a connection to a iModel. Use this method if you need to access iModel information without acquirying a briefcase.
    //! If you already have a briefcase, please use Briefcase.GetiModelConnection()
    IMODELHUBCLIENT_EXPORT iModelConnectionTaskPtr ConnectToiModel(Utf8StringCR iModelId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get list of available repostiories for this client.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has a collection of iModel information as the result. See iModelInfo.
    //! @note Does not return uninitialized iModels or iModels that the user does not have authorization to access.
    IMODELHUBCLIENT_EXPORT iModelsTaskPtr GetiModels(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Gets iModel with the specified name.
    //! @param[in] iModelName
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has iModel information as the result. See iModelInfo.
    //! @note Does not return uninitialized iModels or iModels that the user does not have authorization to access.
    IMODELHUBCLIENT_EXPORT iModelTaskPtr GetiModelByName(Utf8StringCR iModelName, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Gets iModel with the specified id.
    //! @param[in] iModelId
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has iModel information as the result. See iModelInfo.
    //! @note Does not return uninitialized iModels or iModels that the user does not have authorization to access.
    IMODELHUBCLIENT_EXPORT iModelTaskPtr GetiModelById(Utf8StringCR iModelId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Create a new iModel on the server.
    //! @param[in] db A DgnDb file to upload as a seed file for the iModel.
    //! @param[in] waitForInitialized Wait for initialized iModel.
    //! @param[in] callback Progress callback for the file upload.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has created iModel information as the result. See iModelInfo.
    //! @note This method uses name and description properties from dgn_Proj namespace as iModel name and description. If name property is not set, it will use the filename instead.
    //! @note Returned iModel Id might be different from the user supplied iModel name.
    IMODELHUBCLIENT_EXPORT iModelTaskPtr CreateNewiModel(DgnDbCR db, bool waitForInitialized = true, Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Create a new iModel on the server.
    //! @param[in] db A DgnDb file to upload as a seed file for the iModel.
    //! @param[in] iModelName Explicit iModel name.
    //! @param[in] description Explicit description of the iModel.
    //! @param[in] waitForInitialized Wait for initialized iModel.
    //! @param[in] callback Progress callback for the file upload.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has created iModel information as the result. See iModelInfo.
    //! @note CreateNewiModel without iModelName and descriptons arguments should be used instead, to resolve name and description from the dgndb file.
    //! @note Returned iModel Id might be different from the user supplied iModel name.
    IMODELHUBCLIENT_EXPORT iModelTaskPtr CreateNewiModel(DgnDbCR db, Utf8StringCR iModelName, Utf8StringCR description, bool waitForInitialized = true,
                                                                                     Http::Request::ProgressCallbackCR  callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Delete a iModel from server
    //! @param[in] iModelInfo Information of iModel to be deleted. This value should be returned by the server. See Client::GetiModels and Client::CreateNewiModel.
    //! @param[in] cancellationToken Cancellation is not going to prevent iModel deletion, if the request is already sent.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr DeleteiModel(iModelInfoCR iModelInfo, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download a briefcase of a iModel from the server.
    //! @param[in] iModelInfo Information of iModel to be acquired. This value should be returned by the server. See Client::GetiModels and Client::CreateNewiModel.
    //! @param[in] localFileName Path to a local file including filename and extension, where briefcase should be saved.
    //! @param[in] doSync If set to true, it will download all of the changeSets that have not been merged on server and merge locally.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has briefcase info as the result.
    //! @note If localFileName is an existing file, this method will fail. If it is an existing directory, file name retrieved from server will be appended.
    IMODELHUBCLIENT_EXPORT BriefcaseInfoTaskPtr AcquireBriefcase(iModelInfoCR iModelInfo, BeFileNameCR localFileName, bool doSync = true,
                                                                                     Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download a briefcase of a iModel from the server using callback to provide the file path.
    //! @param[in] iModelInfo Information of iModel to be acquired. This value should be returned by the server. See Client::GetiModels and Client::CreateNewiModel.
    //! @param[in] baseDirectory Path to a directory that will be used in callback function.
    //! @param[in] doSync If set to true, it will download all of the changeSets that have not been merged on server and merge locally.
    //! @param[in] fileNameCallback Callback function, that takes baseDirectory, briefcase Id and iModel info as arguments and returns full filename.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has briefcase info as the result.
    //! @note Default callback will save iModel at baseDirectory\\iModelId\\briefcaseId\\fileName
    IMODELHUBCLIENT_EXPORT BriefcaseInfoTaskPtr AcquireBriefcaseToDir(iModelInfoCR iModelInfo, BeFileNameCR baseDirectory, bool doSync = true,
                                                                                     BriefcaseFileNameCallback const& fileNameCallback = DefaultFileNameCallback, 
                                                                                     Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;
   
    //! Abandon a briefcase. It will abandon a briefcase and release all locks and codes associated to it. Make sure you delete briefcase BIM file after calling this.
    //! @param[in] iModelInfo Information of iModel to connect to. This value should be returned by the server. See Client::GetiModels and Client::CreateNewiModel.
    //! @param[in] briefcaseId id that should be abandoned.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that returns error if abandoning briefcase fails.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr AbandonBriefcase(iModelInfoCR iModelInfo, BeSQLite::BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Create a Briefcase instance from a previously downloaded DgnDb file.
    //! @param[in] db Previously downloaded briefcase file. See Client::AcquireBriefcase.
    //! @param[in] doSync If set to true, it will download all of the incomming changeSets and merge locally.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has briefcase instance as result. See Briefcase.
    //! @note This method ignores the server url set in client and uses server url read from the briefcase file.
    IMODELHUBCLIENT_EXPORT BriefcaseTaskPtr OpenBriefcase(DgnDbPtr db, bool doSync = false, Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Redownload briefcase file
    //! @param[in] db Previously downloaded briefcase file. See Client::AcquireBriefcase.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that returns error if download has failed. See Briefcase.
    //! @note Should be used if briefcase file has became invalid.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr RecoverBriefcase(DgnDbPtr db, Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Creates iModel manager that is not managed by Client.
    //! @param[in] iModelInfo
    //! @param[in] fileInfo
    //! @param[in] briefcaseInfo
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has briefcase manager as result. See iModelManager.
    //! @note Should use iModelAdmin provided by Client::GetiModelAdmin.
    IMODELHUBCLIENT_EXPORT iModelManagerTaskPtr CreateiModelManager(iModelInfoCR iModelInfo, FileInfoCR fileInfo, BriefcaseInfoCR briefcaseInfo,
        ICancellationTokenPtr cancellationToken = nullptr);

    //! Opens iModel with schema upgrade.
    //! @param[out] status BE_SQLITE_OK if the DgnDb file was successfully opened, error code otherwise. May be NULL.
    //! @param[in] filePath Path to DgnDb that schema changes should be merged to.
    //! @param[in] changeSets ChangeSets to merge.
    //! @return Returns a shared pointer to opened DgnDb instance.
    IMODELHUBCLIENT_EXPORT static DgnDbPtr OpenWithSchemaUpgrade(BeSQLite::DbResult* status, BeFileName filePath, ChangeSets changeSets);
};

END_BENTLEY_IMODELHUB_NAMESPACE
