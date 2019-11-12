/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/Client/WSClient.h>
#include <WebServices/iModelHub/Client/FileInfo.h>
#include <WebServices/iModelHub/Client/iModelInfo.h>
#include <WebServices/iModelHub/Client/iModelConnection.h>
#include <WebServices/iModelHub/GlobalEvents/GlobalConnection.h>
#include <WebServices/iModelHub/Client/Briefcase.h>
#include <WebServices/iModelHub/Client/iModelManager.h>
#include <WebServices/iModelHub/Client/iModelAdmin.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <DgnPlatform/DgnDomain.h>
#include <WebServices/iModelHub/Client/GlobalRequestOptions.h>
#include <WebServices/iModelHub/Client/iModelCreateInfo.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

typedef RefCountedPtr<struct Client> ClientPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(Client);
DEFINE_TASK_TYPEDEFS(bvector<iModelInfoPtr>, iModels);
DEFINE_TASK_TYPEDEFS(BriefcasePtr, Briefcase);
DEFINE_TASK_TYPEDEFS(iModelManagerPtr, iModelManager);
DEFINE_TASK_TYPEDEFS(BeFileName, BeFileName);

typedef std::function<BeFileName(BeFileName, iModelInfoCR, BriefcaseInfoCR)> BriefcaseFileNameCallback;
typedef std::function<BeFileName(iModelInfoCR, FileInfoCR)> LocalBriefcaseFileNameCallback;

