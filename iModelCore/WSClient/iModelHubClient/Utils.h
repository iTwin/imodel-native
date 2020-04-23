/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <BeHttp/HttpRequest.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/LocksManager.h>
#include <WebServices/iModelHub/Client/iModelConnection.h>
#include "Error.xliff.h"
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE

#define CHECK_BRIEFCASEID(b,t) if (!b.IsValid() || b.IsSnapshot() || b.IsStandAloneId()){ LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "BriefcaseId is not valid."); return CreateCompletedAsyncTask<t>(t::Error(Error::Id::InvalidBriefcase));}
BEGIN_BENTLEY_IMODELHUB_NAMESPACE

namespace ServerSchema
    {
    static Utf8CP Instance = "instance";
    static Utf8CP InstanceId = "instanceId";
    static Utf8CP ChangedInstance = "changedInstance";
    static Utf8CP ChangedInstances = "changedInstances";
    static Utf8CP InstanceAfterChange = "instanceAfterChange";
    static Utf8CP Instances = "instances";
    static Utf8CP Properties = "properties";
    static Utf8CP RelationshipInstances = "relationshipInstances";
    static Utf8CP RelatedInstance = "relatedInstance";
    static Utf8CP SchemaName = "schemaName";
    static Utf8CP ClassName = "className";
    static Utf8CP Direction = "direction";
    namespace Schema
        {
        static Utf8CP iModel = "iModelScope";
        static Utf8CP Project = "ProjectScope";
        static Utf8CP Global = "GlobalScope";
        static Utf8CP Context = "ContextScope";
        }
    namespace Plugin
        {
        static Utf8CP iModel = "iModel";
        static Utf8CP Global = "Global";
        static Utf8CP Project = "Project";
        static Utf8CP Context = "Context";
        }
    namespace Class
        {
        static Utf8CP iModelLock = "iModelLock";
        static Utf8CP Briefcase = "Briefcase";
        static Utf8CP ChangeSet = "ChangeSet";
        static Utf8CP File = "SeedFile";
        static Utf8CP AccessKey = "AccessKey";
        static Utf8CP iModel = "iModel";
        static Utf8CP Lock = "Lock";
        static Utf8CP MultiLock = "MultiLock";
        static Utf8CP Code = "Code";
        static Utf8CP CodeSequence = "CodeSequence";
        static Utf8CP MultiCode = "MultiCode";
        static Utf8CP EventSAS = "EventSAS";
        static Utf8CP EventSubscription = "EventSubscription";
        static Utf8CP GlobalEventSAS = "GlobalEventSAS";
        static Utf8CP GlobalEventSubscription = "GlobalEventSubscription";
        static Utf8CP UserDefinition = "UserDefinition";
        static Utf8CP Version = "Version";
        static Utf8CP UserInfo = "UserInfo";
        static Utf8CP Statistics = "Statistics";
        static Utf8CP SmallThumbnail = "SmallThumbnail";
        static Utf8CP LargeThumbnail = "LargeThumbnail";
        static Utf8CP BridgeProperties = "BridgeProperties";
        }
    namespace Relationship
        {
        static Utf8CP FollowingChangeSet = "FollowingChangeSet";
        static Utf8CP FileAccessKey = "FileAccessKey";
        static Utf8CP HasCreatorInfo = "HasCreatorInfo";
        static Utf8CP CumulativeChangeSet = "CumulativeChangeSet";
        static Utf8CP HasStatistics = "HasStatistics";
        static Utf8CP HasThumbnail = "HasThumbnail";
        static Utf8CP HasBridgeProperties = "HasBridgeProperties";
        }
    namespace RelationshipDirection
        {
        static Utf8CP Forward = "forward";
        }
    namespace Property
        {
        static Utf8CP Id = "Id";
        static Utf8CP iModelName = "Name";
        static Utf8CP FileName = "FileName";
        static Utf8CP Name = "Name";
        static Utf8CP Password = "Password";
        static Utf8CP IsAdmin = "IsAdmin";
        static Utf8CP FileId = "FileId";
        static Utf8CP Index = "Index";
        static Utf8CP Description = "Description";
        static Utf8CP FileDescription = "FileDescription";
        static Utf8CP iModelDescription = "Description";
        static Utf8CP iModelTemplate = "iModelTemplate";
        static Utf8CP Extent = "Extent";
        static Utf8CP UserCreated = "UserCreated";
        static Utf8CP UserOwned = "UserOwned";
        static Utf8CP CreatedDate = "CreatedDate";
        static Utf8CP UserUploaded = "UserUploaded";
        static Utf8CP UploadedDate = "UploadedDate";
        static Utf8CP FileSize = "FileSize";
        static Utf8CP BriefcaseId = "BriefcaseId";
        static Utf8CP BriefcaseIds = "BriefcaseIds";
        static Utf8CP ParentId = "ParentId";
        static Utf8CP SeedFileId = "SeedFileId";
        static Utf8CP MergedChangeSetId = "MergedChangeSetId";
        static Utf8CP PushDate = "PushDate";
        static Utf8CP ObjectId = "ObjectId";
        static Utf8CP ObjectIds = "ObjectIds";
        static Utf8CP QueryOnly = "QueryOnly";
        static Utf8CP LockType = "LockType";
        static Utf8CP LockLevel = "LockLevel";
        static Utf8CP LockOwners = "LockOwners";
        static Utf8CP URL = "URL";
        static Utf8CP IsUploaded = "IsUploaded";
        static Utf8CP ReleasedWithChangeSet = "ReleasedWithChangeSet";
        static Utf8CP ReleasedWithChangeSetIndex = "ReleasedWithChangeSetIndex";
        static Utf8CP ConflictingLocks = "ConflictingLocks";
        static Utf8CP LocksRequiresPull = "LocksRequiresPull";
        static Utf8CP LocksNotFound = "LocksNotFound";
        static Utf8CP CodesRequiresPull = "CodesRequiresPull";
        static Utf8CP EventServiceSASToken = "EventServiceSASToken";
        static Utf8CP BaseAddress = "BaseAddress";
        static Utf8CP EventTypes = "EventTypes";
        static Utf8CP SubscriptionId = "SubscriptionId";
        static Utf8CP IsReadOnly = "IsReadOnly";
        static Utf8CP CodeSpecId = "CodeSpecId";
        static Utf8CP CodeScope = "CodeScope";
        static Utf8CP State = "State";
        static Utf8CP Value = "Value";
        static Utf8CP Values = "Values";
        static Utf8CP ValuePattern = "ValuePattern";
        static Utf8CP Type = "Type";
        static Utf8CP StartIndex = "StartIndex";
        static Utf8CP IncrementBy = "IncrementBy";
        static Utf8CP CodeStateInvalid = "CodeStateInvalid";
        static Utf8CP ConflictingCodes = "ConflictingCodes";
        static Utf8CP CodesNotFound = "CodesNotFound";
        static Utf8CP Initialized = "Initialized";
        static Utf8CP InitializationState = "InitializationState";
        static Utf8CP FileInitialized = "FileInitialized";
        static Utf8CP iModelInitialized = "iModelInitialized";
        static Utf8CP ContainingChanges = "ContainingChanges";
        static Utf8CP UploadUrl = "UploadUrl";
        static Utf8CP DownloadUrl = "DownloadUrl";
        static Utf8CP ChangeSetId = "ChangeSetId";
        static Utf8CP Surname = "Surname";
        static Utf8CP Email = "Email";
        static Utf8CP BriefcasesCount = "BriefcasesCount";
        static Utf8CP OwnedLocksCount = "OwnedLocksCount";
        static Utf8CP PushedChangeSetsCount = "PushedChangeSetsCount";
        static Utf8CP LastChangeSetPushDate = "LastChangeSetPushDate";
        static Utf8CP JobId = "JobId";
        static Utf8CP ChangedFiles = "ChangedFiles";
        static Utf8CP Users = "Users";
        }
    namespace ExtendedParameters
        {
        static Utf8CP DetailedError_Locks = "DetailedError_Locks";
        static Utf8CP DetailedError_Codes = "DetailedError_Codes";
        static Utf8CP SetMaximumInstances = "DetailedError_MaximumInstances";
        static Utf8CP ConflictStrategy = "ConflictStrategy";
        }
    static Utf8CP DeleteAllLocks = "DeleteAll";
    static Utf8CP DiscardReservedCodes = "DiscardReservedCodes";
    static Utf8CP iModelTemplateEmpty = "Empty";
    }

