/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerUtils.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnDbServer/DgnDbServerCommon.h>
#include <BeHttp/HttpRequest.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/LocksManager.h>
#include <DgnDbServer/Client/DgnDbRepositoryConnection.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE

namespace ServerSchema
    {
    static Utf8CP Instance = "instance";
    static Utf8CP InstanceId = "instanceId";
    static Utf8CP ChangedInstance = "changedInstance";
    static Utf8CP ChangedInstances = "changedInstances";
    static Utf8CP InstanceAfterChange = "instanceAfterChange";
    static Utf8CP Instances = "instances";
    static Utf8CP Properties = "properties";
    static Utf8CP SchemaName = "schemaName";
    static Utf8CP ClassName = "className";
    namespace Schema
        {
        static Utf8CP Repository = "BIMCSRepository";
        static Utf8CP Project = "BIMCSProject";
        }
    namespace Plugin
        {
        static Utf8CP Repository = "BIMCSRepository";
        static Utf8CP Project = "BIMCSProject";
        }
    namespace Class
        {
        static Utf8CP RepositoryLock = "RepositoryLock";
        static Utf8CP Briefcase = "BIMBriefcase";
        static Utf8CP Revision = "Revision";
        static Utf8CP File = "BIMFile";
        static Utf8CP Repository = "BIMRepository";
        static Utf8CP Lock = "Lock";
        static Utf8CP MultiLock = "MultiLock";
        static Utf8CP Code = "Code";
        static Utf8CP CodeTemplate = "CodeTemplate";
        static Utf8CP MultiCode = "MultiCode";
        static Utf8CP EventSAS = "EventSAS";
        static Utf8CP EventSubscription = "EventSubscription";
        static Utf8CP UserDefinition = "UserDefinition";
        }
    namespace Relationship
        {
        static Utf8CP FollowingRevision = "FollowingRevision";
        }
    namespace Property
        {
        static Utf8CP Id = "Id";
        static Utf8CP RepositoryName = "RepositoryName";
        static Utf8CP FileName = "FileName";
        static Utf8CP Name = "Name";
        static Utf8CP Password = "Password";
        static Utf8CP IsAdmin = "IsAdmin";
        static Utf8CP FileId = "FileId";
        static Utf8CP Index = "Index";
        static Utf8CP Description = "Description";
        static Utf8CP FileDescription = "FileDescription";
        static Utf8CP RepositoryDescription = "RepositoryDescription";
        static Utf8CP UserCreated = "UserCreated";
        static Utf8CP UserOwned = "UserOwned";
        static Utf8CP CreatedDate = "CreatedDate";
        static Utf8CP UserUploaded = "UserUploaded";
        static Utf8CP UploadedDate = "UploadedDate";
        static Utf8CP FileSize = "FileSize";
        static Utf8CP BriefcaseId = "BriefcaseId";
        static Utf8CP ParentId = "ParentId";
        static Utf8CP MasterFileId = "MasterFileId";
        static Utf8CP MergedRevisionId = "MergedRevisionId";
        static Utf8CP PushDate = "PushDate";
        static Utf8CP ObjectId = "ObjectId";
        static Utf8CP ObjectIds = "ObjectIds";
        static Utf8CP QueryOnly = "QueryOnly";
        static Utf8CP LockType = "LockType";
        static Utf8CP LockLevel = "LockLevel";
        static Utf8CP URL = "URL";
        static Utf8CP IsUploaded = "IsUploaded";
        static Utf8CP ReleasedWithRevision = "ReleasedWithRevision";
        static Utf8CP ReleasedWithRevisionIndex = "ReleasedWithRevisionIndex";
        static Utf8CP ConflictingLocks = "ConflictingLocks";
        static Utf8CP LocksRequiresPull = "LocksRequiresPull";
        static Utf8CP CodesRequiresPull = "CodesRequiresPull";
        static Utf8CP EventServiceSASToken = "EventServiceSASToken";
        static Utf8CP BaseAddress = "BaseAddress";
        static Utf8CP EventTypes = "EventTypes";
        static Utf8CP IsReadOnly = "IsReadOnly";
        static Utf8CP AuthorityId = "AuthorityId";
        static Utf8CP Namespace = "Namespace";
        static Utf8CP Value = "Value";
        static Utf8CP Values = "Values";
        static Utf8CP ValuePattern = "ValuePattern";
        static Utf8CP Type = "Type";
        static Utf8CP StartIndex = "StartIndex";
        static Utf8CP IncrementBy = "IncrementBy";
        static Utf8CP StateRevision = "StateRevision";
        static Utf8CP StateRevisionIndex = "StateRevisionIndex";
        static Utf8CP Reserved = "Reserved";
        static Utf8CP Used = "Used";
        static Utf8CP CodeStateInvalid = "CodeStateInvalid";
        static Utf8CP ConflictingCodes = "ConflictingCodes";
        static Utf8CP CodesNotFound = "CodesNotFound";
        static Utf8CP Initialized = "Initialized";
        static Utf8CP FileInitialized = "FileInitialized";
        static Utf8CP RepositoryInitialized = "RepositoryInitialized";
        static Utf8CP ContainingChanges = "ContainingChanges";
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
        static Utf8CP RepositoryURL = "ServerURL";
        static Utf8CP RepositoryId = "RepositoryId";
        static Utf8CP ParentRevisionId = "ParentRevisionId";
        }
    namespace Properties
        {
        static Utf8CP ProjectNamespace = "dgn_Proj";
        static Utf8CP Name = "Name";
        static Utf8CP Description = "Description";
        }
    }

