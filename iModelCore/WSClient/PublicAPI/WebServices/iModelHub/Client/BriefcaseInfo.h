/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Client/BaseFileInfo.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
typedef RefCountedPtr<struct BriefcaseInfo> BriefcaseInfoPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(BriefcaseInfo);
DEFINE_TASK_TYPEDEFS(BriefcaseInfoPtr, BriefcaseInfo);
DEFINE_TASK_TYPEDEFS(bvector<BriefcaseInfoPtr>, BriefcasesInfo);

//=======================================================================================
//! Information about briefcase.
//@bsiclass                                      julius.cepukenas               08/2015
//=======================================================================================
struct BriefcaseInfo : BaseFileInfo
{
    friend struct iModelAdmin;
    friend struct iModelConnection;
    friend struct Client;
private:
    BeSQLite::BeBriefcaseId m_id;
    Utf8String m_userOwned;
    Utf8String m_mergedChangeSetId;
    BeFileName m_localPath;
    bool m_isReadOnly;

    // Avoid this constructor. It has BriefcaseId only and it's not enough in most cases. It is used in iModelAdmin only.
    BriefcaseInfo(BeSQLite::BeBriefcaseId id) : m_id(id) {}
    BriefcaseInfo(RapidJsonValueCR properties, BeSQLite::BeBriefcaseId id, Utf8StringCR userOwned, Utf8StringCR mergedChangeSetId, bool isReadOnly)
        : BaseFileInfo(properties), m_id(id), m_userOwned(userOwned), m_mergedChangeSetId(mergedChangeSetId), m_isReadOnly(isReadOnly){}

    static BriefcaseInfoPtr Parse(WebServices::WSObjectsReader::Instance instance);
    static BriefcaseInfoPtr ParseRapidJson(RapidJsonValueCR json);
    bool operator==(BriefcaseInfoCR briefcase) const {return briefcase.GetId() == GetId();}
    void SetLocalPath(BeFileName localPath) {m_localPath = localPath;}
public:
    BeSQLite::BeBriefcaseId GetId() const {return m_id;}
    Utf8StringCR GetUserOwned() const {return m_userOwned;}
    Utf8StringCR GetMergedChangeSetId() const {return m_mergedChangeSetId;}
    BeFileName GetLocalPath() const {return m_localPath;}
    bool GetIsReadOnly() const {return m_isReadOnly;}
};
END_BENTLEY_IMODELHUB_NAMESPACE