namespace Locks
    {
    static Utf8CP Description = "Description";
    static Utf8CP Locks = "Locks";
    static Utf8CP LockableId = "LockableId";
    namespace Lockable
        {
        static Utf8CP Id = "Id";
        static Utf8CP Type = "Type";
        }
    static Utf8CP Level = "Level";
    static Utf8CP Status = "Status";
    static Utf8CP Owner = "Owner";
    static Utf8CP DeniedLocks = "DeniedLocks";
    }

namespace Db
    {
    namespace Local
        {
        static Utf8CP ParentChangeSetId = "ParentChangeSetId";
        }
    namespace Properties
        {
        static Utf8CP ProjectNamespace = "dgn_Proj";
        static Utf8CP iModelHubNamespace = "iModel_Hub";
        static Utf8CP Name = "Name";
        static Utf8CP Description = "Description";
        static Utf8CP iModelURL = "iModelHubServiceURL";
        static Utf8CP iModelId = "iModelId";
        }
    }


struct iModelHubSpec : PropertySpec
    {
    iModelHubSpec(Utf8CP name, PropertySpec::Compress compress = PropertySpec::Compress::Yes) : PropertySpec(name, Db::Properties::iModelHubNamespace, Mode::Normal, compress) {}
    };

struct iModelHubProperties
    {
    static iModelHubSpec iModelURL() { return iModelHubSpec(Db::Properties::iModelURL); }
    static iModelHubSpec iModelId() { return iModelHubSpec(Db::Properties::iModelId); }
    };