//=======================================================================================
//! Client of iModelHub.
//! This class provides the interface for the operations with iModel Hub.
//! Client class does not maintain a connection to services itself and just provides
//! the required connection information to other iModel Hub Client classes.
// @bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct Client : RefCountedBase
{
private:
    GlobalRequestOptionsPtr     m_globalRequestOptionsPtr = nullptr;
    Utf8String                  m_serverUrl;
    Credentials                 m_credentials;
    ClientInfoPtr               m_clientInfo;
    IHttpHandlerPtr             m_customHandler;
    iModelAdmin                 m_iModelAdmin;
    GlobalConnectionPtr         m_globalConnectionPtr;

    static StatusResult MergeChangeSetsIntoDgnDb(Dgn::DgnDbPtr db, const ChangeSets changeSets, BeFileNameCR filePath,
                                                 Http::Request::ProgressCallbackCR callback = nullptr,
                                                 ICancellationTokenPtr cancellationToken = nullptr);

    Client(ClientInfoPtr clientInfo, IHttpHandlerPtr customHandler, Utf8StringCR url) : 
        m_globalRequestOptionsPtr(new Hub::GlobalRequestOptions()),
        m_serverUrl(url),
        m_clientInfo(clientInfo),
        m_customHandler(customHandler),
        m_iModelAdmin(this)
        {}

    StatusResult DownloadBriefcase(iModelConnectionPtr connection, BeFileName filePath, BriefcaseInfoCR briefcaseInfo,
                                   bool doSync = true, Http::Request::ProgressCallbackCR callback = nullptr,
                                   ICancellationTokenPtr cancellationToken = nullptr) const;

    iModelTaskPtr CreateiModelInstance(Utf8StringCR contextId, iModelCreateInfoPtr imodelCreateInfo, ICancellationTokenPtr cancellationToken) const;
                                       
    iModelConnectionResult CreateiModelConnection(iModelInfoCR iModelInfo) const
        {
        return iModelConnection::Create(iModelInfo, m_credentials, m_clientInfo, m_globalRequestOptionsPtr, m_customHandler);
        }
    IWSRepositoryClientPtr CreateContextConnection(Utf8StringCR contextId) const;

    BeFileNameTaskPtr DownloadStandaloneBriefcaseInternal(iModelConnectionPtr connection, iModelInfoCR iModelInfo, FileInfoCR fileInfo, 
                                                          bvector<ChangeSetInfoPtr> changeSetsToMerge, 
                                                          LocalBriefcaseFileNameCallback const & fileNameCallBack, 
                                                          Http::Request::ProgressCallback callback, ICancellationTokenPtr cancellationToken) const;

    void SetCredentialsForImodelBank();
    
    iModelTaskPtr GetiModelInternal(Utf8StringCR contextId, WSQuery query, Utf8String methodName, ICancellationTokenPtr cancellationToken = nullptr) const;


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
    Utf8StringCR GetServerUrl() const { return m_serverUrl; }

    //! Returns iModel admin that caches iModelManager instances.
    DgnPlatformLib::Host::RepositoryAdmin* GetiModelAdmin() {return dynamic_cast<DgnPlatformLib::Host::RepositoryAdmin*>(&m_iModelAdmin);}

    IMODELHUBCLIENT_EXPORT static BriefcaseFileNameCallback DefaultFileNameCallback;

    //! Create an instance of the client.
    //! @param[in] clientInfo Application information sent to server.
    //! @param[in] customHandler Http handler for connect authentication.
    //! @param[in] url Optional url for the server.
    //! @return Returns a shared pointer to the created instance.
    IMODELHUBCLIENT_EXPORT static ClientPtr Create(ClientInfoPtr clientInfo, IHttpHandlerPtr customHandler = nullptr, Utf8CP url = nullptr);

    //! Creates a connection to a iModel. Use this method if you need to access iModel information without acquirying a briefcase.
    //! If you already have a briefcase, please use Briefcase.GetiModelConnection()
    IMODELHUBCLIENT_EXPORT iModelConnectionTaskPtr ConnectToiModel(iModelInfoCR iModel, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Creates a connection to a iModel. Use this method if you need to access iModel information without acquirying a briefcase.
    //! If you already have a briefcase, please use Briefcase.GetiModelConnection()
    IMODELHUBCLIENT_EXPORT iModelConnectionTaskPtr ConnectToiModel(Utf8StringCR contextId, Utf8StringCR iModelId, ICancellationTokenPtr 
                                                                   cancellationToken = nullptr) const;

    //! Get list of available repostiories for this client.
    //! @param[in] contextId Context Id to connect to.
    //! @param[in] cancellationToken
    //! @param[in] filterInitialized If set to true, returns only iModels that have been initialized
    //! @return Asynchronous task that has a collection of iModel information as the result. See iModelInfo.
    //! @note Does not return uninitialized iModels or iModels that the user does not have authorization to access.
    IMODELHUBCLIENT_EXPORT iModelsTaskPtr GetiModels(Utf8StringCR contextId, ICancellationTokenPtr cancellationToken = nullptr, bool filterInitialized = false) const;

    //! Gets iModel with the specified name.
    //! @param[in] contextId Context Id to connect to.
    //! @param[in] iModelName
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has iModel information as the result. See iModelInfo.
    //! @note Does not return uninitialized iModels or iModels that the user does not have authorization to access.
    IMODELHUBCLIENT_EXPORT iModelTaskPtr GetiModelByName(Utf8StringCR contextId, Utf8StringCR iModelName, ICancellationTokenPtr
                                                         cancellationToken = nullptr) const;

    //! Gets iModel with the specified id.
    //! @param[in] contextId Context Id to connect to.
    //! @param[in] iModelId
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has iModel information as the result. See iModelInfo.
    //! @note Does not return uninitialized iModels or iModels that the user does not have authorization to access.
    IMODELHUBCLIENT_EXPORT iModelTaskPtr GetiModelById(Utf8StringCR contextId, Utf8StringCR iModelId,
                                                       ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Gets primary iModel.
    //! @param[in] contextId Context Id to connect to.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has iModel information as the result. See iModelInfo.
    IMODELHUBCLIENT_EXPORT iModelTaskPtr GetiModel(Utf8StringCR contextId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Gets the most latest Thumbnail image of an iModel.
    //! @param[in] contextId Context Id to connect to.
    //! @param[in] imodelId
    //! @param[in] size
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has ByteStream of Thumbnail image as the result.
    IMODELHUBCLIENT_EXPORT ThumbnailImageTaskPtr GetiModelThumbnail(Utf8StringCR contextId, Utf8StringCR imodelId, Thumbnail::Size size,
                                                                    ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Create a new iModel on the server.
    //! @param[in] contextId Context Id to connect to.
    //! @param[in] db A DgnDb file to upload as a seed file for the iModel.
    //! @param[in] waitForInitialized Wait for initialized iModel.
    //! @param[in] callback Progress callback for the file upload.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has created iModel information as the result. See iModelInfo.
    //! @note This method uses name and description properties from dgn_Proj namespace as iModel name and description. If name property is not set, it will use the filename instead.
    //! @note Returned iModel Id might be different from the user supplied iModel name.
    IMODELHUBCLIENT_EXPORT iModelTaskPtr CreateNewiModel(Utf8StringCR contextId, DgnDbCR db, bool waitForInitialized = true,
                                                         Http::Request::ProgressCallbackCR callback = nullptr, 
                                                         ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Create a new iModel on the server.
    //! @param[in] contextId Context Id to connect to.
    //! @param[in] db A DgnDb file to upload as a seed file for the iModel.
    //! @param[in] iModelName Explicit iModel name.
    //! @param[in] description Explicit description of the iModel.
    //! @param[in] waitForInitialized Wait for initialized iModel.
    //! @param[in] callback Progress callback for the file upload.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has created iModel information as the result. See iModelInfo.
    //! @note CreateNewiModel without iModelName and descriptons arguments should be used instead, to resolve name and description from the dgndb file.
    //! @note Returned iModel Id might be different from the user supplied iModel name.
    IMODELHUBCLIENT_EXPORT iModelTaskPtr CreateNewiModel(Utf8StringCR contextId, DgnDbCR db, Utf8StringCR iModelName, Utf8StringCR description,
                                                         bool waitForInitialized = true, Http::Request::ProgressCallbackCR  callback = nullptr,
                                                         ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Create a new iModel on the server.
    //! @param[in] contextId Context Id to connect to.
    //! @param[in] db A DgnDb file to upload as a seed file for the iModel.
    //! @param[in] imodelCreateInfo Information of iModel to be created
    //! @param[in] waitForInitialized Wait for initialized iModel.
    //! @param[in] callback Progress callback for the file upload.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has created iModel information as the result. See iModelInfo.
    //! @note CreateNewiModel without iModelName and descriptons arguments should be used instead, to resolve name and description from the dgndb file.
    //! @note Returned iModel Id might be different from the user supplied iModel name.
    IMODELHUBCLIENT_EXPORT iModelTaskPtr CreateNewiModel(Utf8StringCR contextId, DgnDbCR db, iModelCreateInfoPtr imodelCreateInfo,
                                                         bool waitForInitialized = true, Http::Request::ProgressCallbackCR  callback = nullptr,
                                                         ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Create an new iModel on the server from empty template.
    //! @param[in] contextId Context Id to connect to.
    //! @param[in] iModelName Explicit iModel name.
    //! @param[in] description Explicit description of the iModel.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has created iModel information as the result. See iModelInfo.
    IMODELHUBCLIENT_EXPORT iModelTaskPtr CreateEmptyiModel(Utf8StringCR contextId, Utf8StringCR iModelName, Utf8StringCR description,
                                                           ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Create an new iModel on the server from empty template.
    //! @param[in] contextId Context Id to connect to.
    //! @param[in] imodelCreateInfo Information of iModel to be created.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has created iModel information as the result. See iModelInfo.
    IMODELHUBCLIENT_EXPORT iModelTaskPtr CreateEmptyiModel(Utf8StringCR contextId, iModelCreateInfoPtr imodelCreateInfo,
                                                           ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Create an new iModel by cloning another iModel as of a specified changeset.
    //! @param[in] contextId Context Id to connect to.
    //! @param[in] sourceiModelId Id of the iModel that should be cloned.
    //! @param[in] sourceChangeSetId Id of the changeset, that should be merged up to for the new seed file. Can be empty to clone just the seed file.
    //! @param[in] imodelCreateInfo Information of iModel to be created.
    //! @param[in] waitForInitialized Wait for initialized iModel.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has created iModel information as the result. See iModelInfo.
    IMODELHUBCLIENT_EXPORT iModelTaskPtr CloneiModel(Utf8StringCR contextId, Utf8StringCR sourceiModelId, Utf8StringCR sourceChangeSetId,
        iModelCreateInfoPtr imodelCreateInfo, bool waitForInitialized = true, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Update exsiting iModel's name and/or description on the server
    //! @param[in] contextId Context Id to connect to.
    //! @param[in] iModelInfo Information of iModel to be updated.
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT StatusTaskPtr UpdateiModel(Utf8StringCR contextId, iModelInfoCR iModelInfo,
                                                      ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Delete a iModel from server
    //! @param[in] contextId Context Id to connect to.
    //! @param[in] iModelInfo Information of iModel to be deleted. This value should be returned by the server. See Client::GetiModels and Client::CreateNewiModel.
    //! @param[in] cancellationToken Cancellation is not going to prevent iModel deletion, if the request is already sent.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr DeleteiModel(Utf8StringCR contextId, iModelInfoCR iModelInfo,
                                                      ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Download a briefcase of a iModel from the server.
    //! @param[in] iModelInfo Information of iModel to be acquired. This value should be returned by the server. See Client::GetiModels and Client::CreateNewiModel.
    //! @param[in] localFileName Path to a local file including filename and extension, where briefcase should be saved.
    //! @param[in] doSync If set to true, it will download all of the changeSets that have not been merged on server and merge locally.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has briefcase info as the result.
    //! @note If localFileName is an existing file, this method will fail. If it is an existing directory, file name retrieved from server will be appended.
    IMODELHUBCLIENT_EXPORT BriefcaseInfoTaskPtr AcquireBriefcase(iModelInfoCR iModelInfo, BeFileNameCR localFileName, bool doSync = true,
                                                                 Http::Request::ProgressCallbackCR callback = nullptr,
                                                                 ICancellationTokenPtr cancellationToken = nullptr) const;

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
                                                                      Http::Request::ProgressCallbackCR callback = nullptr, 
                                                                      ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Abandon a briefcase. It will abandon a briefcase and release all locks and codes associated to it. Make sure you delete briefcase BIM file after calling this.
    //! @param[in] iModelInfo Information of iModel to connect to. This value should be returned by the server. See Client::GetiModels and Client::CreateNewiModel.
    //! @param[in] briefcaseId id that should be abandoned.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that returns error if abandoning briefcase fails.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr AbandonBriefcase(iModelInfoCR iModelInfo, BeSQLite::BeBriefcaseId briefcaseId, 
                                                          ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Create a Briefcase instance from a previously downloaded DgnDb file.
    //! @param[in] db Previously downloaded briefcase file. See Client::AcquireBriefcase.
    //! @param[in] doSync If set to true, it will download all of the incomming changeSets and merge locally.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has briefcase instance as result. See Briefcase.
    //! @note This method ignores the server url set in client and uses server url read from the briefcase file.
    IMODELHUBCLIENT_EXPORT BriefcaseTaskPtr OpenBriefcase(DgnDbPtr db, bool doSync = false, Http::Request::ProgressCallbackCR callback = nullptr, 
                                                          ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Redownload briefcase file if original became invalid.
    //! @param[in] db Previously downloaded briefcase file. See Client::AcquireBriefcase.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that returns error if download has failed. See Briefcase.
    //! @note Should be used if briefcase file has became invalid.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr RecoverBriefcase(DgnDbPtr db, Http::Request::ProgressCallbackCR callback = nullptr, 
                                                          ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Restore a briefcase of an iModel for a briefcase id that was issued previously. This function should be used in cases when briefcase file was deleted,
    //! but briefcase id remained for future use. For example: briefcase was deleted to free up the space temporarilly.
    //! @param iModelInfo Information of iModel to be acquired. This value should be returned by the server. See Client::GetiModels and Client::CreateNewiModel.
    //! @param briefcaseId Previously Acquired Briefcase Id
    //! @param baseDirectory Path to a directory that will be used in callback function.
    //! @param doSync If set to true, it will download all of the changeSets that have not been merged on server and merge locally.
    //! @param fileNameCallback Callback function, that takes baseDirectory, briefcase Id and iModel info as arguments and returns full filename.
    //! @param callback Download progress callback.
    //! @param cancellationToken
    //! @return Asynchronous task that has briefcase info as the result.
    //! @note Default callback will save iModel at baseDirectory\\iModelId\\briefcaseId\\fileName
    IMODELHUBCLIENT_EXPORT BriefcaseInfoTaskPtr RestoreBriefcase(iModelInfoCR iModelInfo, BeSQLite::BeBriefcaseId briefcaseId, 
                                                                 BeFileNameCR baseDirectory, bool doSync = true, 
                                                                 BriefcaseFileNameCallback const& fileNameCallback = DefaultFileNameCallback, 
                                                                 Http::Request::ProgressCallbackCR callback = nullptr, 
                                                                 ICancellationTokenPtr cancellationToken = nullptr) const;

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
    //! @param[in] filePath Path to DgnDb that schema changes should be added or removed.
    //! @param[in] changeSets ChangeSets to add or remove.
    //! @param[in] processOption Option to control the ChangeSet upgrades
    //! @return Returns a shared pointer to opened DgnDb instance.
    IMODELHUBCLIENT_EXPORT static DgnDbPtr OpenWithSchemaUpgrade(BeSQLite::DbResult* status, BeFileName filePath, ChangeSets changeSets,
                                                                 RevisionProcessOption processOption = RevisionProcessOption::Merge);


    //! Dowloads local briefcase updated to specified version. This briefcase can be used as standalone file and cannot be opened with function OpenBriefcase
    //! @param[in] iModelInfo Information of iModel to be acquired. This value should be returned by the server. See Client::GetiModels and Client::CreateNewiModel.
    //! @param[in] versionId version id briefcase should be updated to.
    //! @param[in] fileNameCallBack Callback function, that takes and iModel info and file info as arguments and returns full filename.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has breifcase file path as the result.
    IMODELHUBCLIENT_EXPORT BeFileNameTaskPtr DownloadStandaloneBriefcaseUpdatedToVersion(iModelInfoCR iModelInfo, Utf8String versionId, 
                                                                                         LocalBriefcaseFileNameCallback const& fileNameCallBack,
                                                                                         Http::Request::ProgressCallbackCR callback = nullptr, 
                                                                                         ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Dowloads local briefcase updated to specified ChangeSet. This briefcase can be used as standalone file and cannot be opened with function OpenBriefcase
    //! @param[in] iModelInfo Information of iModel to be acquired. This value should be returned by the server. See Client::GetiModels and Client::CreateNewiModel.
    //! @param[in] changeSetId Information about versions briefcase should contain.
    //! @param[in] fileNameCallBack Callback function, that takes and iModel info and file info as arguments and returns full filename.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has breifcase file path as the result.
    IMODELHUBCLIENT_EXPORT BeFileNameTaskPtr DownloadStandaloneBriefcaseUpdatedToChangeSet
    (
    iModelInfoCR iModelInfo, Utf8String changeSetId,
    LocalBriefcaseFileNameCallback const& fileNameCallBack,
    Http::Request::ProgressCallbackCR callback = nullptr, 
    ICancellationTokenPtr cancellationToken = nullptr
    ) const;

    //! Dowloads local briefcase from server with version applied. This briefcase can be used as standalone file and cannot be opened with function OpenBriefcase
    //! @param[in] iModelInfo Information of iModel to be acquired. This value should be returned by the server. See Client::GetiModels and Client::CreateNewiModel.
    //! @param[in] fileNameCallBack Callback function, that takes and iModel info and file info as arguments and returns full filename.
    //! @param[in] callback Download progress callback.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has briefcase info as the result.
    IMODELHUBCLIENT_EXPORT BeFileNameTaskPtr DownloadStandaloneBriefcase(iModelInfoCR iModelInfo, 
                                                                         LocalBriefcaseFileNameCallback const& fileNameCallBack, 
                                                                         Http::Request::ProgressCallbackCR callback = nullptr, 
                                                                         ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Provides connection to Global events.
    //! @return Asynchronous task that has Global events connection as the result.
    IMODELHUBCLIENT_EXPORT GlobalConnectionTaskPtr GlobalConnection();

    //! Provides interface to set custom request options.
    //! @return Request options object.
    IMODELHUBCLIENT_EXPORT GlobalRequestOptionsPtr GlobalRequestOptions() const;

};

END_BENTLEY_IMODELHUB_NAMESPACE
