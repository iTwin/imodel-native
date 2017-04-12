/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbRepositoryConnection.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbRepositoryConnection.h>
#include <DgnPlatform/RevisionManager.h>
#include <WebServices/Client/WSChangeset.h>
#include "DgnDbServerUtils.h"
#include <DgnDbServer/Client/Logging.h>
#include <DgnDbServer/Client/DgnDbServerBreakHelper.h>
#include "DgnDbServerEventManager.h"
#include "DgnDbServerPreDownloadManager.h"
#include <DgnDbServer/Client/Events/DgnDbServerRevisionEvent.h>
#include <DgnDbServer/Client/DgnDbServerRevisionInfo.h>

USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGN

# define MAX_AsyncQueries 10
//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
void DgnDbCodeLockSetResultInfo::AddLock(const DgnLock dgnLock, BeBriefcaseId briefcaseId, Utf8StringCR revisionId)
    {
    m_locks.insert(dgnLock);
    AddLockInfoToList(m_lockStates, dgnLock, briefcaseId, revisionId);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
void DgnDbCodeLockSetResultInfo::AddCode(const DgnCode dgnCode, DgnCodeState dgnCodeState, BeBriefcaseId briefcaseId)
    {
    m_codes.insert(dgnCode);
    AddCodeInfoToList(m_codeStates, dgnCode, dgnCodeState, briefcaseId);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   julius.cepukenas              08/2016
//---------------------------------------------------------------------------------------
void DgnDbCodeLockSetResultInfo::Insert
(
const DgnCodeSet& codes,
const DgnCodeInfoSet& codeStates,
const DgnLockSet& locks,
const DgnLockInfoSet& lockStates
)
    {
    m_codes.insert(codes.begin(), codes.end());
    m_codeStates.insert(codeStates.begin(), codeStates.end());
    m_locks.insert(locks.begin(), locks.end());
    m_lockStates.insert(lockStates.begin(), lockStates.end());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   julius.cepukenas              08/2016
//---------------------------------------------------------------------------------------
void DgnDbCodeLockSetResultInfo::Insert(const DgnDbCodeLockSetResultInfo& resultInfo)
    {
    Insert(resultInfo.GetCodes(), resultInfo.GetCodeStates(), resultInfo.GetLocks(), resultInfo.GetLockStates());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas              08/2016
//---------------------------------------------------------------------------------------
bool DgnDbCodeTemplate::operator<(DgnDbCodeTemplate const& rhs) const
    {
    if (GetCodeSpecId().GetValueUnchecked() != rhs.GetCodeSpecId().GetValueUnchecked())
        return GetCodeSpecId().GetValueUnchecked() < rhs.GetCodeSpecId().GetValueUnchecked();

    int cmp = GetValuePattern().CompareTo(rhs.GetValuePattern());
    if (0 != cmp)
        return cmp < 0;

    cmp = GetValue().CompareTo(rhs.GetValue());
    if (0 != cmp)
        return cmp < 0;

    return GetScope().CompareTo(rhs.GetScope()) < 0;
    }

DgnDbServerPreDownloadManagerPtr DgnDbRepositoryConnection::s_preDownloadManager = new DgnDbServerPreDownloadManager();

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbRepositoryConnection::DgnDbRepositoryConnection
(
RepositoryInfoCR           repository,
WebServices::CredentialsCR credentials,
WebServices::ClientInfoPtr clientInfo,
IHttpHandlerPtr            customHandler
) : m_repositoryInfo(repository)
    {
    auto wsRepositoryClient = WSRepositoryClient::Create(repository.GetServerURL(), repository.GetWSRepositoryName(), clientInfo, nullptr, customHandler);
    CompressionOptions options;
    options.EnableRequestCompression(true, 1024);
    wsRepositoryClient->Config().SetCompressionOptions(options);
    wsRepositoryClient->SetCredentials(credentials);
    wsRepositoryClient->GetWSClient()->EnableWsgServerHeader(true);

    m_wsRepositoryClient = wsRepositoryClient;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
DgnDbRepositoryConnection::~DgnDbRepositoryConnection()
    {
    if (m_eventManagerPtr.IsValid())
        m_eventManagerPtr = nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             01/2017
//---------------------------------------------------------------------------------------
void DgnDbRepositoryConnection::SubscribeRevisionsDownload()
    {
    if (m_subscribedForPreDownload)
        return;

    m_subscribedForPreDownload = true;
    s_preDownloadManager->SubscribeRevisionsDownload(this);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             03/2016
//---------------------------------------------------------------------------------------
void DgnDbRepositoryConnection::SetAzureClient(WebServices::IAzureBlobStorageClientPtr azureClient)
    {
    m_azureClient = azureClient;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbRepositoryConnectionResult DgnDbRepositoryConnection::Create
(
RepositoryInfoCR         repository,
CredentialsCR            credentials,
ClientInfoPtr            clientInfo,
IHttpHandlerPtr          customHandler
)
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::Create";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (repository.GetId().empty())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid repository id.");
        return DgnDbRepositoryConnectionResult::Error(DgnDbServerError::Id::InvalidRepositoryId);
        }
    if (repository.GetServerURL().empty())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid server URL.");
        return DgnDbRepositoryConnectionResult::Error(DgnDbServerError::Id::InvalidServerURL);
        }
    if (!credentials.IsValid() && !customHandler)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return DgnDbRepositoryConnectionResult::Error(DgnDbServerError::Id::CredentialsNotSet);
        }

    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    DgnDbRepositoryConnectionPtr repositoryConnection(new DgnDbRepositoryConnection(repository, credentials, clientInfo, customHandler));
    repositoryConnection->SetAzureClient(AzureBlobStorageClient::Create());

    double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
    return DgnDbRepositoryConnectionResult::Success(repositoryConnection);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             09/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::ValidateBriefcase(BeGuidCR fileId, BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken) const
    {
    return QueryBriefcaseInfo(briefcaseId, cancellationToken)->Then<DgnDbServerStatusResult>([=] (DgnDbServerBriefcaseInfoResultCR result)
        {
        if (!result.IsSuccess())
            {
            return DgnDbServerStatusResult::Error(result.GetError());
            }

        auto briefcase = result.GetValue();

        if (briefcase->GetFileId() != fileId)
            {
            return DgnDbServerStatusResult::Error({DgnDbServerError::Id::InvalidBriefcase, DgnDbServerErrorLocalizedString(MESSAGE_BriefcaseOutdated)});
            }

        return DgnDbServerStatusResult::Success();
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
Json::Value CreateFileJson(FileInfoCR fileInfo)
    {
    Json::Value createFileJson = Json::objectValue;
    createFileJson[ServerSchema::Instance] = Json::objectValue;
    createFileJson[ServerSchema::Instance][ServerSchema::SchemaName] = fileInfo.GetObjectId().schemaName;
    createFileJson[ServerSchema::Instance][ServerSchema::ClassName] = fileInfo.GetObjectId().className;
    fileInfo.ToPropertiesJson(createFileJson[ServerSchema::Instance][ServerSchema::Properties]);
    return createFileJson;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerFileTaskPtr DgnDbRepositoryConnection::CreateNewServerFile(FileInfoCR fileInfo, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::CreateNewServerFile";
    std::shared_ptr<DgnDbServerFileResult> finalResult = std::make_shared<DgnDbServerFileResult>();
    return m_wsRepositoryClient->SendCreateObjectRequest(CreateFileJson(fileInfo), BeFileName(), nullptr, cancellationToken)->Then
        ([=] (const WSCreateObjectResult& result)
        {
        if (result.IsSuccess())
            {
            Json::Value json;
            result.GetValue().GetJson(json);
            JsonValueCR instance = json[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
            auto fileInfoPtr = FileInfo::Parse(instance, fileInfo);
            fileInfoPtr->SetFileAccessKey(DgnDbServerFileAccessKey::ParseFromRelated(instance));

            finalResult->SetSuccess(fileInfoPtr);
            return;
            }

        auto error = DgnDbServerError(result.GetError());
        if (DgnDbServerError::Id::FileAlreadyExists != error.GetId())
            {
            finalResult->SetError(error);
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, error.GetMessage().c_str());
            return;
            }

        bool initialized = error.GetExtendedData()[ServerSchema::Property::FileInitialized].asBool();

        if (initialized)
            {
            finalResult->SetError(error);
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, error.GetMessage().c_str());
            return;
            }

        WSQuery fileQuery(ServerSchema::Schema::Repository, ServerSchema::Class::File);
        Utf8String filter;
        filter.Sprintf("(%s+eq+'%s')+and+(%s+eq+'%s')", ServerSchema::Property::FileId, fileInfo.GetFileId().ToString().c_str(),
                       ServerSchema::Property::MergedRevisionId, fileInfo.GetMergedRevisionId().c_str());
        fileQuery.SetFilter(filter);

        Utf8String select("*");
        DgnDbServerFileAccessKey::AddUploadAccessKeySelect(select);
        fileQuery.SetSelect(select);

        m_wsRepositoryClient->SendQueryRequest(fileQuery, nullptr, nullptr, cancellationToken)->Then([=] (WSObjectsResult const& queryResult)
            {
            if (!queryResult.IsSuccess())
                {
                finalResult->SetError(queryResult.GetError());
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, queryResult.GetError().GetMessage().c_str());
                return;
                }
            
            if (queryResult.GetValue().GetRapidJsonDocument().IsNull())
                {
                finalResult->SetError(error);
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, error.GetMessage().c_str());
                return;
                }

            auto resultJson = *queryResult.GetValue().GetInstances().begin();
            auto downloadFileResult = FileInfo::Parse(resultJson, fileInfo);
            downloadFileResult->SetFileAccessKey(DgnDbServerFileAccessKey::ParseFromRelated(resultJson));

            finalResult->SetSuccess(downloadFileResult);
            });

        })->Then<DgnDbServerFileResult>([=] ()
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::UpdateServerFile(FileInfoCR fileInfo, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::UpdateServerFile";
    Json::Value properties = Json::objectValue;
    fileInfo.ToPropertiesJson(properties);
    return m_wsRepositoryClient->SendUpdateObjectRequest(fileInfo.GetObjectId(), properties, nullptr, BeFileName(), nullptr, cancellationToken)->Then<DgnDbServerStatusResult>
    ([=] (const WSUpdateObjectResult& result)
        {
        if (!result.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return DgnDbServerStatusResult::Error(result.GetError());
            }

        return DgnDbServerStatusResult::Success();
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::OnPremiseFileUpload(BeFileNameCR filePath, ObjectIdCR objectId, Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::OnPremiseFileUpload";
    if (objectId.remoteId.empty())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid repository id.");
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::InvalidRepositoryId));
        }

    return m_wsRepositoryClient->SendUpdateFileRequest(objectId, filePath, callback, cancellationToken)
        ->Then<DgnDbServerStatusResult>([=] (const WSUpdateFileResult& uploadFileResult)
        {
        if (!uploadFileResult.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, uploadFileResult.GetError().GetMessage().c_str());
            return DgnDbServerStatusResult::Error(uploadFileResult.GetError());
            }

        return DgnDbServerStatusResult::Success();
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::AzureFileUpload(BeFileNameCR filePath, DgnDbServerFileAccessKeyPtr url, Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::AzureFileUpload";
    return m_azureClient->SendUpdateFileRequest(url->GetUploadUrl(), filePath, callback, cancellationToken)
        ->Then<DgnDbServerStatusResult>([=] (const AzureResult& result)
        {
        if (!result.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return DgnDbServerStatusResult::Error(DgnDbServerError(result.GetError()));
            }

        return DgnDbServerStatusResult::Success();
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::UploadServerFile(BeFileNameCR filePath, FileInfoCR downloadInfo, Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    auto fileAccessKey = downloadInfo.GetFileAccessKey();
    
    if (fileAccessKey.IsNull())
        return OnPremiseFileUpload(filePath, downloadInfo.GetObjectId(), callback, cancellationToken);
    else
        return AzureFileUpload(filePath, fileAccessKey, callback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::InitializeServerFile(FileInfoCR fileInfo, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::InitializeServerFile";
    Json::Value fileProperties;
    fileInfo.ToPropertiesJson(fileProperties);
    fileProperties[ServerSchema::Property::IsUploaded] = true;

    return m_wsRepositoryClient->SendUpdateObjectRequest(fileInfo.GetObjectId(), fileProperties, nullptr, BeFileName(), nullptr, cancellationToken)
        ->Then<DgnDbServerStatusResult>([=] (const WSUpdateObjectResult& initializeFileResult)
        {
        if (!initializeFileResult.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, initializeFileResult.GetError().GetMessage().c_str());
            return DgnDbServerStatusResult::Error(initializeFileResult.GetError());
            }

        return DgnDbServerStatusResult::Success();
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::LockRepository(ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::LockRepository";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    return ExecutionManager::ExecuteWithRetry<void>([=]()
        {
        Json::Value repositoryLockJson = Json::objectValue;
        repositoryLockJson[ServerSchema::Instance] = Json::objectValue;
        repositoryLockJson[ServerSchema::Instance][ServerSchema::SchemaName] = ServerSchema::Schema::Repository;
        repositoryLockJson[ServerSchema::Instance][ServerSchema::ClassName] = ServerSchema::Class::RepositoryLock;

        return m_wsRepositoryClient->SendCreateObjectRequest(repositoryLockJson, BeFileName(), nullptr, cancellationToken)
            ->Then<DgnDbServerStatusResult>([=](const WSCreateObjectResult& result)
            {
            if (!result.IsSuccess())
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                return DgnDbServerStatusResult::Error(result.GetError());
                }

            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
            return DgnDbServerStatusResult::Success();
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::UnlockRepository(ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::LockRepository";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    return ExecutionManager::ExecuteWithRetry<void>([=] ()
        {
        ObjectId id = ObjectId::ObjectId(ServerSchema::Schema::Repository, ServerSchema::Class::RepositoryLock, ServerSchema::Class::RepositoryLock);
        return m_wsRepositoryClient->SendDeleteObjectRequest(id, cancellationToken)
            ->Then<DgnDbServerStatusResult>([=] (const WSDeleteObjectResult& result)
            {
            if (!result.IsSuccess())
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                return DgnDbServerStatusResult::Error(result.GetError());
                }

            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float) (end - start), "");
            return DgnDbServerStatusResult::Success();
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             03/2017
//---------------------------------------------------------------------------------------
bool IsInitializationFinished(InitializationState state)
	{
	return !(InitializationState::NotStarted == state || InitializationState::Scheduled == state);
	}

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
void DgnDbRepositoryConnection::WaitForInitializedBIMFile(BeGuid fileGuid, DgnDbServerFileResultPtr finalResult) const
    {
    InitializationState initializationState = InitializationState::NotStarted;
    int retriesLeft = 300;
    const Utf8String methodName = "DgnDbClient::WaitForInitializedBIMFile";
    BeThreadUtilities::BeSleep(1000);

    while (!IsInitializationFinished(initializationState) && retriesLeft > 0)
        {
        auto masterFilesResult = GetMasterFileById(fileGuid)->GetResult();
        if (!masterFilesResult.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, masterFilesResult.GetError().GetMessage().c_str());
            finalResult->SetError(masterFilesResult.GetError());
            return;
            }

        auto masterFile = masterFilesResult.GetValue();
		initializationState = masterFile->GetInitialized();
        
        if (!IsInitializationFinished(initializationState))
            BeThreadUtilities::BeSleep(1000);
        retriesLeft--;
        }

    if (initializationState != InitializationState::Success)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Repository is not initialized.");
        finalResult->SetError({ DgnDbServerError::Id::RepositoryIsNotInitialized });
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerFileTaskPtr DgnDbRepositoryConnection::UploadNewMasterFile(BeFileNameCR filePath, FileInfoCR fileInfo, bool waitForInitialized, Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::UploadNewMasterFile";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    std::shared_ptr<DgnDbServerFileResult> finalResult = std::make_shared<DgnDbServerFileResult>();
    return ExecutionManager::ExecuteWithRetry<FileInfoPtr>([=]()
        {
        return CreateNewServerFile(fileInfo, cancellationToken)->Then([=] (DgnDbServerFileResultCR fileCreationResult)
            {
#if defined (ENABLE_BIM_CRASH_TESTS)
            DgnDbServerBreakHelper::HitBreakpoint(DgnDbServerBreakpoints::DgnDbRepositoryConnection_AfterCreateNewServerFile);
#endif
            if (!fileCreationResult.IsSuccess())
                {
                finalResult->SetError(fileCreationResult.GetError());
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, fileCreationResult.GetError().GetMessage().c_str());
                return;
                }

            auto createdFileInfo = fileCreationResult.GetValue();
            if (!createdFileInfo->AreFileDetailsAvailable())
                {
                auto fileUpdateResult = UpdateServerFile(*createdFileInfo, cancellationToken)->GetResult();
                if (!fileUpdateResult.IsSuccess())
                    {
                    finalResult->SetError(fileUpdateResult.GetError());
                    DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, fileUpdateResult.GetError().GetMessage().c_str());
                    return;
                    }
                }
            finalResult->SetSuccess(createdFileInfo);

            UploadServerFile(filePath, *createdFileInfo, callback, cancellationToken)->Then([=] (DgnDbServerStatusResultCR uploadResult)
                {
#if defined (ENABLE_BIM_CRASH_TESTS)
                DgnDbServerBreakHelper::HitBreakpoint(DgnDbServerBreakpoints::DgnDbRepositoryConnection_AfterUploadServerFile);
#endif
                if (!uploadResult.IsSuccess() && DgnDbServerError::Id::MissingRequiredProperties != uploadResult.GetError().GetId())
                    {
                    finalResult->SetError(uploadResult.GetError());
                    DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, uploadResult.GetError().GetMessage().c_str());
                    return;
                    }
                InitializeServerFile(*createdFileInfo, cancellationToken)->Then([=] (DgnDbServerStatusResultCR initializationResult)
                    {
                    if (!initializationResult.IsSuccess())
                        {
                        finalResult->SetError(initializationResult.GetError());
                        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, initializationResult.GetError().GetMessage().c_str());
                        return;
                        }

                    if (waitForInitialized)
                        {
                        WaitForInitializedBIMFile(createdFileInfo->GetFileId(), finalResult);
                        }
                    });
                });
            })->Then<DgnDbServerFileResult>([=] ()
                {
                double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
                return *finalResult;
                });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::CancelMasterFileCreation(ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::CancelMasterFileCreation";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::File);
    Utf8String filter;
    filter.Sprintf("%s+gt+0", ServerSchema::Property::InitializationState);
    query.SetFilter(filter);
    std::shared_ptr<DgnDbServerStatusResult> finalResult = std::make_shared<DgnDbServerStatusResult>();
    return m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)->Then([=] (WSObjectsResult const& result)
        {
        if (!result.IsSuccess())
            {
            finalResult->SetError(result.GetError());
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return;
            }
        auto instances = result.GetValue().GetInstances();
        if (instances.IsEmpty())
            {
            finalResult->SetError({DgnDbServerError::Id::FileDoesNotExist, DgnDbServerErrorLocalizedString(MESSAGE_MasterFileNotFound)});
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File does not exist.");
            return;
            }

        auto fileInfo = FileInfo::Parse(*instances.begin(), FileInfo());
        m_wsRepositoryClient->SendDeleteObjectRequest(fileInfo->GetObjectId(), cancellationToken)->Then([=] (WSVoidResult const& deleteResult)
            {
            if (!deleteResult.IsSuccess())
                {
                finalResult->SetError(deleteResult.GetError());
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, deleteResult.GetError().GetMessage().c_str());
                }
            else
                {
                finalResult->SetSuccess();
                double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
                }
            });
        })->Then<DgnDbServerStatusResult>([=] ()
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerFilesTaskPtr DgnDbRepositoryConnection::MasterFilesQuery(WSQuery query, ICancellationTokenPtr cancellationToken) const
    {
    return ExecutionManager::ExecuteWithRetry<bvector<FileInfoPtr>>([=]() {
        return m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)->Then<DgnDbServerFilesResult>([=] (WSObjectsResult const& result)
            {
            if (!result.IsSuccess())
                return DgnDbServerFilesResult::Error(result.GetError());
            bvector<FileInfoPtr> files;
            for (auto const& instance : result.GetValue().GetJsonValue()[ServerSchema::Instances])
                files.push_back(FileInfo::Parse(instance));
            return DgnDbServerFilesResult::Success(files);
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerFilesTaskPtr DgnDbRepositoryConnection::GetMasterFiles(ICancellationTokenPtr cancellationToken) const
    {
    WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::File);
    return MasterFilesQuery(query, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerFileTaskPtr DgnDbRepositoryConnection::GetMasterFileById(BeGuidCR fileId, ICancellationTokenPtr cancellationToken) const
    {

    WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::File);
    Utf8String filter;
    filter.Sprintf("%s+eq+'%s'", ServerSchema::Property::FileId, fileId.ToString().c_str());
    query.SetFilter(filter);

    return MasterFilesQuery(query, cancellationToken)->Then<DgnDbServerFileResult>([=](DgnDbServerFilesResult filesResult)
        {
        if (!filesResult.IsSuccess())
            return DgnDbServerFileResult::Error(filesResult.GetError());

        return DgnDbServerFileResult::Success(*filesResult.GetValue().begin());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusResult DgnDbRepositoryConnection::WriteBriefcaseIdIntoFile
(
BeFileName                     filePath,
BeBriefcaseId                  briefcaseId
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::WriteBriefcaseIdIntoFile";
    BeSQLite::DbResult status;

    Dgn::DgnDbPtr db = Dgn::DgnDb::OpenDgnDb (&status, filePath, Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite));
    if (BeSQLite::DbResult::BE_SQLITE_OK == status && db.IsValid())
        {
        DgnDbServerStatusResult result = m_repositoryInfo.WriteRepositoryInfo (*db, briefcaseId);
#if defined (ENABLE_BIM_CRASH_TESTS)
        DgnDbServerBreakHelper::HitBreakpoint(DgnDbServerBreakpoints::DgnDbRepositoryConnection_AfterWriteRepositoryInfo);
#endif
        db->CloseDb();
        return result;
        }
    else
        {
        auto error = DgnDbServerError(db, status);
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, error.GetMessage().c_str());
        if (db.IsValid())
            db->CloseDb();
        return DgnDbServerStatusResult::Error(error);
        }

    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
DgnDbServerFileAccessKeyTaskPtr DgnDbRepositoryConnection::QueryFileAccessKey
(
ObjectId              objectId,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::QueryFileAccessKey";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    
    WSQuery query(objectId);
    Utf8String selectString = "$id";
    DgnDbServerFileAccessKey::AddDownloadAccessKeySelect(selectString);
    query.SetSelect(selectString);

    return m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)
        ->Then<DgnDbServerFileAccessKeyResult>([=](WSObjectsResult const& result)
        {
        if (!result.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return DgnDbServerFileAccessKeyResult::Error(result.GetError());
            }

        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
        auto fileAccessKey = DgnDbServerFileAccessKey::ParseFromRelated(*result.GetValue().GetInstances().begin());
        return DgnDbServerFileAccessKeyResult::Success(fileAccessKey);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::DownloadFile
(
BeFileName                        localFile,
ObjectIdCR                        fileId,
Http::Request::ProgressCallbackCR callback,
ICancellationTokenPtr             cancellationToken
) const
    {
    auto fileAccessKeyResult = QueryFileAccessKey(fileId, cancellationToken)->GetResult();
    if (!fileAccessKeyResult.IsSuccess())
        return CreateCompletedAsyncTask(DgnDbServerStatusResult::Error(fileAccessKeyResult.GetError()));
    
    auto fileAccessKey = fileAccessKeyResult.GetValue();
    return DownloadFileInternal(localFile, fileId, fileAccessKey, callback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::DownloadMasterFile(BeFileName localFile, Utf8StringCR fileId, Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::DownloadMasterFile";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    ObjectId fileObjectId(ServerSchema::Schema::Repository, ServerSchema::Class::File, fileId);
    return DownloadFile(localFile, fileObjectId, callback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::DownloadFileInternal
(
    BeFileName                        localFile,
    ObjectIdCR                        fileId,
    DgnDbServerFileAccessKeyPtr       fileAccessKey,
    Http::Request::ProgressCallbackCR callback,
    ICancellationTokenPtr             cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::DownloadFileInternal";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    if (fileAccessKey.IsNull())
        {
        return ExecutionManager::ExecuteWithRetry<void>([=]() {
            return m_wsRepositoryClient->SendGetFileRequest(fileId, localFile, nullptr, callback, cancellationToken)
                ->Then<DgnDbServerStatusResult>([=](const WSFileResult& fileResult)
                {
                if (!fileResult.IsSuccess())
                    {
                    DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, fileResult.GetError().GetMessage().c_str());
                    return DgnDbServerStatusResult::Error(fileResult.GetError());
                    }

                double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
                return DgnDbServerStatusResult::Success();
                });
            });
        }
    else
        {
        return ExecutionManager::ExecuteWithRetry<void>([=]() {
            // Download file directly from the url.
            return m_azureClient->SendGetFileRequest(fileAccessKey->GetDownloadUrl(), localFile, callback, cancellationToken)
                ->Then<DgnDbServerStatusResult>([=](const AzureResult& result)
                {
                if (!result.IsSuccess())
                    {
                    DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                    return DgnDbServerStatusResult::Error(DgnDbServerError(result.GetError()));
                    }

                double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
                return DgnDbServerStatusResult::Success();
                });
            });
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusResult DgnDbRepositoryConnection::DownloadBriefcaseFile
(
BeFileName                        localFile,
BeBriefcaseId                     briefcaseId,
Http::Request::ProgressCallbackCR callback,
ICancellationTokenPtr             cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::DownloadBriefcaseFile";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    Utf8String instanceId;
    instanceId.Sprintf("%u", briefcaseId.GetValue());
    ObjectId fileObject(ServerSchema::Schema::Repository, ServerSchema::Class::Briefcase, instanceId);

    auto downloadResult = DownloadFile(localFile, fileObject, callback, cancellationToken)->GetResult();
    if (!downloadResult.IsSuccess())
        return downloadResult;

    return WriteBriefcaseIdIntoFile(localFile, briefcaseId);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnRevisionTaskPtr DgnDbRepositoryConnection::DownloadRevisionFile
(
DgnDbServerRevisionInfoPtr        revision,
Http::Request::ProgressCallbackCR callback,
ICancellationTokenPtr             cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::DownloadRevisionFile";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    RevisionStatus revisionStatus;
    DgnRevisionPtr dgnRevisionptr = DgnRevision::Create(&revisionStatus, revision->GetId(), revision->GetParentRevisionId(), revision->GetDbGuid());
    auto revisionFileName = dgnRevisionptr->GetRevisionChangesFile();

    if (s_preDownloadManager->TryGetRevisionFile(revisionFileName, revision->GetId()))
        return CreateCompletedAsyncTask<DgnRevisionResult>(DgnRevisionResult::Success(dgnRevisionptr));

    ObjectId fileObject(ServerSchema::Schema::Repository, ServerSchema::Class::Revision, revision->GetId());

    if (revision->GetContainsFileAccessKey())
        {
        return DownloadFileInternal(revisionFileName, fileObject, revision->GetFileAccessKey(), callback, cancellationToken)
            ->Then<DgnRevisionResult>([=](DgnDbServerStatusResultCR downloadResult)
            {
            if (!downloadResult.IsSuccess())
                return DgnRevisionResult::Error(downloadResult.GetError());

            return DgnRevisionResult::Success(dgnRevisionptr);
            });
        }

    return DownloadFile(revisionFileName, fileObject, callback, cancellationToken)
        ->Then<DgnRevisionResult>([=](DgnDbServerStatusResultCR downloadResult)
        {
        if (!downloadResult.IsSuccess())
            return DgnRevisionResult::Error(downloadResult.GetError());

        return DgnRevisionResult::Success(dgnRevisionptr);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
Json::Value CreateLockInstanceJson
(
bvector<uint64_t> const& ids,
BeBriefcaseId            briefcaseId,
BeGuidCR                 masterFileId,
Utf8StringCR             releasedWithRevisionId,
LockableType             type,
LockLevel                level,
bool                     queryOnly
)
    {
    Json::Value properties;

    properties[ServerSchema::Property::BriefcaseId]          = briefcaseId.GetValue();
    properties[ServerSchema::Property::MasterFileId]         = masterFileId.ToString();
    properties[ServerSchema::Property::ReleasedWithRevision] = releasedWithRevisionId;
    properties[ServerSchema::Property::QueryOnly]            = queryOnly;
    RepositoryJson::LockableTypeToJson(properties[ServerSchema::Property::LockType], type);
    RepositoryJson::LockLevelToJson(properties[ServerSchema::Property::LockLevel], level);

    properties[ServerSchema::Property::ObjectIds] = Json::arrayValue;
    int i = 0;
    for (auto const& id : ids)
        {
        Utf8String idStr;
        idStr.Sprintf("%llu", id);
        properties[ServerSchema::Property::ObjectIds][i++] = idStr;
        }

    return properties;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
void AddToInstance
(
WSChangeset&                     changeset,
WSChangeset::ChangeState const&  changeState,
bvector<uint64_t> const&         ids,
BeBriefcaseId                    briefcaseId,
BeGuidCR                         masterFileId,
Utf8StringCR                     releasedWithRevisionId,
LockableType                     type,
LockLevel                        level,
bool                             queryOnly
)
    {
    if (ids.empty ())
        return;
    ObjectId lockObject (ServerSchema::Schema::Repository, ServerSchema::Class::MultiLock, "MultiLock");
    auto json = std::make_shared<Json::Value>(CreateLockInstanceJson(ids, briefcaseId, masterFileId, releasedWithRevisionId, type, level, queryOnly));
    changeset.AddInstance (lockObject, changeState, json);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
void SetLocksJsonRequestToChangeSet
(
const DgnLockSet&               locks,
BeBriefcaseId                   briefcaseId,
BeGuidCR                        masterFileId,
Utf8StringCR                    releasedWithRevisionId,
WSChangeset&                    changeset,
const WSChangeset::ChangeState& changeState,
bool                            includeOnlyExclusive = false,
bool                            queryOnly = false
)
    {
    bvector<uint64_t> objects[12];
    for (auto& lock : locks)
        {
        if (includeOnlyExclusive && LockLevel::Exclusive != lock.GetLevel ())
            continue;

        int index = static_cast<int32_t>(lock.GetType ()) * 3 + static_cast<int32_t>(lock.GetLevel ());
        if (index >= 0 && index <= 11)
            objects[index].push_back (lock.GetId ().GetValue ());
        }

    for (int i = 0; i < 12; ++i)
        AddToInstance(changeset, changeState, objects[i], briefcaseId, masterFileId, releasedWithRevisionId, static_cast<LockableType>(i / 3), static_cast<LockLevel>(i % 3), queryOnly);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
uint8_t DgnCodeStateToInt(DgnCodeStateCR state)
    {
    //NEEDSWORK: Make DgnCodeState::Type public
	if (state.IsReserved())
		return 1;
	if (state.IsUsed())
		return 2;

	return 0;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
Json::Value CreateCodeInstanceJson
(
bvector<DgnCode> const&      codes,
DgnCodeStateCR               state,
BeBriefcaseId                briefcaseId,
bool                         queryOnly
)
    {
    Json::Value properties;
    DgnCode const* firstCode = codes.begin();


    properties[ServerSchema::Property::CodeSpecId]   = firstCode->GetCodeSpecId().GetValue();
    properties[ServerSchema::Property::CodeScope]    = firstCode->GetScope();
    properties[ServerSchema::Property::BriefcaseId]  = briefcaseId.GetValue();
    properties[ServerSchema::Property::State]        = DgnCodeStateToInt(state);
    properties[ServerSchema::Property::QueryOnly]    = queryOnly;

    properties[ServerSchema::Property::Values] = Json::arrayValue;
    int i = 0;
    for (auto const& code : codes)
        {
        properties[ServerSchema::Property::Values][i++] = code.GetValue();
        }

    return properties;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
void EncodeIdString
(
Utf8StringR value
)
    {
    if (value.empty())
        return;

    Utf8String reservedChar("-");
    Utf8String replacement;
    replacement.Sprintf("_%2X_", (int)reservedChar[0]);

    value.ReplaceAll(reservedChar.c_str(), replacement.c_str());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
Utf8String UriEncode(Utf8String input)
    {
    Utf8String result = BeStringUtilities::UriEncode(input.c_str());

    Utf8String charsNotEncode = ",!'()";
    for (char character : charsNotEncode)
        {
        Utf8String charString(&character, 1);
        result.ReplaceAll(BeStringUtilities::UriEncode(charString.c_str()).c_str(), charString.c_str());
        }

    result.ReplaceAll("~", "~7E");
    result.ReplaceAll("*", "~2A");
    result.ReplaceAll("%", "~");

    return result;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
Utf8String FormatCodeId
(
uint64_t                         codeSpecId,
Utf8StringCR                     scope,
Utf8StringCR                     value
)
    {
    Utf8String idString;

    Utf8String encodedScope(scope.c_str());
    EncodeIdString(encodedScope);
    encodedScope = UriEncode(encodedScope);

    Utf8String encodedValue(value.c_str());
    EncodeIdString(encodedValue);
    encodedValue = UriEncode(encodedValue);
    
    idString.Sprintf("%d-%s-%s", codeSpecId, encodedScope.c_str(), encodedValue.c_str());

    return idString;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
Utf8String FormatCodeId
(
uint64_t                         codeSpecId,
Utf8StringCR                     scope,
Utf8StringCR                     value,
BeBriefcaseId                    briefcaseId
)
    {
    Utf8String idString;
    idString.Sprintf("%s-%d", FormatCodeId(codeSpecId, scope, value).c_str(), briefcaseId.GetValue());

    return idString;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
void AddCodeToInstance
(
WSChangeset&                     changeset,
WSChangeset::ChangeState const&  changeState,
bvector<DgnCode> const&          codes,
DgnCodeStateCR                   state,
BeBriefcaseId                    briefcaseId,
bool                             queryOnly
)
    {
    if (codes.empty())
        return;

    ObjectId codeObject(ServerSchema::Schema::Repository, ServerSchema::Class::MultiCode, "MultiCode");
    JsonValueCR codeJson = CreateCodeInstanceJson(codes, state, briefcaseId, queryOnly);
    changeset.AddInstance(codeObject, changeState, std::make_shared<Json::Value>(codeJson));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
void GroupCode
(
bmap<Utf8String, bvector<DgnCode>>* groupedCodes,
DgnCode searchCode
)
    {
    Utf8String searchKey = FormatCodeId(searchCode.GetCodeSpecId().GetValue(), searchCode.GetScope(), "");
    auto it = groupedCodes->find(searchKey);
    if (it == groupedCodes->end())
        {
        bvector<DgnCode> codes;
        codes.push_back(searchCode);
        groupedCodes->insert({ searchKey, codes });
        return;
        }
    it->second.push_back(searchCode);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
void SetCodesJsonRequestToChangeSet
(
const DgnCodeSet                codes,
DgnCodeState                    state,
BeBriefcaseId                   briefcaseId,
WSChangeset&                    changeset,
const WSChangeset::ChangeState& changeState,
bool                            queryOnly = false
)
    {
    bmap<Utf8String, bvector<DgnCode>> groupedCodes;
    for (auto& code : codes)
        {
        GroupCode(&groupedCodes, code);
        }

    for (auto& group : groupedCodes)
        {
        AddCodeToInstance(changeset, changeState, group.second, state, briefcaseId, queryOnly);
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
DgnDbStatus GenerateValuePattern(CodeSpec codeSpec, Utf8StringR valuePattern, uint32_t& startIndex, uint32_t& incrementBy)
    {
    valuePattern = "";
    Utf8String placeholder;
    for (CodeFragmentSpecCR fragmentSpec : codeSpec.GetFragmentSpecs())
        {
        switch (fragmentSpec.GetType())
            {
            case CodeFragmentSpec::Type::FixedString:
                valuePattern.append(fragmentSpec.GetFixedString());
                break;
            case CodeFragmentSpec::Type::Sequence:
                startIndex = fragmentSpec.GetStartNumber();
                incrementBy = fragmentSpec.GetNumberGap();

                placeholder = Utf8String(fragmentSpec.GetMinChars(), '#');
                valuePattern.append(placeholder);
                break;
            default:
                return DgnDbStatus::UnknownFormat;
            }
        }

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
DgnDbStatus CreateCodeTemplateJson
(
JsonValueR                   properties,
Utf8StringCR                 valuePattern,
CodeSpecCR                   codeSpec,
DgnDbCodeTemplate::Type      templateType,
int                          startIndex,
int                          incrementBy
)
    {
    Utf8String codeScope;
    codeScope.Sprintf("%d", static_cast<int>(codeSpec.GetScope().GetType()));

    properties[ServerSchema::Property::CodeSpecId] = codeSpec.GetCodeSpecId().GetValue();
    properties[ServerSchema::Property::CodeScope] = codeScope;
    properties[ServerSchema::Property::ValuePattern] = valuePattern;
    properties[ServerSchema::Property::Type] = (int)templateType;

    if (startIndex >= 0 && incrementBy > 0)
        {
        properties[ServerSchema::Property::StartIndex] = startIndex;
        properties[ServerSchema::Property::IncrementBy] = incrementBy;
        }

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             08/2016
//---------------------------------------------------------------------------------------
DgnDbStatus SetCodeTemplatesJsonRequestToChangeSet
(
CodeSpecCR                      codeSpec,
const DgnDbCodeTemplate::Type   templateType,
WSChangeset&                    changeset,
const WSChangeset::ChangeState& changeState
)
    {
    ObjectId codeObject(ServerSchema::Schema::Repository, ServerSchema::Class::CodeTemplate, "");

    Json::Value codeJson;
    Utf8String valuePattern;
    uint32_t startIndex, incrementBy;

    auto status = GenerateValuePattern(codeSpec, valuePattern, startIndex, incrementBy);
    if (DgnDbStatus::Success != status)
        return status;

    status = CreateCodeTemplateJson(codeJson, valuePattern, codeSpec, templateType, startIndex, incrementBy);
    if (DgnDbStatus::Success != status)
        return status;
    changeset.AddInstance(codeObject, changeState, std::make_shared<Json::Value>(codeJson));

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
void LockDeleteAllJsonRequest (std::shared_ptr<WSChangeset> changeSet, const BeBriefcaseId& briefcaseId)
    {
    Utf8String id;
    id.Sprintf ("%s-%d", ServerSchema::DeleteAllLocks, briefcaseId.GetValue ());

    ObjectId lockObject (ServerSchema::Schema::Repository, ServerSchema::Class::Lock, id);

    Json::Value properties;
    changeSet->AddInstance(lockObject, WSChangeset::ChangeState::Deleted, std::make_shared<Json::Value> (properties));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
void CodeDiscardReservedJsonRequest(std::shared_ptr<WSChangeset> changeSet, const BeBriefcaseId& briefcaseId)
    {
    Utf8String id;
    id.Sprintf("%s-%d", ServerSchema::DiscardReservedCodes, briefcaseId.GetValue());

    ObjectId codeObject(ServerSchema::Schema::Repository, ServerSchema::Class::Code, id);

    Json::Value properties;
    changeSet->AddInstance(codeObject, WSChangeset::ChangeState::Deleted, std::make_shared<Json::Value>(properties));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
ObjectId GetLockId
(
LockableId lock,
const BeBriefcaseId*  briefcaseId
)
    {
    Utf8String idString;
    if (nullptr == briefcaseId)
        idString.Sprintf("%d-%llu", (int) lock.GetType(), lock.GetId().GetValue());
    else
        idString.Sprintf("%d-%llu-%u", (int) lock.GetType(), lock.GetId().GetValue(), briefcaseId->GetValue());

    return ObjectId(ServerSchema::Schema::Repository, ServerSchema::Class::Lock, idString);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
ObjectId GetCodeId
(
DgnCode code,
const BeBriefcaseId*  briefcaseId
)
    {
    Utf8String idString;
    if (nullptr != briefcaseId)
        idString.Sprintf("%s", FormatCodeId(code.GetCodeSpecId().GetValue(), code.GetScope(), code.GetValue(), *briefcaseId).c_str());
    else
        idString.Sprintf("%s", FormatCodeId(code.GetCodeSpecId().GetValue(), code.GetScope(), code.GetValue()).c_str());

    return ObjectId(ServerSchema::Schema::Repository, ServerSchema::Class::Code, idString);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             03/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::SendChangesetRequest
(
std::shared_ptr<WSChangeset> changeset,
IBriefcaseManager::ResponseOptions options,
ICancellationTokenPtr cancellationToken
) const
    {
    return ExecutionManager::ExecuteWithRetry<void>([=]() { return SendChangesetRequestInternal(changeset, options, cancellationToken); });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             03/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::SendChangesetRequestInternal
(
std::shared_ptr<WSChangeset> changeset,
IBriefcaseManager::ResponseOptions options,
ICancellationTokenPtr cancellationToken,
IWSRepositoryClient::RequestOptionsPtr requestOptions
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::SendChangesetRequest";

    changeset->GetRequestOptions().SetResponseContent(WSChangeset::Options::ResponseContent::Empty);
    
    if (static_cast<bool>(options & IBriefcaseManager::ResponseOptions::UnlimitedReporting))
        changeset->GetRequestOptions().SetCustomOption(ServerSchema::ExtendedParameters::SetMaximumInstances, "-1");

    if (!static_cast<bool>(options & IBriefcaseManager::ResponseOptions::LockState))
        changeset->GetRequestOptions().SetCustomOption(ServerSchema::ExtendedParameters::DetailedError_Locks, "false");

    if (!static_cast<bool>(options & IBriefcaseManager::ResponseOptions::CodeState))
        changeset->GetRequestOptions().SetCustomOption(ServerSchema::ExtendedParameters::DetailedError_Codes, "false");

    HttpStringBodyPtr request = HttpStringBody::Create(changeset->ToRequestString());
    return m_wsRepositoryClient->SendChangesetRequest(request, nullptr, cancellationToken, requestOptions)->Then<DgnDbServerStatusResult>
        ([=] (const WSChangesetResult& result)
        {
        if (result.IsSuccess())
            return DgnDbServerStatusResult::Success();
        else
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return DgnDbServerStatusResult::Error(result.GetError());
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::AcquireCodesLocks
(
    LockRequestCR                       locks,
    DgnCodeSet                          codes,
    BeBriefcaseId                       briefcaseId,
    BeGuidCR                            masterFileId,
    Utf8StringCR                        lastRevisionId,
    IBriefcaseManager::ResponseOptions  options,
    ICancellationTokenPtr               cancellationToken
) const 
    {
    return ExecutionManager::ExecuteWithRetry<void>([=]() { return AcquireCodesLocksInternal(locks, codes, briefcaseId, masterFileId, lastRevisionId, options, cancellationToken); });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::AcquireCodesLocksInternal
(
    LockRequestCR                       locks,
    DgnCodeSet                          codes,
    BeBriefcaseId                       briefcaseId,
    BeGuidCR                            masterFileId,
    Utf8StringCR                        lastRevisionId,
    IBriefcaseManager::ResponseOptions  options,
    ICancellationTokenPtr               cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::AcquireCodesLocks";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    
    SetLocksJsonRequestToChangeSet(locks.GetLockSet(), briefcaseId, masterFileId, lastRevisionId, *changeset, WSChangeset::ChangeState::Modified);
    
    DgnCodeState state;
    state.SetReserved(briefcaseId);
    SetCodesJsonRequestToChangeSet(codes, state, briefcaseId, *changeset, WSChangeset::ChangeState::Created);

    return SendChangesetRequestInternal(changeset, options, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::QueryCodesLocksAvailability
(
    LockRequestCR                       locks,
    DgnCodeSet                          codes,
    BeBriefcaseId                       briefcaseId,
    BeGuidCR                            masterFileId,
    Utf8StringCR                        lastRevisionId,
    IBriefcaseManager::ResponseOptions  options,
    ICancellationTokenPtr               cancellationToken
) const
    {
    std::shared_ptr<WSChangeset> changeset(new WSChangeset());

    SetLocksJsonRequestToChangeSet(locks.GetLockSet(), briefcaseId, masterFileId, lastRevisionId, *changeset, WSChangeset::ChangeState::Modified, false, true);

    DgnCodeState state;
    state.SetReserved(briefcaseId);
    SetCodesJsonRequestToChangeSet(codes, state, briefcaseId, *changeset, WSChangeset::ChangeState::Created, true);

    return SendChangesetRequest(changeset, options, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::DemoteCodesLocks
(
const DgnLockSet&                       locks,
DgnCodeSet const&                       codes,
BeBriefcaseId                           briefcaseId,
BeGuidCR                                masterFileId,
IBriefcaseManager::ResponseOptions      options,
ICancellationTokenPtr                   cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::DemoteCodesLocks";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    //How to set description here?
    std::shared_ptr<WSChangeset> changeset (new WSChangeset ());
    SetLocksJsonRequestToChangeSet (locks, briefcaseId, masterFileId, "", *changeset, WSChangeset::ChangeState::Modified);

    DgnCodeState state;
    state.SetAvailable();
    SetCodesJsonRequestToChangeSet(codes, state, briefcaseId, *changeset, WSChangeset::ChangeState::Modified);

    return SendChangesetRequestInternal(changeset, options, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::RelinquishCodesLocks
(
BeBriefcaseId                           briefcaseId,
IBriefcaseManager::ResponseOptions      options,
ICancellationTokenPtr                   cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::RelinquishCodesLocks";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    std::shared_ptr<WSChangeset> changeset (new WSChangeset ());
    LockDeleteAllJsonRequest (changeset, briefcaseId);
    CodeDiscardReservedJsonRequest(changeset, briefcaseId);
    return SendChangesetRequest(changeset, options, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerBriefcasesInfoTaskPtr DgnDbRepositoryConnection::QueryAllBriefcasesInfo(ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::QueryAllBriefcasesInfo";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::Briefcase);
    return QueryBriefcaseInfoInternal(query, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerBriefcaseInfoTaskPtr DgnDbRepositoryConnection::QueryBriefcaseInfo(BeSQLite::BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::QueryBriefcaseInfo";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    Utf8String filter;
    filter.Sprintf("%s+eq+%u", ServerSchema::Property::BriefcaseId, briefcaseId);

    WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::Briefcase);
    query.SetFilter(filter);

    return QueryBriefcaseInfoInternal(query, cancellationToken)->Then<DgnDbServerBriefcaseInfoResult>([=] (DgnDbServerBriefcasesInfoResultCR briefcasesResult)
        {
        if (!briefcasesResult.IsSuccess())
            {
            return DgnDbServerBriefcaseInfoResult::Error(briefcasesResult.GetError());
            }
        auto briefcasesInfo = briefcasesResult.GetValue();
        if (briefcasesInfo.empty())
            {
            return DgnDbServerBriefcaseInfoResult::Error({DgnDbServerError::Id::InvalidBriefcase, DgnDbServerErrorLocalizedString(MESSAGE_BriefcaseInfoParseError)});
            }
        return DgnDbServerBriefcaseInfoResult::Success(briefcasesResult.GetValue()[0]);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerBriefcasesInfoTaskPtr DgnDbRepositoryConnection::QueryBriefcasesInfo
(
bvector<BeSQLite::BeBriefcaseId>& briefcaseIds,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::QueryBriefcasesInfo";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    std::deque<ObjectId> queryIds;
    for (auto& id : briefcaseIds)
        {
        Utf8String idString;
        idString.Sprintf("%lu", id.GetValue());
        queryIds.push_back(ObjectId(ServerSchema::Schema::Repository, ServerSchema::Class::Briefcase, idString));
        }

    bset<DgnDbServerBriefcasesInfoTaskPtr> tasks;
    while (!queryIds.empty())
        {
        WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::Briefcase);
        query.AddFilterIdsIn(queryIds);
        auto task = QueryBriefcaseInfoInternal(query, cancellationToken);

        tasks.insert(task);
        }

    auto finalValue = std::make_shared<bvector<DgnDbServerBriefcaseInfoPtr>>();

    return AsyncTask::WhenAll(tasks)
        ->Then<DgnDbServerBriefcasesInfoResult>([=]
        {
        for (auto& task : tasks)
            {
            if (!task->GetResult().IsSuccess())
                return DgnDbServerBriefcasesInfoResult::Error(task->GetResult().GetError());

            auto briefcaseInfo = task->GetResult().GetValue();
            finalValue->insert(finalValue->end(), briefcaseInfo.begin(), briefcaseInfo.end());
            }

        return DgnDbServerBriefcasesInfoResult::Success(*finalValue);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerBriefcasesInfoTaskPtr DgnDbRepositoryConnection::QueryBriefcaseInfoInternal(WSQuery const& query, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::QueryBriefcaseInfoInternal";
    return ExecutionManager::ExecuteWithRetry<bvector<DgnDbServerBriefcaseInfoPtr>>([=]()
        {
        return m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)
            ->Then<DgnDbServerBriefcasesInfoResult>([=](const WSObjectsResult& result)
            {
            if (!result.IsSuccess())
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                return DgnDbServerBriefcasesInfoResult::Error(result.GetError());
                }

            bvector<DgnDbServerBriefcaseInfoPtr> briefcases;
            for (auto& value : result.GetValue().GetJsonValue()[ServerSchema::Instances])
                {
                briefcases.push_back(DgnDbServerBriefcaseInfo::Parse(value));
                }

            return DgnDbServerBriefcasesInfoResult::Success(briefcases);
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerCodeLockSetTaskPtr DgnDbRepositoryConnection::QueryCodesLocksById
(
DgnCodeSet const& codes, 
LockableIdSet const& locks,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::QueryCodesLocksById";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    return QueryCodesLocksInternal(&codes, &locks, nullptr, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerCodeLockSetTaskPtr DgnDbRepositoryConnection::QueryCodesLocksById
(
DgnCodeSet const& codes,
LockableIdSet const& locks,
BeBriefcaseId briefcaseId,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::QueryCodesLocksById";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    return QueryCodesLocksInternal(&codes, &locks, &briefcaseId, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerCodeLockSetTaskPtr DgnDbRepositoryConnection::QueryCodesLocks
(
const BeBriefcaseId  briefcaseId,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::QueryCodesLocks";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    return QueryCodesLocksInternal(nullptr, nullptr, &briefcaseId, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerCodeLockSetTaskPtr DgnDbRepositoryConnection::QueryCodesLocksInternal
(
DgnCodeSet const* codes,
LockableIdSet const* locks,
const BeBriefcaseId*  briefcaseId,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::QueryCodesLocksInternal";

    bset<DgnDbServerStatusTaskPtr> tasks;
    DgnDbCodeLockSetResultInfoPtr finalValue = new DgnDbCodeLockSetResultInfo();

    if (nullptr != codes)
        {
        auto task = QueryCodesInternal(*codes, briefcaseId, finalValue, cancellationToken);
        tasks.insert(task);
        }

    if (nullptr != locks)
        {
        auto task = QueryLocksInternal(*locks, briefcaseId, finalValue, cancellationToken);
        tasks.insert(task);
        }

    //Query codes locks by briefcase id
    if (nullptr != briefcaseId && tasks.empty())
        {
        auto task = QueryCodesInternal(briefcaseId, finalValue, cancellationToken);
        tasks.insert(task);

        task = QueryLocksInternal(briefcaseId, finalValue, cancellationToken);
        tasks.insert(task);
        }

    return AsyncTask::WhenAll(tasks)
        ->Then<DgnDbServerCodeLockSetResult>([=]
        {
        for (auto task : tasks)
            {
            if (!task->GetResult().IsSuccess())
                return DgnDbServerCodeLockSetResult::Error(task->GetResult().GetError());
            }
        return DgnDbServerCodeLockSetResult::Success(*finalValue);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas                02/2017
//---------------------------------------------------------------------------------------
void AddCodes(const WSObjectsReader::Instance& value, DgnDbCodeLockSetResultInfoPtr codesLocksSetOut)
    {
    DgnCode        code;
    DgnCodeState   codeState;
    BeBriefcaseId  briefcaseId;

    if (GetCodeFromServerJson(value.GetProperties(), code, codeState, briefcaseId))
        codesLocksSetOut->AddCode(code, codeState, briefcaseId);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas                01/2017
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::QueryCodesInternal
(
const DgnCodeSet& codes,
const BeSQLite::BeBriefcaseId*  briefcaseId,
DgnDbCodeLockSetResultInfoPtr codesLocksOut,
ICancellationTokenPtr cancellationToken
) const
    {
    WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::Code);

    std::deque<ObjectId> queryIds;
    for (auto& code : codes)
        queryIds.push_back(GetCodeId(code, briefcaseId));

    query.AddFilterIdsIn(queryIds, nullptr, 0, 0);

    return QueryCodesLocksInternal(query, codesLocksOut, AddCodes, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas                01/2017
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::QueryCodesInternal
(
const BeSQLite::BeBriefcaseId*  briefcaseId,
DgnDbCodeLockSetResultInfoPtr codesLocksOut,
ICancellationTokenPtr cancellationToken
) const
    {
    WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::MultiCode);

    Utf8String filter;
    filter.Sprintf("(%s+eq+%u)", ServerSchema::Property::BriefcaseId, briefcaseId->GetValue());
    query.SetFilter(filter);

    auto addMultiCodesCallback = [&] (const WSObjectsReader::Instance& value, DgnDbCodeLockSetResultInfoPtr codesLocksSetOut)
        {
        DgnCode        code;
        DgnCodeState   codeState;
        BeBriefcaseId  briefcaseId;

        DgnCodeSet codeSet;
        if (GetMultiCodeFromServerJson(value.GetProperties(), codeSet, codeState, briefcaseId))
            {
            for (auto const& code : codeSet)
                {
                codesLocksSetOut->AddCode(code, codeState, briefcaseId);
                }
            }
        };

    return QueryCodesLocksInternal(query, codesLocksOut, addMultiCodesCallback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikolinuas             08/2016
//---------------------------------------------------------------------------------------
inline const char * const BoolToString(bool b)
    {
    return b ? "true" : "false";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas                01/2017
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::QueryUnavailableCodesInternal
(
const BeBriefcaseId briefcaseId, 
DgnDbCodeLockSetResultInfoPtr codesLocksOut,
ICancellationTokenPtr cancellationToken
) const
    {
    WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::Code);

    Utf8String filter;
    filter.Sprintf("%s+ne+%u", ServerSchema::Property::BriefcaseId,
                   briefcaseId.GetValue());

    query.SetFilter(filter);

    return QueryCodesLocksInternal(query, codesLocksOut, AddCodes, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas                01/2017
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::QueryLocksInternal
(
const LockableIdSet& locks,
const BeSQLite::BeBriefcaseId*  briefcaseId,
DgnDbCodeLockSetResultInfoPtr codesLocksOut,
ICancellationTokenPtr cancellationToken
) const
    {
    WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::Lock);

    std::deque<ObjectId> queryIds;
    for (auto& lock : locks)
        queryIds.push_back(GetLockId(lock, briefcaseId));

    query.AddFilterIdsIn(queryIds, nullptr, 0, 0);

    auto addLocksCallback = [&] (const WSObjectsReader::Instance& value, DgnDbCodeLockSetResultInfoPtr codesLocksSetOut)
        {
        DgnLock        lock;
        BeBriefcaseId  briefcaseId;
        Utf8String     revisionId;

        if (GetLockFromServerJson(value.GetProperties(), lock, briefcaseId, revisionId))
            {
            if (lock.GetLevel() != LockLevel::None)
                codesLocksSetOut->AddLock(lock, briefcaseId, revisionId);
            }
        };

    return QueryCodesLocksInternal(query, codesLocksOut, addLocksCallback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas                01/2017
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::QueryLocksInternal
(
const BeSQLite::BeBriefcaseId*  briefcaseId,
DgnDbCodeLockSetResultInfoPtr codesLocksOut,
ICancellationTokenPtr cancellationToken
) const
    {
    WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::MultiLock);

    Utf8String filter;
    filter.Sprintf("(%s+eq+%u)", ServerSchema::Property::BriefcaseId, briefcaseId->GetValue());
    query.SetFilter(filter);

    auto addMultiLocksCallback = [&] (const WSObjectsReader::Instance& value, DgnDbCodeLockSetResultInfoPtr codesLocksSetOut)
        {
        DgnLock        lock;
        BeBriefcaseId  briefcaseId;
        Utf8String     revisionId;

        DgnLockSet lockSet;
        if (GetMultiLockFromServerJson(value.GetProperties(), lockSet, briefcaseId, revisionId))
            {
            for (auto const& lock : lockSet)
                {
                codesLocksSetOut->AddLock(lock, briefcaseId, revisionId);
                }
            }
        };

    return QueryCodesLocksInternal(query, codesLocksOut, addMultiLocksCallback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas                01/2017
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::QueryUnavailableLocksInternal
(
const BeBriefcaseId briefcaseId, 
const uint64_t lastRevisionIndex,
DgnDbCodeLockSetResultInfoPtr codesLocksOut,
ICancellationTokenPtr cancellationToken
) const
    {
    WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::Lock);

    Utf8String filter;
    Utf8String locksFilter;
    locksFilter.Sprintf("%s+gt+%u+or+%s+gt+%u", ServerSchema::Property::LockLevel, LockLevel::None,
                        ServerSchema::Property::ReleasedWithRevisionIndex, lastRevisionIndex);

    filter.Sprintf("%s+ne+%u+and+(%s)", ServerSchema::Property::BriefcaseId,
                   briefcaseId.GetValue(), locksFilter.c_str());

    query.SetFilter(filter);


    auto addLocksCallback = [&] (const WSObjectsReader::Instance& value, DgnDbCodeLockSetResultInfoPtr codesLocksSetOut)
        {
        DgnLock        lock;
        BeBriefcaseId  briefcaseId;
        Utf8String     revisionId;

        if (GetLockFromServerJson(value.GetProperties(), lock, briefcaseId, revisionId))
            {
            codesLocksSetOut->AddLock(lock, briefcaseId, revisionId);
            }
        };

    return QueryCodesLocksInternal(query, codesLocksOut, addLocksCallback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::QueryCodesLocksInternal
(
WSQuery query,
DgnDbCodeLockSetResultInfoPtr codesLocksOut,
DgnDbCodeLocksSetAddFunction addFunction,
ICancellationTokenPtr cancellationToken
) const
    {
    return ExecutionManager::ExecuteWithRetry<void>([=]()
        {
        //Execute query
        return m_wsRepositoryClient->SendQueryRequest(query, "", "", cancellationToken)->Then<DgnDbServerStatusResult>
            ([=] (const WSObjectsResult& result)
            {
            if (result.IsSuccess())
                {
                if (!result.GetValue().GetRapidJsonDocument().IsNull())
                    {
                    for (auto const& value : result.GetValue().GetInstances())
                        {
                        addFunction(value, codesLocksOut);
                        }
                    //NEEDSWORK: log an error
                    }            
                return DgnDbServerStatusResult::Success();
                }

            return DgnDbServerStatusResult::Error(result.GetError());
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerCodeLockSetTaskPtr DgnDbRepositoryConnection::QueryUnavailableCodesLocks
(
const BeBriefcaseId   briefcaseId,
Utf8StringCR          lastRevisionId,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::QueryUnavailableCodesLocks";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (briefcaseId.IsMasterId() || briefcaseId.IsStandaloneId())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid briefcase.");
        return CreateCompletedAsyncTask<DgnDbServerCodeLockSetResult>(DgnDbServerCodeLockSetResult::Error(DgnDbServerError::Id::FileIsNotBriefcase));
        }
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    std::shared_ptr<DgnDbServerCodeLockSetResult> finalResult = std::make_shared<DgnDbServerCodeLockSetResult>();
    return ExecutionManager::ExecuteWithRetry<DgnDbCodeLockSetResultInfo>([=]()
        {
        return GetRevisionById(lastRevisionId, cancellationToken)->Then([=] (DgnDbServerRevisionInfoResultCR revisionResult)
            {
            uint64_t revisionIndex = 0;
            if (!revisionResult.IsSuccess() && revisionResult.GetError().GetId() != DgnDbServerError::Id::InvalidRevision)
                {
                finalResult->SetError(revisionResult.GetError());
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, revisionResult.GetError().GetMessage().c_str());
                return;
                }
            else if (revisionResult.IsSuccess())
                {
                revisionIndex = revisionResult.GetValue()->GetIndex();
                }

            DgnDbCodeLockSetResultInfoPtr finalValue = new DgnDbCodeLockSetResultInfo();
            bset<DgnDbServerStatusTaskPtr> tasks;

            auto task = QueryUnavailableCodesInternal(briefcaseId, finalValue, cancellationToken);
            tasks.insert(task);
            task = QueryUnavailableLocksInternal(briefcaseId, revisionIndex, finalValue, cancellationToken);
            tasks.insert(task);

            AsyncTask::WhenAll(tasks)
                ->Then([=]
                {
                for (auto task : tasks)
                    {
                    if (!task->GetResult().IsSuccess())
                        {
                        finalResult->SetError(task->GetResult().GetError());
                        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, task->GetResult().GetError().GetMessage().c_str());
                        return;
                        }
                    }

                finalResult->SetSuccess(*finalValue);
                double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
                });

            })->Then<DgnDbServerCodeLockSetResult>([=] ()
                {
                return *finalResult;
                });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             08/2016
//---------------------------------------------------------------------------------------
Json::Value GetChangedInstances(Utf8String response)
    {
    Json::Reader reader;
    Json::Value responseJson(Json::objectValue);
    if (!reader.parse(response, responseJson) && !responseJson.isArray())
        return nullptr;

    if (responseJson.isNull() || responseJson.empty())
        return nullptr;

    if (!responseJson.isMember(ServerSchema::ChangedInstances) ||
        responseJson[ServerSchema::ChangedInstances].empty() ||
        !responseJson[ServerSchema::ChangedInstances][0].isMember(ServerSchema::InstanceAfterChange))
        return nullptr;

    Json::Value instance(Json::objectValue);
    return responseJson[ServerSchema::ChangedInstances];
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerCodeTemplateTaskPtr DgnDbRepositoryConnection::QueryCodeMaximumIndex
(
CodeSpecCR codeSpec,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::QueryCodeMaximumIndex";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    std::shared_ptr<WSChangeset> changeset(new WSChangeset());

    auto status = SetCodeTemplatesJsonRequestToChangeSet(codeSpec, DgnDbCodeTemplate::Type::Maximum, *changeset, WSChangeset::ChangeState::Created);
    if (DgnDbStatus::Success != status)
        return CreateCompletedAsyncTask<DgnDbServerCodeTemplateResult>(DgnDbServerCodeTemplateResult::Error({DgnDbServerError::Id::BIMCSOperationFailed, DgnDbServerErrorLocalizedString(MESSAGE_CodeTemplateRequestError)}));

    auto requestString = changeset->ToRequestString();
    HttpStringBodyPtr request = HttpStringBody::Create(requestString);
    return ExecutionManager::ExecuteWithRetry<DgnDbCodeTemplate>([=]()
        {
        return m_wsRepositoryClient->SendChangesetRequest(request, nullptr, cancellationToken)->Then<DgnDbServerCodeTemplateResult>
            ([=](const WSChangesetResult& result)
            {
            if (result.IsSuccess())
                {
                Json::Value ptr = GetChangedInstances(result.GetValue()->AsString().c_str());
                
                auto json = ToRapidJson(*ptr.begin());
                DgnDbCodeTemplate        codeTemplate;
                if (!GetCodeTemplateFromServerJson(json[ServerSchema::InstanceAfterChange][ServerSchema::Properties], codeTemplate))
                    {
                    DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Code template parse failed.");
                    return DgnDbServerCodeTemplateResult::Error({DgnDbServerError::Id::InvalidPropertiesValues, DgnDbServerErrorLocalizedString(MESSAGE_CodeTemplateResponseError)});
                    }

                double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
                return DgnDbServerCodeTemplateResult::Success(codeTemplate);
                }
            else
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                return DgnDbServerCodeTemplateResult::Error(result.GetError());
                }
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             03/2017
//---------------------------------------------------------------------------------------
DgnDbServerCodeTemplateTaskPtr DgnDbRepositoryConnection::QueryCodeNextAvailable
(
CodeSpecCR codeSpec, 
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::QueryCodeNextAvailable";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    auto status = SetCodeTemplatesJsonRequestToChangeSet(codeSpec, DgnDbCodeTemplate::Type::NextAvailable, *changeset, WSChangeset::ChangeState::Created);
    if (DgnDbStatus::Success != status)
        return CreateCompletedAsyncTask<DgnDbServerCodeTemplateResult>(DgnDbServerCodeTemplateResult::Error({DgnDbServerError::Id::BIMCSOperationFailed, DgnDbServerErrorLocalizedString(MESSAGE_CodeTemplateRequestError)}));

    HttpStringBodyPtr request = HttpStringBody::Create(changeset->ToRequestString());
    return ExecutionManager::ExecuteWithRetry<DgnDbCodeTemplate>([=]()
        {
        return m_wsRepositoryClient->SendChangesetRequest(request, nullptr, cancellationToken)->Then<DgnDbServerCodeTemplateResult>
            ([=](const WSChangesetResult& result)
            {
            if (result.IsSuccess())
                {
                Json::Value ptr = GetChangedInstances(result.GetValue()->AsString().c_str());
                auto json = ToRapidJson(*ptr.begin());

                DgnDbCodeTemplate        codeTemplate;
                if (!GetCodeTemplateFromServerJson(json[ServerSchema::InstanceAfterChange][ServerSchema::Properties], codeTemplate))
                    {
                    DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Code template parse failed.");
                    return DgnDbServerCodeTemplateResult::Error({DgnDbServerError::Id::InvalidPropertiesValues, DgnDbServerErrorLocalizedString(MESSAGE_CodeTemplateResponseError)});
                    }

                double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
                return DgnDbServerCodeTemplateResult::Success(codeTemplate);
                }
            else
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                return DgnDbServerCodeTemplateResult::Error(result.GetError());
                }
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<WSCreateObjectResult> DgnDbRepositoryConnection::CreateBriefcaseInstance(ICancellationTokenPtr cancellationToken) const
    {
    Json::Value briefcaseIdJson = Json::objectValue;
    briefcaseIdJson[ServerSchema::Instance] = Json::objectValue;
    briefcaseIdJson[ServerSchema::Instance][ServerSchema::SchemaName] = ServerSchema::Schema::Repository;
    briefcaseIdJson[ServerSchema::Instance][ServerSchema::ClassName] = ServerSchema::Class::Briefcase;
    return m_wsRepositoryClient->SendCreateObjectRequest(briefcaseIdJson, BeFileName(), nullptr, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerFileTaskPtr DgnDbRepositoryConnection::GetBriefcaseFileInfo(BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::GetBriefcaseFileInfo";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    Utf8String briefcaseIdString;
    briefcaseIdString.Sprintf("%d", briefcaseId.GetValue());
    ObjectId briefcaseObjectId(ServerSchema::Schema::Repository, ServerSchema::Class::Briefcase, briefcaseIdString);

    return m_wsRepositoryClient->SendGetObjectRequest(briefcaseObjectId, nullptr, cancellationToken)
        ->Then<DgnDbServerFileResult>([=](WSObjectsResult const& result)
        {
        if (!result.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return DgnDbServerFileResult::Error(result.GetError());
            }
        auto fileInfo = FileInfo::Parse(*result.GetValue().GetInstances().begin());
        return DgnDbServerFileResult::Success(fileInfo);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             09/2016
//---------------------------------------------------------------------------------------
DgnDbServerBriefcaseInfoTaskPtr DgnDbRepositoryConnection::AcquireNewBriefcase(ICancellationTokenPtr cancellationToken) const
    {
    return ExecutionManager::ExecuteWithRetry<DgnDbServerBriefcaseInfoPtr>([=]()
        {
        return CreateBriefcaseInstance(cancellationToken)->Then<DgnDbServerBriefcaseInfoResult>([=] (const WSCreateObjectResult& result)
            {
            if (!result.IsSuccess())
                {
                return DgnDbServerBriefcaseInfoResult::Error(result.GetError());
                }

            Json::Value json;
            result.GetValue().GetJson(json);
            JsonValueCR instance = json[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
            return DgnDbServerBriefcaseInfoResult::Success(DgnDbServerBriefcaseInfo::Parse(instance));
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              03/2016
//---------------------------------------------------------------------------------------
DgnDbServerRevisionsInfoTaskPtr DgnDbRepositoryConnection::GetAllRevisions (ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::GetAllRevisions";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    return GetAllRevisionsInternal(false, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas              02/2017
//---------------------------------------------------------------------------------------
DgnDbServerRevisionsInfoTaskPtr DgnDbRepositoryConnection::GetAllRevisionsInternal(bool loadAccessKey, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::GetAllRevisionsInternal";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::Revision);
    return RevisionsFromQuery(query, loadAccessKey, cancellationToken);
    }
//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRevisionInfoTaskPtr DgnDbRepositoryConnection::GetRevisionById
(
Utf8StringCR          revisionId,
ICancellationTokenPtr cancellationToken
) const
    {
    return GetRevisionByIdInternal(revisionId, false, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRevisionInfoTaskPtr DgnDbRepositoryConnection::GetRevisionByIdInternal
(
Utf8StringCR          revisionId,
bool                  loadAccessKey,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::GetRevisionById";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (revisionId.empty())
        {
        // Don't log error here since this is a valid case then there are no revisions locally.
        return CreateCompletedAsyncTask<DgnDbServerRevisionInfoResult>(DgnDbServerRevisionInfoResult::Error(DgnDbServerError::Id::InvalidRevision));
        }
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    ObjectId revisionObject(ServerSchema::Schema::Repository, ServerSchema::Class::Revision, revisionId);

    WSQuery query(revisionObject);
    if (loadAccessKey)
        {
        Utf8String selectString = "*";
        DgnDbServerFileAccessKey::AddDownloadAccessKeySelect(selectString);
        query.SetSelect(selectString);
        }

    return m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)->Then<DgnDbServerRevisionInfoResult>
        ([=] (WSObjectsResult& revisionResult)
        {
        if (revisionResult.IsSuccess())
            {
            auto revisionInstances = revisionResult.GetValue().GetInstances();
            if (revisionInstances.IsEmpty())
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Revision does not exist.");
                return DgnDbServerRevisionInfoResult::Error(DgnDbServerError::Id::RevisionDoesNotExist);
                }

            auto revision = DgnDbServerRevisionInfo::Parse(*revisionInstances.begin());
            if (loadAccessKey)
                {
                revision->SetFileAccessKey(DgnDbServerFileAccessKey::ParseFromRelated(*revisionResult.GetValue().GetInstances().begin()));
                }
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
            return DgnDbServerRevisionInfoResult::Success(revision);
            }
        else
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, revisionResult.GetError().GetMessage().c_str());
            return DgnDbServerRevisionInfoResult::Error(revisionResult.GetError());
            }
        });
    }

/* EventService Methods Begin */

/* Private methods start */

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Json::Value GenerateEventSubscriptionWSChangeSetJson(DgnDbServerEventTypeSet* eventTypes)
    {
    Json::Value properties;
    properties[ServerSchema::Property::EventTypes] = Json::arrayValue;
    if (eventTypes != nullptr)
        {
        int i = 0;
        for (auto eventType : *eventTypes)
            {
            properties[ServerSchema::Property::EventTypes][i] = DgnDbServerEvent::Helper::GetEventNameFromEventType(eventType);
            i++;
            }
        }
    return properties;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventSubscriptionPtr CreateEventSubscription(Utf8String response)
    {
    Json::Reader reader;
    Json::Value responseJson(Json::objectValue);
    if (!reader.parse(response, responseJson) && !responseJson.isArray())
        return nullptr;

    if(responseJson.isNull() || responseJson.empty())
        return nullptr;

    if (!responseJson.isMember(ServerSchema::ChangedInstances) ||
        responseJson[ServerSchema::ChangedInstances].empty() ||
        !responseJson[ServerSchema::ChangedInstances][0].isMember(ServerSchema::InstanceAfterChange))
        return nullptr;

    Json::Value instance(Json::objectValue);
    instance = responseJson[ServerSchema::ChangedInstances][0][ServerSchema::InstanceAfterChange];

    if (!instance.isMember(ServerSchema::InstanceId))
        return nullptr;

    Utf8String eventSubscriptionId = instance[ServerSchema::InstanceId].asString();

    if (
        !instance[ServerSchema::Properties].isMember(ServerSchema::Property::EventTypes) ||
        !instance[ServerSchema::Properties][ServerSchema::Property::EventTypes].isArray()
        )
        return nullptr;

    DgnDbServerEventTypeSet eventTypes;
    for (Json::ValueIterator itr = instance[ServerSchema::Properties][ServerSchema::Property::EventTypes].begin();
        itr != instance[ServerSchema::Properties][ServerSchema::Property::EventTypes].end(); ++itr)
        eventTypes.insert(DgnDbServerEvent::Helper::GetEventTypeFromEventName((*itr).asString().c_str()));

    return DgnDbServerEventSubscription::Create(eventSubscriptionId, eventTypes);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Json::Value GenerateEventSASJson()
    {
    Json::Value request = Json::objectValue;
    JsonValueR instance = request[ServerSchema::Instance] = Json::objectValue;
    instance[ServerSchema::InstanceId] = "";
    instance[ServerSchema::SchemaName] = ServerSchema::Schema::Repository;
    instance[ServerSchema::ClassName] = ServerSchema::Class::EventSAS;
    instance[ServerSchema::Properties] = Json::objectValue;
    JsonValueR properties = instance[ServerSchema::Properties];
    properties[ServerSchema::Property::BaseAddress] = "";
    properties[ServerSchema::Property::EventServiceSASToken] = "";
    return request;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
AzureServiceBusSASDTOPtr CreateEventSAS(JsonValueCR responseJson)
    {
    if(responseJson.isNull() || responseJson.empty())
        return nullptr;

    if (!responseJson.isMember(ServerSchema::ChangedInstance) ||
        !responseJson[ServerSchema::ChangedInstance].isMember(ServerSchema::InstanceAfterChange))
        return nullptr;

    Json::Value instance(Json::objectValue);
    instance = responseJson[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];

    Utf8String sasToken = instance[ServerSchema::Properties][ServerSchema::Property::EventServiceSASToken].asString();
    Utf8String baseAddress = instance[ServerSchema::Properties][ServerSchema::Property::BaseAddress].asString();
    if (Utf8String::IsNullOrEmpty(sasToken.c_str()) || Utf8String::IsNullOrEmpty(baseAddress.c_str()))
        return nullptr;

    return AzureServiceBusSASDTO::Create(sasToken, baseAddress);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
//Returns true if same, else false
bool CompareEventTypes
(
DgnDbServerEventTypeSet* newEventTypes,
DgnDbServerEventTypeSet oldEventTypes
)
    {
    if (
        (newEventTypes == nullptr || newEventTypes->size() == 0) &&
        oldEventTypes.size() == 0
        )
        return true;
    
    if (
        oldEventTypes.size() > 0 &&
        (newEventTypes == nullptr || newEventTypes->size() != oldEventTypes.size())
        )
        return false;

    for (auto newEventType : *newEventTypes)
        {
        if (oldEventTypes.end() == oldEventTypes.find(newEventType))
            {
            return false;
            }
        }

    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
bool DgnDbRepositoryConnection::SetEventSASToken(ICancellationTokenPtr cancellationToken)
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::SetEventSASToken";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    auto sasToken = GetEventServiceSASToken(cancellationToken)->GetResult();
    if (!sasToken.IsSuccess())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, sasToken.GetError().GetMessage().c_str());
        return false;
        }

    m_eventSAS = sasToken.GetValue();

    double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
bool DgnDbRepositoryConnection::SetEventSubscription(DgnDbServerEventTypeSet* eventTypes, ICancellationTokenPtr cancellationToken)
    {
    DgnDbServerEventSubscriptionTaskPtr eventSubscription = nullptr;

    if (m_eventSubscription == nullptr)
        eventSubscription = GetEventServiceSubscriptionId(eventTypes, cancellationToken);
    else if (!CompareEventTypes(eventTypes, m_eventSubscription->GetEventTypes()))
        eventSubscription = UpdateEventServiceSubscriptionId(eventTypes, cancellationToken);
    else
        return true;

    if (!eventSubscription->GetResult().IsSuccess())
        return false;

    m_eventSubscription = eventSubscription->GetResult().GetValue();
    return SetEventSASToken(cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            05/2016
//---------------------------------------------------------------------------------------
bool DgnDbRepositoryConnection::SetEventServiceClient
(
DgnDbServerEventTypeSet* eventTypes,
ICancellationTokenPtr cancellationToken
)
    {
    if (!SetEventSubscription(eventTypes, cancellationToken))
        return false;

    if (m_eventServiceClient == nullptr)
        m_eventServiceClient = new EventServiceClient(m_eventSAS->GetBaseAddress(), m_repositoryInfo.GetId(), m_eventSubscription->GetSubscriptionId());

    m_eventServiceClient->UpdateSASToken(m_eventSAS->GetSASToken());
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
AzureServiceBusSASDTOTaskPtr DgnDbRepositoryConnection::GetEventServiceSASToken(ICancellationTokenPtr cancellationToken) const
    {
    //POST to https://{server}/{version}/Repositories/DgnDbServer--{repoId}/DgnDbServer/EventSAS
    std::shared_ptr<AzureServiceBusSASDTOResult> finalResult = std::make_shared<AzureServiceBusSASDTOResult>();
    return m_wsRepositoryClient->SendCreateObjectRequest(GenerateEventSASJson(), BeFileName(), nullptr, cancellationToken)
        ->Then([=] (const WSCreateObjectResult& result)
        {
        if (result.IsSuccess())
            {
            Json::Value json;
            result.GetValue().GetJson(json);
            AzureServiceBusSASDTOPtr ptr = CreateEventSAS(json);
            if (ptr == nullptr)
                {
                finalResult->SetError(DgnDbServerError::Id::NoSASFound);
                return;
                }

            finalResult->SetSuccess(ptr);
            }
        else
            {
            finalResult->SetError(result.GetError());
            }
        })->Then<AzureServiceBusSASDTOResult>([=]
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Arvind.Venkateswaran           07/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventSubscriptionTaskPtr DgnDbRepositoryConnection::SendEventChangesetRequest
(
std::shared_ptr<WSChangeset> changeset,
ICancellationTokenPtr cancellationToken
) const
    {
    //https://{server}/{version}/Repositories/DgnDbServer--{repoId}/DgnDbServer/EventSubscription
    HttpStringBodyPtr request = HttpStringBody::Create(changeset->ToRequestString());
    std::shared_ptr<DgnDbServerEventSubscriptionResult> finalResult = std::make_shared<DgnDbServerEventSubscriptionResult>();
    return m_wsRepositoryClient->SendChangesetRequest(request, nullptr, cancellationToken)->Then([=] (const WSChangesetResult& result)
        {
        if (result.IsSuccess())
            {
            DgnDbServerEventSubscriptionPtr ptr = CreateEventSubscription(result.GetValue()->AsString().c_str());
            if (ptr == nullptr)
                {
                finalResult->SetError(DgnDbServerError::Id::NoSubscriptionFound);
                return;
                }

            finalResult->SetSuccess(ptr);
            }
        else
            {
            finalResult->SetError(result.GetError());
            }
        })->Then<DgnDbServerEventSubscriptionResult>([=]
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Caleb.Shafer                   06/2016
//---------------------------------------------------------------------------------------
void SetEventSubscriptionJsonRequestToChangeSet
(
DgnDbServerEventTypeSet* eventTypes,
Utf8String                                       eventSubscriptionId,
WSChangeset&                                     changeset,
const WSChangeset::ChangeState&                  changeState
)
    {
    ObjectId eventSubscriptionObject
        (
        ServerSchema::Schema::Repository,
        ServerSchema::Class::EventSubscription,
        eventSubscriptionId
        );

    changeset.AddInstance
        (
        eventSubscriptionObject,
        changeState,
        std::make_shared<Json::Value>(GenerateEventSubscriptionWSChangeSetJson(eventTypes))
        );
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventSubscriptionTaskPtr DgnDbRepositoryConnection::GetEventServiceSubscriptionId
(
DgnDbServerEventTypeSet* eventTypes,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::GetEventServiceSubscriptionId";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    SetEventSubscriptionJsonRequestToChangeSet(eventTypes, "", *changeset, WSChangeset::Created);

    return SendEventChangesetRequest(changeset, cancellationToken)
        ->Then<DgnDbServerEventSubscriptionResult>([=] (DgnDbServerEventSubscriptionResultCR result)
        {
        if (!result.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            }
        else
            {
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
            }
        return result;
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                    06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventSubscriptionTaskPtr DgnDbRepositoryConnection::UpdateEventServiceSubscriptionId
(
DgnDbServerEventTypeSet* eventTypes,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::UpdateEventServiceSubscriptionId";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    SetEventSubscriptionJsonRequestToChangeSet(eventTypes, m_eventSubscription->GetSubscriptionId(), *changeset, WSChangeset::Modified);

    return SendEventChangesetRequest(changeset, cancellationToken)
        ->Then<DgnDbServerEventSubscriptionResult>([=] (DgnDbServerEventSubscriptionResultCR result)
        {
        if (!result.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            }
        else
            {
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
            }
        return result;
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            08/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventReponseTaskPtr DgnDbRepositoryConnection::GetEventServiceResponse
(
int numOfRetries, 
bool longpolling
)
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::GetEventServiceResponse";
    return m_eventServiceClient->MakeReceiveDeleteRequest(longpolling)
        ->Then<DgnDbServerEventReponseResult>([=](const EventServiceResult& result)
    {
        if (result.IsSuccess())
        {
            Http::Response response = result.GetValue();
            if (response.GetHttpStatus() != HttpStatus::OK)
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                return DgnDbServerEventReponseResult::Error(DgnDbServerError(result.GetError()));
                }

            return DgnDbServerEventReponseResult::Success(response);
        }
        else
        {
            HttpStatus status = result.GetError().GetHttpStatus();
            if (status == HttpStatus::NoContent || status == HttpStatus::None)
            {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_WARNING, methodName, "No events found.");
                return DgnDbServerEventReponseResult::Error(DgnDbServerError::Id::NoEventsFound);
            }
            else if (status == HttpStatus::Unauthorized && numOfRetries > 0)
            {
                if (SetEventSASToken())
                    m_eventServiceClient->UpdateSASToken(m_eventSAS->GetSASToken());
                int nextLoopValue = numOfRetries - 1;
                return GetEventServiceResponse(nextLoopValue, longpolling)->GetResult();
            }
            else
            {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                return DgnDbServerEventReponseResult::Error(DgnDbServerError(result.GetError()));
            }
        }
    });
}

/* Private methods end */

/* Public methods start */
//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas            12/2016
//---------------------------------------------------------------------------------------
bool DgnDbRepositoryConnection::IsSubscribedToEvents() const
    {
    return m_eventServiceClient != nullptr && m_eventSubscription != nullptr && m_eventSAS != nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventTaskPtr DgnDbRepositoryConnection::GetEvent
(
bool longPolling,
ICancellationTokenPtr cancellationToken
)
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::GetEvent";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (m_eventServiceClient == nullptr || m_eventSubscription == nullptr || m_eventSAS == nullptr)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Not subscribed to event service.");
        return CreateCompletedAsyncTask<DgnDbServerEventResult>
            (DgnDbServerEventResult::Error(DgnDbServerError::Id::NotSubscribedToEventService));
        }

    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    return GetEventServiceResponse(3, longPolling)->Then<DgnDbServerEventResult>
        ([=] (DgnDbServerEventReponseResult& result)
        {
        if (result.IsSuccess())
            {
            Http::Response response = result.GetValue();
            DgnDbServerEventPtr ptr = DgnDbServerEventParser::ParseEvent(response.GetHeaders().GetContentType(), response.GetBody().AsString());
            if (ptr == nullptr)
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_WARNING, methodName, "No events found.");
                return DgnDbServerEventResult::Error(DgnDbServerError::Id::NoEventsFound);
                }
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
            return DgnDbServerEventResult::Success(ptr);
            }
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
        return DgnDbServerEventResult::Error(result.GetError());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Caleb.Shafer                    06/2016
//                                               Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::SubscribeToEvents
(
DgnDbServerEventTypeSet* eventTypes,
ICancellationTokenPtr cancellationToken
)
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::SubscribeToEvents";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    return ExecutionManager::ExecuteWithRetry<void>([=]()
        {
        return (SetEventServiceClient(eventTypes, cancellationToken)) ?
            CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Success()) :
            CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::EventServiceSubscribingError));
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Arvind.Venkateswaran            06/2016
//Todo: Add another method to only cancel GetEvent Operation and not the entire connection
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr  DgnDbRepositoryConnection::UnsubscribeToEvents()
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::UnsubscribeToEvents";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (m_eventServiceClient != nullptr)
        m_eventServiceClient->CancelRequest();
    m_eventServiceClient = nullptr;
    m_eventSubscription = nullptr;
    m_eventSAS = nullptr;
    return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Success());    
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas            12/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::SubscribeEventsCallback(DgnDbServerEventTypeSet* eventTypes, DgnDbServerEventCallbackPtr callback)
    {
    if (m_eventManagerPtr.IsNull())
        {
        m_eventManagerPtr = new DgnDbServerEventManager(this);
        }
    return m_eventManagerPtr->Subscribe(eventTypes, callback);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas            12/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::UnsubscribeEventsCallback(DgnDbServerEventCallbackPtr callback)
    {
    if (m_eventManagerPtr.IsNull())
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Success());

    if (!callback)
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::EventCallbackNotSpecified));

    bool dispose = false;
    auto result = m_eventManagerPtr->Unsubscribe(callback, &dispose);
    if (dispose)
        m_eventManagerPtr = nullptr;

    return result;
    }

/* Public methods end */

/* EventService Methods End */

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRevisionsInfoTaskPtr DgnDbRepositoryConnection::RevisionsFromQuery
(
const WebServices::WSQuery& query,
bool                        parseFileAccessKey,
ICancellationTokenPtr       cancellationToken
) const
    {
    return ExecutionManager::ExecuteWithRetry<bvector<DgnDbServerRevisionInfoPtr>>([=]()
        {
        return RevisionsFromQueryInternal(query, parseFileAccessKey, cancellationToken);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRevisionsInfoTaskPtr DgnDbRepositoryConnection::RevisionsFromQueryInternal
(
const WebServices::WSQuery& query,
bool                        parseFileAccessKey,
ICancellationTokenPtr       cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::RevisionsFromQuery";
    return m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)->Then<DgnDbServerRevisionsInfoResult>
        ([=](const WSObjectsResult& revisionsInfoResult)
        {
        if (revisionsInfoResult.IsSuccess())
            {
            bvector<DgnDbServerRevisionInfoPtr> indexedRevisions;
            if (!revisionsInfoResult.GetValue().GetRapidJsonDocument().IsNull())
                {
                for (auto const& value : revisionsInfoResult.GetValue().GetInstances())
                    {
                    auto revisionInfo = DgnDbServerRevisionInfo::Parse(value);
                    if (parseFileAccessKey)
                        {
                        auto fileAccessKey = DgnDbServerFileAccessKey::ParseFromRelated(value);
                        revisionInfo->SetFileAccessKey(fileAccessKey);
                        }

                    indexedRevisions.push_back(revisionInfo);
                    }
                }

            std::sort(indexedRevisions.begin(), indexedRevisions.end(), [](DgnDbServerRevisionInfoPtr a, DgnDbServerRevisionInfoPtr b)
                {
                return a->GetIndex() < b->GetIndex();
                });
            return DgnDbServerRevisionsInfoResult::Success(indexedRevisions);
            }
        else
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, revisionsInfoResult.GetError().GetMessage().c_str());
            return DgnDbServerRevisionsInfoResult::Error(revisionsInfoResult.GetError());
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
WSQuery DgnDbRepositoryConnection::CreateRevisionsAfterIdQuery
(
Utf8StringCR          revisionId,
BeGuidCR              fileId
) const
    {
    WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::Revision);
    BeGuid id = fileId;
    Utf8String queryFilter;
    
    if (Utf8String::IsNullOrEmpty(revisionId.c_str()))
        {
        if (id.IsValid())
            queryFilter.Sprintf("%s+eq+'%s'", ServerSchema::Property::MasterFileId, id.ToString().c_str());
        }
    else
        {
        if (id.IsValid())
            queryFilter.Sprintf("%s-backward-%s.%s+eq+'%s'+and+%s+eq+'%s'", ServerSchema::Relationship::FollowingRevision, ServerSchema::Class::Revision,
                ServerSchema::Property::Id, revisionId.c_str(),
                ServerSchema::Property::MasterFileId, id.ToString().c_str());
        else
            queryFilter.Sprintf("%s-backward-%s.%s+eq+'%s'", ServerSchema::Relationship::FollowingRevision, ServerSchema::Class::Revision,
                ServerSchema::Property::Id, revisionId.c_str());
        }

    query.SetFilter(queryFilter);

    return query;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
DgnDbServerRevisionsInfoTaskPtr DgnDbRepositoryConnection::GetRevisionsInternal
(
const WebServices::WSQuery& query,
bool                        parseFileAccessKey,
ICancellationTokenPtr       cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::GetRevisionsAfterId";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    std::shared_ptr<DgnDbServerRevisionsInfoResult> finalResult = std::make_shared<DgnDbServerRevisionsInfoResult>();

    return ExecutionManager::ExecuteWithRetry<bvector<DgnDbServerRevisionInfoPtr>>([=]()
        {
        return RevisionsFromQueryInternal(query, parseFileAccessKey, cancellationToken)->Then([=](DgnDbServerRevisionsInfoResultCR revisionsResult)
            {
            if (revisionsResult.IsSuccess())
                {
                finalResult->SetSuccess(revisionsResult.GetValue());
                double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
                }
            else
                {
                finalResult->SetError(revisionsResult.GetError());
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, revisionsResult.GetError().GetMessage().c_str());
                }
            })->Then<DgnDbServerRevisionsInfoResult>([=]()
                {
                return *finalResult;
                });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
DgnDbServerRevisionsInfoTaskPtr DgnDbRepositoryConnection::GetRevisionsAfterIdInternal
(
Utf8StringCR          revisionId,
BeGuidCR              fileId,
bool                  loadAccessKey,
ICancellationTokenPtr cancellationToken
) const
    {
    auto query = CreateRevisionsAfterIdQuery(revisionId, fileId);

    if (loadAccessKey)
        {
        Utf8String selectString;
        selectString.Sprintf("%s,%s,%s,%s", ServerSchema::Property::Id, ServerSchema::Property::Index, ServerSchema::Property::ParentId, ServerSchema::Property::MasterFileId);
        DgnDbServerFileAccessKey::AddDownloadAccessKeySelect(selectString);
        query.SetSelect(selectString);
        }

    return GetRevisionsInternal(query, loadAccessKey, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
DgnDbServerRevisionsInfoTaskPtr DgnDbRepositoryConnection::GetRevisionsAfterId
(
Utf8StringCR          revisionId,
BeGuidCR              fileId,
ICancellationTokenPtr cancellationToken
) const
    {
    return GetRevisionsAfterIdInternal(revisionId, fileId, false, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
WSQuery DgnDbRepositoryConnection::CreateRevisionsByIdQuery
(
std::deque<ObjectId>& revisionIds
) const
    {
    WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::Revision);
    query.AddFilterIdsIn(revisionIds, nullptr, 0, 0);
    return query;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
DgnRevisionsTaskPtr DgnDbRepositoryConnection::DownloadRevisions(const bvector<Utf8String>& revisionIds, Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    std::deque<ObjectId> queryIds;
    for (auto revisionId : revisionIds)
        {
        queryIds.push_back(ObjectId(ServerSchema::Schema::Repository, ServerSchema::Class::Revision, revisionId));
        }

    return DownloadRevisions(queryIds, callback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
DgnRevisionsTaskPtr DgnDbRepositoryConnection::DownloadRevisions(const bvector<DgnDbServerRevisionInfoPtr>& revisions, Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    std::deque<ObjectId> queryIds;
    for (auto revision : revisions)
        {
        queryIds.push_back(ObjectId(ServerSchema::Schema::Repository, ServerSchema::Class::Revision, revision->GetId()));
        }

    return DownloadRevisions(queryIds, callback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
DgnRevisionsTaskPtr DgnDbRepositoryConnection::DownloadRevisions(std::deque<ObjectId>& revisionIds, Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    auto query = CreateRevisionsByIdQuery(revisionIds);

    Utf8String selectString;
    selectString.Sprintf("%s,%s,%s,%s", ServerSchema::Property::Id, ServerSchema::Property::Index, ServerSchema::Property::ParentId, ServerSchema::Property::MasterFileId);
    DgnDbServerFileAccessKey::AddDownloadAccessKeySelect(selectString);
    query.SetSelect(selectString);

    auto revisionsQueryResult = GetRevisionsInternal(query, true, cancellationToken)->GetResult();
    if (!revisionsQueryResult.IsSuccess())
        return CreateCompletedAsyncTask(DgnRevisionsResult::Error(revisionsQueryResult.GetError()));
    
    return DownloadRevisionsInternal(revisionsQueryResult.GetValue(), callback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
DgnRevisionsTaskPtr DgnDbRepositoryConnection::DownloadRevisionsInternal
(
const bvector<DgnDbServerRevisionInfoPtr>& revisions,
Http::Request::ProgressCallbackCR      callback,
ICancellationTokenPtr                  cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::DownloadRevisions";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    bset<std::shared_ptr<AsyncTask>> tasks;
    bmap<Utf8String, int64_t> revisionIdIndexMap;
    for (auto& revision : revisions)
        {
        tasks.insert(DownloadRevisionFile(revision, callback, cancellationToken));
        revisionIdIndexMap.Insert(revision->GetId(), revision->GetIndex());
        }

    return AsyncTask::WhenAll(tasks)->Then<DgnRevisionsResult>([=] ()
        {
        DgnRevisions resultRevisions;
        for (auto task : tasks)
            {
            auto result = dynamic_pointer_cast<DgnRevisionTask>(task)->GetResult();
            if (!result.IsSuccess())
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                return DgnRevisionsResult::Error(result.GetError());
                }
            resultRevisions.push_back(result.GetValue());
            }

        std::sort(resultRevisions.begin(), resultRevisions.end(), [revisionIdIndexMap](DgnRevisionPtr a, DgnRevisionPtr b)
            {
            auto itemA = revisionIdIndexMap.find(a->GetId());
            auto itemB = revisionIdIndexMap.find(b->GetId());

            if (revisionIdIndexMap.end() == itemA || revisionIdIndexMap.end() == itemB)
                {
                BeAssert(false && "Revision not found in the map.");
                return true;
                }

            return itemA->second < itemB->second;
            });

        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
        return DgnRevisionsResult::Success(resultRevisions);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnRevisionsTaskPtr DgnDbRepositoryConnection::DownloadRevisionsAfterId
(
Utf8StringCR                        revisionId,
BeGuidCR                            fileId,
Http::Request::ProgressCallbackCR   callback,
ICancellationTokenPtr               cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::DownloadRevisionsAfterId";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    std::shared_ptr<DgnRevisionsResult> finalResult = std::make_shared<DgnRevisionsResult>();
    return GetRevisionsAfterIdInternal(revisionId, fileId, true, cancellationToken)->Then([=] (DgnDbServerRevisionsInfoResultCR revisionsResult)
        {
        if (revisionsResult.IsSuccess())
            {
            DownloadRevisionsInternal(revisionsResult.GetValue(), callback, cancellationToken)->Then([=](DgnRevisionsResultCR downloadResult)
                {
                if (downloadResult.IsSuccess())
                    {
                    double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
                    finalResult->SetSuccess(downloadResult.GetValue());
                    }
                else
                    {
                    DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, downloadResult.GetError().GetMessage().c_str());
                    finalResult->SetError(downloadResult.GetError());
                    }
                });
            }
        else
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, revisionsResult.GetError().GetMessage().c_str());
            finalResult->SetError(revisionsResult.GetError());
            }
        })->Then<DgnRevisionsResult>([=] ()
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
Json::Value PushRevisionJson
(
Dgn::DgnRevisionPtr revision,
BeBriefcaseId       briefcaseId,
bool                containsSchemaChanges
)
    {
    Json::Value pushRevisionJson = Json::objectValue;
    JsonValueR instance = pushRevisionJson[ServerSchema::Instance] = Json::objectValue;
    instance[ServerSchema::SchemaName] = ServerSchema::Schema::Repository;
    instance[ServerSchema::ClassName] = ServerSchema::Class::Revision;
    instance[ServerSchema::Properties] = Json::objectValue;

    JsonValueR properties = instance[ServerSchema::Properties];
    properties[ServerSchema::Property::Id] = revision->GetId();
    properties[ServerSchema::Property::Description] = revision->GetSummary();
    uint64_t size;
    revision->GetRevisionChangesFile().GetFileSize(size);
    properties[ServerSchema::Property::FileSize] = size;
    properties[ServerSchema::Property::ParentId] = revision->GetParentId();
    properties[ServerSchema::Property::MasterFileId] = revision->GetDbGuid();
    properties[ServerSchema::Property::BriefcaseId] = briefcaseId.GetValue ();
    properties[ServerSchema::Property::IsUploaded] = false;
    properties[ServerSchema::Property::ContainingChanges] = containsSchemaChanges ? 1 : 0;
    return pushRevisionJson;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              02/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::InitializeRevision
(
Dgn::DgnRevisionPtr             revision,
Dgn::DgnDbCR                    dgndb,
JsonValueR                      pushJson,
ObjectId                        revisionObjectId,
bool                            relinquishCodesLocks,
Http::Request::ProgressCallbackCR callback,
ICancellationTokenPtr           cancellationToken
) const
    {
    BeBriefcaseId briefcaseId = dgndb.GetBriefcaseId();
    std::shared_ptr<WSChangeset> changeset (new WSChangeset ());

    //Set Revision initialization request to ECChangeSet
    JsonValueR revisionProperties = pushJson[ServerSchema::Instance][ServerSchema::Properties];
    revisionProperties[ServerSchema::Property::IsUploaded] = true;
    changeset->AddInstance (revisionObjectId, WSChangeset::ChangeState::Modified, std::make_shared<Json::Value> (revisionProperties));

    //Set used locks to the ECChangeSet
    LockRequest usedLocks;
    usedLocks.FromRevision (*revision, dgndb);
    BeGuid masterFileId;
    masterFileId.FromString(revision->GetDbGuid().c_str());
    if (!usedLocks.IsEmpty ())
        SetLocksJsonRequestToChangeSet (usedLocks.GetLockSet (), briefcaseId, masterFileId, revision->GetId (), *changeset, WSChangeset::ChangeState::Modified, true);

    DgnCodeSet assignedCodes, discardedCodes;
    revision->ExtractCodes(assignedCodes, discardedCodes, dgndb);

    if (!assignedCodes.empty())
        {
        DgnCodeState state;
        state.SetUsed(revision->GetId());
        SetCodesJsonRequestToChangeSet(assignedCodes, state, briefcaseId, *changeset, WSChangeset::ChangeState::Modified);
        }

    if (!discardedCodes.empty())
        {
        DgnCodeState state;
        state.SetDiscarded(revision->GetId());
        SetCodesJsonRequestToChangeSet(discardedCodes, state, briefcaseId, *changeset, WSChangeset::ChangeState::Modified);
        }

    if (relinquishCodesLocks)
        {
        LockDeleteAllJsonRequest (changeset, briefcaseId);
        CodeDiscardReservedJsonRequest(changeset, briefcaseId);
        }

    //Push Revision initialization request and Locks update in a single batch
    const Utf8String methodName = "DgnDbRepositoryConnection::InitializeRevision";
    HttpStringBodyPtr request = HttpStringBody::Create(changeset->ToRequestString());

    std::shared_ptr<DgnDbServerStatusResult> finalResult = std::make_shared<DgnDbServerStatusResult>();

    auto requestOptions = std::make_shared<WSRepositoryClient::RequestOptions>();
    requestOptions->SetTransferTimeOut(WSRepositoryClient::Timeout::Transfer::LongUpload);

    return SendChangesetRequestInternal(changeset, IBriefcaseManager::ResponseOptions::None, cancellationToken, requestOptions)
        ->Then([=] (const DgnDbServerStatusResult& initializeRevisionResult)
        {
        if (initializeRevisionResult.IsSuccess())
            {
            finalResult->SetSuccess();
            return;
            }

        auto errorId = DgnDbServerError(initializeRevisionResult.GetError()).GetId();
        if (DgnDbServerError::Id::LockDoesNotExist != errorId && DgnDbServerError::Id::CodeDoesNotExist != errorId)
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, initializeRevisionResult.GetError().GetMessage().c_str());
            finalResult->SetError(initializeRevisionResult.GetError());
            return;
            }

        //Try to acquire all required locks and codes.
        DgnCodeSet codesToReserve = assignedCodes;
        codesToReserve.insert(discardedCodes.begin(), discardedCodes.end());

        AcquireCodesLocksInternal(usedLocks, codesToReserve, briefcaseId, masterFileId, revision->GetParentId(), IBriefcaseManager::ResponseOptions::None, cancellationToken)
            ->Then([=] (DgnDbServerStatusResultCR acquireCodesLocksResult)
            {
            if (!acquireCodesLocksResult.IsSuccess())
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, acquireCodesLocksResult.GetError().GetMessage().c_str());
                finalResult->SetError(acquireCodesLocksResult.GetError());
                return;
                }

            //Push Revision initialization request again.
            m_wsRepositoryClient->SendChangesetRequest(request, nullptr, cancellationToken)
                ->Then([=] (const WSChangesetResult& repeatInitializeRevisionResult)
                {
                if (repeatInitializeRevisionResult.IsSuccess())
                    finalResult->SetSuccess();
                else
                    {
                    DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, repeatInitializeRevisionResult.GetError().GetMessage().c_str());
                    finalResult->SetError(repeatInitializeRevisionResult.GetError());
                    }
                });
            });
        })->Then<DgnDbServerStatusResult>([=]
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::Push
(
Dgn::DgnRevisionPtr               revision,
Dgn::DgnDbCR                      dgndb,
bool                              relinquishCodesLocks,
Http::Request::ProgressCallbackCR callback,
ICancellationTokenPtr             cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::Push";
    DgnDbCP pDgnDb = &dgndb;

    // Stage 1. Create revision.
    std::shared_ptr<Json::Value> pushJson = std::make_shared<Json::Value>(PushRevisionJson(revision, dgndb.GetBriefcaseId(), revision->ContainsSchemaChanges(dgndb)));
    std::shared_ptr<DgnDbServerStatusResult> finalResult = std::make_shared<DgnDbServerStatusResult>();
    return ExecutionManager::ExecuteWithRetry<void>([=]()
        {
        return m_wsRepositoryClient->SendCreateObjectRequest(*pushJson, BeFileName(), callback, cancellationToken)
            ->Then([=] (const WSCreateObjectResult& initializePushResult)
            {
    #if defined (ENABLE_BIM_CRASH_TESTS)
            DgnDbServerBreakHelper::HitBreakpoint(DgnDbServerBreakpoints::DgnDbRepositoryConnection_AfterCreateRevisionRequest);
    #endif
            if (!initializePushResult.IsSuccess())
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, initializePushResult.GetError().GetMessage().c_str());
                finalResult->SetError(initializePushResult.GetError());
                return;
                }
        
            // Stage 2. Upload revision file. 
            Json::Value json;
            initializePushResult.GetValue().GetJson(json);
            JsonValueCR revisionInstance = json[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
            Utf8String  revisionInstanceId = revisionInstance[ServerSchema::InstanceId].asString();
            ObjectId    revisionObjectId   = ObjectId(ServerSchema::Schema::Repository, ServerSchema::Class::Revision, revisionInstanceId);
            auto fileAccessKey = DgnDbServerFileAccessKey::ParseFromRelated(revisionInstance);

            if (fileAccessKey.IsNull())
                {
                m_wsRepositoryClient->SendUpdateFileRequest(revisionObjectId, revision->GetRevisionChangesFile(), callback, cancellationToken)
                    ->Then([=] (const WSUpdateObjectResult& uploadRevisionResult)
                    {
    #if defined (ENABLE_BIM_CRASH_TESTS)
                    DgnDbServerBreakHelper::HitBreakpoint(DgnDbServerBreakpoints::DgnDbRepositoryConnection_AfterUploadRevisionFile);
    #endif
                    if (!uploadRevisionResult.IsSuccess())
                        {
                        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, uploadRevisionResult.GetError().GetMessage().c_str());
                        finalResult->SetError(uploadRevisionResult.GetError());
                        return;
                        }

                    // Stage 3. Initialize revision.
                    InitializeRevision(revision, *pDgnDb, *pushJson, revisionObjectId, relinquishCodesLocks, callback, cancellationToken)
                        ->Then([=] (DgnDbServerStatusResultCR result)
                        {
                        if (result.IsSuccess())
                            finalResult->SetSuccess();
                        else
                            {
                            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                            finalResult->SetError(result.GetError());
                            }
                        });
                    });
                }
            else
                {
                m_azureClient->SendUpdateFileRequest(fileAccessKey->GetUploadUrl(), revision->GetRevisionChangesFile(), callback, cancellationToken)
                    ->Then([=] (const AzureResult& result)
                    {
#if defined (ENABLE_BIM_CRASH_TESTS)
                    DgnDbServerBreakHelper::HitBreakpoint(DgnDbServerBreakpoints::DgnDbRepositoryConnection_AfterUploadRevisionFile);
#endif
                    if (!result.IsSuccess())
                        {
                        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                        finalResult->SetError(DgnDbServerError(result.GetError()));
                        return;
                        }

                    // Stage 3. Initialize revision.
                    InitializeRevision(revision, *pDgnDb, *pushJson, revisionObjectId, relinquishCodesLocks, callback, cancellationToken)
                        ->Then([=] (DgnDbServerStatusResultCR result)
                        {
                        if (result.IsSuccess())
                            finalResult->SetSuccess();
                        else
                            {
                            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                            finalResult->SetError(result.GetError());
                            }
                        });
                    });
                }
            })->Then<DgnDbServerStatusResult>([=]
                {
                return *finalResult;
                });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::VerifyConnection(ICancellationTokenPtr cancellationToken) const
    {
    return ExecutionManager::ExecuteWithRetry<void>([=]()
        {
        return m_wsRepositoryClient->VerifyAccess(cancellationToken)->Then<DgnDbServerStatusResult>([] (const AsyncResult<void, WSError>& result)
            {
            const Utf8String methodName = "DgnDbRepositoryConnection::VerifyConnection";
            DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
            if (result.IsSuccess())
                return DgnDbServerStatusResult::Success();
            else
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                return DgnDbServerStatusResult::Error(result.GetError());
                }
            });
        });
    }