struct ServerProperties
    {
    static BeVersion ServiceVersion() { return BeVersion(1, 1); }
    };

bool GetMultiLockFromServerJson(RapidJsonValueCR serverJson, DgnLockSet& lockSet, BeSQLite::BeBriefcaseId& briefcaseId, Utf8StringR iModelId);
bool GetLockFromServerJson (RapidJsonValueCR serverJson, DgnLockR lock, BeSQLite::BeBriefcaseId& briefcaseId, Utf8StringR iModelId);
bool AddLockInfoToListFromErrorJson(DgnLockInfoSet& lockInfos, RapidJsonValueCR serverJson);
void AddLockInfoToList (DgnLockInfoSet& lockInfos, const DgnLock& dgnLock, const BeSQLite::BeBriefcaseId briefcaseId, Utf8StringCR iModelId);
bool GetMultiCodeFromServerJson(RapidJsonValueCR serverJson, DgnCodeSet& codeSet, DgnCodeStateR codeState, BeSQLite::BeBriefcaseId& briefcaseId);
bool GetCodeFromServerJson (RapidJsonValueCR serverJson, DgnCodeR code, DgnCodeStateR codeState, BeSQLite::BeBriefcaseId& briefcaseId);
void AddCodeInfoToList (DgnCodeInfoSet& codeInfos, const DgnCode& dgnCode, DgnCodeState codeState, const BeSQLite::BeBriefcaseId briefcaseId);
bool GetCodeSequenceFromServerJson(RapidJsonValueCR serverJson, CodeSequence& codeSequence);
rapidjson::Document ToRapidJson(JsonValueCR source);

void SetCodesLocksStates(IBriefcaseManager::Response& response, IBriefcaseManager::ResponseOptions options, JsonValueCR deniedLocks, JsonValueCR deniedCodes);
RepositoryStatus GetErrorResponseStatus(Result<void> result);
IBriefcaseManager::Response ConvertErrorResponse(IBriefcaseManager::Request const& request, Result<void> result, IBriefcaseManager::RequestPurpose purpose);

bool ContainsSchemaChanges(ChangeSets const& changeSets, DgnDbR db);
void ConvertToChangeSetPointersVector(ChangeSets changeSets, bvector<DgnRevisionCP>& pointersVector);
Utf8String FormatBeInt64Id(BeInt64Id int64Value);

bool TryParseStringProperty(Utf8StringR result, RapidJsonValueCR propertiesInstance, Utf8CP propertyName);
bool TryParseStringArrayProperty(bvector<Utf8String>& result, RapidJsonValueCR propertiesInstance, Utf8CP propertyName);
bool TryParseGuidProperty(BeSQLite::BeGuid& result, RapidJsonValueCR propertiesInstance, Utf8CP propertyName);
bool TryGetRelatedInstance(WSObjectsReader::Instance& result, WSObjectsReader::Instance mainInstance, Utf8CP className);
void FillArrayPropertyJson(JsonValueR propertiesJson, Utf8CP propertyName, bvector<Utf8String> arrayValues);

//=======================================================================================
//@bsiclass                                      Algirdas.Mikoliunas             10/2016
// This class is created for tasks retry if unknown error occured.
// DO NOT nest methods that are using this method, because it will result in nested retry
// DO add this wrapper for top level methods that do not fail if executed multiple times in a row
//=======================================================================================

bool IsErrorForRetry(Error::Id errorId);

template <class T, class T2>
using ExecuteResult = AsyncResult<T, T2>;
template <class T, class T2>
using ExecuteTaskPtr = AsyncTaskPtr<ExecuteResult<T, T2>>;

template <typename T, class T2>
static ExecuteTaskPtr<T,T2> ExecuteWithRetry(const std::function<ExecuteTaskPtr<T,T2>()> taskCallback)
    {
    std::shared_ptr<ExecuteResult<T,T2>> finalResult = std::make_shared<ExecuteResult<T,T2>>();
    return taskCallback()->Then([=](ExecuteResult<T,T2>const& res)
        {
        *finalResult = res;
        if (!res.IsSuccess())
            {
            auto error = Error(res.GetError());
            if (IsErrorForRetry(error.GetId()))
                {
                taskCallback()->Then([=](ExecuteResult<T,T2>const& res)
                    {
                    *finalResult = res;
                    });
                }
            }

        return ;
        })->template Then<ExecuteResult<T,T2>>([=]()
            {
            return *finalResult;
            });
    }

template <typename T>
static std::shared_ptr<T> ExecuteAsync(AsyncTaskPtr<T> task)
    {
    return std::make_shared<T>(task->GetResult());
    }

END_BENTLEY_IMODELHUB_NAMESPACE
