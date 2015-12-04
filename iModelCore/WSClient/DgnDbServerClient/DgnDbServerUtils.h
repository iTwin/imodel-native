/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerUtils.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnClientFx/Utils/Http/HttpRequest.h>
#include <DgnPlatform/DgnPlatformLib.h>

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
        static Utf8CP Repository = "DgnDbServerSchema";
        static Utf8CP Admin = "DgnDbServerAdminSchema";
        }
    namespace Plugin
        {
        static Utf8CP Repository = "Bentley.DgnDbServerECPlugin";
        static Utf8CP Admin = "Bentley.DgnDbServerAdminECPlugin";
        }
    namespace Class
        {
        static Utf8CP Briefcase = "DgnDbBriefcase";
        static Utf8CP Revision = "Revision";
        static Utf8CP File = "DgnDbFile";
        static Utf8CP Repository = "DgnDbRepository";
        static Utf8CP Lock = "Lock";
        }
    namespace Property
        {
        static Utf8CP Id = "Id";
        static Utf8CP FileName = "FileName";
        static Utf8CP FileId = "FileId";
        static Utf8CP Index = "Index";
        static Utf8CP Description = "Description";
        static Utf8CP UserUploaded = "UserUploaded";
        static Utf8CP UploadedDate = "UploadedDate";
        static Utf8CP FileSize = "FileSize";
        static Utf8CP BriefcaseId = "BriefcaseId";
        static Utf8CP ParentId = "ParentId";
        static Utf8CP MasterFileId = "MasterFileId";
        static Utf8CP UserCreated = "UserCreated";
        static Utf8CP PushDate = "PushDate";
        static Utf8CP Published = "Published";
        static Utf8CP ObjectId = "ObjectId";
        static Utf8CP LockType = "LockType";
        static Utf8CP LockLevel = "LockLevel";
        }
    }

namespace Db
    {
    namespace Local
        {
        static Utf8CP LastRevision = "ParentRevisionId";
        static Utf8CP RepositoryURL = "ServerURL";
        static Utf8CP RepositoryId = "RepositoryId";
        }
    namespace Properties
        {
        static Utf8CP ProjectNamespace = "dgn_Proj";
        static Utf8CP Name = "Name";
        static Utf8CP Description = "Description";
        }
    }

namespace Error
    {
    static Utf8CP NotInitialized = "DgnDbServerClient library not initialized";
    static Utf8CP InvalidRepository = "Invalid repository.";
    static Utf8CP InvalidRevision = "Invalid revision.";
    static Utf8CP InvalidCredentials = "Invalid credentials.";
    static Utf8CP InvalidServerURL = "Invalid server URL";
    static Utf8CP DbNotFound = "Could not find briefcase db file.";
    static Utf8CP DbNotRepository = "Db file is not a valid briefcase.";
    static Utf8CP ConnectionNotFound = "Connection is not open.";
    static Utf8CP DbReadOnly = "Briefcase db is open as read only.";
    static Utf8CP RevisionsMerge = "Unable to merge revisions.";
    static Utf8CP RevisionsFinish = "Unable to finish creating revision.";
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
            DgnClientFx::Utils::HttpRequest::ProgressCallback callback;
            CallbackQueue& m_queue;
            double m_bytesTransfered;
            double m_bytesTotal;
            Callback(CallbackQueue& queue);
            };
        friend struct CallbackQueue::Callback;
        bvector<std::shared_ptr<CallbackQueue::Callback>> m_callbacks;
        DgnClientFx::Utils::HttpRequest::ProgressCallbackCR m_callback;
    public:
        CallbackQueue(DgnClientFx::Utils::HttpRequest::ProgressCallbackCR callback);

        DgnClientFx::Utils::HttpRequest::ProgressCallbackCR NewCallback();
    };

//=======================================================================================
//@bsiclass                                      Karolis.Dziedzelis             11/2015
//=======================================================================================
struct DgnDbServerHost : public Dgn::DgnPlatformLib::Host
    {
    private:
        BeFileNameCR m_temp;
        BeFileNameCR m_assets;
        static std::unique_ptr<DgnDbServerHost> m_host;
    public:
        DgnDbServerHost(BeFileNameCR temp, BeFileNameCR assets);
        ~DgnDbServerHost();
        static void Initialize(BeFileNameCR temp, BeFileNameCR assets);
        static bool IsInitialized();

        virtual void _SupplyProductName(Utf8StringR name) override { name.assign("DgnDb Server"); }
        virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override;
        virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() { return BeSQLite::L10N::SqlangFiles(BeFileName()); }

        static DgnDbServerHost& Host();
    };
END_BENTLEY_DGNDBSERVER_NAMESPACE
