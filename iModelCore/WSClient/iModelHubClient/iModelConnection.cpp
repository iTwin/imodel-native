/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/iModelConnection.h>
#include <DgnPlatform/RevisionManager.h>
#include <WebServices/Client/WSChangeset.h>
#include "Utils.h"
#include "Logging.h"
#include <WebServices/iModelHub/Client/BreakHelper.h>
#include <WebServices/iModelHub/Client/UserInfoManager.h>
#include "Events/EventCallbackManager.h"
#include <WebServices/iModelHub/Events/ChangeSetPostPushEvent.h>
#include <WebServices/iModelHub/Client/ChangeSetInfo.h>
#include <WebServices/iModelHub/Client/ChangeSetArguments.h>
#include <WebServices/iModelHub/Client/ChangeSetQuery.h>
#include "MultiProgressCallbackHandler.h"
#include "JsonFormatters/MultiLockFormatter.h"
#include "JsonFormatters/MultiCodeFormatter.h"
#include "JsonFormatters/ChangeSetFormatter.h"
#include "Storage/PendingChangeSetStorage.h"
#include "Events/EventManager.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGN

# define MAX_AsyncQueries 10

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
iModelConnection::iModelConnection
(
iModelInfoCR           iModel,
WebServices::CredentialsCR credentials,
WebServices::ClientInfoPtr clientInfo,
GlobalRequestOptionsPtr    globalRequestOptions,
IHttpHandlerPtr            customHandler,
bool                       enableCompression,
IAzureBlobStorageClientPtr storageClient
) : m_iModelInfo(iModel), m_customHandler(customHandler), m_globalRequestOptionsPtr(globalRequestOptions)
    {
    auto wsRepositoryClient = WSRepositoryClient::Create(iModel.GetServerURL(), ServerProperties::ServiceVersion(), iModel.GetWSRepositoryName(), clientInfo, nullptr, customHandler);
    if (enableCompression)
        {
        CompressionOptions options;
        options.EnableRequestCompression(true, 1024);
        wsRepositoryClient->Config().SetCompressionOptions(options);
        }
    wsRepositoryClient->SetCredentials(credentials);

    SetRepositoryClient(wsRepositoryClient);
    m_eventManagerPtr = new EventManager(wsRepositoryClient);
    if (!storageClient)
        storageClient = AzureBlobStorageClient::Create();
    SetAzureBlobStorageClient(storageClient);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
iModelConnection::iModelConnection
(
    iModelInfoCR           iModel,
    WebServices::CredentialsCR credentials,
    WebServices::ClientInfoPtr clientInfo,
    GlobalRequestOptionsPtr    globalRequestOptions,
    IHttpHandlerPtr            customHandler,
    bool                       enableCompression
) : iModelConnection(iModel, credentials, clientInfo, globalRequestOptions, customHandler, enableCompression, nullptr)
    {
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::DownloadFileInternal
(
BeFileName                        localFile,
ObjectIdCR                        fileId,
FileAccessKeyPtr                  fileAccessKey,
Http::Request::ProgressCallbackCR callback,
ICancellationTokenPtr             cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::DownloadFileInternal";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    // Download file directly from the url.
    return ExecuteWithRetry<void>([=]()
        {
        return m_azureClient->SendGetFileRequest(fileAccessKey->GetDownloadUrl(), localFile, callback, nullptr, cancellationToken)
            ->Then<StatusResult>([=](const AzureResult& result)
            {
            if (!result.IsSuccess())
                {
                HttpError error(result.GetError());
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, error.GetMessage().c_str());
                return StatusResult::Error(Error(error));
                }

            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
            return StatusResult::Success();
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::DownloadFile
(
BeFileName                        localFile,
ObjectIdCR                        fileId,
FileAccessKeyPtr                  fileAccessKey,
Http::Request::ProgressCallbackCR callback,
ICancellationTokenPtr             cancellationToken
) const
    {
    if (fileAccessKey.IsNull())
        {
        auto fileAccessKeyResult = ExecuteAsync(QueryFileAccessKey(fileId, cancellationToken));
        if (!fileAccessKeyResult->IsSuccess())
            return CreateCompletedAsyncTask(StatusResult::Error(fileAccessKeyResult->GetError()));

        fileAccessKey = fileAccessKeyResult->GetValue();
        }

    return DownloadFileInternal(localFile, fileId, fileAccessKey, callback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
FileAccessKeyTaskPtr iModelConnection::QueryFileAccessKey
(
ObjectId              objectId,
ICancellationTokenPtr cancellationToken
) const
    {
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    const Utf8String methodName = "iModelConnection::QueryFileAccessKey";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions,"Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    WSQuery query(objectId);
    Utf8String selectString = "$id";
    FileAccessKey::AddDownloadAccessKeySelect(selectString);
    query.SetSelect(selectString);

    return m_wsRepositoryClient->SendQueryRequestWithOptions(query, nullptr, nullptr, requestOptions, cancellationToken)
        ->Then<FileAccessKeyResult>([=](WSObjectsResult const& result)
        {
        if (!result.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, result.GetError().GetMessage().c_str());
            return FileAccessKeyResult::Error(result.GetError());
            }

        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), requestOptions, "");
        auto fileAccessKey = FileAccessKey::ParseFromRelated(*result.GetValue().GetInstances().begin());
        return FileAccessKeyResult::Success(fileAccessKey);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
Json::Value iModelConnection::CreateFileJson(FileInfoCR fileInfo)
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
FileTaskPtr iModelConnection::CreateNewServerFile(FileInfoCR fileInfo, ICancellationTokenPtr cancellationToken) const
    {
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    const Utf8String methodName = "iModelConnection::CreateNewServerFile";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");

    std::shared_ptr<FileResult> finalResult = std::make_shared<FileResult>();
    return m_wsRepositoryClient->SendCreateObjectRequestWithOptions(CreateFileJson(fileInfo), BeFileName(), nullptr, requestOptions, cancellationToken)->Then
    ([=](const WSCreateObjectResult& result)
        {
        if (result.IsSuccess())
            {
            Json::Value json;
            result.GetValue().GetJson(json);
            JsonValueCR instance = json[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
            auto fileInfoPtr = FileInfo::Parse(ToRapidJson(instance[ServerSchema::Properties]), fileInfo);
            fileInfoPtr->SetFileAccessKey(FileAccessKey::ParseFromRelated(instance));

            finalResult->SetSuccess(fileInfoPtr);
            return;
            }

        auto error = Error(result.GetError());
        if (Error::Id::FileAlreadyExists != error.GetId())
            {
            finalResult->SetError(error);
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, error.GetMessage().c_str());
            return;
            }

        bool initialized = error.GetExtendedData()[ServerSchema::Property::FileInitialized].asBool();

        if (initialized)
            {
            finalResult->SetError(error);
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, error.GetMessage().c_str());
            return;
            }

        WSQuery fileQuery(ServerSchema::Schema::iModel, ServerSchema::Class::File);
        Utf8String filter;
        filter.Sprintf("(%s+eq+'%s')+and+(%s+eq+'%s')", ServerSchema::Property::FileId, fileInfo.GetFileId().ToString().c_str(),
                       ServerSchema::Property::MergedChangeSetId, fileInfo.GetMergedChangeSetId().c_str());
        fileQuery.SetFilter(filter);

        Utf8String select("*");
        FileAccessKey::AddUploadAccessKeySelect(select);
        fileQuery.SetSelect(select);

        m_wsRepositoryClient->SendQueryRequestWithOptions(fileQuery, nullptr, nullptr, requestOptions, cancellationToken)->Then([=](WSObjectsResult const& queryResult)
            {
            if (!queryResult.IsSuccess())
                {
                finalResult->SetError(queryResult.GetError());
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, queryResult.GetError().GetMessage().c_str());
                return;
                }

            if (queryResult.GetValue().GetRapidJsonDocument().IsNull())
                {
                finalResult->SetError(error);
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, error.GetMessage().c_str());
                return;
                }

            auto resultJson = *queryResult.GetValue().GetInstances().begin();
            auto downloadFileResult = FileInfo::Parse(resultJson, fileInfo);
            downloadFileResult->SetFileAccessKey(FileAccessKey::ParseFromRelated(resultJson));

            finalResult->SetSuccess(downloadFileResult);
            });

        })->Then<FileResult>([=]()
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::AzureFileUpload(BeFileNameCR filePath, FileAccessKeyPtr url, Http::Request::ProgressCallbackCR callback, 
                                                ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::AzureFileUpload";
    return m_azureClient->SendUpdateFileRequest(url->GetUploadUrl(), filePath, callback, nullptr, cancellationToken)
        ->Then<StatusResult>([=](const AzureResult& result)
        {
        if (!result.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, result.GetError().GetMessage().c_str());
            return StatusResult::Error(result.GetError());
            }

        return StatusResult::Success();
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::UpdateServerFile(FileInfoCR fileInfo, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::UpdateServerFile";
    Json::Value properties = Json::objectValue;
    fileInfo.ToPropertiesJson(properties);

    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");

    return m_wsRepositoryClient->SendUpdateObjectRequestWithOptions(fileInfo.GetObjectId(), properties, nullptr, BeFileName(), nullptr, requestOptions, cancellationToken)
        ->Then<StatusResult>
        ([=](const WSUpdateObjectResult& result)
        {
        if (!result.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, result.GetError().GetMessage().c_str());
            return StatusResult::Error(result.GetError());
            }

        return StatusResult::Success();
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::InitializeServerFile(FileInfoCR fileInfo, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::InitializeServerFile";
    Json::Value fileProperties;
    fileInfo.ToPropertiesJson(fileProperties);
    fileProperties[ServerSchema::Property::IsUploaded] = true;

    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");

    return m_wsRepositoryClient->SendUpdateObjectRequestWithOptions(fileInfo.GetObjectId(), fileProperties, nullptr, BeFileName(), nullptr, requestOptions, cancellationToken)
        ->Then<StatusResult>([=](const WSUpdateObjectResult& initializeFileResult)
        {
        if (!initializeFileResult.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, initializeFileResult.GetError().GetMessage().c_str());
            return StatusResult::Error(initializeFileResult.GetError());
            }

        return StatusResult::Success();
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
FilesTaskPtr iModelConnection::SeedFilesQuery(WSQuery query, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::SeedFilesQuery";
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");

    return ExecuteWithRetry<bvector<FileInfoPtr>>([=]()
        {
        return m_wsRepositoryClient->SendQueryRequestWithOptions(query, nullptr, nullptr, requestOptions, cancellationToken)
            ->Then<FilesResult>([=](WSObjectsResult const& result)
            {
            if (!result.IsSuccess())
                return FilesResult::Error(result.GetError());
            bvector<FileInfoPtr> files;
            for (auto const& instance : result.GetValue().GetInstances())
                files.push_back(FileInfo::Parse(instance));
            return FilesResult::Success(files);
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
StatusResult iModelConnection::WriteBriefcaseIdIntoFile
(
BeFileName                     filePath,
BeBriefcaseId                  briefcaseId
) const
    {
    const Utf8String methodName = "iModelConnection::WriteBriefcaseIdIntoFile";
    BeSQLite::DbResult status;

    Dgn::DgnDbPtr db = Dgn::DgnDb::OpenDgnDb(&status, filePath, 
                                             Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes,
                                                                    SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck)));
    if (BeSQLite::DbResult::BE_SQLITE_OK == status && db.IsValid())
        {
        StatusResult result = m_iModelInfo.WriteiModelInfo(*db, briefcaseId);
#if defined (ENABLE_BIM_CRASH_TESTS)
        BreakHelper::HitBreakpoint(Breakpoints::iModelConnection_AfterWriteiModelInfo);
#endif
        db->CloseDb();
        return result;
        }
    else
        {
        auto error = Error(db, status);
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, error.GetMessage().c_str());
        if (db.IsValid())
            db->CloseDb();
        return StatusResult::Error(error);
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             03/2015
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::SendChangesetRequestWithRetry
(
std::shared_ptr<WSChangeset> changeset,
ChangesetResponseOptions const options,
ICancellationTokenPtr cancellationToken
) const
    {
    return ExecuteWithRetry<void>([=]() { return SendChangesetRequest(changeset, options, cancellationToken); });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             03/2015
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::SendChangesetRequest
(
std::shared_ptr<WSChangeset> changeset,
ChangesetResponseOptions const options,
ICancellationTokenPtr cancellationToken,
IWSRepositoryClient::RequestOptionsPtr requestOptions
) const
    {
    const Utf8String methodName = "iModelConnection::SendChangesetRequestInternal";

    changeset->GetRequestOptions().SetResponseContent(WSChangeset::Options::ResponseContent::Empty);
    m_globalRequestOptionsPtr->InsertRequestOptions(changeset);

    if (static_cast<bool>(options.GetBriefcaseResponseOptions() & IBriefcaseManager::ResponseOptions::UnlimitedReporting))
        changeset->GetRequestOptions().SetCustomOption(ServerSchema::ExtendedParameters::SetMaximumInstances, "-1");

    if (!static_cast<bool>(options.GetBriefcaseResponseOptions() & IBriefcaseManager::ResponseOptions::LockState))
        changeset->GetRequestOptions().SetCustomOption(ServerSchema::ExtendedParameters::DetailedError_Locks, "false");

    if (!static_cast<bool>(options.GetBriefcaseResponseOptions() & IBriefcaseManager::ResponseOptions::CodeState))
        changeset->GetRequestOptions().SetCustomOption(ServerSchema::ExtendedParameters::DetailedError_Codes, "false");

    if (options.GetConflictStrategy() == ConflictStrategy::Continue)
        changeset->GetRequestOptions().SetCustomOption(ServerSchema::ExtendedParameters::ConflictStrategy, "Continue");

    requestOptions = requestOptions ? requestOptions : std::make_shared<IWSRepositoryClient::RequestOptions>();
    LogHelper::FilliModelHubRequestOptions(requestOptions);
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");

    HttpStringBodyPtr request = HttpStringBody::Create(changeset->ToRequestString());
    return m_wsRepositoryClient->SendChangesetRequestWithOptions(request, nullptr, requestOptions, cancellationToken)->Then<StatusResult>
        ([=](const WSChangesetResult& result)
        {
        if (result.IsSuccess())
            return StatusResult::Success();
        else
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, result.GetError().GetMessage().c_str());
            return StatusResult::Error(result.GetError());
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas            03/2018
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::SendChunkedChangesetRequest
(
ChunkedWSChangeset const& chunkedChangeset,
ChangesetResponseOptions options,
ICancellationTokenPtr cancellationToken,
IWSRepositoryClient::RequestOptionsPtr requestOptions,
uint8_t const attemptsLimit,
ConflictsInfoPtr conflictsInfo
) const
    {
    StatusResultPtr finalResult = std::make_shared<StatusResult>();
    finalResult->SetSuccess();
    return SendChunkedChangesetRequestRecursive(chunkedChangeset, options, cancellationToken, requestOptions, 0, 1, attemptsLimit, finalResult, conflictsInfo);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas            03/2018
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::SendChunkedChangesetRequestRecursive
(
ChunkedWSChangeset const& chunkedChangeset,
ChangesetResponseOptions options,
ICancellationTokenPtr cancellationToken,
IWSRepositoryClient::RequestOptionsPtr requestOptions,
uint64_t changesetIndex,
uint8_t attempt,
uint8_t const attemptsLimit,
StatusResultPtr finalResult,
ConflictsInfoPtr conflictsInfo
) const
    {
    const Utf8String methodName = "iModelConnection::SendChunkedChangesetRequestRecursive";

    if (changesetIndex >= chunkedChangeset.GetChunks().size())
        {
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Finished sending chunked request");
        return CreateCompletedAsyncTask(*finalResult);
        }

    auto chunk = chunkedChangeset.GetChunks().at(changesetIndex);
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Starting sending chunk %d/%d", changesetIndex + 1, chunkedChangeset.GetChunks().size());

    return SendChangesetRequest(chunk, options, cancellationToken, requestOptions)->Then([=](StatusResultCR chunkResult)
        {
        uint64_t nextChangesetIndex = changesetIndex + 1;
        uint64_t nextAttempt = 1;

        if (!chunkResult.IsSuccess() && chunkResult.GetError().GetId() == Error::Id::ConflictsAggregate && nullptr != conflictsInfo)
            {
            IBriefcaseManager::Request request(IBriefcaseManager::ResponseOptions::All);
            auto response = ConvertErrorResponse(request, chunkResult, IBriefcaseManager::RequestPurpose::Acquire);
            conflictsInfo->AddFromResponse(response);
            }

        if (!chunkResult.IsSuccess() && chunkResult.GetError().GetId() != Error::Id::ConflictsAggregate)
            {
            if (attempt >= attemptsLimit || !IsErrorForRetry(chunkResult.GetError().GetId()))
                {
                finalResult->SetError(chunkResult.GetError());
                return;
                }

            // Retry same changeset
            nextChangesetIndex = changesetIndex;
            nextAttempt = attempt + 1;
            }

        SendChunkedChangesetRequestRecursive(chunkedChangeset, options, cancellationToken, requestOptions, 
            nextChangesetIndex, nextAttempt, attemptsLimit, finalResult, conflictsInfo)->Then([=](StatusResultCR chunkResult) {});

        })->Then<StatusResult>([=]
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
StatusResult iModelConnection::DownloadBriefcaseFile
(
BeFileName                        localFile,
BeBriefcaseId                     briefcaseId,
FileAccessKeyPtr                  fileAccessKey,
Http::Request::ProgressCallbackCR callback,
ICancellationTokenPtr             cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::DownloadBriefcaseFile";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    Utf8String instanceId;
    instanceId.Sprintf("%u", briefcaseId.GetValue());
    ObjectId fileObject(ServerSchema::Schema::iModel, ServerSchema::Class::Briefcase, instanceId);

    auto downloadResult = DownloadFile(localFile, fileObject, fileAccessKey, callback, cancellationToken)->GetResult();

    if (!downloadResult.IsSuccess())
        return downloadResult;

    return WriteBriefcaseIdIntoFile(localFile, briefcaseId);
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
DgnDbStatus CreateCodeSequenceJson
(
JsonValueR                   properties,
BeInt64Id                    codeSpecId,
Utf8StringCR                 codeScope,
Utf8StringCR                 valuePattern,
CodeSequence::Type           templateType,
int                          startIndex,
int                          incrementBy
)
    {
    properties[ServerSchema::Property::CodeSpecId] = FormatBeInt64Id(codeSpecId);
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
DgnDbStatus SetCodeSequencesJsonRequestToChangeSet
(
CodeSequenceCR                  codeSequence,
int                             startIndex,
int                             incrementBy,
const CodeSequence::Type        templateType,
WSChangeset&                    changeset,
const WSChangeset::ChangeState& changeState
)
    {
    ObjectId codeObject(ServerSchema::Schema::iModel, ServerSchema::Class::CodeSequence, "");

    Json::Value codeJson;
    auto status = CreateCodeSequenceJson(codeJson, codeSequence.GetCodeSpecId(), codeSequence.GetScope(), codeSequence.GetValuePattern(), 
                                         templateType, startIndex, incrementBy);
    if (DgnDbStatus::Success != status)
        return status;
    changeset.AddInstance(codeObject, changeState, std::make_shared<Json::Value>(codeJson));

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
void LockDeleteAllJsonRequest(std::shared_ptr<WSChangeset> changeSet, const BeBriefcaseId& briefcaseId)
    {
    Utf8String id;
    id.Sprintf("%s-%d", ServerSchema::DeleteAllLocks, briefcaseId.GetValue());

    ObjectId lockObject(ServerSchema::Schema::iModel, ServerSchema::Class::Lock, id);

    Json::Value properties;
    changeSet->AddInstance(lockObject, WSChangeset::ChangeState::Deleted, std::make_shared<Json::Value>(properties));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
void CodeDiscardReservedJsonRequest(std::shared_ptr<WSChangeset> changeSet, const BeBriefcaseId& briefcaseId)
    {
    Utf8String id;
    id.Sprintf("%s-%d", ServerSchema::DiscardReservedCodes, briefcaseId.GetValue());

    ObjectId codeObject(ServerSchema::Schema::iModel, ServerSchema::Class::Code, id);

    Json::Value properties;
    changeSet->AddInstance(codeObject, WSChangeset::ChangeState::Deleted, std::make_shared<Json::Value>(properties));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
ObjectId GetLockId
(
LockableId lock,
const BeBriefcaseId briefcaseId
)
    {
    Utf8String idString;
    if (briefcaseId.IsValid())
        idString.Sprintf("%d-%s-%u", (int) lock.GetType(), FormatBeInt64Id(lock.GetId()).c_str(), briefcaseId.GetValue());
    else
        idString.Sprintf("%d-%s", (int) lock.GetType(), FormatBeInt64Id(lock.GetId()).c_str());

    return ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::Lock, idString);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::AcquireCodesLocksInternal
(
LockRequestCR                       locks,
DgnCodeSet                          codes,
BeBriefcaseId                       briefcaseId,
BeGuidCR                            seedFileId,
Utf8StringCR                        lastChangeSetId,
IBriefcaseManager::ResponseOptions  options,
ICancellationTokenPtr               cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::AcquireCodesLocksInternal";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    ChunkedWSChangeset chunkedChangeset;
    MultiLockFormatter::SetToChunkedChangeset(locks.GetLockSet(), briefcaseId, seedFileId, lastChangeSetId, chunkedChangeset, WSChangeset::ChangeState::Modified);

    DgnCodeState state;
    state.SetReserved(briefcaseId);
    MultiCodeFormatter::SetToChunkedChangeset(codes, state, briefcaseId, chunkedChangeset, WSChangeset::ChangeState::Created);

    ChangesetResponseOptions changesetOptions(options);
    return SendChunkedChangesetRequest(chunkedChangeset, changesetOptions, cancellationToken, nullptr, 2);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
BriefcasesInfoTaskPtr iModelConnection::QueryBriefcaseInfoInternal(WSQuery const& query, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::QueryBriefcaseInfoInternal";
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");

    return ExecuteWithRetry<bvector<BriefcaseInfoPtr>>([=]()
        {
        return m_wsRepositoryClient->SendQueryRequestWithOptions(query, nullptr, nullptr, requestOptions, cancellationToken)
            ->Then<BriefcasesInfoResult>([=](const WSObjectsResult& result)
            {
            if (!result.IsSuccess())
                {
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, result.GetError().GetMessage().c_str());
                return BriefcasesInfoResult::Error(result.GetError());
                }

            bvector<BriefcaseInfoPtr> briefcases;
            for (auto const& instance : result.GetValue().GetInstances())
                {
                briefcases.push_back(BriefcaseInfo::Parse(instance));
                }

            return BriefcasesInfoResult::Success(briefcases);
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                   05/2019
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::QueryCodesLocksByChunks
(
WSQuery query,
CodeLockSetResultInfoPtr codesLocksOut,
CodeLocksSetAddFunction addFunction,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::QueryCodesLocksByChunks";
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");

    query.SetTop(m_codesLocksPageSize);

    SkipTokenResultPtr result;
    Utf8String nextSkipToken = nullptr;
    do
        {
        result = ExecuteAsync(QueryCodesLocksInternal(query, nextSkipToken, codesLocksOut, addFunction, requestOptions, cancellationToken));
        if (!result->IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, result->GetError().GetMessage().c_str());
            return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(result->GetError()));
            }
        nextSkipToken = result->GetValue();
        } while (!nextSkipToken.empty());

    return CreateCompletedAsyncTask<StatusResult>(StatusResult::Success());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
SkipTokenTaskPtr iModelConnection::QueryCodesLocksInternal
(
WSQueryCR query,
Utf8StringCR skipToken,
CodeLockSetResultInfoPtr codesLocksOut,
CodeLocksSetAddFunction addFunction,
IWSRepositoryClient::RequestOptionsPtr requestOptions,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::QueryCodesLocksInternal";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");

    return ExecuteWithRetry<Utf8String>([=]()
        {
        //Execute query
        return m_wsRepositoryClient->SendQueryRequestWithOptions(query, "", skipToken, requestOptions, cancellationToken)->Then<SkipTokenResult>
            ([=](const WSObjectsResult& result)
            {
            if (!result.IsSuccess())
                return SkipTokenResult::Error(result.GetError());

            if (!result.GetValue().GetRapidJsonDocument().IsNull())
                {
                for (auto const& value : result.GetValue().GetInstances())
                    {
                    addFunction(value, codesLocksOut);
                    }
                //NEEDSWORK: log an error
                }
            return SkipTokenResult::Success(result.GetValue().GetSkipToken());
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas                02/2017
//---------------------------------------------------------------------------------------
void AddCodes(const WSObjectsReader::Instance& value, CodeLockSetResultInfoPtr codesLocksSetOut)
    {
    DgnCode        code;
    DgnCodeState   codeState;
    BeBriefcaseId  briefcaseId;

    if (GetCodeFromServerJson(value.GetProperties(), code, codeState, briefcaseId))
        codesLocksSetOut->AddCode(code, codeState, briefcaseId);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis                   01/2018
//---------------------------------------------------------------------------------------
void AddMultiCodes(const WSObjectsReader::Instance& value, CodeLockSetResultInfoPtr codesLocksSetOut)
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
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                   03/2019
//---------------------------------------------------------------------------------------
void AddLocks(const WSObjectsReader::Instance& value, CodeLockSetResultInfoPtr codesLocksSetOut)
    {
    DgnLock        lock;
    BeBriefcaseId  briefcaseId;
    Utf8String     changeSetId;

    if (GetLockFromServerJson(value.GetProperties(), lock, briefcaseId, changeSetId))
        codesLocksSetOut->AddLock(lock, briefcaseId, changeSetId);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis                   01/2018
//---------------------------------------------------------------------------------------
void AddMultiLocks(const WSObjectsReader::Instance& value, CodeLockSetResultInfoPtr codesLocksSetOut)
    {
    DgnLock        lock;
    BeBriefcaseId  briefcaseId;
    Utf8String     changeSetId;

    DgnLockSet lockSet;
    if (GetMultiLockFromServerJson(value.GetProperties(), lockSet, briefcaseId, changeSetId))
        {
        for (auto const& lock : lockSet)
            {
            codesLocksSetOut->AddLock(lock, briefcaseId, changeSetId);
            }
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas                01/2017
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::QueryCodesInternal
(
const DgnCodeSet& codes,
const BeSQLite::BeBriefcaseId briefcaseId,
CodeLockSetResultInfoPtr codesLocksOut,
ICancellationTokenPtr cancellationToken
) const
    {
    BeAssert(0 != codes.size() && "Query Ids in empty array is not supported.");

    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::Code);

    std::deque<ObjectId> queryIds;
    for (auto& code : codes)
        queryIds.push_back(MultiCodeFormatter::GetCodeId(code, briefcaseId));

    query.AddFilterIdsIn(queryIds, nullptr, 0, 0);

    return QueryCodesLocksByChunks(query, codesLocksOut, AddCodes, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas                01/2017
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::QueryCodesInternal
(
const BeSQLite::BeBriefcaseId  briefcaseId,
CodeLockSetResultInfoPtr codesLocksOut,
ICancellationTokenPtr cancellationToken
) const
    {
    BeAssert(briefcaseId.IsValid() && "Query by invalid briefcase id is not supported.");
    
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::MultiCode);
    query.SetFilter(Utf8PrintfString("%s+eq+%u", ServerSchema::Property::BriefcaseId, briefcaseId.GetValue()));

    return QueryCodesLocksByChunks(query, codesLocksOut, AddMultiCodes, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas                01/2017
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::QueryUnavailableCodesInternal
(
const BeBriefcaseId briefcaseId,
CodeLockSetResultInfoPtr codesLocksOut,
ICancellationTokenPtr cancellationToken
) const
    {
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::MultiCode);
    query.SetFilter(Utf8PrintfString("%s+ne+%u", ServerSchema::Property::BriefcaseId, briefcaseId.GetValue()));

    return QueryCodesLocksByChunks(query, codesLocksOut, AddMultiCodes, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas                01/2017
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::QueryLocksInternal
(
const LockableIdSet& locks,
const BeSQLite::BeBriefcaseId  briefcaseId,
CodeLockSetResultInfoPtr codesLocksOut,
ICancellationTokenPtr cancellationToken
) const
    {
    BeAssert(0 != locks.size() && "Query Ids in empty array is not supported.");

    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::Lock);
    query.SetFilter(Utf8PrintfString("%s+gt+%u", ServerSchema::Property::LockLevel, LockLevel::None));

    std::deque<ObjectId> queryIds;
    for (auto& lock : locks)
        queryIds.push_back(GetLockId(lock, briefcaseId));

    query.AddFilterIdsIn(queryIds, nullptr, 0, 0);

    return QueryCodesLocksByChunks(query, codesLocksOut, AddLocks, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas                01/2017
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::QueryLocksInternal
(
const BeSQLite::BeBriefcaseId  briefcaseId,
CodeLockSetResultInfoPtr codesLocksOut,
ICancellationTokenPtr cancellationToken
) const
    {
    BeAssert(briefcaseId.IsValid() && "Query by invalid briefcase id is not supported.");

    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::MultiLock);
    query.SetFilter(Utf8PrintfString("%s+eq+%u", ServerSchema::Property::BriefcaseId, briefcaseId.GetValue()));

    return QueryCodesLocksByChunks(query, codesLocksOut, AddMultiLocks, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas                01/2017
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::QueryUnavailableLocksInternal
(
const BeBriefcaseId briefcaseId,
const uint64_t lastChangeSetIndex,
CodeLockSetResultInfoPtr codesLocksOut,
ICancellationTokenPtr cancellationToken
) const
    {
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::MultiLock);
    query.SetFilter(Utf8PrintfString("%s+ne+%u+and+(%s+gt+%u+or+%s+gt+%u)", ServerSchema::Property::BriefcaseId,
                                     briefcaseId.GetValue(), ServerSchema::Property::LockLevel, LockLevel::None,
                                     ServerSchema::Property::ReleasedWithChangeSetIndex, lastChangeSetIndex));

    return QueryCodesLocksByChunks(query, codesLocksOut, AddMultiLocks, cancellationToken);
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
CodeSequenceTaskPtr iModelConnection::QueryCodeMaximumIndexInternal
(
std::shared_ptr<WSChangeset> changeSet,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::QueryCodeMaximumIndexInternal";
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    auto requestString = changeSet->ToRequestString();
    HttpStringBodyPtr request = HttpStringBody::Create(requestString);
    return ExecuteWithRetry<CodeSequence>([=]()
        {
        return m_wsRepositoryClient->SendChangesetRequestWithOptions(request, nullptr, requestOptions, cancellationToken)->Then<CodeSequenceResult>
            ([=](const WSChangesetResult& result)
            {
            if (result.IsSuccess())
                {
                Json::Value ptr = GetChangedInstances(result.GetValue()->AsString().c_str());

                auto json = ToRapidJson(*ptr.begin());
                CodeSequence        codeSequence;
                if (!GetCodeSequenceFromServerJson(json[ServerSchema::InstanceAfterChange][ServerSchema::Properties], codeSequence))
                    {
                    LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, "Code template parse failed.");
                    return CodeSequenceResult::Error({Error::Id::InvalidPropertiesValues, ErrorLocalizedString(MESSAGE_CodeSequenceResponseError)});
                    }

                double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), requestOptions, "");
                return CodeSequenceResult::Success(codeSequence);
                }
            else
                {
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, result.GetError().GetMessage().c_str());
                return CodeSequenceResult::Error(result.GetError());
                }
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             03/2017
//---------------------------------------------------------------------------------------
CodeSequenceTaskPtr iModelConnection::QueryCodeNextAvailableInternal
(
std::shared_ptr<WSChangeset> changeSet,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::QueryCodeNextAvailableInternal";
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    HttpStringBodyPtr request = HttpStringBody::Create(changeSet->ToRequestString());
    return ExecuteWithRetry<CodeSequence>([=]()
        {
        return m_wsRepositoryClient->SendChangesetRequestWithOptions(request, nullptr, requestOptions, cancellationToken)->Then<CodeSequenceResult>
            ([=](const WSChangesetResult& result)
            {
            if (result.IsSuccess())
                {
                Json::Value ptr = GetChangedInstances(result.GetValue()->AsString().c_str());
                auto json = ToRapidJson(*ptr.begin());

                CodeSequence        codeSequence;
                if (!GetCodeSequenceFromServerJson(json[ServerSchema::InstanceAfterChange][ServerSchema::Properties], codeSequence))
                    {
                    LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, "Code template parse failed.");
                    return CodeSequenceResult::Error({Error::Id::InvalidPropertiesValues, ErrorLocalizedString(MESSAGE_CodeSequenceResponseError)});
                    }

                double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), requestOptions, "");
                return CodeSequenceResult::Success(codeSequence);
                }
            else
                {
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, result.GetError().GetMessage().c_str());
                return CodeSequenceResult::Error(result.GetError());
                }
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<WSCreateObjectResult> iModelConnection::CreateBriefcaseInstance(ICancellationTokenPtr cancellationToken) const
    {
    Json::Value briefcaseIdJson = Json::objectValue;
    briefcaseIdJson[ServerSchema::Instance] = Json::objectValue;
    briefcaseIdJson[ServerSchema::Instance][ServerSchema::SchemaName] = ServerSchema::Schema::iModel;
    briefcaseIdJson[ServerSchema::Instance][ServerSchema::ClassName] = ServerSchema::Class::Briefcase;

    const Utf8String methodName = "iModelConnection::CreateBriefcaseInstance";
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");

    return m_wsRepositoryClient->SendCreateObjectRequestWithOptions(briefcaseIdJson, BeFileName(), nullptr, requestOptions, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
ChangeSetInfoTaskPtr iModelConnection::GetChangeSetByIdInternal
(
Utf8StringCR          changeSetId,
bool                  loadAccessKey,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::GetChangeSetByIdInternal";
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");

    if (changeSetId.empty())
        {
        // Don't log error here since this is a valid case then there are no changeSets locally.
        return CreateCompletedAsyncTask<ChangeSetInfoResult>(ChangeSetInfoResult::Error(Error::Id::InvalidChangeSet));
        }
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    ObjectId changeSetObject(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet, changeSetId);

    WSQuery query(changeSetObject);
    if (loadAccessKey)
        {
        Utf8String selectString = "*";
        FileAccessKey::AddDownloadAccessKeySelect(selectString);
        query.SetSelect(selectString);
        }

    return m_wsRepositoryClient->SendQueryRequestWithOptions(query, nullptr, nullptr, requestOptions, cancellationToken)->Then<ChangeSetInfoResult>
        ([=](WSObjectsResult& changeSetResult)
        {
        if (changeSetResult.IsSuccess())
            {
            auto changeSetInstances = changeSetResult.GetValue().GetInstances();
            if (changeSetInstances.IsEmpty())
                {
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, "ChangeSet does not exist.");
                return ChangeSetInfoResult::Error(Error::Id::ChangeSetDoesNotExist);
                }

            auto changeSet = ChangeSetInfo::Parse(*changeSetInstances.begin());
            if (loadAccessKey)
                {
                changeSet->SetFileAccessKey(FileAccessKey::ParseFromRelated(*changeSetResult.GetValue().GetInstances().begin()));
                }
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), requestOptions, "");
            return ChangeSetInfoResult::Success(changeSet);
            }
        else
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, changeSetResult.GetError().GetMessage().c_str());
            return ChangeSetInfoResult::Error(changeSetResult.GetError());
            }
        });
    }

/* Private methods start */

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas            12/2016
//---------------------------------------------------------------------------------------
bool iModelConnection::IsSubscribedToEvents() const
    {
    return m_eventManagerPtr->IsSubscribedToEvents(nullptr);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
EventTaskPtr iModelConnection::GetEvent
(
const bool longPolling,
const ICancellationTokenPtr cancellationToken
) const
    {
    return m_eventManagerPtr->GetEvent(longPolling, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Caleb.Shafer                    06/2016
//                                               Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::SubscribeToEvents
(
EventTypeSet* eventTypes,
ICancellationTokenPtr cancellationToken
)
    {
    const Utf8String methodName = "iModelConnection::SubscribeToEvents";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    return ExecuteWithRetry<void>([=]()
        {
        return m_eventManagerPtr->SetEventServiceClient(eventTypes, cancellationToken);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Arvind.Venkateswaran            06/2016
//Todo: Add another method to only cancel GetEvent Operation and not the entire connection
//---------------------------------------------------------------------------------------
StatusTaskPtr  iModelConnection::UnsubscribeToEvents()
    {
    return m_eventManagerPtr->UnsubscribeEvents();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas            12/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::SubscribeEventsCallback(EventTypeSet* eventTypes, EventCallbackPtr callback, iModelConnectionP imodelConnectionP)
    {
    if (m_eventCallbackManagerPtr.IsNull())
        {
        m_eventCallbackManagerPtr = new EventCallbackManager(imodelConnectionP);
        }
    return m_eventCallbackManagerPtr->Subscribe(eventTypes, callback);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas            12/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::UnsubscribeEventsCallback(EventCallbackPtr callback)
    {
    if (m_eventCallbackManagerPtr.IsNull())
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Success());

    if (!callback)
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error::Id::EventCallbackNotSpecified));

    bool dispose = false;
    auto result = m_eventCallbackManagerPtr->Unsubscribe(callback, &dispose);
    if (dispose)
        m_eventCallbackManagerPtr = nullptr;

    return result;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                   03/2019
//---------------------------------------------------------------------------------------
ChangeSetsInfoTaskPtr iModelConnection::GetChangeSetsFromQueryByChunks
(
WSQuery                     query,
bool                        parseFileAccessKey,
ICancellationTokenPtr       cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::GetChangeSetsFromQueryByChunks";
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");

    query.SetOrderBy(ServerSchema::Property::Index);
    query.SetTop(m_changeSetsPageSize);

    ChangeSetsInfoResultPtr finalResult = std::make_shared<ChangeSetsInfoResult>();
    finalResult->SetSuccess(bvector<ChangeSetInfoPtr>());

    SkipTokenResultPtr result;
    Utf8String nextSkipToken = nullptr;
    do
        {
        result = ExecuteAsync(ChangeSetsFromQueryInternal(query, nextSkipToken, parseFileAccessKey, finalResult, requestOptions, cancellationToken));
        if (!result->IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, result->GetError().GetMessage().c_str());
            return CreateCompletedAsyncTask<ChangeSetsInfoResult>(ChangeSetsInfoResult::Error(result->GetError()));
            }
        nextSkipToken = result->GetValue();
        } while (!nextSkipToken.empty());

    return CreateCompletedAsyncTask<ChangeSetsInfoResult>(ChangeSetsInfoResult::Success(finalResult->GetValue()));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
SkipTokenTaskPtr iModelConnection::ChangeSetsFromQueryInternal
(
WSQueryCR                   query,
Utf8StringCR                skipToken,
bool                        parseFileAccessKey,
ChangeSetsInfoResultPtr     finalResult,
IWSRepositoryClient::RequestOptionsPtr requestOptions,
ICancellationTokenPtr       cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::ChangeSetsFromQueryInternal";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");

    return ExecuteWithRetry<Utf8String>([=]()
        {
        return m_wsRepositoryClient->SendQueryRequestWithOptions(query, nullptr, skipToken, requestOptions, cancellationToken)->Then<SkipTokenResult>
            ([=](const WSObjectsResult& changeSetsInfoResult)
            {
            if (changeSetsInfoResult.IsSuccess())
                {
                if (!changeSetsInfoResult.GetValue().GetRapidJsonDocument().IsNull())
                    {
                    for (auto const& value : changeSetsInfoResult.GetValue().GetInstances())
                        {
                        auto changeSetInfo = ChangeSetInfo::Parse(value);
                        if (parseFileAccessKey)
                            {
                            auto fileAccessKey = FileAccessKey::ParseFromRelated(value);
                            changeSetInfo->SetFileAccessKey(fileAccessKey);
                            }

                        finalResult->GetValue().push_back(changeSetInfo);
                        }
                    }
                return SkipTokenResult::Success(changeSetsInfoResult.GetValue().GetSkipToken());
                }
            else
                {
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, changeSetsInfoResult.GetError().GetMessage().c_str());
                return SkipTokenResult::Error(changeSetsInfoResult.GetError());
                }
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
ChangeSetsInfoTaskPtr iModelConnection::GetChangeSetsAfterIdInternal
(
Utf8StringCR          changeSetId,
BeGuidCR              fileId,
bool                  loadAccessKey,
ICancellationTokenPtr cancellationToken
) const
    {
    ChangeSetQuery changeSetQuery;
    changeSetQuery.FilterChangeSetsAfterId(changeSetId);
    changeSetQuery.FilterBySeedFileId(fileId);

    if (loadAccessKey)
        changeSetQuery.SelectDownloadAccessKey();

    return GetChangeSetsFromQueryByChunks(changeSetQuery.GetWSQuery(), loadAccessKey, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
ChangeSetsTaskPtr iModelConnection::DownloadChangeSetsInternal
(
const bvector<ChangeSetInfoPtr>&       changeSets,
Http::Request::ProgressCallbackCR      callback,
ICancellationTokenPtr                  cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::DownloadChangeSetsInternal";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (changeSets.empty())
        {
        return CreateCompletedAsyncTask<ChangeSetsResult>(ChangeSetsResult::Success(ChangeSets {}));
        }
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    double totalSize = 0.0;
    for (ChangeSetInfoPtr const& changeSet : changeSets)
        {
        totalSize += changeSet->GetFileSize();
        }
    MultiProgressCallbackHandlerPtr callbacksHandlerPtr = new MultiProgressCallbackHandler(callback, totalSize);


    bset<std::shared_ptr<AsyncTask>> tasks;
    bmap<Utf8String, int64_t> changeSetIdIndexMap;
    for (auto& changeSet : changeSets)
        {
        Http::Request::ProgressCallback changeSetCallback;
        callbacksHandlerPtr->AddCallback(changeSetCallback);
        tasks.insert(DownloadChangeSetFile(changeSet, changeSetCallback, cancellationToken));
        changeSetIdIndexMap.Insert(changeSet->GetId(), changeSet->GetIndex());
        }

    return AsyncTask::WhenAll(tasks)->Then<ChangeSetsResult>([=]()
        {
        ChangeSets resultChangeSets;
        for (auto task : tasks)
            {
            auto result = dynamic_pointer_cast<ChangeSetTask>(task)->GetResult();
            if (!result.IsSuccess())
                {
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, result.GetError().GetMessage().c_str());
                return ChangeSetsResult::Error(result.GetError());
                }
            resultChangeSets.push_back(result.GetValue());
            }

        std::sort(resultChangeSets.begin(), resultChangeSets.end(), [changeSetIdIndexMap](DgnRevisionPtr a, DgnRevisionPtr b)
            {
            auto itemA = changeSetIdIndexMap.find(a->GetId());
            auto itemB = changeSetIdIndexMap.find(b->GetId());

            if (changeSetIdIndexMap.end() == itemA || changeSetIdIndexMap.end() == itemB)
                {
                BeAssert(false && "ChangeSet not found in the map.");
                return true;
                }

            return itemA->second < itemB->second;
            });

        callbacksHandlerPtr->SetFinished();
        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
        return ChangeSetsResult::Success(resultChangeSets);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
ChangeSetsTaskPtr iModelConnection::DownloadChangeSets(std::deque<ObjectId>& changeSetIds, Http::Request::ProgressCallbackCR callback, 
                                                       ICancellationTokenPtr cancellationToken) const
    {
    if (0 == changeSetIds.size())
        return CreateCompletedAsyncTask<ChangeSetsResult>(ChangeSetsResult::Error(Error::Id::QueryIdsNotSpecified));
    
    ChangeSetQuery changeSetQuery;
    changeSetQuery.FilterByIds(changeSetIds);
    changeSetQuery.SelectDownloadAccessKey();
    
    std::shared_ptr<ChangeSetsResult> finalResult = std::make_shared<ChangeSetsResult>();

    return GetChangeSetsFromQueryByChunks(changeSetQuery.GetWSQuery(), true, cancellationToken)
        ->Then([=](ChangeSetsInfoResultCR changeSetsQueryResult) {
        if (!changeSetsQueryResult.IsSuccess())
            {
            finalResult->SetError(changeSetsQueryResult.GetError());
            return;
            }

        DownloadChangeSetsInternal(changeSetsQueryResult.GetValue(), callback, cancellationToken)->Then([=](ChangeSetsResultCR changesetsResult) {
            if (!changesetsResult.IsSuccess())
                {
                finalResult->SetError(changesetsResult.GetError());
                return;
                }

            finalResult->SetSuccess(changesetsResult.GetValue());
            });
        })->Then<ChangeSetsResult>([=]
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             05/2019
//---------------------------------------------------------------------------------------
ChangeSetsTaskPtr iModelConnection::DownloadChangeSetsBetween(Utf8StringCR firstChangeSetId, Utf8StringCR secondChangeSetId, 
    BeSQLite::BeGuidCR fileId, Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    if (Utf8String::IsNullOrEmpty(firstChangeSetId.c_str()) && Utf8String::IsNullOrEmpty(secondChangeSetId.c_str()))
        return CreateCompletedAsyncTask<ChangeSetsResult>(ChangeSetsResult::Error(Error::Id::InvalidChangeSet));

    ChangeSetQuery changeSetQuery = CreateBetweenChangeSetsQuery(firstChangeSetId, secondChangeSetId, fileId);
    changeSetQuery.SelectDownloadAccessKey();

    std::shared_ptr<ChangeSetsResult> finalResult = std::make_shared<ChangeSetsResult>();

    return GetChangeSetsFromQueryByChunks(changeSetQuery.GetWSQuery(), true, cancellationToken)
        ->Then([=](ChangeSetsInfoResultCR changeSetsQueryResult) {
        if (!changeSetsQueryResult.IsSuccess())
            {
            finalResult->SetError(changeSetsQueryResult.GetError());
            return;
            }

        DownloadChangeSetsInternal(changeSetsQueryResult.GetValue(), callback, cancellationToken)->Then([=](ChangeSetsResultCR changesetsResult) {
            if (!changesetsResult.IsSuccess())
                {
                finalResult->SetError(changesetsResult.GetError());
                return;
                }

            finalResult->SetSuccess(changesetsResult.GetValue());
            });
        })->Then<ChangeSetsResult>([=]
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
ChangeSetTaskPtr iModelConnection::DownloadChangeSetFile
(
ChangeSetInfoPtr                  changeSet,
Http::Request::ProgressCallbackCR callback,
ICancellationTokenPtr             cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::DownloadChangeSetFile";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    RevisionStatus changeSetStatus;
    DgnRevisionPtr changeSetPtr = DgnRevision::Create(&changeSetStatus, changeSet->GetId(), changeSet->GetParentChangeSetId(), 
                                                      changeSet->GetDbGuid());
    if (!changeSetPtr.IsValid())
        {
        return CreateCompletedAsyncTask<ChangeSetResult>(ChangeSetResult::Error(changeSetStatus));
        }
    auto changeSetFileName = changeSetPtr->GetRevisionChangesFile();

    if (m_changeSetCacheManager.TryGetChangeSetFile(changeSetFileName, changeSet->GetId()))
        return CreateCompletedAsyncTask<ChangeSetResult>(ChangeSetResult::Success(changeSetPtr));

    ObjectId fileObject(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet, changeSet->GetId());
    return DownloadFile(changeSetFileName, fileObject, changeSet->GetFileAccessKey(), callback, cancellationToken)
        ->Then<ChangeSetResult>([=](StatusResultCR downloadResult)
        {
        if (!downloadResult.IsSuccess())
            return ChangeSetResult::Error(downloadResult.GetError());

        return ChangeSetResult::Success(changeSetPtr);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas              02/2017
//---------------------------------------------------------------------------------------
ChangeSetsInfoTaskPtr iModelConnection::GetAllChangeSetsInternal(bool loadAccessKey, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::GetAllChangeSetsInternal";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet);
    return GetChangeSetsFromQueryByChunks(query, loadAccessKey, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             03/2017
//---------------------------------------------------------------------------------------
bool IsInitializationFinished(InitializationState state)
    {
    return !(InitializationState::NotStarted == state || InitializationState::Scheduled == state);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             09/2019
//---------------------------------------------------------------------------------------
void ProcessInitializationFailure(InitializationState initializationState, FileResultPtr finalResult, Utf8StringCR methodName)
    {
    switch (initializationState)
        {
        case InitializationState::Scheduled:
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Scheduled");
            finalResult->SetError({ Error::Id::FileIsNotYetInitialized });
            break;
        case InitializationState::OutdatedFile:
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File is outdated");
            finalResult->SetError({ Error::Id::FileIsOutdated });
            break;
        case InitializationState::CodeTooLong:
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Code too long");
            finalResult->SetError({ Error::Id::FileCodeTooLong });
            break;
        case InitializationState::SeedFileIsBriefcase:
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Uploaded seed file is a briefcase");
            finalResult->SetError({ Error::Id::FileIsBriefcase });
            break;
        default:
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File initialization failed");
            finalResult->SetError({ Error::Id::FileInitializationFailed });
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
void iModelConnection::WaitForInitializedBIMFile(BeGuid fileGuid, FileResultPtr finalResult, ICancellationTokenPtr cancellationToken) const
    {
    InitializationState initializationState = InitializationState::NotStarted;
    int retriesLeft = 300;
    const Utf8String methodName = "iModelConnection::WaitForInitializedBIMFile";
    BeThreadUtilities::BeSleep(1000);

    while (!IsInitializationFinished(initializationState) && retriesLeft > 0 && (cancellationToken == nullptr || !cancellationToken->IsCanceled()))
        {
        auto seedFilesResult = ExecuteAsync(GetSeedFileById(fileGuid));
        if (!seedFilesResult->IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, seedFilesResult->GetError().GetMessage().c_str()); 
            finalResult->SetError(seedFilesResult->GetError());
            return;
            }

        auto seedFile = seedFilesResult->GetValue();
        initializationState = seedFile->GetInitialized();

        if (!IsInitializationFinished(initializationState))
            BeThreadUtilities::BeSleep(1000);
        retriesLeft--;
        }

    if (initializationState != InitializationState::Success)
        {
        ProcessInitializationFailure(initializationState, finalResult, methodName);
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             09/2019
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::WaitForInitialization(ICancellationTokenPtr cancellationToken) const
    {
    FileResultPtr finalResult = std::make_shared<FileResult>();
    finalResult->SetSuccess(nullptr);
    BeGuid imodelId;
    BentleyStatus status = imodelId.FromString(GetiModelInfo().GetId().c_str());
    if (status != BentleyStatus::SUCCESS)
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error::Id::InvalidiModelId));

    WaitForInitializedBIMFile(imodelId, finalResult, cancellationToken);
    if (!finalResult->IsSuccess())
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(finalResult->GetError()));
    return CreateCompletedAsyncTask<StatusResult>(StatusResult::Success());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::Push
(
Dgn::DgnRevisionPtr                 changeSet,
Dgn::DgnDbR                         dgndb,
PushChangeSetArgumentsPtr           pushArguments
) const
    {
    const Utf8String methodName = "iModelConnection::Push";
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");
    DgnDbP pDgnDb = &dgndb;

    StatusResult validatePushInfoResult = pushArguments->Validate(changeSet, dgndb);
    if (!validatePushInfoResult.IsSuccess())
        return CreateCompletedAsyncTask(validatePushInfoResult);

    // Stage 1. Create changeSet.
    std::shared_ptr<Json::Value> pushJson = std::make_shared<Json::Value>(ChangeSetFormatter::Format(changeSet, dgndb.GetBriefcaseId(), pushArguments));
    std::shared_ptr<StatusResult> finalResult = std::make_shared<StatusResult>();
    return ExecuteWithRetry<void>([=]()
        {
        return m_wsRepositoryClient->SendCreateObjectRequestWithOptions(*pushJson, BeFileName(), nullptr, requestOptions, pushArguments->GetCancelationToken())
            ->Then([=](const WSCreateObjectResult& initializePushResult)
            {
#if defined (ENABLE_BIM_CRASH_TESTS)
            BreakHelper::HitBreakpoint(Breakpoints::iModelConnection_AfterCreateChangeSetRequest);
#endif
            if (!initializePushResult.IsSuccess())
                {
                Error error(initializePushResult.GetError());
                if (Error::Id::ChangeSetAlreadyExists == error.GetId())
                    {
                    finalResult->SetSuccess();
                    }
                else
                    {
                    finalResult->SetError(error);
                    LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, initializePushResult.GetError().GetMessage().c_str());
                    }
                return;
                }

            // Stage 2. Upload changeSet file. 
            Json::Value json;
            initializePushResult.GetValue().GetJson(json);
            JsonValueCR changeSetInstance = json[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
            Utf8String  changeSetInstanceId = changeSetInstance[ServerSchema::InstanceId].asString();
            ObjectId    changeSetObjectId = ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet, changeSetInstanceId);
            auto fileAccessKey = FileAccessKey::ParseFromRelated(changeSetInstance);

            m_azureClient->SendUpdateFileRequest(fileAccessKey->GetUploadUrl(), changeSet->GetRevisionChangesFile(), pushArguments->GetProgressCallback(), nullptr, pushArguments->GetCancelationToken())
                ->Then([=] (const AzureResult& result)
                {
#if defined (ENABLE_BIM_CRASH_TESTS)
                BreakHelper::HitBreakpoint(Breakpoints::iModelConnection_AfterUploadChangeSetFile);
#endif
                if (!result.IsSuccess())
                    {
                    LogHelper::Log(SEVERITY::LOG_WARNING, methodName, result.GetError().GetMessage().c_str());
                    finalResult->SetError(result.GetError());
                    return;
                    }

                // Stage 3. Initialize changeSet.
                InitializeChangeSet(changeSet, *pDgnDb, *pushJson, changeSetObjectId, pushArguments)
                    ->Then([=] (StatusResultCR result)
                    {
                    if (result.IsSuccess())
                        finalResult->SetSuccess();
                    else
                        {
                        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, result.GetError().GetMessage().c_str());
                        finalResult->SetError(result.GetError());
                        }
                    });
                });
            })->Then<StatusResult>([=]
                {
                return *finalResult;
                });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
FileTaskPtr iModelConnection::GetSeedFileById(BeGuidCR fileId, ICancellationTokenPtr cancellationToken) const
    {
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::File);
    Utf8String filter;
    filter.Sprintf("%s+eq+'%s'", ServerSchema::Property::FileId, fileId.ToString().c_str());
    query.SetFilter(filter);

    return SeedFilesQuery(query, cancellationToken)->Then<FileResult>([=](FilesResult filesResult)
        {
        if (!filesResult.IsSuccess())
            return FileResult::Error(filesResult.GetError());

        bvector<FileInfoPtr> files = filesResult.GetValue();
        if (files.empty())
            return FileResult::Error(Error::Id::FileDoesNotExist);
        return FileResult::Success(*files.begin());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
FileTaskPtr iModelConnection::GetLatestSeedFile(ICancellationTokenPtr cancellationToken) const
    {
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::File);
    Utf8String orderByClouse;
    orderByClouse.Sprintf("%s+%s", ServerSchema::Property::Index, "desc");
    query.SetOrderBy(orderByClouse);
    query.SetTop(1);

    return SeedFilesQuery(query, cancellationToken)->Then<FileResult>([=](FilesResult filesResult)
        {
        if (!filesResult.IsSuccess())
            return FileResult::Error(filesResult.GetError());

        return FileResult::Success(*filesResult.GetValue().begin());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             07/2018
//---------------------------------------------------------------------------------------
void FilterExternalCodes
(
DgnCodeSet&             filteredCodes,
DgnCodeSet*             externalCodes,
DgnCodeSet const&       allCodes,
Dgn::DgnDbR             dgndb
)
    {
    for (DgnCodeCR code : allCodes)
        {
        CodeSpecCPtr codeSpec = dgndb.CodeSpecs().GetCodeSpec(code.GetCodeSpecId());
        BeAssert(codeSpec.IsValid() && "Invalid CodeSpec for pushed ChangeSet");
        if (codeSpec->IsManagedWithDgnDb())
            filteredCodes.insert(code);
        else if (nullptr != externalCodes)
            externalCodes->insert(code);
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             07/2018
//---------------------------------------------------------------------------------------
void ExtractCodes
(
DgnCodeSet&             assignedCodes,
DgnCodeSet&             discardedCodes,
DgnCodeSet*             externalAssignedCodes,
DgnCodeSet*             externalDiscardedCodes,
Dgn::DgnRevisionPtr     changeSet,
Dgn::DgnDbR             dgndb
)
    {
    DgnCodeSet allAssignedCodes, allDiscardedCodes;
    changeSet->ExtractCodes(allAssignedCodes, allDiscardedCodes, dgndb);
    FilterExternalCodes(assignedCodes, externalAssignedCodes, allAssignedCodes, dgndb);
    FilterExternalCodes(discardedCodes, externalDiscardedCodes, allDiscardedCodes, dgndb);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas            03/2017
//---------------------------------------------------------------------------------------
void ExtractCodesLocksChangeset
(
Dgn::DgnRevisionPtr     changeSet,
ChunkedWSChangeset&     chunkedChangeset,
Dgn::DgnDbR             dgndb,
bool                    relinquishCodesLocks,
DgnCodeSet*             externalAssignedCodes = nullptr,
DgnCodeSet*             externalDiscardedCodes = nullptr
)
    {
    BeBriefcaseId briefcaseId = dgndb.GetBriefcaseId();

    //Set used locks to the ECChangeSet
    LockRequest usedLocks;
    usedLocks.FromRevision(*changeSet, dgndb, !relinquishCodesLocks);

    BeGuid seedFileId;
    seedFileId.FromString(changeSet->GetDbGuid().c_str());
    if (!usedLocks.IsEmpty())
        MultiLockFormatter::SetToChunkedChangeset(usedLocks.GetLockSet(), briefcaseId, seedFileId, changeSet->GetId(), chunkedChangeset,
                                       WSChangeset::ChangeState::Modified, true);

    DgnCodeSet assignedCodes, discardedCodes;
    ExtractCodes(assignedCodes, discardedCodes, externalAssignedCodes, externalDiscardedCodes, changeSet, dgndb);

    if (!assignedCodes.empty())
        {
        DgnCodeState state;
        state.SetUsed(changeSet->GetId());
        MultiCodeFormatter::SetToChunkedChangeset(assignedCodes, state, briefcaseId, chunkedChangeset, WSChangeset::ChangeState::Created);
        }

    if (!discardedCodes.empty())
        {
        DgnCodeState state;
        state.SetDiscarded(changeSet->GetId());
        MultiCodeFormatter::SetToChunkedChangeset(discardedCodes, state, briefcaseId, chunkedChangeset, WSChangeset::ChangeState::Modified);
        }

    if (relinquishCodesLocks)
        {
        LockDeleteAllJsonRequest(chunkedChangeset.GetCurrentChangeset(), briefcaseId);
        CodeDiscardReservedJsonRequest(chunkedChangeset.GetCurrentChangeset(), briefcaseId);
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas            03/2018
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::PushPendingCodesLocks
(
Dgn::DgnDbPtr                       dgndb,
ICancellationTokenPtr               cancellationToken
) const
    {
    bvector<Utf8String> pendingChangeSets;
    PendingChangeSetStorage::GetItems(*dgndb, pendingChangeSets);

    const Utf8String methodName = "iModelConnection::PushPendingCodesLocks";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    
    if (pendingChangeSets.size() == 0)
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Success());

    return DownloadChangeSets(pendingChangeSets)
        ->Then([=](const ChangeSetsResult& changesetsResult) {
        
        auto downloadedChangesets = changesetsResult.GetValue();
        for (auto changeSetEntry : downloadedChangesets)
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Pushing changeset %s codes and locks.", changeSetEntry->GetId().c_str());

            if (!changesetsResult.IsSuccess())
                return;

            ChunkedWSChangeset codesLocksChangeSet;
            auto firstChangeSet = changesetsResult.GetValue().begin();
            ExtractCodesLocksChangeset(*firstChangeSet, codesLocksChangeSet, *dgndb, false);

            // Send codes/locks request
            ChangesetResponseOptions changesetOptions(IBriefcaseManager::ResponseOptions::None, ConflictStrategy::Continue);
            SendChunkedChangesetRequest(codesLocksChangeSet, changesetOptions, cancellationToken)
                ->Then([=](const StatusResult& codesLocksResult) {
                if (!codesLocksResult.IsSuccess() && codesLocksResult.GetError().GetId() != Error::Id::ConflictsAggregate)
                    return;

                PendingChangeSetStorage::RemoveItem(*dgndb, changeSetEntry->GetId());
                });
            }
        })->Then<StatusResult>([=]{
        LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method finished.");
        return StatusResult::Success();
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              02/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::InitializeChangeSet
(
Dgn::DgnRevisionPtr                 changeSet,
Dgn::DgnDbR                         dgndb,
JsonValueR                          pushJson,
ObjectId                            changeSetObjectId, 
PushChangeSetArgumentsPtr           pushArguments
) const
    {
    DgnDbP pDgnDb = &dgndb;
    //BeBriefcaseId briefcaseId = dgndb.GetBriefcaseId();
    std::shared_ptr<WSChangeset> changeset(new WSChangeset());

    //Set ChangeSet initialization request to ECChangeSet
    JsonValueR changeSetProperties = pushJson[ServerSchema::Instance][ServerSchema::Properties];
    changeSetProperties[ServerSchema::Property::IsUploaded] = true;
    changeset->AddInstance(changeSetObjectId, WSChangeset::ChangeState::Modified, std::make_shared<Json::Value>(changeSetProperties));

    //Push ChangeSet initialization request and Locks update in a single batch
    const Utf8String methodName = "iModelConnection::InitializeChangeSet";

    std::shared_ptr<StatusResult> finalResult = std::make_shared<StatusResult>();

    auto requestOptions = std::make_shared<WSRepositoryClient::RequestOptions>();
    requestOptions->SetTransferTimeOut(WSRepositoryClient::Timeout::Transfer::LongUpload);

    PendingChangeSetStorage::AddItem(*pDgnDb, changeSet->GetId());

    return SendChangesetRequest(changeset, pushArguments->GetResponseOptions(), pushArguments->GetCancelationToken(), requestOptions)
        ->Then([=](const StatusResult& initializeChangeSetResult)
        {
        if (!initializeChangeSetResult.IsSuccess())
            {
            PendingChangeSetStorage::RemoveItem(*pDgnDb, changeSet->GetId());
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, initializeChangeSetResult.GetError().GetMessage().c_str());
            finalResult->SetError(initializeChangeSetResult.GetError());
            return;
            }

        ChunkedWSChangeset codesLocksChangeSet;
        DgnCodeSet externalAssignedCodes, externalDiscardedCodes;
        ExtractCodesLocksChangeset(changeSet, codesLocksChangeSet, *pDgnDb, pushArguments->GetRelinquishCodesLocks(), &externalAssignedCodes, &externalDiscardedCodes);

        ChangesetResponseOptions changesetOptions(IBriefcaseManager::ResponseOptions::All, ConflictStrategy::Continue);

        bset<std::shared_ptr<AsyncTask>> tasks;
        auto imodelHubCodesLocksTask = SendChunkedChangesetRequest(codesLocksChangeSet, changesetOptions, pushArguments->GetCancelationToken(), requestOptions, 3, pushArguments->GetConflictsInfo())
            ->Then<StatusResult>([=](const StatusResult& codesLocksResult) {
            if (codesLocksResult.IsSuccess() || codesLocksResult.GetError().GetId() == Error::Id::ConflictsAggregate)
                PendingChangeSetStorage::RemoveItem(*pDgnDb, changeSet->GetId());

            return codesLocksResult;
            });
        tasks.insert(imodelHubCodesLocksTask);

        CodeCallbackFunction* codesCallback = pushArguments->GetCodesCallback();
        if (nullptr != codesCallback)
            {
            auto extenalCodesTask = (*codesCallback)(externalAssignedCodes, externalDiscardedCodes);
            tasks.insert(extenalCodesTask);
            }

        AsyncTask::WhenAll(tasks)->Then([=] ()
            {
            *finalResult = imodelHubCodesLocksTask->GetResult();
            });

        })->Then<StatusResult>([=]
            {
            return *finalResult;
            });
    }

/* Private methods end */

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
void CodeLockSetResultInfo::AddLock(const DgnLock dgnLock, BeBriefcaseId briefcaseId, Utf8StringCR changeSetId)
    {
    m_locks.insert(dgnLock);
    AddLockInfoToList(m_lockStates, dgnLock, briefcaseId, changeSetId);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
void CodeLockSetResultInfo::AddCode(const DgnCode dgnCode, DgnCodeState dgnCodeState, BeBriefcaseId briefcaseId)
    {
    m_codes.insert(dgnCode);
    AddCodeInfoToList(m_codeStates, dgnCode, dgnCodeState, briefcaseId);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   julius.cepukenas              08/2016
//---------------------------------------------------------------------------------------
void CodeLockSetResultInfo::Insert
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
void CodeLockSetResultInfo::Insert(const CodeLockSetResultInfo& resultInfo)
    {
    Insert(resultInfo.GetCodes(), resultInfo.GetCodeStates(), resultInfo.GetLocks(), resultInfo.GetLockStates());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas              08/2016
//---------------------------------------------------------------------------------------
bool CodeSequence::operator<(CodeSequence const& rhs) const
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

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
iModelConnection::~iModelConnection()
    {
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
iModelConnectionResult iModelConnection::Create
(
iModelInfoCR     iModel,
CredentialsCR    credentials,
ClientInfoPtr    clientInfo,
GlobalRequestOptionsPtr globalRequestOptions,
IHttpHandlerPtr  customHandler,
bool             enableCompression,
IAzureBlobStorageClientPtr storageClient
)
    {
    const Utf8String methodName = "iModelConnection::Create";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (iModel.GetId().empty())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid iModel id.");
        return iModelConnectionResult::Error(Error::Id::InvalidiModelId);
        }
    if (iModel.GetServerURL().empty())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid server URL.");
        return iModelConnectionResult::Error(Error::Id::InvalidServerURL);
        }
    if (!credentials.IsValid() && !customHandler)
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return iModelConnectionResult::Error(Error::Id::CredentialsNotSet);
        }

    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    iModelConnectionPtr imodelConnection(new iModelConnection(iModel, credentials, clientInfo, globalRequestOptions, customHandler, enableCompression, storageClient));

    double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
    return iModelConnectionResult::Success(imodelConnection);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             09/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::ValidateBriefcase(BeGuidCR fileId, BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken) const
    {
    return QueryBriefcaseInfo(briefcaseId, cancellationToken)->Then<StatusResult>([=](BriefcaseInfoResultCR result)
        {
        if (!result.IsSuccess())
            {
            return StatusResult::Error(result.GetError());
            }

        auto briefcase = result.GetValue();

        if (briefcase->GetFileId() != fileId)
            {
            return StatusResult::Error({Error::Id::InvalidBriefcase, ErrorLocalizedString(MESSAGE_BriefcaseOutdated)});
            }

        return StatusResult::Success();
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::LockiModel(ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::LockiModel";
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    return ExecuteWithRetry<void>([=]()
        {
        Json::Value iModelLockJson = Json::objectValue;
        iModelLockJson[ServerSchema::Instance] = Json::objectValue;
        iModelLockJson[ServerSchema::Instance][ServerSchema::SchemaName] = ServerSchema::Schema::iModel;
        iModelLockJson[ServerSchema::Instance][ServerSchema::ClassName] = ServerSchema::Class::iModelLock;

        return m_wsRepositoryClient->SendCreateObjectRequestWithOptions(iModelLockJson, BeFileName(), nullptr, requestOptions, cancellationToken)
            ->Then<StatusResult>([=](const WSCreateObjectResult& result)
            {
            if (!result.IsSuccess())
                {
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, result.GetError().GetMessage().c_str());
                return StatusResult::Error(result.GetError());
                }

            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), requestOptions, "");
            return StatusResult::Success();
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::UnlockiModel(ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::LockiModel";
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    return ExecuteWithRetry<void>([=]()
        {
        ObjectId id = ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::iModelLock, ServerSchema::Class::iModelLock);
        return m_wsRepositoryClient->SendDeleteObjectRequestWithOptions(id, requestOptions, cancellationToken)
            ->Then<StatusResult>([=](const WSDeleteObjectResult& result)
            {
            if (!result.IsSuccess())
                {
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, result.GetError().GetMessage().c_str());
                return StatusResult::Error(result.GetError());
                }

            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), requestOptions, "");
            return StatusResult::Success();
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
FileTaskPtr iModelConnection::UploadNewSeedFile(BeFileNameCR filePath, FileInfoCR fileInfo, bool waitForInitialized, 
                                                Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::UploadNewSeedFile";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    std::shared_ptr<FileResult> finalResult = std::make_shared<FileResult>();
    return ExecuteWithRetry<FileInfoPtr>([=]()
        {
        return CreateNewServerFile(fileInfo, cancellationToken)->Then([=](FileResultCR fileCreationResult)
            {
#if defined (ENABLE_BIM_CRASH_TESTS)
            BreakHelper::HitBreakpoint(Breakpoints::iModelConnection_AfterCreateNewServerFile);
#endif
            if (!fileCreationResult.IsSuccess())
                {
                finalResult->SetError(fileCreationResult.GetError());
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, fileCreationResult.GetError().GetMessage().c_str());
                return;
                }

            auto createdFileInfo = fileCreationResult.GetValue();
            if (!createdFileInfo->AreFileDetailsAvailable())
                {
                auto fileUpdateResult = UpdateServerFile(*createdFileInfo, cancellationToken)->GetResult();
                if (!fileUpdateResult.IsSuccess())
                    {
                    finalResult->SetError(fileUpdateResult.GetError());
                    LogHelper::Log(SEVERITY::LOG_WARNING, methodName, fileUpdateResult.GetError().GetMessage().c_str());
                    return;
                    }
                }
            finalResult->SetSuccess(createdFileInfo);

            AzureFileUpload(filePath, createdFileInfo->GetFileAccessKey(), callback, cancellationToken)->Then([=](StatusResultCR uploadResult)
                {
#if defined (ENABLE_BIM_CRASH_TESTS)
                BreakHelper::HitBreakpoint(Breakpoints::iModelConnection_AfterUploadServerFile);
#endif
                if (!uploadResult.IsSuccess() && Error::Id::MissingRequiredProperties != uploadResult.GetError().GetId())
                    {
                    finalResult->SetError(uploadResult.GetError());
                    LogHelper::Log(SEVERITY::LOG_WARNING, methodName, uploadResult.GetError().GetMessage().c_str());
                    return;
                    }
                InitializeServerFile(*createdFileInfo, cancellationToken)->Then([=](StatusResultCR initializationResult)
                    {
                    if (!initializationResult.IsSuccess())
                        {
                        finalResult->SetError(initializationResult.GetError());
                        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, initializationResult.GetError().GetMessage().c_str());
                        return;
                        }

                    if (waitForInitialized)
                        {
                        WaitForInitializedBIMFile(createdFileInfo->GetFileId(), finalResult, cancellationToken);
                        }
                    });
                });
            })->Then<FileResult>([=]()
                {
                double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
                return *finalResult;
                });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::CancelSeedFileCreation(ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::CancelSeedFileCreation";
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::File);
    Utf8String filter;
    filter.Sprintf("%s+gt+0", ServerSchema::Property::InitializationState);
    query.SetFilter(filter);

    std::shared_ptr<StatusResult> finalResult = std::make_shared<StatusResult>();
    return m_wsRepositoryClient->SendQueryRequestWithOptions(query, nullptr, nullptr, requestOptions, cancellationToken)->Then([=](WSObjectsResult const& result)
        {
        if (!result.IsSuccess())
            {
            finalResult->SetError(result.GetError());
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, result.GetError().GetMessage().c_str());
            return;
            }
        auto instances = result.GetValue().GetInstances();
        if (instances.IsEmpty())
            {
            finalResult->SetError({Error::Id::FileDoesNotExist, ErrorLocalizedString(MESSAGE_SeedFileNotFound)});
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, "File does not exist.");
            return;
            }

        auto fileInfo = FileInfo::Parse(*instances.begin(), FileInfo());
        m_wsRepositoryClient->SendDeleteObjectRequestWithOptions(fileInfo->GetObjectId(), requestOptions, cancellationToken)->Then([=](WSVoidResult const& deleteResult)
            {
            if (!deleteResult.IsSuccess())
                {
                finalResult->SetError(deleteResult.GetError());
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, deleteResult.GetError().GetMessage().c_str());
                }
            else
                {
                finalResult->SetSuccess();
                double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), requestOptions, "");
                }
            });
        })->Then<StatusResult>([=]()
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
FilesTaskPtr iModelConnection::GetSeedFiles(ICancellationTokenPtr cancellationToken) const
    {
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::File);
    return SeedFilesQuery(query, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::DownloadSeedFile(BeFileName localFile, Utf8StringCR fileId, Http::Request::ProgressCallbackCR callback, 
                                                 ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::DownloadSeedFile";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    ObjectId fileObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::File, fileId);
    return DownloadFile(localFile, fileObjectId, nullptr, callback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::AcquireCodesLocks
(
LockRequestCR                       locks,
DgnCodeSet                          codes,
BeBriefcaseId                       briefcaseId,
BeGuidCR                            seedFileId,
Utf8StringCR                        lastChangeSetId,
IBriefcaseManager::ResponseOptions  options,
ICancellationTokenPtr               cancellationToken
) const
    {
    return AcquireCodesLocksInternal(locks, codes, briefcaseId, seedFileId, lastChangeSetId, options, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::QueryCodesLocksAvailability
(
LockRequestCR                       locks,
DgnCodeSet                          codes,
BeBriefcaseId                       briefcaseId,
BeGuidCR                            seedFileId,
Utf8StringCR                        lastChangeSetId,
IBriefcaseManager::ResponseOptions  options,
ICancellationTokenPtr               cancellationToken
) const
    {
    ChunkedWSChangeset chunkedChangeset;

    MultiLockFormatter::SetToChunkedChangeset(locks.GetLockSet(), briefcaseId, seedFileId, lastChangeSetId, chunkedChangeset, WSChangeset::ChangeState::Modified,
                                   false, true);

    DgnCodeState state;
    state.SetReserved(briefcaseId);
    MultiCodeFormatter::SetToChunkedChangeset(codes, state, briefcaseId, chunkedChangeset, WSChangeset::ChangeState::Created, true);

    return SendChunkedChangesetRequest(chunkedChangeset, options, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::DemoteCodesLocks
(
const DgnLockSet&                       locks,
DgnCodeSet const&                       codes,
BeBriefcaseId                           briefcaseId,
BeGuidCR                                seedFileId,
IBriefcaseManager::ResponseOptions      options,
ICancellationTokenPtr                   cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::DemoteCodesLocks";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    CHECK_BRIEFCASEID(briefcaseId, StatusResult);
    //How to set description here?
    ChunkedWSChangeset chunkedChangeset;
    MultiLockFormatter::SetToChunkedChangeset(locks, briefcaseId, seedFileId, "", chunkedChangeset, WSChangeset::ChangeState::Modified);

    DgnCodeState state;
    state.SetAvailable();
    MultiCodeFormatter::SetToChunkedChangeset(codes, state, briefcaseId, chunkedChangeset, WSChangeset::ChangeState::Modified);

    return SendChunkedChangesetRequest(chunkedChangeset, options, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas           10/2017
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::RelinquishCodesLocksInternal
(
IBriefcaseManager::Resources            resourcesToRelinquish,
BeBriefcaseId                           briefcaseId,
IBriefcaseManager::ResponseOptions      options,
ICancellationTokenPtr                   cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::RelinquishCodesLocks";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    CHECK_BRIEFCASEID(briefcaseId, StatusResult);
    std::shared_ptr<WSChangeset> changeset(new WSChangeset());

    if (static_cast<bool>(resourcesToRelinquish & IBriefcaseManager::Resources::Locks))
        LockDeleteAllJsonRequest(changeset, briefcaseId);

    if (static_cast<bool>(resourcesToRelinquish & IBriefcaseManager::Resources::Codes))
        CodeDiscardReservedJsonRequest(changeset, briefcaseId);

    return SendChangesetRequestWithRetry(changeset, options, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::RelinquishCodesLocks
(
BeBriefcaseId                           briefcaseId,
IBriefcaseManager::ResponseOptions      options,
ICancellationTokenPtr                   cancellationToken
) const
    {
    return RelinquishCodesLocksInternal(IBriefcaseManager::Resources::All, briefcaseId, options, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
BriefcasesInfoTaskPtr iModelConnection::QueryAllBriefcasesInfo(ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::QueryAllBriefcasesInfo";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::Briefcase);
    return QueryBriefcaseInfoInternal(query, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
BriefcaseInfoTaskPtr iModelConnection::QueryBriefcaseInfo(BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::QueryBriefcaseInfo";
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");
    CHECK_BRIEFCASEID(briefcaseId, BriefcaseInfoResult);

    Utf8String briefcaseIdString;
    briefcaseIdString.Sprintf("%d", briefcaseId.GetValue());
    ObjectId briefcaseObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::Briefcase, briefcaseIdString);

    return m_wsRepositoryClient->SendGetObjectRequestWithOptions(briefcaseObjectId, nullptr, requestOptions, cancellationToken)
        ->Then<BriefcaseInfoResult>([=](WSObjectsResult const& result)
        {
        if (!result.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, result.GetError().GetMessage().c_str());
            WSError error = result.GetError();
            if (WSError::Id::InstanceNotFound == error.GetId())
                return BriefcaseInfoResult::Error(Error(Error::Id::BriefcaseDoesNotExist, error.GetMessage(), error.GetDescription()));
            return BriefcaseInfoResult::Error(error);
            }
        BriefcaseInfoPtr briefcaseInfo = BriefcaseInfo::Parse(*result.GetValue().GetInstances().begin());
        return BriefcaseInfoResult::Success(briefcaseInfo);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
BriefcasesInfoTaskPtr iModelConnection::QueryBriefcasesInfo
(
bvector<BeSQLite::BeBriefcaseId>& briefcaseIds,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::QueryBriefcasesInfo";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    std::deque<ObjectId> queryIds;
    for (auto& id : briefcaseIds)
        {
        Utf8String idString;
        idString.Sprintf("%lu", id.GetValue());
        queryIds.push_back(ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::Briefcase, idString));
        }

    bset<BriefcasesInfoTaskPtr> tasks;
    while (!queryIds.empty())
        {
        WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::Briefcase);
        query.AddFilterIdsIn(queryIds);
        auto task = QueryBriefcaseInfoInternal(query, cancellationToken);

        tasks.insert(task);
        }

    auto finalValue = std::make_shared<bvector<BriefcaseInfoPtr>>();

    return AsyncTask::WhenAll(tasks)
        ->Then<BriefcasesInfoResult>([=]
        {
        for (auto& task : tasks)
            {
            auto taskResultPtr = ExecuteAsync(task);
            if (!taskResultPtr->IsSuccess())
                return BriefcasesInfoResult::Error(taskResultPtr->GetError());

            auto briefcaseInfo = taskResultPtr->GetValue();
            finalValue->insert(finalValue->end(), briefcaseInfo.begin(), briefcaseInfo.end());
            }

        return BriefcasesInfoResult::Success(*finalValue);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
CodeLockSetTaskPtr ExecuteCodesLocksQueryTasks
(
bset<StatusTaskPtr> tasks,
CodeLockSetResultInfoPtr finalValue
)
    {
    return AsyncTask::WhenAll(tasks)
        ->Then<CodeLockSetResult>([=]
        {
        for (auto task : tasks)
            {
            auto taskResult = ExecuteAsync(task);
            if (!taskResult->IsSuccess())
                return CodeLockSetResult::Error(taskResult->GetError());
            }
        return CodeLockSetResult::Success(*finalValue);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis             01/2018
//---------------------------------------------------------------------------------------
CodeLockSetTaskPtr iModelConnection::QueryAllCodesLocks
(
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::QueryAllCodesLocks";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    
    CodeLockSetResultInfoPtr finalValue = new CodeLockSetResultInfo();
    bset<StatusTaskPtr> tasks;

    WSQuery queryCodes(ServerSchema::Schema::iModel, ServerSchema::Class::MultiCode);
    WSQuery queryLocks(ServerSchema::Schema::iModel, ServerSchema::Class::MultiLock);

    tasks.insert(QueryCodesLocksByChunks(queryCodes, finalValue, AddMultiCodes, cancellationToken));
    tasks.insert(QueryCodesLocksByChunks(queryLocks, finalValue, AddMultiLocks, cancellationToken));

    return ExecuteCodesLocksQueryTasks(tasks, finalValue);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
CodeLockSetTaskPtr iModelConnection::QueryCodesLocksById
(
DgnCodeSet const& codes,
LockableIdSet const& locks,
ICancellationTokenPtr cancellationToken
) const
    {
    return QueryCodesLocksByIdInternal(codes, locks, BeBriefcaseId(), cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
CodeLockSetTaskPtr iModelConnection::QueryCodesLocksById
(
DgnCodeSet const& codes,
LockableIdSet const& locks,
BeBriefcaseId briefcaseId,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::QueryCodesLocksById";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    CHECK_BRIEFCASEID(briefcaseId, CodeLockSetResult);
    return QueryCodesLocksByIdInternal(codes, locks, briefcaseId, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis        01/2018
//---------------------------------------------------------------------------------------
CodeLockSetTaskPtr iModelConnection::QueryCodesLocksByIdInternal
(
DgnCodeSet const& codes,
LockableIdSet const& locks,
BeBriefcaseId briefcaseId,
ICancellationTokenPtr cancellationToken
) const
    {
    CodeLockSetResultInfoPtr finalValue = new CodeLockSetResultInfo();
    bset<StatusTaskPtr> tasks;
    
    if (0 < codes.size())
        tasks.insert(QueryCodesInternal(codes, briefcaseId, finalValue, cancellationToken));

    if (0 < locks.size())
        tasks.insert(QueryLocksInternal(locks, briefcaseId, finalValue, cancellationToken));

    return ExecuteCodesLocksQueryTasks(tasks, finalValue);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
CodeLockSetTaskPtr iModelConnection::QueryCodesLocks
(
const BeBriefcaseId  briefcaseId,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::QueryCodesLocks";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    CHECK_BRIEFCASEID(briefcaseId, CodeLockSetResult);

    bset<StatusTaskPtr> tasks;
    CodeLockSetResultInfoPtr finalValue = new CodeLockSetResultInfo();

    //Query codes locks by briefcase id
    if (!briefcaseId.IsValid())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "BriefcaseId is invalid.");
        return CreateCompletedAsyncTask<CodeLockSetResult>(CodeLockSetResult::Error(Error::Id::InvalidBriefcase));
        }

    tasks.insert(QueryCodesInternal(briefcaseId, finalValue, cancellationToken));
    tasks.insert(QueryLocksInternal(briefcaseId, finalValue, cancellationToken));

    return ExecuteCodesLocksQueryTasks(tasks, finalValue);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis             01/2018
//---------------------------------------------------------------------------------------
CodeInfoSetTaskPtr iModelConnection::QueryAllCodes
(
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::QueryAllCodes";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::MultiCode);
    CodeLockSetResultInfoPtr finalValue = new CodeLockSetResultInfo();

    auto result = ExecuteAsync(QueryCodesLocksByChunks(query, finalValue, AddMultiCodes, cancellationToken));

    if (result->IsSuccess())
        return CreateCompletedAsyncTask(CodeInfoSetResult::Success(finalValue->GetCodeStates()));
    return CreateCompletedAsyncTask(CodeInfoSetResult::Error(result->GetError()));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis             01/2018
//---------------------------------------------------------------------------------------
CodeInfoSetTaskPtr iModelConnection::QueryCodesByIds
(
DgnCodeSet const& codes,
ICancellationTokenPtr cancellationToken
) const
    {
    return QueryCodesByIdsInternal(codes, BeBriefcaseId(), cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis             01/2018
//---------------------------------------------------------------------------------------
CodeInfoSetTaskPtr iModelConnection::QueryCodesByIds
(
DgnCodeSet const& codes,
BeBriefcaseId briefcaseId,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::QueryCodesByIds";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    CHECK_BRIEFCASEID(briefcaseId, CodeInfoSetResult);
    return QueryCodesByIdsInternal(codes, briefcaseId, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis        01/2018
//---------------------------------------------------------------------------------------
CodeInfoSetTaskPtr iModelConnection::QueryCodesByIdsInternal
(
DgnCodeSet const& codes,
BeBriefcaseId briefcaseId,
ICancellationTokenPtr cancellationToken
) const
    {
    CodeLockSetResultInfoPtr finalValue = new CodeLockSetResultInfo();

    if (0 == codes.size())
        return CreateCompletedAsyncTask(CodeInfoSetResult::Success(finalValue->GetCodeStates()));

    auto result = ExecuteAsync(QueryCodesInternal(codes, briefcaseId, finalValue, cancellationToken));

    if (result->IsSuccess())
        return CreateCompletedAsyncTask(CodeInfoSetResult::Success(finalValue->GetCodeStates()));
    return CreateCompletedAsyncTask(CodeInfoSetResult::Error(result->GetError()));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis             01/2018
//---------------------------------------------------------------------------------------
CodeInfoSetTaskPtr iModelConnection::QueryCodesByBriefcaseId
(
BeBriefcaseId briefcaseId,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::QueryCodesByBriefcaseId";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    CHECK_BRIEFCASEID(briefcaseId, CodeInfoSetResult);

    CodeLockSetResultInfoPtr finalValue = new CodeLockSetResultInfo();

    if (!briefcaseId.IsValid())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "BriefcaseId is invalid.");
        return CreateCompletedAsyncTask(CodeInfoSetResult::Error(Error::Id::InvalidBriefcase));
        }

    auto result = ExecuteAsync(QueryCodesInternal(briefcaseId, finalValue, cancellationToken));

    if (result->IsSuccess())
        return CreateCompletedAsyncTask(CodeInfoSetResult::Success(finalValue->GetCodeStates()));
    return CreateCompletedAsyncTask(CodeInfoSetResult::Error(result->GetError()));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis             01/2018
//---------------------------------------------------------------------------------------
LockInfoSetTaskPtr iModelConnection::QueryAllLocks
(
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::QueryAllLocks";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::MultiLock);
    CodeLockSetResultInfoPtr finalValue = new CodeLockSetResultInfo();

    auto result = ExecuteAsync(QueryCodesLocksByChunks(query, finalValue, AddMultiLocks, cancellationToken));

    if (result->IsSuccess())
        return CreateCompletedAsyncTask(LockInfoSetResult::Success(finalValue->GetLockStates()));
    return CreateCompletedAsyncTask(LockInfoSetResult::Error(result->GetError()));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis             01/2018
//---------------------------------------------------------------------------------------
LockInfoSetTaskPtr iModelConnection::QueryLocksByIds
(
LockableIdSet const& locks,
ICancellationTokenPtr cancellationToken
) const
    {
    return QueryLocksByIdsInternal(locks, BeBriefcaseId(), cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis             01/2018
//---------------------------------------------------------------------------------------
LockInfoSetTaskPtr iModelConnection::QueryLocksByIds
(
LockableIdSet const& locks,
BeBriefcaseId briefcaseId,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::QueryLocksByIds";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    CHECK_BRIEFCASEID(briefcaseId, LockInfoSetResult);
    return QueryLocksByIdsInternal(locks, briefcaseId, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis        01/2018
//---------------------------------------------------------------------------------------
LockInfoSetTaskPtr iModelConnection::QueryLocksByIdsInternal
(
LockableIdSet const& locks,
BeBriefcaseId briefcaseId,
ICancellationTokenPtr cancellationToken
) const
    {
    CodeLockSetResultInfoPtr finalValue = new CodeLockSetResultInfo();

    if (0 == locks.size())
        return CreateCompletedAsyncTask(LockInfoSetResult::Success(finalValue->GetLockStates()));

    auto result = ExecuteAsync(QueryLocksInternal(locks, briefcaseId, finalValue, cancellationToken));

    if (result->IsSuccess())
        return CreateCompletedAsyncTask(LockInfoSetResult::Success(finalValue->GetLockStates()));
    return CreateCompletedAsyncTask(LockInfoSetResult::Error(result->GetError()));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis             01/2018
//---------------------------------------------------------------------------------------
LockInfoSetTaskPtr iModelConnection::QueryLocksByBriefcaseId
(
BeBriefcaseId briefcaseId,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::QueryLocksByBriefcaseId";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    CHECK_BRIEFCASEID(briefcaseId, LockInfoSetResult);

    if (!briefcaseId.IsValid())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "BriefcaseId is invalid.");
        return CreateCompletedAsyncTask(LockInfoSetResult::Error(Error::Id::InvalidBriefcase));
        }

    CodeLockSetResultInfoPtr finalValue = new CodeLockSetResultInfo();
    auto result = ExecuteAsync(QueryLocksInternal(briefcaseId, finalValue, cancellationToken));

    if (result->IsSuccess())
        return CreateCompletedAsyncTask(LockInfoSetResult::Success(finalValue->GetLockStates()));
    return CreateCompletedAsyncTask(LockInfoSetResult::Error(result->GetError()));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             06/2016
//---------------------------------------------------------------------------------------
CodeLockSetTaskPtr iModelConnection::QueryUnavailableCodesLocks
(
const BeBriefcaseId   briefcaseId,
Utf8StringCR          lastChangeSetId,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::QueryUnavailableCodesLocks";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    CHECK_BRIEFCASEID(briefcaseId, CodeLockSetResult);

    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    auto changeSetResult = ExecuteAsync(GetChangeSetById(lastChangeSetId, cancellationToken));
    uint64_t changeSetIndex = 0;
    if (changeSetResult->IsSuccess())
        {
        changeSetIndex = changeSetResult->GetValue()->GetIndex();
        }
    else if (changeSetResult->GetError().GetId() != Error::Id::InvalidChangeSet)
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, changeSetResult->GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<CodeLockSetResult>(CodeLockSetResult::Error(changeSetResult->GetError()));
        }

    CodeLockSetResultInfoPtr finalValue = new CodeLockSetResultInfo();
    bset<StatusTaskPtr> tasks;

    tasks.insert(QueryUnavailableCodesInternal(briefcaseId, finalValue, cancellationToken));
    tasks.insert(QueryUnavailableLocksInternal(briefcaseId, changeSetIndex, finalValue, cancellationToken));

    auto tasksResult = ExecuteCodesLocksQueryTasks(tasks, finalValue)->GetResult();
    
    if (!tasksResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, tasksResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<CodeLockSetResult>(CodeLockSetResult::Error(tasksResult.GetError()));
        }

    double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
    return CreateCompletedAsyncTask<CodeLockSetResult>(CodeLockSetResult::Success(tasksResult.GetValue()));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             04/2017
//---------------------------------------------------------------------------------------
CodeSequenceTaskPtr iModelConnection::QueryCodeMaximumIndex
(
CodeSequenceCR codeSequence,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::QueryCodeMaximumIndex";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    auto status = SetCodeSequencesJsonRequestToChangeSet(codeSequence, 0, 1, CodeSequence::Type::Maximum, *changeset, 
                                                         WSChangeset::ChangeState::Created);
    if (DgnDbStatus::Success != status)
        return CreateCompletedAsyncTask<CodeSequenceResult>(CodeSequenceResult::Error({Error::Id::OperationFailed, 
                                                                                      ErrorLocalizedString(MESSAGE_CodeSequenceRequestError)}));

    return QueryCodeMaximumIndexInternal(changeset, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             03/2017
//---------------------------------------------------------------------------------------
CodeSequenceTaskPtr iModelConnection::QueryCodeNextAvailable
(
CodeSequenceCR codeSequence,
int startIndex,
int incrementBy,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::QueryCodeNextAvailable";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    auto status = SetCodeSequencesJsonRequestToChangeSet(codeSequence, startIndex, incrementBy, CodeSequence::Type::NextAvailable, *changeset, 
                                                         WSChangeset::ChangeState::Created);
    if (DgnDbStatus::Success != status)
        return CreateCompletedAsyncTask<CodeSequenceResult>(CodeSequenceResult::Error({Error::Id::OperationFailed, 
                                                                                      ErrorLocalizedString(MESSAGE_CodeSequenceRequestError)}));

    return QueryCodeNextAvailableInternal(changeset, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             09/2016
//---------------------------------------------------------------------------------------
BriefcaseInfoTaskPtr iModelConnection::AcquireNewBriefcase(ICancellationTokenPtr cancellationToken) const
    {
    return ExecuteWithRetry<BriefcaseInfoPtr>([=]()
        {
        return CreateBriefcaseInstance(cancellationToken)->Then<BriefcaseInfoResult>([=](const WSCreateObjectResult& result)
            {
            if (!result.IsSuccess())
                {
                return BriefcaseInfoResult::Error(result.GetError());
                }

            Json::Value json;
            result.GetValue().GetJson(json);
            JsonValueCR instance = json[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
            return BriefcaseInfoResult::Success(BriefcaseInfo::ParseRapidJson(ToRapidJson(instance[ServerSchema::Properties])));
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2019
//---------------------------------------------------------------------------------------
ChangeSetsInfoTaskPtr iModelConnection::GetChangeSets(ChangeSetQuery query, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::GetChangeSets";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    
    return GetChangeSetsFromQueryByChunks(query.GetWSQuery(), false, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              03/2016
//---------------------------------------------------------------------------------------
ChangeSetsInfoTaskPtr iModelConnection::GetAllChangeSets(ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::GetAllChangeSets";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    return GetAllChangeSetsInternal(false, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
ChangeSetInfoTaskPtr iModelConnection::GetChangeSetById
(
Utf8StringCR          changeSetId,
ICancellationTokenPtr cancellationToken
) const
    {
    return GetChangeSetByIdInternal(changeSetId, false, cancellationToken);
    }

/* EventService Methods Begin */

/* Public methods start */

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas            12/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::SubscribeEventsCallback(EventTypeSet* eventTypes, EventCallbackPtr callback)
    {
    return SubscribeEventsCallback(eventTypes, callback, this);
    }

/* Public methods end */

/* EventService Methods End */

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
ChangeSetsInfoTaskPtr iModelConnection::GetChangeSetsAfterId
(
Utf8StringCR          changeSetId,
BeGuidCR              fileId,
ICancellationTokenPtr cancellationToken
) const
    {
    return GetChangeSetsAfterIdInternal(changeSetId, fileId, false, cancellationToken);
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite           08/2017
//---------------------------------------------------------------------------------------
ChangeSetQuery iModelConnection::CreateBetweenChangeSetsQuery
(
Utf8StringCR firstchangeSetId,
Utf8StringCR secondChangeSetId,
BeSQLite::BeGuidCR fileId
) const
    {
    ChangeSetQuery changeSetQuery;
    changeSetQuery.FilterChangeSetsBetween(firstchangeSetId, secondChangeSetId);
    changeSetQuery.FilterBySeedFileId(fileId);

    return changeSetQuery;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite           08/2017
//---------------------------------------------------------------------------------------
ChangeSetsInfoTaskPtr iModelConnection::GetChangeSetsBetween(Utf8StringCR firstChangeSetId, Utf8StringCR secondChangeSetId, 
                                                             BeSQLite::BeGuidCR fileId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::GetChangeSetsBetween";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (Utf8String::IsNullOrEmpty(firstChangeSetId.c_str()) && Utf8String::IsNullOrEmpty(secondChangeSetId.c_str()))
        return CreateCompletedAsyncTask(ChangeSetsInfoResult::Error(Error::Id::InvalidChangeSet));

    ChangeSetQuery changeSetQuery = CreateBetweenChangeSetsQuery(firstChangeSetId, secondChangeSetId, fileId);
    return GetChangeSetsFromQueryByChunks(changeSetQuery.GetWSQuery(), false, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
ChangeSetsTaskPtr iModelConnection::DownloadChangeSets(const bvector<Utf8String>& changeSetIds, Http::Request::ProgressCallbackCR callback, 
                                                       ICancellationTokenPtr cancellationToken) const
    {
    std::deque<ObjectId> queryIds;
    for (auto changeSetId : changeSetIds)
        {
        queryIds.push_back(ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet, changeSetId));
        }

    return DownloadChangeSets(queryIds, callback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
ChangeSetsTaskPtr iModelConnection::DownloadChangeSets(const bvector<ChangeSetInfoPtr>& changeSets, Http::Request::ProgressCallbackCR callback, 
                                                       ICancellationTokenPtr cancellationToken) const
    {
    std::deque<ObjectId> queryIds;
    for (auto changeSet : changeSets)
        {
        queryIds.push_back(ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet, changeSet->GetId()));
        }

    return DownloadChangeSets(queryIds, callback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
ChangeSetsTaskPtr iModelConnection::DownloadChangeSetsAfterId
(
Utf8StringCR                        changeSetId,
BeGuidCR                            fileId,
Http::Request::ProgressCallbackCR   callback,
ICancellationTokenPtr               cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::DownloadChangeSetsAfterId";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    std::shared_ptr<ChangeSetsResult> finalResult = std::make_shared<ChangeSetsResult>();
    return GetChangeSetsAfterIdInternal(changeSetId, fileId, true, cancellationToken)->Then([=](ChangeSetsInfoResultCR changeSetsResult)
        {
        if (changeSetsResult.IsSuccess())
            {
            DownloadChangeSetsInternal(changeSetsResult.GetValue(), callback, cancellationToken)->Then([=](ChangeSetsResultCR downloadResult)
                {
                if (downloadResult.IsSuccess())
                    {
                    double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                    LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
                    finalResult->SetSuccess(downloadResult.GetValue());
                    }
                else
                    {
                    LogHelper::Log(SEVERITY::LOG_WARNING, methodName, downloadResult.GetError().GetMessage().c_str());
                    finalResult->SetError(downloadResult.GetError());
                    }
                });
            }
        else
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, changeSetsResult.GetError().GetMessage().c_str());
            finalResult->SetError(changeSetsResult.GetError());
            }
        })->Then<ChangeSetsResult>([=]()
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::VerifyConnection(ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::VerifyConnection";
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");

    return ExecuteWithRetry<void>([=]()
        {
        return m_wsRepositoryClient->VerifyAccessWithOptions(requestOptions, cancellationToken)->Then<StatusResult>([=](const AsyncResult<void, WSError>& result)
            {
            if (result.IsSuccess())
                return StatusResult::Success();
            else
                {
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, requestOptions, result.GetError().GetMessage().c_str());
                return StatusResult::Error(result.GetError());
                }
            });
        });
    }
