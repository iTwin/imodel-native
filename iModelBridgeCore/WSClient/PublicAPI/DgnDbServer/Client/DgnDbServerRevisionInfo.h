/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbServerRevisionInfo.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/DgnDbServerResult.h>
#include <WebServices/Client/Response/WSObjectsReader.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_WEBSERVICES
typedef std::shared_ptr<struct DgnDbServerRevisionInfo> DgnDbServerRevisionInfoPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(DgnDbServerRevisionInfo);
DEFINE_TASK_TYPEDEFS(DgnDbServerRevisionInfoPtr, DgnDbServerRevisionInfo);
DEFINE_TASK_TYPEDEFS(bvector<DgnDbServerRevisionInfoPtr>, DgnDbServerRevisionsInfo);
typedef bvector<Dgn::DgnRevisionPtr> DgnRevisions;
DEFINE_TASK_TYPEDEFS(DgnRevisions, DgnRevisions);
typedef std::shared_ptr<struct DgnDbServerRevisionInfo> DgnDbServerRevisionInfoPtr;
DEFINE_TASK_TYPEDEFS(Dgn::DgnRevisionPtr, DgnRevision);

//=======================================================================================
//! Information about revision.
//@bsiclass                                      Algirdas.Mikoliunas            01/2017
//=======================================================================================
struct DgnDbServerRevisionInfo
    {
    public:
        enum ContainingChanges
            {
            Regular = 0,
            Schema = 1 // Revision contains minor schema changes
            };

    //__PUBLISH_SECTION_END__
    private:
        Utf8String m_id;
        Utf8String m_parentRevisionId;
        Utf8String m_dbGuid;
        uint64_t   m_index;
        Utf8String m_url;
        Utf8String m_description;
        uint64_t   m_fileSize;
        Utf8String m_userCreated;
        DateTime   m_pushDate;
        BeSQLite::BeBriefcaseId m_briefcaseId;
        ContainingChanges       m_containingChanges;

        //__PUBLISH_SECTION_START__
    public:
        
        DGNDBSERVERCLIENT_EXPORT DgnDbServerRevisionInfo(Utf8String id, Utf8String parentRevisionId, Utf8String dbGuid, int64_t index, Utf8String url,
            Utf8String description, int64_t fileSize, BeSQLite::BeBriefcaseId briefcaseId, Utf8String userCreated, DateTime pushDate, ContainingChanges containingChanges);

        //__PUBLISH_SECTION_END__
        bool operator==(DgnDbServerRevisionInfoCR briefcase) const;
        static DgnDbServerRevisionInfoPtr Parse(WSObjectsReader::Instance instance);
        //! DEPRECATED: Use Parse from Instance
        static DgnDbServerRevisionInfoPtr Parse(JsonValueCR json);
        //__PUBLISH_SECTION_START__
        DGNDBSERVERCLIENT_EXPORT Utf8String GetId() const;
        DGNDBSERVERCLIENT_EXPORT Utf8String GetParentRevisionId() const;
        DGNDBSERVERCLIENT_EXPORT Utf8String GetDbGuid() const;
        DGNDBSERVERCLIENT_EXPORT uint64_t   GetIndex() const;
        DGNDBSERVERCLIENT_EXPORT Utf8String GetUrl() const;
        DGNDBSERVERCLIENT_EXPORT Utf8String GetDescription() const;
        DGNDBSERVERCLIENT_EXPORT uint64_t   GetFileSize() const;
        DGNDBSERVERCLIENT_EXPORT Utf8String GetUserCreated() const;
        DGNDBSERVERCLIENT_EXPORT DateTime   GetPushDate() const;
        DGNDBSERVERCLIENT_EXPORT ContainingChanges       GetContainingChanges() const;
        DGNDBSERVERCLIENT_EXPORT BeSQLite::BeBriefcaseId GetBriefcaseId() const;
    };
END_BENTLEY_DGNDBSERVER_NAMESPACE
