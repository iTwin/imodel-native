/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbServerBriefcaseInfo.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/DgnDbServerResult.h>
#include <WebServices/Client/Response/WSObjectsReader.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_WEBSERVICES
typedef std::shared_ptr<struct DgnDbServerBriefcaseInfo> DgnDbServerBriefcaseInfoPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(DgnDbServerBriefcaseInfo);
DEFINE_TASK_TYPEDEFS(DgnDbServerBriefcaseInfoPtr, DgnDbServerBriefcaseInfo);
DEFINE_TASK_TYPEDEFS(bvector<DgnDbServerBriefcaseInfoPtr>, DgnDbServerBriefcasesInfo);

//=======================================================================================
//! Information about briefcase.
//@bsiclass                                      julius.cepukenas               08/2015
//=======================================================================================
struct DgnDbServerBriefcaseInfo
    {
    //__PUBLISH_SECTION_END__
    private:
        BeSQLite::BeBriefcaseId m_id;
        BeSQLite::BeGuid m_fileId;
        Utf8String m_userOwned;
        BeFileName m_localPath;
        bool m_isReadOnly;

        //__PUBLISH_SECTION_START__
    public:
        DGNDBSERVERCLIENT_EXPORT DgnDbServerBriefcaseInfo();
        DGNDBSERVERCLIENT_EXPORT DgnDbServerBriefcaseInfo(BeSQLite::BeBriefcaseId id);
        DGNDBSERVERCLIENT_EXPORT DgnDbServerBriefcaseInfo(BeSQLite::BeBriefcaseId id, Utf8StringCR userOwned, BeSQLite::BeGuid fileId, bool isReadOnly);

        //__PUBLISH_SECTION_END__
        bool operator==(DgnDbServerBriefcaseInfoCR briefcase) const;
        static DgnDbServerBriefcaseInfoPtr Parse(WSObjectsReader::Instance instance);
        //! DEPRECATED: Use Parse from Instance
        static DgnDbServerBriefcaseInfoPtr Parse(JsonValueCR json);
        void SetLocalPath(BeFileName localPath);
        //__PUBLISH_SECTION_START__
        DGNDBSERVERCLIENT_EXPORT BeSQLite::BeBriefcaseId GetId() const;
        DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetUserOwned() const;
        DGNDBSERVERCLIENT_EXPORT BeSQLite::BeGuid GetFileId() const;
        DGNDBSERVERCLIENT_EXPORT BeFileName GetLocalPath() const;
        DGNDBSERVERCLIENT_EXPORT bool GetIsReadOnly() const;
    };
END_BENTLEY_DGNDBSERVER_NAMESPACE