//=======================================================================================
//@bsiclass                                      Karolis.Dziedzelis             11/2015
//=======================================================================================
struct CallbackQueue
    {
    private:
        void Notify();
        struct Callback
            {
            Http::Request::ProgressCallback callback;
            CallbackQueue& m_queue;
            double m_bytesTransfered;
            double m_bytesTotal;
            Callback(CallbackQueue& queue);
            };
        friend struct CallbackQueue::Callback;
        bvector<std::shared_ptr<CallbackQueue::Callback>> m_callbacks;
        Http::Request::ProgressCallbackCR m_callback;
    public:
        CallbackQueue(Http::Request::ProgressCallbackCR callback);

        Http::Request::ProgressCallbackCR NewCallback();
    };
bool GetMultiLockFromServerJson(RapidJsonValueCR serverJson, DgnLockSet& lockSet, BeSQLite::BeBriefcaseId& briefcaseId, Utf8StringR repositoryId);
bool GetLockFromServerJson (RapidJsonValueCR serverJson, DgnLockR lock, BeSQLite::BeBriefcaseId& briefcaseId, Utf8StringR repositoryId);
void AddLockInfoToList (DgnLockInfoSet& lockInfos, const DgnLock& dgnLock, const BeSQLite::BeBriefcaseId briefcaseId, Utf8StringCR repositoryId);
bool GetMultiCodeFromServerJson(RapidJsonValueCR serverJson, DgnCodeSet& codeSet, DgnCodeStateR codeState, BeSQLite::BeBriefcaseId& briefcaseId, Utf8StringR revisionId);
bool GetCodeFromServerJson (RapidJsonValueCR serverJson, DgnCodeR code, DgnCodeStateR codeState, BeSQLite::BeBriefcaseId& briefcaseId, Utf8StringR repositoryId);
void AddCodeInfoToList (DgnCodeInfoSet& codeInfos, const DgnCode& dgnCode, DgnCodeState codeState, const BeSQLite::BeBriefcaseId briefcaseId, Utf8StringCR revisionId);
bool GetCodeTemplateFromServerJson(RapidJsonValueCR serverJson, DgnDbCodeTemplate& codeTemplate);
rapidjson::Document ToRapidJson(JsonValueCR source);

//=======================================================================================
//@bsiclass                                      Algirdas.Mikoliunas             10/2016
// This class is created for tasks retry if unknown error occured.
// DO NOT nest methods that are using this method, because it will result in nested retry
// DO add this wrapper for top level methods that do not fail if executed multiple times in a row
//=======================================================================================
struct ExecutionManager
    {
    static bool IsErrorForRetry(DgnDbServerError::Id errorId);

    template <typename T>
    static DgnDbServerTaskPtr<T> ExecuteWithRetry(const std::function<DgnDbServerTaskPtr<T>()> taskCallback)
        {
        std::shared_ptr<DgnDbServerResult<T>> finalResult = std::make_shared<DgnDbServerResult<T>>();
        return taskCallback()->Then([=](DgnDbServerResult<T>const& res) 
            {
            *finalResult = res;
            if (!res.IsSuccess())
                {
                if (IsErrorForRetry(res.GetError().GetId()))
                    {
                    taskCallback()->Then([=](DgnDbServerResult<T>const& res)
                        {
                        *finalResult = res;
                        });
                    }
                }

            return ;
            })->template Then<DgnDbServerResult<T>>([=]()
                {
                return *finalResult;
                });
        }
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE
