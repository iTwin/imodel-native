/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbServerBriefcaseInfo.h $
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
typedef RefCountedPtr<struct DgnDbServerBriefcaseInfo> DgnDbServerBriefcaseInfoPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(DgnDbServerBriefcaseInfo);
DEFINE_TASK_TYPEDEFS(DgnDbServerBriefcaseInfoPtr, DgnDbServerBriefcaseInfo);
DEFINE_TASK_TYPEDEFS(bvector<DgnDbServerBriefcaseInfoPtr>, DgnDbServerBriefcasesInfo);

//=======================================================================================
//! Information about briefcase.
//@bsiclass                                      julius.cepukenas               08/2015
//=======================================================================================
struct DgnDbServerBriefcaseInfo : RefCountedBase
{
private:
    BeSQLite::BeBriefcaseId m_id;
    BeSQLite::BeGuid m_fileId;
    Utf8String m_userOwned;
    BeFileName m_localPath;
    bool m_isReadOnly;

public:
    DgnDbServerBriefcaseInfo() {}
    DgnDbServerBriefcaseInfo(BeSQLite::BeBriefcaseId id) : m_id(id) {}
    DgnDbServerBriefcaseInfo(BeSQLite::BeBriefcaseId id, Utf8StringCR userOwned, BeSQLite::BeGuid fileId, bool isReadOnly) 
        : m_id(id), m_fileId(fileId), m_userOwned(userOwned), m_isReadOnly(isReadOnly) {}

    static DgnDbServerBriefcaseInfoPtr Parse(WSObjectsReader::Instance instance);
    //! DEPRECATED: Use Parse from Instance
    static DgnDbServerBriefcaseInfoPtr Parse(JsonValueCR json);

    bool operator==(DgnDbServerBriefcaseInfoCR briefcase) const {return briefcase.GetId() == GetId();}
    void SetLocalPath(BeFileName localPath) {m_localPath = localPath;}

    BeSQLite::BeBriefcaseId GetId() const {return m_id; }
    Utf8StringCR GetUserOwned() const {return m_userOwned;}
    BeSQLite::BeGuid GetFileId() const {return m_fileId;}
    BeFileName GetLocalPath() const {return m_localPath;}
    bool GetIsReadOnly() const {return m_isReadOnly;}
};
END_BENTLEY_DGNDBSERVER_NAMESPACE
