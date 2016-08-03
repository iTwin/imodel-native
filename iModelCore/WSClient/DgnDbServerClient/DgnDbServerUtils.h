/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerUtils.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/DgnDbServerResult.h>
#include <BeHttp/HttpRequest.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/LocksManager.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE

namespace ServerSchema
    {
    static Utf8CP Instance = "instance";
    static Utf8CP InstanceId = "instanceId";
    static Utf8CP ChangedInstance = "changedInstance";
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
        static Utf8CP Briefcase = "DgnDbBriefcase";
        static Utf8CP Revision = "Revision";
        static Utf8CP File = "DgnDbFile";
        static Utf8CP Repository = "BIMRepository";
        static Utf8CP Lock = "Lock";
        static Utf8CP MultiLock = "MultiLock";
        static Utf8CP Code = "Code";
        static Utf8CP MultiCode = "MultiCode";
        }
    namespace Property
        {
        static Utf8CP Id = "Id";
        static Utf8CP RepositoryName = "RepositoryName";
        static Utf8CP FileName = "FileName";
        static Utf8CP FileId = "FileId";
        static Utf8CP Index = "Index";
        static Utf8CP Description = "Description";
        static Utf8CP UserCreated = "UserCreated";
        static Utf8CP CreatedDate = "CreatedDate";
        static Utf8CP UserUploaded = "UserUploaded";
        static Utf8CP UploadedDate = "UploadedDate";
        static Utf8CP FileSize = "FileSize";
        static Utf8CP BriefcaseId = "BriefcaseId";
        static Utf8CP ParentId = "ParentId";
        static Utf8CP MasterFileId = "MasterFileId";
        static Utf8CP MergedRevisionId = "MergedRevisionId";
        static Utf8CP PushDate = "PushDate";
        static Utf8CP Published = "Published";
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
        static Utf8CP IsReadOnly = "IsReadOnly";
        static Utf8CP AuthorityId = "AuthorityId";
        static Utf8CP Namespace = "Namespace";
        static Utf8CP Value = "Value";
        static Utf8CP Values = "Values";
        static Utf8CP StateRevision = "StateRevision";
        static Utf8CP StateRevisionIndex = "StateRevisionIndex";
        static Utf8CP State = "State";
        static Utf8CP CodeStateInvalid = "CodeStateInvalid";
        static Utf8CP ConflictingCodes = "ConflictingCodes";
        static Utf8CP CodesNotFound = "CodesNotFound";
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

//=======================================================================================
//@bsiclass                                      Karolis.Dziedzelis             11/2015
//=======================================================================================
struct DgnDbServerHost : public Dgn::DgnPlatformLib::Host
    {
    DEFINE_T_SUPER(Dgn::DgnPlatformLib::Host);
    private:
        bool m_initialized;
        bool m_terminated;
        static BeFileName s_temp;
        static BeFileName s_assets;
    public:
        DgnDbServerHost();
        ~DgnDbServerHost();

        virtual void _SupplyProductName(Utf8StringR name) override { name.assign("DgnDb Server"); }
        virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override;
        virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() { return BeSQLite::L10N::SqlangFiles(BeFileName()); }

        static void Initialize(BeFileNameCR temp, BeFileNameCR assets);
        static void Adopt(std::shared_ptr<DgnDbServerHost> const& host);
        static void Forget(std::shared_ptr<DgnDbServerHost> const& host, bool terminate = true);

        static bool IsInitialized();
    };

bool GetLockFromServerJson (JsonValueCR serverJson, DgnLockR lock, BeSQLite::BeBriefcaseId& briefcaseId, Utf8StringR repositoryId);
void AddLockInfoToList (DgnLockInfoSet& lockInfos, const DgnLock& dgnLock, const BeSQLite::BeBriefcaseId briefcaseId, Utf8StringCR repositoryId);
bool GetCodeFromServerJson (JsonValueCR serverJson, DgnCodeR code, DgnCodeStateR codeState, BeSQLite::BeBriefcaseId& briefcaseId, Utf8StringR repositoryId);
void AddCodeInfoToList (DgnCodeInfoSet& codeInfos, const DgnCode& dgnCode, DgnCodeState codeState, const BeSQLite::BeBriefcaseId briefcaseId, Utf8StringCR revisionId);

END_BENTLEY_DGNDBSERVER_NAMESPACE
