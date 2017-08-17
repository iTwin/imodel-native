/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Client/BriefcaseInfo.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/Result.h>
#include <WebServices/Client/Response/WSObjectsReader.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
typedef RefCountedPtr<struct BriefcaseInfo> BriefcaseInfoPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(BriefcaseInfo);
DEFINE_TASK_TYPEDEFS(BriefcaseInfoPtr, BriefcaseInfo);
DEFINE_TASK_TYPEDEFS(bvector<BriefcaseInfoPtr>, BriefcasesInfo);

//=======================================================================================
//! Information about briefcase.
//@bsiclass                                      julius.cepukenas               08/2015
//=======================================================================================
struct BriefcaseInfo : RefCountedBase
{
friend struct iModelAdmin;
friend struct iModelConnection;
friend struct Client;
private:
    BeSQLite::BeBriefcaseId m_id;
    BeSQLite::BeGuid m_fileId;
    Utf8String m_userOwned;
    BeFileName m_localPath;
    bool m_isReadOnly;

    BriefcaseInfo() {}
    BriefcaseInfo(BeSQLite::BeBriefcaseId id) : m_id(id) {}
    BriefcaseInfo(BeSQLite::BeBriefcaseId id, Utf8StringCR userOwned, BeSQLite::BeGuid fileId, bool isReadOnly)
        : m_id(id), m_fileId(fileId), m_userOwned(userOwned), m_isReadOnly(isReadOnly) {}

    static BriefcaseInfoPtr Parse(WebServices::WSObjectsReader::Instance instance);
    static BriefcaseInfoPtr ParseRapidJson(RapidJsonValueCR json);
    bool operator==(BriefcaseInfoCR briefcase) const { return briefcase.GetId() == GetId(); }
    void SetLocalPath(BeFileName localPath) { m_localPath = localPath; }
public:
    BeSQLite::BeBriefcaseId GetId() const {return m_id; }
    Utf8StringCR GetUserOwned() const {return m_userOwned;}
    BeSQLite::BeGuid GetFileId() const {return m_fileId;}
    BeFileName GetLocalPath() const {return m_localPath;}
    bool GetIsReadOnly() const {return m_isReadOnly;}
};
END_BENTLEY_IMODELHUB_NAMESPACE
