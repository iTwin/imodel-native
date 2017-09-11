/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/iModelConnection.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/iModelConnection.h>
#include <DgnPlatform/RevisionManager.h>
#include <WebServices/Client/WSChangeset.h>
#include "Utils.h"
#include "Logging.h"
#include <WebServices/iModelHub/Client/BreakHelper.h>
#include <WebServices/iModelHub/Client/UserInfoManager.h>
#include "Events/EventManager.h"
#include "PredownloadManager.h"
#include <WebServices/iModelHub/Events/ChangeSetPostPushEvent.h>
#include <WebServices/iModelHub/Client/ChangeSetInfo.h>

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGN

# define MAX_AsyncQueries 10

PredownloadManagerPtr iModelConnection::s_preDownloadManager = new PredownloadManager();

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
iModelConnection::iModelConnection
(
iModelInfoCR           iModel,
WebServices::CredentialsCR credentials,
WebServices::ClientInfoPtr clientInfo,
IHttpHandlerPtr            customHandler
) : m_iModelInfo(iModel)
    {
    auto wsRepositoryClient = WSRepositoryClient::Create(iModel.GetServerURL(), iModel.GetWSRepositoryName(), clientInfo, nullptr, customHandler);
    CompressionOptions options;
    options.EnableRequestCompression(true, 1024);
    wsRepositoryClient->Config().SetCompressionOptions(options);
    wsRepositoryClient->SetCredentials(credentials);
    wsRepositoryClient->GetWSClient()->EnableWsgServerHeader(true);

    SetRepositoryClient(wsRepositoryClient);
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

    if (typeid(NotUsedFileAccessKey) == typeid(*fileAccessKey))
        {
        // This environment does not use fileAccessKey
        return ExecutionManager::ExecuteWithRetry<void>([=]() {
            return m_wsRepositoryClient->SendGetFileRequest(fileId, localFile, nullptr, callback, cancellationToken)
                ->Then<StatusResult>([=](const WSFileResult& fileResult)
                {
                if (!fileResult.IsSuccess())
                    {
                    LogHelper::Log(SEVERITY::LOG_ERROR, methodName, fileResult.GetError().GetMessage().c_str());
                    return StatusResult::Error(fileResult.GetError());
                    }

                double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
                return StatusResult::Success();
                });
            });
        }

    // Download file directly from the url.
    return ExecutionManager::ExecuteWithRetry<void>([=]() {
        return m_azureClient->SendGetFileRequest(fileAccessKey->GetDownloadUrl(), localFile, callback, cancellationToken)
            ->Then<StatusResult>([=](const AzureResult& result)
            {
            if (!result.IsSuccess())
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                return StatusResult::Error(Error(result.GetError()));
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
        auto fileAccessKeyResult = QueryFileAccessKey(fileId, cancellationToken)->GetResult();
        if (!fileAccessKeyResult.IsSuccess())
            return CreateCompletedAsyncTask(StatusResult::Error(fileAccessKeyResult.GetError()));

        fileAccessKey = fileAccessKeyResult.GetValue();
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
    const Utf8String methodName = "iModelConnection::QueryFileAccessKey";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    
    WSQuery query(objectId);
    Utf8String selectString = "$id";
    FileAccessKey::AddDownloadAccessKeySelect(selectString);
    query.SetSelect(selectString);

    return m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)
        ->Then<FileAccessKeyResult>([=](WSObjectsResult const& result)
        {
        if (!result.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return FileAccessKeyResult::Error(result.GetError());
            }

        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
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
    const Utf8String methodName = "iModelConnection::CreateNewServerFile";
    std::shared_ptr<FileResult> finalResult = std::make_shared<FileResult>();
    return m_wsRepositoryClient->SendCreateObjectRequest(CreateFileJson(fileInfo), BeFileName(), nullptr, cancellationToken)->Then
        ([=] (const WSCreateObjectResult& result)
        {
        if (result.IsSuccess())
            {
            Json::Value json;
            result.GetValue().GetJson(json);
            JsonValueCR instance = json[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
            auto fileInfoPtr = FileInfo::Parse(ToRapidJson(instance[ServerSchema::Properties]), instance[ServerSchema::InstanceId].asString(), fileInfo);
            fileInfoPtr->SetFileAccessKey(FileAccessKey::ParseFromRelated(instance));

            finalResult->SetSuccess(fileInfoPtr);
            return;
            }

        auto error = Error(result.GetError());
        if (Error::Id::FileAlreadyExists != error.GetId())
            {
            finalResult->SetError(error);
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, error.GetMessage().c_str());
            return;
            }

        bool initialized = error.GetExtendedData()[ServerSchema::Property::FileInitialized].asBool();

        if (initialized)
            {
            finalResult->SetError(error);
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, error.GetMessage().c_str());
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

        m_wsRepositoryClient->SendQueryRequest(fileQuery, nullptr, nullptr, cancellationToken)->Then([=] (WSObjectsResult const& queryResult)
            {
            if (!queryResult.IsSuccess())
                {
                finalResult->SetError(queryResult.GetError());
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, queryResult.GetError().GetMessage().c_str());
                return;
                }
            
            if (queryResult.GetValue().GetRapidJsonDocument().IsNull())
                {
                finalResult->SetError(error);
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, error.GetMessage().c_str());
                return;
                }

            auto resultJson = *queryResult.GetValue().GetInstances().begin();
            auto downloadFileResult = FileInfo::Parse(resultJson, fileInfo);
            downloadFileResult->SetFileAccessKey(FileAccessKey::ParseFromRelated(resultJson));

            finalResult->SetSuccess(downloadFileResult);
            });

        })->Then<FileResult>([=] ()
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::AzureFileUpload(BeFileNameCR filePath, FileAccessKeyPtr url, Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::AzureFileUpload";
    return m_azureClient->SendUpdateFileRequest(url->GetUploadUrl(), filePath, callback, cancellationToken)
        ->Then<StatusResult>([=] (const AzureResult& result)
        {
        if (!result.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return StatusResult::Error(Error(result.GetError()));
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
    return m_wsRepositoryClient->SendUpdateObjectRequest(fileInfo.GetObjectId(), properties, nullptr, BeFileName(), nullptr, cancellationToken)->Then<StatusResult>
    ([=] (const WSUpdateObjectResult& result)
        {
        if (!result.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
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

    return m_wsRepositoryClient->SendUpdateObjectRequest(fileInfo.GetObjectId(), fileProperties, nullptr, BeFileName(), nullptr, cancellationToken)
        ->Then<StatusResult>([=] (const WSUpdateObjectResult& initializeFileResult)
        {
        if (!initializeFileResult.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, initializeFileResult.GetError().GetMessage().c_str());
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
    return ExecutionManager::ExecuteWithRetry<bvector<FileInfoPtr>>([=]() {
        return m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)->Then<FilesResult>([=] (WSObjectsResult const& result)
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

    Dgn::DgnDbPtr db = Dgn::DgnDb::OpenDgnDb (&status, filePath, Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite));
    if (BeSQLite::DbResult::BE_SQLITE_OK == status && db.IsValid())
        {
        StatusResult result = m_iModelInfo.WriteiModelInfo (*db, briefcaseId);
#if defined (ENABLE_BIM_CRASH_TESTS)
        BreakHelper::HitBreakpoint(Breakpoints::iModelConnection_AfterWriteiModelInfo);
#endif
        db->CloseDb();
        return result;
        }
    else
        {
        auto error = Error(db, status);
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, error.GetMessage().c_str());
        if (db.IsValid())
            db->CloseDb();
        return StatusResult::Error(error);
        }

    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             03/2015
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::SendChangesetRequest
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
StatusTaskPtr iModelConnection::SendChangesetRequestInternal
(
std::shared_ptr<WSChangeset> changeset,
IBriefcaseManager::ResponseOptions options,
ICancellationTokenPtr cancellationToken,
IWSRepositoryClient::RequestOptionsPtr requestOptions
) const
    {
    const Utf8String methodName = "iModelConnection::SendChangesetRequest";

    changeset->GetRequestOptions().SetResponseContent(WSChangeset::Options::ResponseContent::Empty);
    
    if (static_cast<bool>(options & IBriefcaseManager::ResponseOptions::UnlimitedReporting))
        changeset->GetRequestOptions().SetCustomOption(ServerSchema::ExtendedParameters::SetMaximumInstances, "-1");

    if (!static_cast<bool>(options & IBriefcaseManager::ResponseOptions::LockState))
        changeset->GetRequestOptions().SetCustomOption(ServerSchema::ExtendedParameters::DetailedError_Locks, "false");

    if (!static_cast<bool>(options & IBriefcaseManager::ResponseOptions::CodeState))
        changeset->GetRequestOptions().SetCustomOption(ServerSchema::ExtendedParameters::DetailedError_Codes, "false");

    HttpStringBodyPtr request = HttpStringBody::Create(changeset->ToRequestString());
    return m_wsRepositoryClient->SendChangesetRequest(request, nullptr, cancellationToken, requestOptions)->Then<StatusResult>
        ([=] (const WSChangesetResult& result)
        {
        if (result.IsSuccess())
            return StatusResult::Success();
        else
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return StatusResult::Error(result.GetError());
            }
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
//@bsimethod                                     Algirdas.Mikoliunas             07/2017
//---------------------------------------------------------------------------------------
Utf8String FormatBeInt64Id(BeInt64Id int64Value)
    {
    return int64Value.ToString(BeInt64Id::UseHex::Yes);
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

    properties[ServerSchema::Property::CodeSpecId]   = FormatBeInt64Id(firstCode->GetCodeSpecId());
    properties[ServerSchema::Property::CodeScope]    = firstCode->GetScopeString();
    properties[ServerSchema::Property::BriefcaseId]  = briefcaseId.GetValue();
    properties[ServerSchema::Property::State]        = DgnCodeStateToInt(state);
    properties[ServerSchema::Property::QueryOnly]    = queryOnly;

    properties[ServerSchema::Property::Values] = Json::arrayValue;
    int i = 0;
    for (auto const& code : codes)
        {
        properties[ServerSchema::Property::Values][i++] = code.GetValueUtf8();
        }

    return properties;
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

    ObjectId codeObject(ServerSchema::Schema::iModel, ServerSchema::Class::MultiCode, "MultiCode");
    JsonValueCR codeJson = CreateCodeInstanceJson(codes, state, briefcaseId, queryOnly);
    changeset.AddInstance(codeObject, changeState, std::make_shared<Json::Value>(codeJson));
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
BeInt64Id                        codeSpecId,
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
    
    idString.Sprintf("%s-%s-%s", FormatBeInt64Id(codeSpecId).c_str(), encodedScope.c_str(), encodedValue.c_str());

    return idString;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
Utf8String FormatCodeId
(
BeInt64Id                        codeSpecId,
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
void GroupCode
(
bmap<Utf8String, bvector<DgnCode>>* groupedCodes,
DgnCode searchCode
)
    {
    Utf8String searchKey = FormatCodeId(searchCode.GetCodeSpecId(), searchCode.GetScopeString(), "");
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
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
Json::Value CreateLockInstanceJson
(
bvector<BeInt64Id> const& ids,
BeBriefcaseId            briefcaseId,
BeGuidCR                 seedFileId,
Utf8StringCR             releasedWithChangeSetId,
LockableType             type,
LockLevel                level,
bool                     queryOnly
)
    {
    Json::Value properties;

    properties[ServerSchema::Property::BriefcaseId]           = briefcaseId.GetValue();
    properties[ServerSchema::Property::SeedFileId]            = seedFileId.ToString();
    properties[ServerSchema::Property::ReleasedWithChangeSet] = releasedWithChangeSetId;
    properties[ServerSchema::Property::QueryOnly]             = queryOnly;
    RepositoryJson::LockableTypeToJson(properties[ServerSchema::Property::LockType], type);
    RepositoryJson::LockLevelToJson(properties[ServerSchema::Property::LockLevel], level);

    properties[ServerSchema::Property::ObjectIds] = Json::arrayValue;
    int i = 0;
    for (auto const& id : ids)
        {
        properties[ServerSchema::Property::ObjectIds][i++] = FormatBeInt64Id(id);
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
bvector<BeInt64Id> const&        ids,
BeBriefcaseId                    briefcaseId,
BeGuidCR                         seedFileId,
Utf8StringCR                     releasedWithChangeSetId,
LockableType                     type,
LockLevel                        level,
bool                             queryOnly
)
    {
    if (ids.empty ())
        return;
    ObjectId lockObject (ServerSchema::Schema::iModel, ServerSchema::Class::MultiLock, "MultiLock");
    auto json = std::make_shared<Json::Value>(CreateLockInstanceJson(ids, briefcaseId, seedFileId, releasedWithChangeSetId, type, level, queryOnly));
    changeset.AddInstance (lockObject, changeState, json);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             12/2015
//---------------------------------------------------------------------------------------
void SetLocksJsonRequestToChangeSet
(
const DgnLockSet&               locks,
BeBriefcaseId                   briefcaseId,
BeGuidCR                        seedFileId,
Utf8StringCR                    releasedWithChangeSetId,
WSChangeset&                    changeset,
const WSChangeset::ChangeState& changeState,
bool                            includeOnlyExclusive = false,
bool                            queryOnly = false
)
    {
    bvector<BeInt64Id> objects[12];
    for (auto& lock : locks)
        {
        if (includeOnlyExclusive && LockLevel::Exclusive != lock.GetLevel ())
            continue;

        int index = static_cast<int32_t>(lock.GetType ()) * 3 + static_cast<int32_t>(lock.GetLevel ());
        if (index >= 0 && index <= 11)
            objects[index].push_back (lock.GetId ());
        }

    for (int i = 0; i < 12; ++i)
        AddToInstance(changeset, changeState, objects[i], briefcaseId, seedFileId, releasedWithChangeSetId, static_cast<LockableType>(i / 3), static_cast<LockLevel>(i % 3), queryOnly);
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
    auto status = CreateCodeSequenceJson(codeJson, codeSequence.GetCodeSpecId(), codeSequence.GetScope(), codeSequence.GetValuePattern(), templateType, startIndex, incrementBy);
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

    ObjectId lockObject (ServerSchema::Schema::iModel, ServerSchema::Class::Lock, id);

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
const BeBriefcaseId*  briefcaseId
)
    {
    Utf8String idString;
    if (nullptr == briefcaseId)
        idString.Sprintf("%d-%s", (int) lock.GetType(), FormatBeInt64Id(lock.GetId()).c_str());
    else
        idString.Sprintf("%d-%s-%u", (int) lock.GetType(), FormatBeInt64Id(lock.GetId()).c_str(), briefcaseId->GetValue());

    return ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::Lock, idString);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
ObjectId GetCodeId
(
DgnCodeCR code,
const BeBriefcaseId*  briefcaseId
)
    {
    Utf8String idString;
    if (nullptr != briefcaseId)
        idString.Sprintf("%s", FormatCodeId(code.GetCodeSpecId(), code.GetScopeString(), code.GetValueUtf8(), *briefcaseId).c_str());
    else
        idString.Sprintf("%s", FormatCodeId(code.GetCodeSpecId(), code.GetScopeString(), code.GetValueUtf8()).c_str());

    return ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::Code, idString);
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
    const Utf8String methodName = "iModelConnection::AcquireCodesLocks";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    
    SetLocksJsonRequestToChangeSet(locks.GetLockSet(), briefcaseId, seedFileId, lastChangeSetId, *changeset, WSChangeset::ChangeState::Modified);
    
    DgnCodeState state;
    state.SetReserved(briefcaseId);
    SetCodesJsonRequestToChangeSet(codes, state, briefcaseId, *changeset, WSChangeset::ChangeState::Created);

    return SendChangesetRequestInternal(changeset, options, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
BriefcasesInfoTaskPtr iModelConnection::QueryBriefcaseInfoInternal(WSQuery const& query, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::QueryBriefcaseInfoInternal";
    return ExecutionManager::ExecuteWithRetry<bvector<BriefcaseInfoPtr>>([=]()
        {
        return m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)
            ->Then<BriefcasesInfoResult>([=](const WSObjectsResult& result)
            {
            if (!result.IsSuccess())
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
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
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
CodeLockSetTaskPtr iModelConnection::QueryCodesLocksInternal
(
DgnCodeSet const* codes,
LockableIdSet const* locks,
const BeBriefcaseId*  briefcaseId,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::QueryCodesLocksInternal";

    bset<StatusTaskPtr> tasks;
    CodeLockSetResultInfoPtr finalValue = new CodeLockSetResultInfo();

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
        ->Then<CodeLockSetResult>([=]
        {
        for (auto task : tasks)
            {
            if (!task->GetResult().IsSuccess())
                return CodeLockSetResult::Error(task->GetResult().GetError());
            }
        return CodeLockSetResult::Success(*finalValue);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::QueryCodesLocksInternal
(
WSQuery query,
CodeLockSetResultInfoPtr codesLocksOut,
CodeLocksSetAddFunction addFunction,
ICancellationTokenPtr cancellationToken
) const
    {
    return ExecutionManager::ExecuteWithRetry<void>([=]()
        {
        //Execute query
        return m_wsRepositoryClient->SendQueryRequest(query, "", "", cancellationToken)->Then<StatusResult>
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
                return StatusResult::Success();
                }

            return StatusResult::Error(result.GetError());
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
//@bsimethod                                     julius.cepukenas                01/2017
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::QueryCodesInternal
(
const DgnCodeSet& codes,
const BeSQLite::BeBriefcaseId*  briefcaseId,
CodeLockSetResultInfoPtr codesLocksOut,
ICancellationTokenPtr cancellationToken
) const
    {
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::Code);

    std::deque<ObjectId> queryIds;
    for (auto& code : codes)
        queryIds.push_back(GetCodeId(code, briefcaseId));

    query.AddFilterIdsIn(queryIds, nullptr, 0, 0);

    return QueryCodesLocksInternal(query, codesLocksOut, AddCodes, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas                01/2017
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::QueryCodesInternal
(
const BeSQLite::BeBriefcaseId*  briefcaseId,
CodeLockSetResultInfoPtr codesLocksOut,
ICancellationTokenPtr cancellationToken
) const
    {
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::MultiCode);

    Utf8String filter;
    filter.Sprintf("(%s+eq+%u)", ServerSchema::Property::BriefcaseId, briefcaseId->GetValue());
    query.SetFilter(filter);

    auto addMultiCodesCallback = [&] (const WSObjectsReader::Instance& value, CodeLockSetResultInfoPtr codesLocksSetOut)
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
//@bsimethod                                     julius.cepukenas                01/2017
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::QueryUnavailableCodesInternal
(
const BeBriefcaseId briefcaseId, 
CodeLockSetResultInfoPtr codesLocksOut,
ICancellationTokenPtr cancellationToken
) const
    {
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::Code);

    Utf8String filter;
    filter.Sprintf("%s+ne+%u", ServerSchema::Property::BriefcaseId,
                   briefcaseId.GetValue());

    query.SetFilter(filter);

    return QueryCodesLocksInternal(query, codesLocksOut, AddCodes, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas                01/2017
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::QueryLocksInternal
(
const LockableIdSet& locks,
const BeSQLite::BeBriefcaseId*  briefcaseId,
CodeLockSetResultInfoPtr codesLocksOut,
ICancellationTokenPtr cancellationToken
) const
    {
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::Lock);

    std::deque<ObjectId> queryIds;
    for (auto& lock : locks)
        queryIds.push_back(GetLockId(lock, briefcaseId));

    query.AddFilterIdsIn(queryIds, nullptr, 0, 0);

    auto addLocksCallback = [&] (const WSObjectsReader::Instance& value, CodeLockSetResultInfoPtr codesLocksSetOut)
        {
        DgnLock        lock;
        BeBriefcaseId  briefcaseId;
        Utf8String     changeSetId;

        if (GetLockFromServerJson(value.GetProperties(), lock, briefcaseId, changeSetId))
            {
            if (lock.GetLevel() != LockLevel::None)
                codesLocksSetOut->AddLock(lock, briefcaseId, changeSetId);
            }
        };

    return QueryCodesLocksInternal(query, codesLocksOut, addLocksCallback, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas                01/2017
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::QueryLocksInternal
(
const BeSQLite::BeBriefcaseId*  briefcaseId,
CodeLockSetResultInfoPtr codesLocksOut,
ICancellationTokenPtr cancellationToken
) const
    {
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::MultiLock);

    Utf8String filter;
    filter.Sprintf("(%s+eq+%u)", ServerSchema::Property::BriefcaseId, briefcaseId->GetValue());
    query.SetFilter(filter);

    auto addMultiLocksCallback = [&] (const WSObjectsReader::Instance& value, CodeLockSetResultInfoPtr codesLocksSetOut)
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
        };

    return QueryCodesLocksInternal(query, codesLocksOut, addMultiLocksCallback, cancellationToken);
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
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::Lock);

    Utf8String filter;
    Utf8String locksFilter;
    locksFilter.Sprintf("%s+gt+%u+or+%s+gt+%u", ServerSchema::Property::LockLevel, LockLevel::None,
                        ServerSchema::Property::ReleasedWithChangeSetIndex, lastChangeSetIndex);

    filter.Sprintf("%s+ne+%u+and+(%s)", ServerSchema::Property::BriefcaseId,
                   briefcaseId.GetValue(), locksFilter.c_str());

    query.SetFilter(filter);


    auto addLocksCallback = [&] (const WSObjectsReader::Instance& value, CodeLockSetResultInfoPtr codesLocksSetOut)
        {
        DgnLock        lock;
        BeBriefcaseId  briefcaseId;
        Utf8String     changeSetId;

        if (GetLockFromServerJson(value.GetProperties(), lock, briefcaseId, changeSetId))
            {
            codesLocksSetOut->AddLock(lock, briefcaseId, changeSetId);
            }
        };

    return QueryCodesLocksInternal(query, codesLocksOut, addLocksCallback, cancellationToken);
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
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    
    auto requestString = changeSet->ToRequestString();
    HttpStringBodyPtr request = HttpStringBody::Create(requestString);
    return ExecutionManager::ExecuteWithRetry<CodeSequence>([=]()
        {
        return m_wsRepositoryClient->SendChangesetRequest(request, nullptr, cancellationToken)->Then<CodeSequenceResult>
            ([=](const WSChangesetResult& result)
            {
            if (result.IsSuccess())
                {
                Json::Value ptr = GetChangedInstances(result.GetValue()->AsString().c_str());
                
                auto json = ToRapidJson(*ptr.begin());
                CodeSequence        codeSequence;
                if (!GetCodeSequenceFromServerJson(json[ServerSchema::InstanceAfterChange][ServerSchema::Properties], codeSequence))
                    {
                    LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Code template parse failed.");
                    return CodeSequenceResult::Error({Error::Id::InvalidPropertiesValues, ErrorLocalizedString(MESSAGE_CodeSequenceResponseError)});
                    }

                double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
                return CodeSequenceResult::Success(codeSequence);
                }
            else
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
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
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    HttpStringBodyPtr request = HttpStringBody::Create(changeSet->ToRequestString());
    return ExecutionManager::ExecuteWithRetry<CodeSequence>([=]()
        {
        return m_wsRepositoryClient->SendChangesetRequest(request, nullptr, cancellationToken)->Then<CodeSequenceResult>
            ([=](const WSChangesetResult& result)
            {
            if (result.IsSuccess())
                {
                Json::Value ptr = GetChangedInstances(result.GetValue()->AsString().c_str());
                auto json = ToRapidJson(*ptr.begin());

                CodeSequence        codeSequence;
                if (!GetCodeSequenceFromServerJson(json[ServerSchema::InstanceAfterChange][ServerSchema::Properties], codeSequence))
                    {
                    LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Code template parse failed.");
                    return CodeSequenceResult::Error({Error::Id::InvalidPropertiesValues, ErrorLocalizedString(MESSAGE_CodeSequenceResponseError)});
                    }

                double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
                return CodeSequenceResult::Success(codeSequence);
                }
            else
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
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
    return m_wsRepositoryClient->SendCreateObjectRequest(briefcaseIdJson, BeFileName(), nullptr, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
FileTaskPtr iModelConnection::GetBriefcaseFileInfo(BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::GetBriefcaseFileInfo";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    Utf8String briefcaseIdString;
    briefcaseIdString.Sprintf("%d", briefcaseId.GetValue());
    ObjectId briefcaseObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::Briefcase, briefcaseIdString);

    return m_wsRepositoryClient->SendGetObjectRequest(briefcaseObjectId, nullptr, cancellationToken)
        ->Then<FileResult>([=](WSObjectsResult const& result)
        {
        if (!result.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return FileResult::Error(result.GetError());
            }
        auto fileInfo = FileInfo::Parse(*result.GetValue().GetInstances().begin());
        return FileResult::Success(fileInfo);
        });
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
    const Utf8String methodName = "iModelConnection::GetChangeSetById";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
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

    return m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)->Then<ChangeSetInfoResult>
        ([=] (WSObjectsResult& changeSetResult)
        {
        if (changeSetResult.IsSuccess())
            {
            auto changeSetInstances = changeSetResult.GetValue().GetInstances();
            if (changeSetInstances.IsEmpty())
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "ChangeSet does not exist.");
                return ChangeSetInfoResult::Error(Error::Id::ChangeSetDoesNotExist);
                }

            auto changeSet = ChangeSetInfo::Parse(*changeSetInstances.begin());
            if (loadAccessKey)
                {
                changeSet->SetFileAccessKey(FileAccessKey::ParseFromRelated(*changeSetResult.GetValue().GetInstances().begin()));
                }
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
            return ChangeSetInfoResult::Success(changeSet);
            }
        else
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, changeSetResult.GetError().GetMessage().c_str());
            return ChangeSetInfoResult::Error(changeSetResult.GetError());
            }
        });
    }

/* Private methods start */

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Json::Value GenerateEventSubscriptionWSChangeSetJson(EventTypeSet* eventTypes)
    {
    Json::Value properties;
    properties[ServerSchema::Property::EventTypes] = Json::arrayValue;
    if (eventTypes != nullptr)
        {
        int i = 0;
        for (auto eventType : *eventTypes)
            {
            properties[ServerSchema::Property::EventTypes][i] = Event::Helper::GetEventNameFromEventType(eventType);
            i++;
            }
        }
    return properties;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
EventSubscriptionPtr CreateEventSubscription(Utf8String response)
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

    EventTypeSet eventTypes;
    for (Json::ValueIterator itr = instance[ServerSchema::Properties][ServerSchema::Property::EventTypes].begin();
        itr != instance[ServerSchema::Properties][ServerSchema::Property::EventTypes].end(); ++itr)
        eventTypes.insert(Event::Helper::GetEventTypeFromEventName((*itr).asString().c_str()));

    return EventSubscription::Create(eventSubscriptionId, eventTypes);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Json::Value GenerateEventSASJson()
    {
    Json::Value request = Json::objectValue;
    JsonValueR instance = request[ServerSchema::Instance] = Json::objectValue;
    instance[ServerSchema::InstanceId] = "";
    instance[ServerSchema::SchemaName] = ServerSchema::Schema::iModel;
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
EventTypeSet* newEventTypes,
EventTypeSet oldEventTypes
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
bool iModelConnection::SetEventSASToken(ICancellationTokenPtr cancellationToken)
    {
    const Utf8String methodName = "iModelConnection::SetEventSASToken";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    auto sasToken = GetEventServiceSASToken(cancellationToken)->GetResult();
    if (!sasToken.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, sasToken.GetError().GetMessage().c_str());
        return false;
        }

    m_eventSAS = sasToken.GetValue();

    double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
bool iModelConnection::SetEventSubscription(EventTypeSet* eventTypes, ICancellationTokenPtr cancellationToken)
    {
    EventSubscriptionTaskPtr eventSubscription = nullptr;

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
bool iModelConnection::SetEventServiceClient
(
EventTypeSet* eventTypes,
ICancellationTokenPtr cancellationToken
)
    {
    if (!SetEventSubscription(eventTypes, cancellationToken))
        return false;

    BeMutexHolder lock(m_eventServiceClientMutex);

    if (m_eventServiceClient == nullptr)
        m_eventServiceClient = new EventServiceClient(m_eventSAS->GetBaseAddress(), m_iModelInfo.GetId(), m_eventSubscription->GetSubscriptionId());

    m_eventServiceClient->UpdateSASToken(m_eventSAS->GetSASToken());
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
AzureServiceBusSASDTOTaskPtr iModelConnection::GetEventServiceSASToken(ICancellationTokenPtr cancellationToken) const
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
                finalResult->SetError(Error::Id::NoSASFound);
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
EventSubscriptionTaskPtr iModelConnection::SendEventChangesetRequest
(
std::shared_ptr<WSChangeset> changeset,
ICancellationTokenPtr cancellationToken
) const
    {
    //https://{server}/{version}/Repositories/DgnDbServer--{repoId}/DgnDbServer/EventSubscription
    HttpStringBodyPtr request = HttpStringBody::Create(changeset->ToRequestString());
    std::shared_ptr<EventSubscriptionResult> finalResult = std::make_shared<EventSubscriptionResult>();
    return m_wsRepositoryClient->SendChangesetRequest(request, nullptr, cancellationToken)->Then([=] (const WSChangesetResult& result)
        {
        if (result.IsSuccess())
            {
            EventSubscriptionPtr ptr = CreateEventSubscription(result.GetValue()->AsString().c_str());
            if (ptr == nullptr)
                {
                finalResult->SetError(Error::Id::NoSubscriptionFound);
                return;
                }

            finalResult->SetSuccess(ptr);
            }
        else
            {
            finalResult->SetError(result.GetError());
            }
        })->Then<EventSubscriptionResult>([=]
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Caleb.Shafer                   06/2016
//---------------------------------------------------------------------------------------
void SetEventSubscriptionJsonRequestToChangeSet
(
EventTypeSet* eventTypes,
Utf8String                                       eventSubscriptionId,
WSChangeset&                                     changeset,
const WSChangeset::ChangeState&                  changeState
)
    {
    ObjectId eventSubscriptionObject
        (
        ServerSchema::Schema::iModel,
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
EventSubscriptionTaskPtr iModelConnection::GetEventServiceSubscriptionId
(
EventTypeSet* eventTypes,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::GetEventServiceSubscriptionId";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    SetEventSubscriptionJsonRequestToChangeSet(eventTypes, "", *changeset, WSChangeset::Created);

    return SendEventChangesetRequest(changeset, cancellationToken)
        ->Then<EventSubscriptionResult>([=] (EventSubscriptionResultCR result)
        {
        if (!result.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            }
        else
            {
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
            }
        return result;
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                    06/2016
//---------------------------------------------------------------------------------------
EventSubscriptionTaskPtr iModelConnection::UpdateEventServiceSubscriptionId
(
EventTypeSet* eventTypes,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::UpdateEventServiceSubscriptionId";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    SetEventSubscriptionJsonRequestToChangeSet(eventTypes, m_eventSubscription->GetSubscriptionId(), *changeset, WSChangeset::Modified);

    return SendEventChangesetRequest(changeset, cancellationToken)
        ->Then<EventSubscriptionResult>([=] (EventSubscriptionResultCR result)
        {
        if (!result.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            }
        else
            {
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
            }
        return result;
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            08/2016
//---------------------------------------------------------------------------------------
EventReponseTaskPtr iModelConnection::GetEventServiceResponse
(
int numOfRetries, 
bool longpolling
)
    {
    const Utf8String methodName = "iModelConnection::GetEventServiceResponse";
    BeMutexHolder lock(m_eventServiceClientMutex);

    if (!IsSubscribedToEvents())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Not subscribed to event service.");
        return CreateCompletedAsyncTask<EventReponseResult>
            (EventReponseResult::Error(Error::Id::NotSubscribedToEventService));
        }

    return m_eventServiceClient->MakeReceiveDeleteRequest(longpolling)
        ->Then<EventReponseResult>([=](const EventServiceResult& result)
        {
        if (result.IsSuccess())
            {
            Http::Response response = result.GetValue();
            if (response.GetHttpStatus() != HttpStatus::OK)
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                return EventReponseResult::Error(Error(result.GetError()));
                }

            return EventReponseResult::Success(response);
            }
        else
            {
            HttpStatus status = result.GetError().GetHttpStatus();
            if (status == HttpStatus::NoContent || status == HttpStatus::None)
                {
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "No events found.");
                return EventReponseResult::Error(Error::Id::NoEventsFound);
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
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                return EventReponseResult::Error(Error(result.GetError()));
                }
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas            12/2016
//---------------------------------------------------------------------------------------
bool iModelConnection::IsSubscribedToEvents() const
    {
    return m_eventServiceClient != nullptr && m_eventSubscription != nullptr && m_eventSAS != nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
EventTaskPtr iModelConnection::GetEvent
(
bool longPolling,
ICancellationTokenPtr cancellationToken
)
    {
    BeMutexHolder lock(m_eventServiceClientMutex);

    const Utf8String methodName = "iModelConnection::GetEvent";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (!IsSubscribedToEvents())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Not subscribed to event service.");
        return CreateCompletedAsyncTask<EventResult>
            (EventResult::Error(Error::Id::NotSubscribedToEventService));
        }

    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    return GetEventServiceResponse(3, longPolling)->Then<EventResult>
        ([=] (EventReponseResult& result)
        {
        if (result.IsSuccess())
            {
            Http::Response response = result.GetValue();
            EventPtr ptr = EventParser::ParseEvent(response.GetHeaders().GetContentType(), response.GetBody().AsString());
            if (ptr == nullptr)
                {
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "No events found.");
                return EventResult::Error(Error::Id::NoEventsFound);
                }
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
            return EventResult::Success(ptr);
            }
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
        return EventResult::Error(result.GetError());
        });
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
    return ExecutionManager::ExecuteWithRetry<void>([=]()
        {
        return (SetEventServiceClient(eventTypes, cancellationToken)) ?
            CreateCompletedAsyncTask<StatusResult>(StatusResult::Success()) :
            CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error::Id::EventServiceSubscribingError));
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Arvind.Venkateswaran            06/2016
//Todo: Add another method to only cancel GetEvent Operation and not the entire connection
//---------------------------------------------------------------------------------------
StatusTaskPtr  iModelConnection::UnsubscribeToEvents()
    {
    BeMutexHolder lock(m_eventServiceClientMutex);

    const Utf8String methodName = "iModelConnection::UnsubscribeToEvents";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (m_eventServiceClient != nullptr)
        m_eventServiceClient->CancelRequest();
    m_eventServiceClient = nullptr;
    m_eventSubscription = nullptr;
    m_eventSAS = nullptr;
    return CreateCompletedAsyncTask<StatusResult>(StatusResult::Success());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas            12/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::SubscribeEventsCallback(EventTypeSet* eventTypes, EventCallbackPtr callback, iModelConnectionP imodelConnectionP)
    {
    if (m_eventManagerPtr.IsNull())
        {
        m_eventManagerPtr = new EventManager(imodelConnectionP);
        }
    return m_eventManagerPtr->Subscribe(eventTypes, callback);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas            12/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::UnsubscribeEventsCallback(EventCallbackPtr callback)
    {
    if (m_eventManagerPtr.IsNull())
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Success());

    if (!callback)
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error::Id::EventCallbackNotSpecified));

    bool dispose = false;
    auto result = m_eventManagerPtr->Unsubscribe(callback, &dispose);
    if (dispose)
        m_eventManagerPtr = nullptr;

    return result;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
ChangeSetsInfoTaskPtr iModelConnection::ChangeSetsFromQueryInternal
(
const WebServices::WSQuery& query,
bool                        parseFileAccessKey,
ICancellationTokenPtr       cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::ChangeSetsFromQuery";
    return m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)->Then<ChangeSetsInfoResult>
        ([=](const WSObjectsResult& changeSetsInfoResult)
        {
        if (changeSetsInfoResult.IsSuccess())
            {
            bvector<ChangeSetInfoPtr> indexedChangeSets;
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

                    indexedChangeSets.push_back(changeSetInfo);
                    }
                }

            std::sort(indexedChangeSets.begin(), indexedChangeSets.end(), [](ChangeSetInfoPtr a, ChangeSetInfoPtr b)
                {
                return a->GetIndex() < b->GetIndex();
                });
            return ChangeSetsInfoResult::Success(indexedChangeSets);
            }
        else
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, changeSetsInfoResult.GetError().GetMessage().c_str());
            return ChangeSetsInfoResult::Error(changeSetsInfoResult.GetError());
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
ChangeSetsInfoTaskPtr iModelConnection::ChangeSetsFromQuery
(
const WebServices::WSQuery& query,
bool                        parseFileAccessKey,
ICancellationTokenPtr       cancellationToken
) const
    {
    return ExecutionManager::ExecuteWithRetry<bvector<ChangeSetInfoPtr>>([=]()
        {
        return ChangeSetsFromQueryInternal(query, parseFileAccessKey, cancellationToken);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
ChangeSetsInfoTaskPtr iModelConnection::GetChangeSetsInternal
(
const WebServices::WSQuery& query,
bool                        parseFileAccessKey,
ICancellationTokenPtr       cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::GetChangeSetsAfterId";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    std::shared_ptr<ChangeSetsInfoResult> finalResult = std::make_shared<ChangeSetsInfoResult>();

    return ExecutionManager::ExecuteWithRetry<bvector<ChangeSetInfoPtr>>([=]()
        {
        return ChangeSetsFromQueryInternal(query, parseFileAccessKey, cancellationToken)->Then([=](ChangeSetsInfoResultCR changeSetsResult)
            {
            if (changeSetsResult.IsSuccess())
                {
                finalResult->SetSuccess(changeSetsResult.GetValue());
                double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
                }
            else
                {
                finalResult->SetError(changeSetsResult.GetError());
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, changeSetsResult.GetError().GetMessage().c_str());
                }
            })->Then<ChangeSetsInfoResult>([=]()
                {
                return *finalResult;
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
    auto query = CreateChangeSetsAfterIdQuery(changeSetId, fileId);

    if (loadAccessKey)
        {
        Utf8String selectString;
        selectString.Sprintf("%s,%s,%s,%s", ServerSchema::Property::Id, ServerSchema::Property::Index, ServerSchema::Property::ParentId, ServerSchema::Property::SeedFileId);
        FileAccessKey::AddDownloadAccessKeySelect(selectString);
        query.SetSelect(selectString);
        }

    return GetChangeSetsInternal(query, loadAccessKey, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
WSQuery iModelConnection::CreateChangeSetsAfterIdQuery
(
Utf8StringCR          changeSetId,
BeGuidCR              fileId
) const
    {
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet);
    BeGuid id = fileId;
    Utf8String queryFilter;
    
    if (Utf8String::IsNullOrEmpty(changeSetId.c_str()))
        {
        if (id.IsValid())
            queryFilter.Sprintf("%s+eq+'%s'", ServerSchema::Property::SeedFileId, id.ToString().c_str());
        }
    else
        {
        if (id.IsValid())
            queryFilter.Sprintf("%s-backward-%s.%s+eq+'%s'+and+%s+eq+'%s'", ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::ChangeSet,
                ServerSchema::Property::Id, changeSetId.c_str(),
                ServerSchema::Property::SeedFileId, id.ToString().c_str());
        else
            queryFilter.Sprintf("%s-backward-%s.%s+eq+'%s'", ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::ChangeSet,
                ServerSchema::Property::Id, changeSetId.c_str());
        }

    query.SetFilter(queryFilter);

    return query;
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
    const Utf8String methodName = "iModelConnection::DownloadChangeSets";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    bset<std::shared_ptr<AsyncTask>> tasks;
    bmap<Utf8String, int64_t> changeSetIdIndexMap;
    for (auto& changeSet : changeSets)
        {
        tasks.insert(DownloadChangeSetFile(changeSet, callback, cancellationToken));
        changeSetIdIndexMap.Insert(changeSet->GetId(), changeSet->GetIndex());
        }

    return AsyncTask::WhenAll(tasks)->Then<ChangeSetsResult>([=] ()
        {
        ChangeSets resultChangeSets;
        for (auto task : tasks)
            {
            auto result = dynamic_pointer_cast<ChangeSetTask>(task)->GetResult();
            if (!result.IsSuccess())
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
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

        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
        return ChangeSetsResult::Success(resultChangeSets);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
ChangeSetsTaskPtr iModelConnection::DownloadChangeSets(std::deque<ObjectId>& changeSetIds, Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    auto query = CreateChangeSetsByIdQuery(changeSetIds);

    Utf8String selectString;
    selectString.Sprintf("%s,%s,%s,%s", ServerSchema::Property::Id, ServerSchema::Property::Index, ServerSchema::Property::ParentId, ServerSchema::Property::SeedFileId);
    FileAccessKey::AddDownloadAccessKeySelect(selectString);
    query.SetSelect(selectString);

    auto changeSetsQueryResult = GetChangeSetsInternal(query, true, cancellationToken)->GetResult();
    if (!changeSetsQueryResult.IsSuccess())
        return CreateCompletedAsyncTask(ChangeSetsResult::Error(changeSetsQueryResult.GetError()));
    
    return DownloadChangeSetsInternal(changeSetsQueryResult.GetValue(), callback, cancellationToken);
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
    DgnRevisionPtr changeSetPtr = DgnRevision::Create(&changeSetStatus, changeSet->GetId(), changeSet->GetParentChangeSetId(), changeSet->GetDbGuid());
    auto changeSetFileName = changeSetPtr->GetRevisionChangesFile();

    if (s_preDownloadManager->TryGetChangeSetFile(changeSetFileName, changeSet->GetId()))
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
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
WSQuery iModelConnection::CreateChangeSetsByIdQuery
(
std::deque<ObjectId>& changeSetIds
) const
    {
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet);
    query.AddFilterIdsIn(changeSetIds, nullptr, 0, 0);
    return query;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas              02/2017
//---------------------------------------------------------------------------------------
ChangeSetsInfoTaskPtr iModelConnection::GetAllChangeSetsInternal(bool loadAccessKey, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::GetAllChangeSetsInternal";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet);
    return ChangeSetsFromQuery(query, loadAccessKey, cancellationToken);
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
void iModelConnection::WaitForInitializedBIMFile(BeGuid fileGuid, FileResultPtr finalResult) const
    {
    InitializationState initializationState = InitializationState::NotStarted;
    int retriesLeft = 300;
    const Utf8String methodName = "Client::WaitForInitializedBIMFile";
    BeThreadUtilities::BeSleep(1000);

    while (!IsInitializationFinished(initializationState) && retriesLeft > 0)
        {
        auto seedFilesResult = GetSeedFileById(fileGuid)->GetResult();
        if (!seedFilesResult.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, seedFilesResult.GetError().GetMessage().c_str());
            finalResult->SetError(seedFilesResult.GetError());
            return;
            }

        auto seedFile = seedFilesResult.GetValue();
        initializationState = seedFile->GetInitialized();
        
        if (!IsInitializationFinished(initializationState))
            BeThreadUtilities::BeSleep(1000);
        retriesLeft--;
        }

    if (initializationState != InitializationState::Success)
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File is not initialized.");
        switch (initializationState)
            {
            case InitializationState::Scheduled:
                finalResult->SetError({ Error::Id::FileIsNotYetInitialized });
            case InitializationState::OutdatedFile:
                finalResult->SetError({ Error::Id::FileIsOutdated });
            case InitializationState::IncorrectFileId:
                finalResult->SetError({ Error::Id::FileHasDifferentId });
            default:
                finalResult->SetError({ Error::Id::FileInitializationFailed });
            }
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
Json::Value PushChangeSetJson
(
Dgn::DgnRevisionPtr changeSet,
BeBriefcaseId       briefcaseId,
bool                containsSchemaChanges
)
    {
    Json::Value pushChangeSetJson = Json::objectValue;
    JsonValueR instance = pushChangeSetJson[ServerSchema::Instance] = Json::objectValue;
    instance[ServerSchema::SchemaName] = ServerSchema::Schema::iModel;
    instance[ServerSchema::ClassName] = ServerSchema::Class::ChangeSet;
    instance[ServerSchema::Properties] = Json::objectValue;

    JsonValueR properties = instance[ServerSchema::Properties];
    properties[ServerSchema::Property::Id] = changeSet->GetId();
    properties[ServerSchema::Property::Description] = changeSet->GetSummary();
    uint64_t size;
    changeSet->GetRevisionChangesFile().GetFileSize(size);
    properties[ServerSchema::Property::FileSize] = size;
    properties[ServerSchema::Property::ParentId] = changeSet->GetParentId();
    properties[ServerSchema::Property::SeedFileId] = changeSet->GetDbGuid();
    properties[ServerSchema::Property::BriefcaseId] = briefcaseId.GetValue ();
    properties[ServerSchema::Property::IsUploaded] = false;
    properties[ServerSchema::Property::ContainingChanges] = containsSchemaChanges ? 1 : 0;
    return pushChangeSetJson;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::Push
(
Dgn::DgnRevisionPtr               changeSet,
Dgn::DgnDbCR                      dgndb,
bool                              relinquishCodesLocks,
Http::Request::ProgressCallbackCR callback,
ICancellationTokenPtr             cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::Push";
    DgnDbCP pDgnDb = &dgndb;

    // Stage 1. Create changeSet.
    std::shared_ptr<Json::Value> pushJson = std::make_shared<Json::Value>(PushChangeSetJson(changeSet, dgndb.GetBriefcaseId(), changeSet->ContainsSchemaChanges(dgndb)));
    std::shared_ptr<StatusResult> finalResult = std::make_shared<StatusResult>();
    return ExecutionManager::ExecuteWithRetry<void>([=]()
        {
        return m_wsRepositoryClient->SendCreateObjectRequest(*pushJson, BeFileName(), callback, cancellationToken)
            ->Then([=] (const WSCreateObjectResult& initializePushResult)
            {
    #if defined (ENABLE_BIM_CRASH_TESTS)
            BreakHelper::HitBreakpoint(Breakpoints::iModelConnection_AfterCreateChangeSetRequest);
    #endif
            if (!initializePushResult.IsSuccess())
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, initializePushResult.GetError().GetMessage().c_str());
                finalResult->SetError(initializePushResult.GetError());
                return;
                }
        
            // Stage 2. Upload changeSet file. 
            Json::Value json;
            initializePushResult.GetValue().GetJson(json);
            JsonValueCR changeSetInstance = json[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
            Utf8String  changeSetInstanceId = changeSetInstance[ServerSchema::InstanceId].asString();
            ObjectId    changeSetObjectId   = ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet, changeSetInstanceId);
            auto fileAccessKey = FileAccessKey::ParseFromRelated(changeSetInstance);

            if (fileAccessKey.IsNull())
                {
                m_wsRepositoryClient->SendUpdateFileRequest(changeSetObjectId, changeSet->GetRevisionChangesFile(), callback, cancellationToken)
                    ->Then([=] (const WSUpdateObjectResult& uploadChangeSetResult)
                    {
    #if defined (ENABLE_BIM_CRASH_TESTS)
                    BreakHelper::HitBreakpoint(Breakpoints::iModelConnection_AfterUploadChangeSetFile);
    #endif
                    if (!uploadChangeSetResult.IsSuccess())
                        {
                        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, uploadChangeSetResult.GetError().GetMessage().c_str());
                        finalResult->SetError(uploadChangeSetResult.GetError());
                        return;
                        }

                    // Stage 3. Initialize changeSet.
                    InitializeChangeSet(changeSet, *pDgnDb, *pushJson, changeSetObjectId, relinquishCodesLocks, callback, cancellationToken)
                        ->Then([=] (StatusResultCR result)
                        {
                        if (result.IsSuccess())
                            finalResult->SetSuccess();
                        else
                            {
                            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                            finalResult->SetError(result.GetError());
                            }
                        });
                    });
                }
            else
                {
                m_azureClient->SendUpdateFileRequest(fileAccessKey->GetUploadUrl(), changeSet->GetRevisionChangesFile(), callback, cancellationToken)
                    ->Then([=] (const AzureResult& result)
                    {
#if defined (ENABLE_BIM_CRASH_TESTS)
                    BreakHelper::HitBreakpoint(Breakpoints::iModelConnection_AfterUploadChangeSetFile);
#endif
                    if (!result.IsSuccess())
                        {
                        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                        finalResult->SetError(Error(result.GetError()));
                        return;
                        }

                    // Stage 3. Initialize changeSet.
                    InitializeChangeSet(changeSet, *pDgnDb, *pushJson, changeSetObjectId, relinquishCodesLocks, callback, cancellationToken)
                        ->Then([=] (StatusResultCR result)
                        {
                        if (result.IsSuccess())
                            finalResult->SetSuccess();
                        else
                            {
                            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                            finalResult->SetError(result.GetError());
                            }
                        });
                    });
                }
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

        return FileResult::Success(*filesResult.GetValue().begin());
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

    return SeedFilesQuery(query, cancellationToken)->Then<FileResult>([=] (FilesResult filesResult)
        {
        if (!filesResult.IsSuccess())
            return FileResult::Error(filesResult.GetError());

        return FileResult::Success(*filesResult.GetValue().begin());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              02/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::InitializeChangeSet
(
Dgn::DgnRevisionPtr             changeSet,
Dgn::DgnDbCR                    dgndb,
JsonValueR                      pushJson,
ObjectId                        changeSetObjectId,
bool                            relinquishCodesLocks,
Http::Request::ProgressCallbackCR callback,
ICancellationTokenPtr           cancellationToken
) const
    {
    BeBriefcaseId briefcaseId = dgndb.GetBriefcaseId();
    std::shared_ptr<WSChangeset> changeset (new WSChangeset ());

    //Set ChangeSet initialization request to ECChangeSet
    JsonValueR changeSetProperties = pushJson[ServerSchema::Instance][ServerSchema::Properties];
    changeSetProperties[ServerSchema::Property::IsUploaded] = true;
    changeset->AddInstance (changeSetObjectId, WSChangeset::ChangeState::Modified, std::make_shared<Json::Value> (changeSetProperties));

    //Set used locks to the ECChangeSet
    LockRequest usedLocks;
    usedLocks.FromRevision (*changeSet, dgndb);
    BeGuid seedFileId;
    seedFileId.FromString(changeSet->GetDbGuid().c_str());
    if (!usedLocks.IsEmpty ())
        SetLocksJsonRequestToChangeSet (usedLocks.GetLockSet (), briefcaseId, seedFileId, changeSet->GetId (), *changeset, WSChangeset::ChangeState::Modified, true);

    DgnCodeSet assignedCodes, discardedCodes;
    changeSet->ExtractCodes(assignedCodes, discardedCodes, dgndb);

    if (!assignedCodes.empty())
        {
        DgnCodeState state;
        state.SetUsed(changeSet->GetId());
        SetCodesJsonRequestToChangeSet(assignedCodes, state, briefcaseId, *changeset, WSChangeset::ChangeState::Modified);
        }

    if (!discardedCodes.empty())
        {
        DgnCodeState state;
        state.SetDiscarded(changeSet->GetId());
        SetCodesJsonRequestToChangeSet(discardedCodes, state, briefcaseId, *changeset, WSChangeset::ChangeState::Modified);
        }

    if (relinquishCodesLocks)
        {
        LockDeleteAllJsonRequest (changeset, briefcaseId);
        CodeDiscardReservedJsonRequest(changeset, briefcaseId);
        }

    //Push ChangeSet initialization request and Locks update in a single batch
    const Utf8String methodName = "iModelConnection::InitializeChangeSet";
    HttpStringBodyPtr request = HttpStringBody::Create(changeset->ToRequestString());

    std::shared_ptr<StatusResult> finalResult = std::make_shared<StatusResult>();

    auto requestOptions = std::make_shared<WSRepositoryClient::RequestOptions>();
    requestOptions->SetTransferTimeOut(WSRepositoryClient::Timeout::Transfer::LongUpload);

    return SendChangesetRequestInternal(changeset, IBriefcaseManager::ResponseOptions::None, cancellationToken, requestOptions)
        ->Then([=] (const StatusResult& initializeChangeSetResult)
        {
        if (initializeChangeSetResult.IsSuccess())
            {
            finalResult->SetSuccess();
            return;
            }

        auto errorId = Error(initializeChangeSetResult.GetError()).GetId();
        if (Error::Id::LockDoesNotExist != errorId && Error::Id::CodeDoesNotExist != errorId)
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, initializeChangeSetResult.GetError().GetMessage().c_str());
            finalResult->SetError(initializeChangeSetResult.GetError());
            return;
            }

        //Try to acquire all required locks and codes.
        DgnCodeSet codesToReserve = assignedCodes;
        codesToReserve.insert(discardedCodes.begin(), discardedCodes.end());

        AcquireCodesLocksInternal(usedLocks, codesToReserve, briefcaseId, seedFileId, changeSet->GetParentId(), IBriefcaseManager::ResponseOptions::None, cancellationToken)
            ->Then([=] (StatusResultCR acquireCodesLocksResult)
            {
            if (!acquireCodesLocksResult.IsSuccess())
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, acquireCodesLocksResult.GetError().GetMessage().c_str());
                finalResult->SetError(acquireCodesLocksResult.GetError());
                return;
                }

            //Push ChangeSet initialization request again.
            m_wsRepositoryClient->SendChangesetRequest(request, nullptr, cancellationToken)
                ->Then([=] (const WSChangesetResult& repeatInitializeChangeSetResult)
                {
                if (repeatInitializeChangeSetResult.IsSuccess())
                    finalResult->SetSuccess();
                else
                    {
                    LogHelper::Log(SEVERITY::LOG_ERROR, methodName, repeatInitializeChangeSetResult.GetError().GetMessage().c_str());
                    finalResult->SetError(repeatInitializeChangeSetResult.GetError());
                    }
                });
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
IHttpHandlerPtr  customHandler
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
    iModelConnectionPtr imodelConnection(new iModelConnection(iModel, credentials, clientInfo, customHandler));
    imodelConnection->SetAzureClient(AzureBlobStorageClient::Create());

    double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
    return iModelConnectionResult::Success(imodelConnection);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             09/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::ValidateBriefcase(BeGuidCR fileId, BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken) const
    {
    return QueryBriefcaseInfo(briefcaseId, cancellationToken)->Then<StatusResult>([=] (BriefcaseInfoResultCR result)
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
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    return ExecutionManager::ExecuteWithRetry<void>([=]()
        {
        Json::Value iModelLockJson = Json::objectValue;
        iModelLockJson[ServerSchema::Instance] = Json::objectValue;
        iModelLockJson[ServerSchema::Instance][ServerSchema::SchemaName] = ServerSchema::Schema::iModel;
        iModelLockJson[ServerSchema::Instance][ServerSchema::ClassName] = ServerSchema::Class::iModelLock;

        return m_wsRepositoryClient->SendCreateObjectRequest(iModelLockJson, BeFileName(), nullptr, cancellationToken)
            ->Then<StatusResult>([=](const WSCreateObjectResult& result)
            {
            if (!result.IsSuccess())
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                return StatusResult::Error(result.GetError());
                }

            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
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
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    return ExecutionManager::ExecuteWithRetry<void>([=] ()
        {
        ObjectId id = ObjectId::ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::iModelLock, ServerSchema::Class::iModelLock);
        return m_wsRepositoryClient->SendDeleteObjectRequest(id, cancellationToken)
            ->Then<StatusResult>([=] (const WSDeleteObjectResult& result)
            {
            if (!result.IsSuccess())
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                return StatusResult::Error(result.GetError());
                }

            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float) (end - start), "");
            return StatusResult::Success();
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
FileTaskPtr iModelConnection::UploadNewSeedFile(BeFileNameCR filePath, FileInfoCR fileInfo, bool waitForInitialized, Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::UploadNewSeedFile";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    std::shared_ptr<FileResult> finalResult = std::make_shared<FileResult>();
    return ExecutionManager::ExecuteWithRetry<FileInfoPtr>([=]()
        {
        return CreateNewServerFile(fileInfo, cancellationToken)->Then([=] (FileResultCR fileCreationResult)
            {
#if defined (ENABLE_BIM_CRASH_TESTS)
            BreakHelper::HitBreakpoint(Breakpoints::iModelConnection_AfterCreateNewServerFile);
#endif
            if (!fileCreationResult.IsSuccess())
                {
                finalResult->SetError(fileCreationResult.GetError());
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, fileCreationResult.GetError().GetMessage().c_str());
                return;
                }

            auto createdFileInfo = fileCreationResult.GetValue();
            if (!createdFileInfo->AreFileDetailsAvailable())
                {
                auto fileUpdateResult = UpdateServerFile(*createdFileInfo, cancellationToken)->GetResult();
                if (!fileUpdateResult.IsSuccess())
                    {
                    finalResult->SetError(fileUpdateResult.GetError());
                    LogHelper::Log(SEVERITY::LOG_ERROR, methodName, fileUpdateResult.GetError().GetMessage().c_str());
                    return;
                    }
                }
            finalResult->SetSuccess(createdFileInfo);

            AzureFileUpload(filePath, createdFileInfo->GetFileAccessKey(), callback, cancellationToken)->Then([=] (StatusResultCR uploadResult)
                {
#if defined (ENABLE_BIM_CRASH_TESTS)
                BreakHelper::HitBreakpoint(Breakpoints::iModelConnection_AfterUploadServerFile);
#endif
                if (!uploadResult.IsSuccess() && Error::Id::MissingRequiredProperties != uploadResult.GetError().GetId())
                    {
                    finalResult->SetError(uploadResult.GetError());
                    LogHelper::Log(SEVERITY::LOG_ERROR, methodName, uploadResult.GetError().GetMessage().c_str());
                    return;
                    }
                InitializeServerFile(*createdFileInfo, cancellationToken)->Then([=] (StatusResultCR initializationResult)
                    {
                    if (!initializationResult.IsSuccess())
                        {
                        finalResult->SetError(initializationResult.GetError());
                        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, initializationResult.GetError().GetMessage().c_str());
                        return;
                        }

                    if (waitForInitialized)
                        {
                        WaitForInitializedBIMFile(createdFileInfo->GetFileId(), finalResult);
                        }
                    });
                });
            })->Then<FileResult>([=] ()
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
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::File);
    Utf8String filter;
    filter.Sprintf("%s+gt+0", ServerSchema::Property::InitializationState);
    query.SetFilter(filter);
    std::shared_ptr<StatusResult> finalResult = std::make_shared<StatusResult>();
    return m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)->Then([=] (WSObjectsResult const& result)
        {
        if (!result.IsSuccess())
            {
            finalResult->SetError(result.GetError());
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return;
            }
        auto instances = result.GetValue().GetInstances();
        if (instances.IsEmpty())
            {
            finalResult->SetError({Error::Id::FileDoesNotExist, ErrorLocalizedString(MESSAGE_SeedFileNotFound)});
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File does not exist.");
            return;
            }

        auto fileInfo = FileInfo::Parse(*instances.begin(), FileInfo());
        m_wsRepositoryClient->SendDeleteObjectRequest(fileInfo->GetObjectId(), cancellationToken)->Then([=] (WSVoidResult const& deleteResult)
            {
            if (!deleteResult.IsSuccess())
                {
                finalResult->SetError(deleteResult.GetError());
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, deleteResult.GetError().GetMessage().c_str());
                }
            else
                {
                finalResult->SetSuccess();
                double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
                }
            });
        })->Then<StatusResult>([=] ()
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
StatusTaskPtr iModelConnection::DownloadSeedFile(BeFileName localFile, Utf8StringCR fileId, Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
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
    return ExecutionManager::ExecuteWithRetry<void>([=]() { return AcquireCodesLocksInternal(locks, codes, briefcaseId, seedFileId, lastChangeSetId, options, cancellationToken); });
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
    std::shared_ptr<WSChangeset> changeset(new WSChangeset());

    SetLocksJsonRequestToChangeSet(locks.GetLockSet(), briefcaseId, seedFileId, lastChangeSetId, *changeset, WSChangeset::ChangeState::Modified, false, true);

    DgnCodeState state;
    state.SetReserved(briefcaseId);
    SetCodesJsonRequestToChangeSet(codes, state, briefcaseId, *changeset, WSChangeset::ChangeState::Created, true);

    return SendChangesetRequest(changeset, options, cancellationToken);
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
    //How to set description here?
    std::shared_ptr<WSChangeset> changeset (new WSChangeset ());
    SetLocksJsonRequestToChangeSet (locks, briefcaseId, seedFileId, "", *changeset, WSChangeset::ChangeState::Modified);

    DgnCodeState state;
    state.SetAvailable();
    SetCodesJsonRequestToChangeSet(codes, state, briefcaseId, *changeset, WSChangeset::ChangeState::Modified);

    return SendChangesetRequestInternal(changeset, options, cancellationToken);
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
    const Utf8String methodName = "iModelConnection::RelinquishCodesLocks";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    std::shared_ptr<WSChangeset> changeset (new WSChangeset ());
    LockDeleteAllJsonRequest (changeset, briefcaseId);
    CodeDiscardReservedJsonRequest(changeset, briefcaseId);
    return SendChangesetRequest(changeset, options, cancellationToken);
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
BriefcaseInfoTaskPtr iModelConnection::QueryBriefcaseInfo(BeSQLite::BeBriefcaseId briefcaseId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::QueryBriefcaseInfo";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    Utf8String filter;
    filter.Sprintf("%s+eq+%u", ServerSchema::Property::BriefcaseId, briefcaseId);

    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::Briefcase);
    query.SetFilter(filter);

    return QueryBriefcaseInfoInternal(query, cancellationToken)->Then<BriefcaseInfoResult>([=] (BriefcasesInfoResultCR briefcasesResult)
        {
        if (!briefcasesResult.IsSuccess())
            {
            return BriefcaseInfoResult::Error(briefcasesResult.GetError());
            }
        auto briefcasesInfo = briefcasesResult.GetValue();
        if (briefcasesInfo.empty())
            {
            return BriefcaseInfoResult::Error({Error::Id::InvalidBriefcase, ErrorLocalizedString(MESSAGE_BriefcaseInfoParseError)});
            }
        return BriefcaseInfoResult::Success(briefcasesResult.GetValue()[0]);
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
            if (!task->GetResult().IsSuccess())
                return BriefcasesInfoResult::Error(task->GetResult().GetError());

            auto briefcaseInfo = task->GetResult().GetValue();
            finalValue->insert(finalValue->end(), briefcaseInfo.begin(), briefcaseInfo.end());
            }

        return BriefcasesInfoResult::Success(*finalValue);
        });
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
    const Utf8String methodName = "iModelConnection::QueryCodesLocksById";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    return QueryCodesLocksInternal(&codes, &locks, nullptr, cancellationToken);
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
    return QueryCodesLocksInternal(&codes, &locks, &briefcaseId, cancellationToken);
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
    return QueryCodesLocksInternal(nullptr, nullptr, &briefcaseId, cancellationToken);
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
    if (briefcaseId.IsMasterId() || briefcaseId.IsStandaloneId())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid briefcase.");
        return CreateCompletedAsyncTask<CodeLockSetResult>(CodeLockSetResult::Error(Error::Id::FileIsNotBriefcase));
        }
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    std::shared_ptr<CodeLockSetResult> finalResult = std::make_shared<CodeLockSetResult>();
    return ExecutionManager::ExecuteWithRetry<CodeLockSetResultInfo>([=]()
        {
        return GetChangeSetById(lastChangeSetId, cancellationToken)->Then([=] (ChangeSetInfoResultCR changeSetResult)
            {
            uint64_t changeSetIndex = 0;
            if (!changeSetResult.IsSuccess() && changeSetResult.GetError().GetId() != Error::Id::InvalidChangeSet)
                {
                finalResult->SetError(changeSetResult.GetError());
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, changeSetResult.GetError().GetMessage().c_str());
                return;
                }
            else if (changeSetResult.IsSuccess())
                {
                changeSetIndex = changeSetResult.GetValue()->GetIndex();
                }

            CodeLockSetResultInfoPtr finalValue = new CodeLockSetResultInfo();
            bset<StatusTaskPtr> tasks;

            auto task = QueryUnavailableCodesInternal(briefcaseId, finalValue, cancellationToken);
            tasks.insert(task);
            task = QueryUnavailableLocksInternal(briefcaseId, changeSetIndex, finalValue, cancellationToken);
            tasks.insert(task);

            AsyncTask::WhenAll(tasks)
                ->Then([=]
                {
                for (auto task : tasks)
                    {
                    if (!task->GetResult().IsSuccess())
                        {
                        finalResult->SetError(task->GetResult().GetError());
                        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, task->GetResult().GetError().GetMessage().c_str());
                        return;
                        }
                    }

                finalResult->SetSuccess(*finalValue);
                double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
                });

            })->Then<CodeLockSetResult>([=] ()
                {
                return *finalResult;
                });
        });
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
    auto status = SetCodeSequencesJsonRequestToChangeSet(codeSequence, 0, 1, CodeSequence::Type::Maximum, *changeset, WSChangeset::ChangeState::Created);
    if (DgnDbStatus::Success != status)
        return CreateCompletedAsyncTask<CodeSequenceResult>(CodeSequenceResult::Error({ Error::Id::iModelHubOperationFailed, ErrorLocalizedString(MESSAGE_CodeSequenceRequestError) }));

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
    auto status = SetCodeSequencesJsonRequestToChangeSet(codeSequence, startIndex, incrementBy, CodeSequence::Type::NextAvailable, *changeset, WSChangeset::ChangeState::Created);
    if (DgnDbStatus::Success != status)
        return CreateCompletedAsyncTask<CodeSequenceResult>(CodeSequenceResult::Error({ Error::Id::iModelHubOperationFailed, ErrorLocalizedString(MESSAGE_CodeSequenceRequestError) }));

    return QueryCodeNextAvailableInternal(changeset, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             09/2016
//---------------------------------------------------------------------------------------
BriefcaseInfoTaskPtr iModelConnection::AcquireNewBriefcase(ICancellationTokenPtr cancellationToken) const
    {
    return ExecutionManager::ExecuteWithRetry<BriefcaseInfoPtr>([=]()
        {
        return CreateBriefcaseInstance(cancellationToken)->Then<BriefcaseInfoResult>([=] (const WSCreateObjectResult& result)
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
//@bsimethod                                     Eligijus.Mauragas              03/2016
//---------------------------------------------------------------------------------------
ChangeSetsInfoTaskPtr iModelConnection::GetAllChangeSets (ICancellationTokenPtr cancellationToken) const
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
WSQuery iModelConnection::CreateBetweenChangeSetsQuery
(
Utf8StringCR firstchangeSetId, 
Utf8StringCR secondChangeSetId,
BeSQLite::BeGuidCR fileId
) const
    {
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet);
    Utf8String queryFilter;

    Utf8String notNullChangeSetId = "";
    if (Utf8String::IsNullOrEmpty(firstchangeSetId.c_str()))
        notNullChangeSetId = secondChangeSetId;

    if (Utf8String::IsNullOrEmpty(secondChangeSetId.c_str()))
        notNullChangeSetId = firstchangeSetId;

    if(!Utf8String::IsNullOrEmpty(notNullChangeSetId.c_str()))
        {
        if (fileId.IsValid())
            queryFilter.Sprintf("%s-backward-%s.%s+eq+'%s'+and+%s+eq+'%s'",
                                ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, notNullChangeSetId.c_str(),
                                ServerSchema::Property::SeedFileId, fileId.ToString().c_str());
        else
            queryFilter.Sprintf("%s-backward-%s.%s+eq+'%s'",
                                ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, notNullChangeSetId.c_str());
        }
    else
        {
        if (fileId.IsValid())
            queryFilter.Sprintf("(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')+or+(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')+and+%s+eq+'%s'",
                                ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, firstchangeSetId.c_str(),
                                ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, secondChangeSetId.c_str(),
                                ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, secondChangeSetId.c_str(),
                                ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, firstchangeSetId.c_str(),
                                ServerSchema::Property::SeedFileId, fileId.ToString().c_str());
        else
            queryFilter.Sprintf("(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')+or+(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')",
                                ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, firstchangeSetId.c_str(),
                                ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, secondChangeSetId.c_str(),
                                ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, secondChangeSetId.c_str(),
                                ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, firstchangeSetId.c_str());
        }

    query.SetFilter(queryFilter);

    return query;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite           08/2017
//---------------------------------------------------------------------------------------
ChangeSetsInfoTaskPtr iModelConnection::GetChangeSetsBetween(Utf8StringCR firstChangeSetId, Utf8StringCR secondChangeSetId, BeSQLite::BeGuidCR fileId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::GetChangeSetsBetween";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (Utf8String::IsNullOrEmpty(firstChangeSetId.c_str()) && Utf8String::IsNullOrEmpty(secondChangeSetId.c_str()))
        return CreateCompletedAsyncTask(ChangeSetsInfoResult::Error(Error::Id::InvalidChangeSet));

    return ChangeSetsFromQueryInternal(CreateBetweenChangeSetsQuery(firstChangeSetId, secondChangeSetId, fileId), false, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
ChangeSetsTaskPtr iModelConnection::DownloadChangeSets(const bvector<Utf8String>& changeSetIds, Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
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
ChangeSetsTaskPtr iModelConnection::DownloadChangeSets(const bvector<ChangeSetInfoPtr>& changeSets, Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
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
    return GetChangeSetsAfterIdInternal(changeSetId, fileId, true, cancellationToken)->Then([=] (ChangeSetsInfoResultCR changeSetsResult)
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
                    LogHelper::Log(SEVERITY::LOG_ERROR, methodName, downloadResult.GetError().GetMessage().c_str());
                    finalResult->SetError(downloadResult.GetError());
                    }
                });
            }
        else
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, changeSetsResult.GetError().GetMessage().c_str());
            finalResult->SetError(changeSetsResult.GetError());
            }
        })->Then<ChangeSetsResult>([=] ()
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             01/2017
//---------------------------------------------------------------------------------------
void iModelConnection::SubscribeChangeSetsDownload()
    {
    if (m_subscribedForPreDownload)
        return;

    m_subscribedForPreDownload = true;
    s_preDownloadManager->SubscribeChangeSetsDownload(this);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
StatusTaskPtr iModelConnection::VerifyConnection(ICancellationTokenPtr cancellationToken) const
    {
    return ExecutionManager::ExecuteWithRetry<void>([=]()
        {
        return m_wsRepositoryClient->VerifyAccess(cancellationToken)->Then<StatusResult>([] (const AsyncResult<void, WSError>& result)
            {
            const Utf8String methodName = "iModelConnection::VerifyConnection";
            LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
            if (result.IsSuccess())
                return StatusResult::Success();
            else
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
                return StatusResult::Error(result.GetError());
                }
            });
        });
    }
