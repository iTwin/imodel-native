/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbRepositoryConnection.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbRepositoryConnection.h>
#include <DgnPlatform/RevisionManager.h>
#include <WebServices/Client/WSChangeset.h>
#include "DgnDbServerUtils.h"
#include <DgnDbServer/Client/DgnDbServerRevision.h>
#include <DgnDbServer/Client/Logging.h>
#include <DgnDbServer/Client/DgnDbServerBreakHelper.h>

USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
void DgnDbCodeLockSetResultInfo::AddLock(const DgnLock dgnLock, BeBriefcaseId briefcaseId, Utf8StringCR repositoryId)
    {
    m_locks.insert(dgnLock);
    AddLockInfoToList(m_lockStates, dgnLock, briefcaseId, repositoryId);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
const DgnLockSet& DgnDbCodeLockSetResultInfo::GetLocks() const { return m_locks; }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
const DgnLockInfoSet& DgnDbCodeLockSetResultInfo::GetLockStates() const { return m_lockStates; }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
void DgnDbCodeLockSetResultInfo::AddCode(const DgnCode dgnCode, DgnCodeState dgnCodeState, BeBriefcaseId briefcaseId, Utf8StringCR repositoryId)
    {
    m_codes.insert(dgnCode);
    AddCodeInfoToList(m_codeStates, dgnCode, dgnCodeState, briefcaseId, repositoryId);
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
    if (GetAuthorityId().GetValueUnchecked() != rhs.GetAuthorityId().GetValueUnchecked())
        return GetAuthorityId().GetValueUnchecked() < rhs.GetAuthorityId().GetValueUnchecked();

    int cmp = GetValuePattern().CompareTo(rhs.GetValuePattern());
    if (0 != cmp)
        return cmp < 0;

    cmp = GetValue().CompareTo(rhs.GetValue());
    if (0 != cmp)
        return cmp < 0;

    return GetNamespace().CompareTo(rhs.GetNamespace()) < 0;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas              08/2016
//---------------------------------------------------------------------------------------
void DgnDbCodeTemplateSetResultInfo::AddCodeTemplate(const DgnDbCodeTemplate codeTemplate)
    {
    m_codeTemplates.insert(codeTemplate);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas              08/2016
//---------------------------------------------------------------------------------------
const DgnDbCodeTemplateSet& DgnDbCodeTemplateSetResultInfo::GetTemplates() const
    {
    return m_codeTemplates;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
const DgnCodeSet& DgnDbCodeLockSetResultInfo::GetCodes() const { return m_codes; }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
const DgnCodeInfoSet& DgnDbCodeLockSetResultInfo::GetCodeStates() const { return m_codeStates; }

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
    m_wsRepositoryClient = WSRepositoryClient::Create(repository.GetServerURL(), repository.GetWSRepositoryName(), clientInfo, nullptr, customHandler);
    m_wsRepositoryClient->SetCredentials(credentials);
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
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid repository name.");
        return DgnDbRepositoryConnectionResult::Error(DgnDbServerError::Id::InvalidRepositoryName);
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
    #ifndef DEBUG
    if (Utf8String::npos != repositoryConnection->GetRepositoryInfo().GetServerURL().rfind("cloudapp.net"))
    #endif
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
            return DgnDbServerStatusResult::Error(DgnDbServerError::Id::InvalidBriefcase);
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
            JsonValueCR instance = result.GetValue().GetObject()[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
            finalResult->SetSuccess(FileInfo::FromJson(instance, fileInfo));
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
        m_wsRepositoryClient->SendQueryRequest(fileQuery, nullptr, nullptr, cancellationToken)->Then([=] (WSObjectsResult const& queryResult)
            {
            if (!queryResult.IsSuccess())
                {
                finalResult->SetError(queryResult.GetError());
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, queryResult.GetError().GetMessage().c_str());
                return;
                }
            JsonValueCR repositoryInstances = queryResult.GetValue().GetJsonValue()[ServerSchema::Instances];
            if (repositoryInstances.isArray())
                finalResult->SetSuccess(FileInfo::FromJson(repositoryInstances[0], fileInfo));
            else
                {
                finalResult->SetError(error);
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, error.GetMessage().c_str());
                }
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
    return m_wsRepositoryClient->SendUpdateObjectRequest(fileInfo.GetObjectId(), properties, nullptr, nullptr, cancellationToken)->Then<DgnDbServerStatusResult>
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
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid repository name.");
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::InvalidRepositoryName)); //NEEDSWORK
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
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::AzureFileUpload(BeFileNameCR filePath, Utf8StringCR url, Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::AzureFileUpload";
    IAzureBlobStorageClientPtr azureClient = AzureBlobStorageClient::Create();
    return azureClient->SendUpdateFileRequest(url, filePath, callback, cancellationToken)
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
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::UploadServerFile(BeFileNameCR filePath, FileInfoCR fileInfo, Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    if (fileInfo.GetFileURL().empty())
        return OnPremiseFileUpload(filePath, fileInfo.GetObjectId(), callback, cancellationToken);
    else
        return AzureFileUpload(filePath, fileInfo.GetFileURL(), callback, cancellationToken);
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

    return m_wsRepositoryClient->SendUpdateObjectRequest(fileInfo.GetObjectId(), fileProperties, nullptr, nullptr, cancellationToken)
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
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::LockRepository(BeGuidCR fileId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::LockRepository";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    FileInfoPtr fileInfo = FileInfo::Create(fileId);
    return ExecutionManager::ExecuteWithRetry<void>([=]()
        {
        return CreateNewServerFile(*fileInfo, cancellationToken)->Then<DgnDbServerStatusResult>([=](DgnDbServerFileResultCR result)
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
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
void DgnDbRepositoryConnection::WaitForInitializedBIMFile(BeGuid fileGuid, DgnDbServerFileResultPtr finalResult) const
    {
    bool fileInitialized = false;
    int retriesLeft = 100;
    const Utf8String methodName = "DgnDbClient::WaitForInitializedBIMFile";
    BeThreadUtilities::BeSleep(1000);

    while (!fileInitialized && retriesLeft > 0)
        {
        auto masterFilesResult = GetMasterFileById(fileGuid)->GetResult();
        if (!masterFilesResult.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, masterFilesResult.GetError().GetMessage().c_str());
            finalResult->SetError(masterFilesResult.GetError());
            return;
            }

        auto masterFile = masterFilesResult.GetValue();
        fileInitialized = masterFile->GetInitialized();
        
        if (!fileInitialized)
            BeThreadUtilities::BeSleep(200);
        retriesLeft--;
        }

    if (!fileInitialized)
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

            FileInfoPtr createdFileInfo = fileCreationResult.GetValue();
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
    filter.Sprintf("%s+eq+false", ServerSchema::Property::Initialized);
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
        
        auto instances = result.GetValue().GetJsonValue()[ServerSchema::Instances];
        if (!instances.isArray())
            {
            finalResult->SetError(DgnDbServerError::Id::FileDoesNotExist);
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File does not exist.");
            return;
            }
        auto fileInfo = FileInfo::FromJson(instances[0], FileInfo());
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
                files.push_back(FileInfo::FromJson(instance));
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

    std::shared_ptr<DgnDbServerHost> host = std::make_shared<DgnDbServerHost>();
    DgnDbServerHost::Adopt (host);

    Dgn::DgnDbPtr db = Dgn::DgnDb::OpenDgnDb (&status, filePath, Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite));
    DgnDbServerStatusResult result;
    if (BeSQLite::DbResult::BE_SQLITE_OK == status && db.IsValid())
        {
        result = m_repositoryInfo.WriteRepositoryInfo (*db, briefcaseId);
#if defined (ENABLE_BIM_CRASH_TESTS)
        DgnDbServerBreakHelper::HitBreakpoint(DgnDbServerBreakpoints::DgnDbRepositoryConnection_AfterWriteRepositoryInfo);
#endif
        db->CloseDb ();
        }
    else
        {
        auto error = DgnDbServerError(db, status);
        result = DgnDbServerStatusResult::Error(error);
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, error.GetMessage().c_str());
        if (db.IsValid())
            db->CloseDb();
        }

    DgnDbServerHost::Forget(host, true);
    return result;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::DownloadMasterFile
(
BeFileName                      localFile,
FileInfoCR                      fileInfo,
Http::Request::ProgressCallbackCR callback,
ICancellationTokenPtr           cancellationToken
) const
    {
    if (fileInfo.GetFileURL().empty())
        {
        return m_wsRepositoryClient->SendGetFileRequest(fileInfo.GetObjectId(), localFile, nullptr, callback, cancellationToken)
            ->Then<DgnDbServerStatusResult>([=] (const WSFileResult& fileResult)
            {
            if (!fileResult.IsSuccess())
                return DgnDbServerStatusResult::Error(fileResult.GetError());
            return DgnDbServerStatusResult::Success();
            });
        }
    else
        {
        // Download file directly from the url.
        return m_azureClient->SendGetFileRequest(fileInfo.GetFileURL(), localFile, callback, cancellationToken)
            ->Then<DgnDbServerStatusResult>([=] (const AzureResult& result)
            {
            if (!result.IsSuccess())
                return DgnDbServerStatusResult::Error(DgnDbServerError(result.GetError()));
            return DgnDbServerStatusResult::Success();
            });
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::DownloadBriefcaseFile
(
BeFileName                      localFile,
BeBriefcaseId                   briefcaseId,
Utf8StringCR                    url,
Http::Request::ProgressCallbackCR callback,
ICancellationTokenPtr           cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::DownloadBriefcaseFile";
    if (url.empty())
        {
        Utf8String instanceId;
        instanceId.Sprintf("%u", briefcaseId.GetValue());
        ObjectId fileObject(ServerSchema::Schema::Repository, ServerSchema::Class::Briefcase, instanceId);
        return m_wsRepositoryClient->SendGetFileRequest(fileObject, localFile, nullptr, callback, cancellationToken)
            ->Then<DgnDbServerStatusResult>([=] (const WSFileResult& fileResult)
            {
            if (fileResult.IsSuccess())
                return WriteBriefcaseIdIntoFile(fileResult.GetValue().GetFilePath(), briefcaseId);
            else
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, fileResult.GetError().GetMessage().c_str());
                return DgnDbServerStatusResult::Error(fileResult.GetError());
                }
            });
        }
    else
        {
        // Download file directly from the url.
        return m_azureClient->SendGetFileRequest(url, localFile, callback, cancellationToken)
            ->Then<DgnDbServerStatusResult>([=] (const AzureResult& result)
            {
            if (result.IsSuccess())
                return WriteBriefcaseIdIntoFile(localFile, briefcaseId);
            else
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                return DgnDbServerStatusResult::Error(DgnDbServerError(result.GetError()));
                }
            });
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::DownloadRevisionFile
(
DgnDbServerRevisionPtr          revision,
Http::Request::ProgressCallbackCR callback,
ICancellationTokenPtr           cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::DownloadRevisionFile";
    ObjectId fileObject(ServerSchema::Schema::Repository, ServerSchema::Class::Revision, revision->GetRevision()->GetId());
    
    if (revision->GetURL().empty())
        {
        return m_wsRepositoryClient->SendGetFileRequest(fileObject, revision->GetRevision()->GetRevisionChangesFile(), nullptr, callback, cancellationToken)
            ->Then<DgnDbServerStatusResult>([=](const WSFileResult& fileResult)
            {
            if (fileResult.IsSuccess())
                return DgnDbServerStatusResult::Success();
            else
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, fileResult.GetError().GetMessage().c_str());
                return DgnDbServerStatusResult::Error(fileResult.GetError());
                }
            });
        }
    else
        {
        // Download file directly from the url.
        return m_azureClient->SendGetFileRequest(revision->GetURL(), revision->GetRevision()->GetRevisionChangesFile(), nullptr, cancellationToken)
            ->Then<DgnDbServerStatusResult>([=] (const AzureResult& result)
            {
            if (result.IsSuccess())
                return DgnDbServerStatusResult::Success();
            else
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                return DgnDbServerStatusResult::Error(DgnDbServerError(result.GetError()));
                }
            });
        }
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
    bvector<uint64_t> objects[9];
    for (auto& lock : locks)
        {
        if (includeOnlyExclusive && LockLevel::Exclusive != lock.GetLevel ())
            continue;

        int index = static_cast<int32_t>(lock.GetType ()) * 3 + static_cast<int32_t>(lock.GetLevel ());
        if (index >= 0 && index <= 8)
            objects[index].push_back (lock.GetId ().GetValue ());
        }

    for (int i = 0; i < 9; ++i)
        AddToInstance(changeset, changeState, objects[i], briefcaseId, masterFileId, releasedWithRevisionId, static_cast<LockableType>(i / 3), static_cast<LockLevel>(i % 3), queryOnly);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
void CodeStateToReservedUsed(DgnCodeStateCR state, bool* reserved, bool* used)
    {
    //NEEDSWORK: Make DgnCodeState::Type public
    if (state.IsReserved())
        {
        *reserved = true;
        *used     = false;
        return;
        }
    if (state.IsUsed())
        {
        *reserved = false;
        *used     = true;
        return;
        }
    
    *reserved = false;
    *used     = false;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
Json::Value CreateCodeInstanceJson
(
bvector<DgnCode> const&      codes,
DgnCodeStateCR               state,
BeBriefcaseId                briefcaseId,
BeGuidCR                     masterFileId,
Utf8StringCR                 stateRevisionId,
bool                         queryOnly
)
    {
    Json::Value properties;
    DgnCode const* firstCode = codes.begin();

    bool reserved, used;
    CodeStateToReservedUsed(state, &reserved, &used);

    properties[ServerSchema::Property::AuthorityId]   = firstCode->GetAuthority().GetValue();
    properties[ServerSchema::Property::Namespace]     = firstCode->GetNamespace();
    properties[ServerSchema::Property::BriefcaseId]   = briefcaseId.GetValue();
    properties[ServerSchema::Property::Reserved]      = reserved;
    properties[ServerSchema::Property::Used]          = used;
    properties[ServerSchema::Property::MasterFileId]  = masterFileId.ToString();
    properties[ServerSchema::Property::StateRevision] = stateRevisionId;
    properties[ServerSchema::Property::QueryOnly]     = queryOnly;

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
uint64_t                         authorityId,
Utf8StringCR                     nameSpace,
Utf8StringCR                     value
)
    {
    Utf8String idString;

    Utf8String encodedNamespace(nameSpace.c_str());
    EncodeIdString(encodedNamespace);
    encodedNamespace = UriEncode(encodedNamespace);

    Utf8String encodedValue(value.c_str());
    EncodeIdString(encodedValue);
    encodedValue = UriEncode(encodedValue);
    
    idString.Sprintf("%d-%s-%s", authorityId, encodedNamespace.c_str(), encodedValue.c_str());

    return idString;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
Utf8String FormatCodeId
(
uint64_t                         authorityId,
Utf8StringCR                     nameSpace,
Utf8StringCR                     value,
BeBriefcaseId                    briefcaseId
)
    {
    Utf8String idString;
    idString.Sprintf("%s-%d", FormatCodeId(authorityId, nameSpace, value).c_str(), briefcaseId.GetValue());

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
BeGuidCR                         masterFileId,
Utf8StringCR                     stateRevisionId,
bool                             queryOnly
)
    {
    if (codes.empty())
        return;

    ObjectId codeObject(ServerSchema::Schema::Repository, ServerSchema::Class::MultiCode, "MultiCode");
    JsonValueCR codeJson = CreateCodeInstanceJson(codes, state, briefcaseId, masterFileId, stateRevisionId, queryOnly);
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
    Utf8String searchKey = FormatCodeId(searchCode.GetAuthority().GetValue(), searchCode.GetNamespace(), "");
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
BeGuidCR                        masterFileId,
Utf8StringCR                    stateRevisionId,
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
        AddCodeToInstance(changeset, changeState, group.second, state, briefcaseId, masterFileId, stateRevisionId, queryOnly);
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
Json::Value CreateCodeTemplateJson
(
    const DgnDbCodeTemplate      codeTemplate,
    DgnDbCodeTemplate::Type      templateType,
    int                          startIndex,
    int                          incrementBy
)
    {
    Json::Value properties;

    properties[ServerSchema::Property::AuthorityId] = codeTemplate.GetAuthorityId().GetValue();
    properties[ServerSchema::Property::Namespace] = codeTemplate.GetNamespace();
    properties[ServerSchema::Property::ValuePattern] = codeTemplate.GetValuePattern();
    properties[ServerSchema::Property::Type] = (int)templateType;

    if (startIndex >= 0 && incrementBy > 0)
        {
        properties[ServerSchema::Property::StartIndex] = startIndex;
        properties[ServerSchema::Property::IncrementBy] = incrementBy;
        }

    return properties;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             08/2016
//---------------------------------------------------------------------------------------
void SetCodeTemplatesJsonRequestToChangeSet
(
    const DgnDbCodeTemplateSet      templates,
    const DgnDbCodeTemplate::Type   templateType,
    WSChangeset&                    changeset,
    const WSChangeset::ChangeState& changeState,
    int                             startIndex = -1,
    int                             incrementBy = -1
)
    {
    ObjectId codeObject(ServerSchema::Schema::Repository, ServerSchema::Class::CodeTemplate, "");
    
    for (auto& codeTemplate : templates)
        {
        JsonValueCR codeJson = CreateCodeTemplateJson(codeTemplate, templateType, startIndex, incrementBy);
        changeset.AddInstance(codeObject, changeState, std::make_shared<Json::Value>(codeJson));
        }
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
        idString.Sprintf("%s", FormatCodeId(code.GetAuthority().GetValue(), code.GetNamespace(), code.GetValue(), *briefcaseId).c_str());
    else
        idString.Sprintf("%s", FormatCodeId(code.GetAuthority().GetValue(), code.GetNamespace(), code.GetValue()).c_str());

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
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::SendChangesetRequest";

    changeset->GetRequestOptions().SetResponseContent(WSChangeset::Options::ResponseContent::Empty);

    if (IBriefcaseManager::ResponseOptions::None == options || IBriefcaseManager::ResponseOptions::LockState != options)
        changeset->GetRequestOptions().SetCustomOption(ServerSchema::ExtendedParameters::DetailedError_Locks, "false");

    if (IBriefcaseManager::ResponseOptions::None == options || IBriefcaseManager::ResponseOptions::CodeState != options)
        changeset->GetRequestOptions().SetCustomOption(ServerSchema::ExtendedParameters::DetailedError_Codes, "false");

    HttpStringBodyPtr request = HttpStringBody::Create(changeset->ToRequestString());
    return m_wsRepositoryClient->SendChangesetRequest(request, nullptr, cancellationToken)->Then<DgnDbServerStatusResult>
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
    SetCodesJsonRequestToChangeSet(codes, state, briefcaseId, masterFileId, lastRevisionId, *changeset, WSChangeset::ChangeState::Created);

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
    SetCodesJsonRequestToChangeSet(codes, state, briefcaseId, masterFileId, lastRevisionId, *changeset, WSChangeset::ChangeState::Created, true);

    return SendChangesetRequest(changeset, options, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
IWSRepositoryClientPtr DgnDbRepositoryConnection::GetRepositoryClient()
    {
    return m_wsRepositoryClient;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
void DgnDbRepositoryConnection::SetRepositoryClient(IWSRepositoryClientPtr client)
    {
    m_wsRepositoryClient.swap(client);
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
    SetCodesJsonRequestToChangeSet(codes, state, briefcaseId, masterFileId, "", *changeset, WSChangeset::ChangeState::Modified);

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
            return DgnDbServerBriefcaseInfoResult::Error(DgnDbServerError::Id::InvalidBriefcase);
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

    auto finalValue = std::make_shared<bvector<std::shared_ptr<DgnDbServerBriefcaseInfo>>>();

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

            bvector<std::shared_ptr<DgnDbServerBriefcaseInfo>> briefcases;
            for (auto& value : result.GetValue().GetJsonValue()[ServerSchema::Instances])
                {
                briefcases.push_back(DgnDbServerBriefcaseInfo::FromJson(value));
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

    std::set<Utf8String> classes;
    classes.insert(ServerSchema::Class::Lock);
    classes.insert(ServerSchema::Class::Code);

    bset<DgnDbServerCodeLockSetTaskPtr> tasks;

    std::deque<ObjectId> queryIds;

    if (nullptr != codes)
        {
        for (auto& code : *codes)
            queryIds.push_back(GetCodeId(code, briefcaseId));
        }

    if (nullptr != locks)
        {
        for (auto& lock : *locks)
            queryIds.push_back(GetLockId(lock, briefcaseId));
        }

    if (nullptr != briefcaseId && queryIds.empty())
        {
        Utf8String filter;
        filter.Sprintf("(%s+eq+%u)", ServerSchema::Property::BriefcaseId, briefcaseId->GetValue());

        WSQuery query(ServerSchema::Schema::Repository, classes);
        query.SetFilter(filter);
        auto task = QueryCodesLocksInternal(query, cancellationToken);

        tasks.insert(task);
        }

    while (!queryIds.empty())
        {
        WSQuery query(ServerSchema::Schema::Repository, classes);

        query.AddFilterIdsIn(queryIds);
        auto task = QueryCodesLocksInternal(query, cancellationToken);

        tasks.insert(task);
        }

    auto finalValue = std::make_shared<DgnDbCodeLockSetResultInfo>();

    return AsyncTask::WhenAll(tasks)
        ->Then<DgnDbServerCodeLockSetResult>([=]
        {
        for (auto& task : tasks)
            {
            if (!task->GetResult().IsSuccess())
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, task->GetResult().GetError().GetMessage().c_str());
                return DgnDbServerCodeLockSetResult::Error(task->GetResult().GetError());
                }

            auto CodeLockSet = task->GetResult().GetValue();
            finalValue->Insert(CodeLockSet);
            }

        return DgnDbServerCodeLockSetResult::Success(*finalValue);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerCodeLockSetTaskPtr DgnDbRepositoryConnection::QueryCodesLocksInternal
(
WSQuery query,
ICancellationTokenPtr cancellationToken
) const
    {
    return ExecutionManager::ExecuteWithRetry<DgnDbCodeLockSetResultInfo>([=]()
        {
        //Execute query
        return m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)->Then<DgnDbServerCodeLockSetResult>
            ([=] (const WSObjectsResult& result)
            {
            if (result.IsSuccess())
                {
                DgnDbCodeLockSetResultInfo codesLocks;
                for (auto& value : result.GetValue().GetJsonValue()[ServerSchema::Instances])
                    {
                    DgnCode        code;
                    DgnCodeState   codeState;
                    DgnLock        lock;
                    BeBriefcaseId  briefcaseId;
                    Utf8String     repositoryId;
                    if (GetLockFromServerJson(value[ServerSchema::Properties], lock, briefcaseId, repositoryId))
                        {
                        if (lock.GetLevel() != LockLevel::None)
                            codesLocks.AddLock(lock, briefcaseId, repositoryId);
                        }
                    else if (GetCodeFromServerJson(value[ServerSchema::Properties], code, codeState, briefcaseId, repositoryId))
                        {
                        codesLocks.AddCode(code, codeState, briefcaseId, repositoryId);
                        }
                    //NEEDSWORK: log an error
                    }

                return DgnDbServerCodeLockSetResult::Success(codesLocks);
                }
            else
                {
                return DgnDbServerCodeLockSetResult::Error(result.GetError());
                }
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikolinuas             08/2016
//---------------------------------------------------------------------------------------
inline const char * const BoolToString(bool b)
    {
    return b ? "true" : "false";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             06/2016
//---------------------------------------------------------------------------------------
WSQuery CreateUnavailableLocksQuery(const BeBriefcaseId briefcaseId, const uint64_t lastRevisionIndex)
    {
    std::set<Utf8String> classes;
    classes.insert(ServerSchema::Class::Lock);
    classes.insert(ServerSchema::Class::Code);

    WSQuery query(ServerSchema::Schema::Repository, classes);
    Utf8String filter;
    Utf8String locksFilter, codesFilter;
    locksFilter.Sprintf("%s+gt+%u+or+%s+gt+%u", ServerSchema::Property::LockLevel, LockLevel::None,
                        ServerSchema::Property::ReleasedWithRevisionIndex, lastRevisionIndex);

    //NEEDSWORK: Make DgnCodeState::Type public
    codesFilter.Sprintf("%s+eq+%s+or+%s+eq+%s+or+%s+gt+%u", ServerSchema::Property::Reserved, BoolToString(true),
                        ServerSchema::Property::Used, BoolToString(true), ServerSchema::Property::StateRevisionIndex, lastRevisionIndex);

    filter.Sprintf("%s+ne+%u+and+((%s)+or+(%s))", ServerSchema::Property::BriefcaseId,
                   briefcaseId.GetValue(), locksFilter.c_str(), codesFilter.c_str());

    query.SetFilter(filter);
    return query;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             06/2016
//---------------------------------------------------------------------------------------
void AddCodeLock(DgnDbCodeLockSetResultInfo& codesLocksSet, JsonValueCR codeLockJson)
    {
    DgnCode        code;
    DgnCodeState   codeState;
    DgnLock        lock;
    BeBriefcaseId  briefcase;
    Utf8String     repository;
    if (GetLockFromServerJson(codeLockJson[ServerSchema::Properties], lock, briefcase, repository))
        {
        codesLocksSet.AddLock(lock, briefcase, repository);
        }
    else if (GetCodeFromServerJson(codeLockJson[ServerSchema::Properties], code, codeState, briefcase, repository))
        {
        codesLocksSet.AddCode(code, codeState, briefcase, repository);
        }
    //NEEDSWORK: log an error
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
        return CreateCompletedAsyncTask<DgnDbServerCodeLockSetResult>(DgnDbServerCodeLockSetResult::Error(DgnDbServerError::Id::InvalidBriefcase));
        }
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    std::shared_ptr<DgnDbServerCodeLockSetResult> finalResult = std::make_shared<DgnDbServerCodeLockSetResult>();
    return ExecutionManager::ExecuteWithRetry<DgnDbCodeLockSetResultInfo>([=]()
        {
        return GetRevisionById(lastRevisionId, cancellationToken)->Then([=] (DgnDbServerRevisionResultCR revisionResult)
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

            auto query = CreateUnavailableLocksQuery(briefcaseId, revisionIndex);
            m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)->Then([=] (const WSObjectsResult& result)
                {
                if (!result.IsSuccess())
                    {
                    finalResult->SetError(result.GetError());
                    DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                    return;
                    }
                DgnDbCodeLockSetResultInfo codesLocks;
                for (auto const& value : result.GetValue().GetJsonValue()[ServerSchema::Instances])
                    {
                    AddCodeLock(codesLocks, value);
                    }
                finalResult->SetSuccess(codesLocks);
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
DgnDbServerCodeTemplateSetTaskPtr DgnDbRepositoryConnection::QueryCodeMaximumIndex
(
DgnDbCodeTemplateSet codeTemplates,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::QueryCodeMaximumIndex";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    std::shared_ptr<WSChangeset> changeset(new WSChangeset());

    SetCodeTemplatesJsonRequestToChangeSet(codeTemplates, DgnDbCodeTemplate::Type::Maximum, *changeset, WSChangeset::ChangeState::Created);
    
    auto requestString = changeset->ToRequestString();
    HttpStringBodyPtr request = HttpStringBody::Create(requestString);
    return ExecutionManager::ExecuteWithRetry<DgnDbCodeTemplateSetResultInfo>([=]()
        {
        return m_wsRepositoryClient->SendChangesetRequest(request, nullptr, cancellationToken)->Then<DgnDbServerCodeTemplateSetResult>
            ([=](const WSChangesetResult& result)
            {
            if (result.IsSuccess())
                {
                DgnDbCodeTemplateSetResultInfo templates;
                Json::Value ptr = GetChangedInstances(result.GetValue()->AsString().c_str());
            
                for (auto& value : ptr)
                    {
                    DgnDbCodeTemplate        codeTemplate;
                    if (GetCodeTemplateFromServerJson(value[ServerSchema::InstanceAfterChange][ServerSchema::Properties], codeTemplate))
                        {
                        templates.AddCodeTemplate(codeTemplate);
                        }
                    //NEEDSWORK: log an error
                    }
                double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
                return DgnDbServerCodeTemplateSetResult::Success(templates);
                }
            else
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                return DgnDbServerCodeTemplateSetResult::Error(result.GetError());
                }
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerCodeTemplateSetTaskPtr DgnDbRepositoryConnection::QueryCodeNextAvailable(DgnDbCodeTemplateSet codeTemplates, int startIndex, int incrementBy, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::QueryCodeNextAvailable";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    std::shared_ptr<WSChangeset> changeset(new WSChangeset());

    SetCodeTemplatesJsonRequestToChangeSet(codeTemplates, DgnDbCodeTemplate::Type::NextAvailable, *changeset, WSChangeset::ChangeState::Created, startIndex, incrementBy);

    HttpStringBodyPtr request = HttpStringBody::Create(changeset->ToRequestString());
    return ExecutionManager::ExecuteWithRetry<DgnDbCodeTemplateSetResultInfo>([=]()
        {
        return m_wsRepositoryClient->SendChangesetRequest(request, nullptr, cancellationToken)->Then<DgnDbServerCodeTemplateSetResult>
            ([=](const WSChangesetResult& result)
            {
            if (result.IsSuccess())
                {
                DgnDbCodeTemplateSetResultInfo templates;
                Json::Value ptr = GetChangedInstances(result.GetValue()->AsString().c_str());

                for (auto& value : ptr)
                    {
                    DgnDbCodeTemplate        codeTemplate;
                    if (GetCodeTemplateFromServerJson(value[ServerSchema::InstanceAfterChange][ServerSchema::Properties], codeTemplate))
                        {
                        templates.AddCodeTemplate(codeTemplate);
                        }
                    //NEEDSWORK: log an error
                    }
                double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
                return DgnDbServerCodeTemplateSetResult::Success(templates);
                }
            else
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                return DgnDbServerCodeTemplateSetResult::Error(result.GetError());
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

        JsonValueCR instance = result.GetValue().GetJsonValue()[ServerSchema::Instances][0];
        auto fileInfo = FileInfo::FromJson(instance);
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

            JsonValueCR instance = result.GetValue().GetObject()[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
            return DgnDbServerBriefcaseInfoResult::Success(DgnDbServerBriefcaseInfo::FromJson(instance));
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRevisionPtr ParseRevision (JsonValueCR jsonValue)
    {
    std::shared_ptr<DgnDbServerHost> host = std::make_shared<DgnDbServerHost>();
    DgnDbServerHost::Adopt(host);
    RevisionStatus status;
    DgnDbServerRevisionPtr indexedRevision = DgnDbServerRevision::Create(DgnRevision::Create(&status, jsonValue[ServerSchema::Property::Id].asString(),
    jsonValue[ServerSchema::Property::ParentId].asString(), jsonValue[ServerSchema::Property::MasterFileId].asString()));
    DgnDbServerHost::Forget(host);
    if (RevisionStatus::Success == status)
        {
        indexedRevision->GetRevision()->SetSummary(jsonValue[ServerSchema::Property::Description].asCString());
        DateTime pushDate = DateTime();
        DateTime::FromString(pushDate, jsonValue[ServerSchema::Property::PushDate].asCString());
        indexedRevision->GetRevision()->SetDateTime(pushDate);
        indexedRevision->GetRevision()->SetUserName(jsonValue[ServerSchema::Property::UserCreated].asCString());
        indexedRevision->SetIndex(jsonValue[ServerSchema::Property::Index].asUInt64());
        indexedRevision->SetURL(jsonValue["URL"].asString());
        }
    return indexedRevision;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              03/2016
//---------------------------------------------------------------------------------------
DgnDbServerRevisionsTaskPtr DgnDbRepositoryConnection::GetAllRevisions (ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::GetAllRevisions";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    BeAssert (DgnDbServerHost::IsInitialized ());
    WSQuery query (ServerSchema::Schema::Repository, ServerSchema::Class::Revision);
    return RevisionsFromQuery (query, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRevisionTaskPtr DgnDbRepositoryConnection::GetRevisionById
(
Utf8StringCR          revisionId,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::GetRevisionById";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    BeAssert(DgnDbServerHost::IsInitialized());
    if (revisionId.empty())
        {
        // Don't log error here since this is a valid case then there are no revisions locally.
        return CreateCompletedAsyncTask<DgnDbServerRevisionResult>(DgnDbServerRevisionResult::Error(DgnDbServerError::Id::InvalidRevision));
        }
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    ObjectId revisionObject(ServerSchema::Schema::Repository, ServerSchema::Class::Revision, revisionId);
    return m_wsRepositoryClient->SendGetObjectRequest(revisionObject, nullptr, cancellationToken)->Then<DgnDbServerRevisionResult>
        ([=] (WSObjectsResult& revisionResult)
        {
        if (revisionResult.IsSuccess())
            {
            auto revision = ParseRevision(revisionResult.GetValue().GetJsonValue()[ServerSchema::Instances][0][ServerSchema::Properties]);
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
            return DgnDbServerRevisionResult::Success(revision);
            }
        else
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, revisionResult.GetError().GetMessage().c_str());
            return DgnDbServerRevisionResult::Error(revisionResult.GetError());
            }
        });
    }

/* EventService Methods Begin */

/* Private methods start */

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            05/2016
//---------------------------------------------------------------------------------------
EventServiceClient* DgnDbRepositoryConnection::m_eventServiceClient = nullptr;

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Json::Value GenerateEventSubscriptionWSChangeSetJson(bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes)
    {
    Json::Value properties;
    properties[ServerSchema::Property::EventTypes] = Json::arrayValue;
    if (eventTypes != nullptr)
        {
        bmap<DgnDbServerEvent::DgnDbServerEventType, Utf8CP> repetitiveMap;
        for (int i = 0; i < (int) eventTypes->size(); i++)
            {
            //Check for repetition
            if (repetitiveMap.end() != repetitiveMap.find(eventTypes->at(i)))
                continue;
            repetitiveMap.Insert(eventTypes->at(i), "");
            properties[ServerSchema::Property::EventTypes][i] = DgnDbServerEvent::Helper::GetEventNameFromEventType(eventTypes->at(i));
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

    bvector<DgnDbServerEvent::DgnDbServerEventType> eventTypes;
    for (Json::ValueIterator itr = instance[ServerSchema::Properties][ServerSchema::Property::EventTypes].begin();
        itr != instance[ServerSchema::Properties][ServerSchema::Property::EventTypes].end(); ++itr)
        eventTypes.push_back(DgnDbServerEvent::Helper::GetEventTypeFromEventName((*itr).asString().c_str()));

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
bvector<DgnDbServerEvent::DgnDbServerEventType>* newEventTypes,
bvector<DgnDbServerEvent::DgnDbServerEventType> oldEventTypes
)
    {
    if (
        (newEventTypes == nullptr || newEventTypes->size() == 0) &&
        (oldEventTypes.size() == 0)
        )
        return true;
    else if (
        (newEventTypes != nullptr && newEventTypes->size() > 0) &&
        (oldEventTypes.size() > 0) &&
        (newEventTypes->size() == oldEventTypes.size())
        )
        {
        bmap<DgnDbServerEvent::DgnDbServerEventType, Utf8CP> oldEventTypesMap;
        for (int i = 0; i < (int) oldEventTypes.size(); i++)
            {
            oldEventTypesMap.Insert(oldEventTypes.at(i), "");
            }
        bool isSame = true;
        for (int j = 0; j < (int) newEventTypes->size(); j++)
            {
            //Item in newEventTypes not found in map
            if (oldEventTypesMap.end() == oldEventTypesMap.find(newEventTypes->at(j)))
                {
                isSame = false;
                break;
                }
            }
        return isSame;
        }
    else
        return false;

    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
bool DgnDbRepositoryConnection::SetEventSASToken(ICancellationTokenPtr cancellationToken)
    {
    auto sasToken = GetEventServiceSASToken(cancellationToken)->GetResult();
    if (!sasToken.IsSuccess())
        return false;

    m_eventSAS = sasToken.GetValue();
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
bool DgnDbRepositoryConnection::SetEventSubscription(bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes, ICancellationTokenPtr cancellationToken)
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
bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes,
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
            AzureServiceBusSASDTOPtr ptr = CreateEventSAS(result.GetValue().GetObject());
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
bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes,
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
bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes,
ICancellationTokenPtr cancellationToken
) const
    {
    std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    SetEventSubscriptionJsonRequestToChangeSet(eventTypes, "", *changeset, WSChangeset::Created);
    return SendEventChangesetRequest(changeset, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                    06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventSubscriptionTaskPtr DgnDbRepositoryConnection::UpdateEventServiceSubscriptionId
(
bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes,
ICancellationTokenPtr cancellationToken
) const
    {
    std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    SetEventSubscriptionJsonRequestToChangeSet(eventTypes, m_eventSubscription->GetSubscriptionId(), *changeset, WSChangeset::Modified);
    return SendEventChangesetRequest(changeset, cancellationToken);
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
bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes,
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

/* Public methods end */

/* EventService Methods End */

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRevisionsTaskPtr DgnDbRepositoryConnection::RevisionsFromQuery
(
const WebServices::WSQuery& query,
ICancellationTokenPtr       cancellationToken
) const
    {
    return ExecutionManager::ExecuteWithRetry<bvector<DgnDbServerRevisionPtr>>([=]()
        {
        return RevisionsFromQueryInternal(query, cancellationToken);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRevisionsTaskPtr DgnDbRepositoryConnection::RevisionsFromQueryInternal
(
const WebServices::WSQuery& query,
ICancellationTokenPtr       cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::RevisionsFromQuery";
    BeAssert(DgnDbServerHost::IsInitialized());
    return m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)->Then<DgnDbServerRevisionsResult>
        ([=](const WSObjectsResult& revisionsInfoResult)
        {
        if (revisionsInfoResult.IsSuccess())
            {
            bvector<DgnDbServerRevisionPtr> indexedRevisions;
            for (auto& value : revisionsInfoResult.GetValue().GetJsonValue()[ServerSchema::Instances])
                indexedRevisions.push_back(ParseRevision(value[ServerSchema::Properties]));
            std::sort(indexedRevisions.begin(), indexedRevisions.end(), [](DgnDbServerRevisionPtr a, DgnDbServerRevisionPtr b)
                {
                return a->GetIndex() < b->GetIndex();
                });
            return DgnDbServerRevisionsResult::Success(indexedRevisions);
            }
        else
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, revisionsInfoResult.GetError().GetMessage().c_str());
            return DgnDbServerRevisionsResult::Error(revisionsInfoResult.GetError());
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRevisionsTaskPtr DgnDbRepositoryConnection::GetRevisionsAfterId
(
Utf8StringCR          revisionId,
BeGuidCR              fileId,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::GetRevisionsAfterId";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    BeAssert(DgnDbServerHost::IsInitialized());
    std::shared_ptr<DgnDbServerRevisionsResult> finalResult = std::make_shared<DgnDbServerRevisionsResult>();
    return ExecutionManager::ExecuteWithRetry<bvector<DgnDbServerRevisionPtr>>([=]()
        {
        return GetRevisionById(revisionId, cancellationToken)->Then([=](DgnDbServerRevisionResultCR result)
            {
            if (!result.IsSuccess() && DgnDbServerError::Id::InvalidRevision != result.GetError().GetId())
                {
                finalResult->SetError(result.GetError());
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                }

            WSQuery query(ServerSchema::Schema::Repository, ServerSchema::Class::Revision);
            uint64_t index = 0;
            BeGuid id = fileId;
            if (result.IsSuccess())
                {
                auto revision = result.GetValue();
                index = revision->GetIndex() + 1;
                if (!id.IsValid())
                    {
                    Utf8String dbGuid = revision->GetRevision()->GetDbGuid();
                    id.FromString(dbGuid.c_str());
                    }
                }

            Utf8String queryFilter;
            if (id.IsValid())
                queryFilter.Sprintf("%s+ge+%llu+and+%s+eq+'%s'", ServerSchema::Property::Index, index,
                    ServerSchema::Property::MasterFileId, id.ToString().c_str());
            else
                queryFilter.Sprintf("%s+ge+%llu", ServerSchema::Property::Index, index);
            query.SetFilter(queryFilter);
            RevisionsFromQueryInternal(query, cancellationToken)->Then([=](DgnDbServerRevisionsResultCR revisionsResult)
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
                });
            })->Then<DgnDbServerRevisionsResult>([=]()
                {
                return *finalResult;
                });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbRepositoryConnection::DownloadRevisions
(
const bvector<DgnDbServerRevisionPtr>& revisions,
Http::Request::ProgressCallbackCR      callback,
ICancellationTokenPtr                  cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::DownloadRevisions";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    bset<std::shared_ptr<AsyncTask>> tasks;
    for (auto& revision : revisions)
        tasks.insert(DownloadRevisionFile(revision, callback, cancellationToken));
    return AsyncTask::WhenAll(tasks)->Then<DgnDbServerStatusResult>([=] ()
        {
        for (auto task : tasks)
            {
            auto result = dynamic_pointer_cast<DgnDbServerStatusTask>(task)->GetResult();
            if (!result.IsSuccess())
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                return DgnDbServerStatusResult::Error(result.GetError());
                }
            }
        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
        return DgnDbServerStatusResult::Success();
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRevisionsTaskPtr DgnDbRepositoryConnection::DownloadRevisionsAfterId
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
    std::shared_ptr<DgnDbServerRevisionsResult> finalResult = std::make_shared<DgnDbServerRevisionsResult>();
    return GetRevisionsAfterId(revisionId, fileId, cancellationToken)->Then([=] (DgnDbServerRevisionsResultCR revisionsResult)
        {
        if (revisionsResult.IsSuccess())
            {
            DownloadRevisions(revisionsResult.GetValue(), callback, cancellationToken)->Then([=](DgnDbServerStatusResultCR downloadResult)
                {
                if (downloadResult.IsSuccess())
                    {
                    double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
                    finalResult->SetSuccess(revisionsResult.GetValue());
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
        })->Then<DgnDbServerRevisionsResult>([=] ()
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
Json::Value PushRevisionJson
(
Dgn::DgnRevisionPtr            revision,
Utf8StringCR                   repositoryName,
BeBriefcaseId                  briefcaseId
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
        SetCodesJsonRequestToChangeSet(assignedCodes, state, briefcaseId, masterFileId, revision->GetId(), *changeset, WSChangeset::ChangeState::Modified);
        }

    if (!discardedCodes.empty())
        {
        DgnCodeState state;
        state.SetDiscarded(revision->GetId());
        SetCodesJsonRequestToChangeSet(discardedCodes, state, briefcaseId, masterFileId, revision->GetId(), *changeset, WSChangeset::ChangeState::Modified);
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
    return SendChangesetRequestInternal(changeset, IBriefcaseManager::ResponseOptions::None, cancellationToken)
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
Dgn::DgnRevisionPtr             revision,
Dgn::DgnDbCR                    dgndb,
bool                            relinquishCodesLocks,
Http::Request::ProgressCallbackCR callback,
ICancellationTokenPtr           cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbRepositoryConnection::Push";
    DgnDbCP pDgnDb = &dgndb;

    // Stage 1. Create revision.
    std::shared_ptr<Json::Value> pushJson = std::make_shared<Json::Value>(PushRevisionJson(revision, m_repositoryInfo.GetId(), dgndb.GetBriefcaseId()));
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
            JsonValueCR revisionInstance   = initializePushResult.GetValue().GetObject()[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
            Utf8String  revisionInstanceId = revisionInstance[ServerSchema::InstanceId].asString();
            ObjectId    revisionObjectId   = ObjectId(ServerSchema::Schema::Repository, ServerSchema::Class::Revision, revisionInstanceId);
            Utf8StringCR url = revisionInstance[ServerSchema::Properties][ServerSchema::Property::URL].asString();

            if (url.empty())
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
                m_azureClient->SendUpdateFileRequest(url, revision->GetRevisionChangesFile(), callback, cancellationToken)
                    ->Then([=] (const AzureResult& result)
                    {
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
RepositoryInfoCR DgnDbRepositoryConnection::GetRepositoryInfo() const
    {
    return m_repositoryInfo;
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
