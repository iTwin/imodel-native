/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/Utils.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/iModelHub/Common.h>
#include <BeHttp/HttpRequest.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/LocksManager.h>
#include <WebServices/iModelHub/Client/iModelConnection.h>
#include "Error.xliff.h"
USING_NAMESPACE_BENTLEY_DGN

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
    namespace Schema
        {
        static Utf8CP iModel = "iModelScope";
        static Utf8CP Project = "ProjectScope";
        }
    namespace Plugin
        {
        static Utf8CP iModel = "iModel";
        static Utf8CP Project = "Project";
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
        static Utf8CP UserDefinition = "UserDefinition";
        static Utf8CP Version = "Version";
        static Utf8CP UserInfo = "UserInfo";
        }
    namespace Relationship
        {
        static Utf8CP FollowingChangeSet = "FollowingChangeSet";
        static Utf8CP FileAccessKey = "FileAccessKey";
        static Utf8CP iModelOwnerInfo = "iModelOwnerInfo";
		static Utf8CP CumulativeChangeSet = "CumulativeChangeSet";
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
        static Utf8CP CodesRequiresPull = "CodesRequiresPull";
        static Utf8CP EventServiceSASToken = "EventServiceSASToken";
        static Utf8CP BaseAddress = "BaseAddress";
        static Utf8CP EventTypes = "EventTypes";
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
        }
    namespace ExtendedParameters
        {
        static Utf8CP DetailedError_Locks = "DetailedError_Locks";
        static Utf8CP DetailedError_Codes = "DetailedError_Codes";
        static Utf8CP SetMaximumInstances = "DetailedError_MaximumInstances";
        }
    static Utf8CP DeleteAllLocks = "DeleteAll";
    static Utf8CP DiscardReservedCodes = "DiscardReservedCodes";
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
        static Utf8CP iModelURL = "iModelHubServiceURL";
        static Utf8CP iModelId = "iModelId";
        static Utf8CP ParentChangeSetId = "ParentChangeSetId";
        }
    namespace Properties
        {
        static Utf8CP ProjectNamespace = "dgn_Proj";
        static Utf8CP Name = "Name";
        static Utf8CP Description = "Description";
        }
    }

bool GetMultiLockFromServerJson(RapidJsonValueCR serverJson, DgnLockSet& lockSet, BeSQLite::BeBriefcaseId& briefcaseId, Utf8StringR iModelId);
bool GetLockFromServerJson (RapidJsonValueCR serverJson, DgnLockR lock, BeSQLite::BeBriefcaseId& briefcaseId, Utf8StringR iModelId);
bool AddLockInfoToListFromErrorJson(DgnLockInfoSet& lockInfos, RapidJsonValueCR serverJson);
void AddLockInfoToList (DgnLockInfoSet& lockInfos, const DgnLock& dgnLock, const BeSQLite::BeBriefcaseId briefcaseId, Utf8StringCR iModelId);
bool GetMultiCodeFromServerJson(RapidJsonValueCR serverJson, DgnCodeSet& codeSet, DgnCodeStateR codeState, BeSQLite::BeBriefcaseId& briefcaseId);
bool GetCodeFromServerJson (RapidJsonValueCR serverJson, DgnCodeR code, DgnCodeStateR codeState, BeSQLite::BeBriefcaseId& briefcaseId);
void AddCodeInfoToList (DgnCodeInfoSet& codeInfos, const DgnCode& dgnCode, DgnCodeState codeState, const BeSQLite::BeBriefcaseId briefcaseId);
bool GetCodeSequenceFromServerJson(RapidJsonValueCR serverJson, CodeSequence& codeSequence);
rapidjson::Document ToRapidJson(JsonValueCR source);

RevisionStatus ValidateChangeSets(ChangeSets const& changeSets, DgnDbR db);
void ConvertToChangeSetPointersVector(ChangeSets changeSets, bvector<DgnRevisionCP>& pointersVector);

//=======================================================================================
//@bsiclass                                      Algirdas.Mikoliunas             10/2016
// This class is created for tasks retry if unknown error occured.
// DO NOT nest methods that are using this method, because it will result in nested retry
// DO add this wrapper for top level methods that do not fail if executed multiple times in a row
//=======================================================================================

bool IsErrorForRetry(Error::Id errorId);

template <typename T>
static TaskPtr<T> ExecuteWithRetry(const std::function<TaskPtr<T>()> taskCallback)
    {
    std::shared_ptr<Result<T>> finalResult = std::make_shared<Result<T>>();
    return taskCallback()->Then([=](Result<T>const& res) 
        {
        *finalResult = res;
        if (!res.IsSuccess())
            {
            if (IsErrorForRetry(res.GetError().GetId()))
                {
                taskCallback()->Then([=](Result<T>const& res)
                    {
                    *finalResult = res;
                    });
                }
            }

        return ;
        })->template Then<Result<T>>([=]()
            {
            return *finalResult;
            });
    }

template <typename T>
static T& ExecuteAsync(AsyncTaskPtr<T> task, int wait = 60000)
    {
    task->WaitFor(wait);

    if (task->IsCompleted())
        return task->GetResult();

    return Result<T>::Error(Error::Id::ExecutionTimeout).GetValue();
    }

   

END_BENTLEY_IMODELHUB_NAMESPACE
