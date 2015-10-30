/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/RevisionInfo.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/RevisionInfo.h>

USING_NAMESPACE_BENTLEY_DGNDBSERVER

RevisionInfo::RevisionInfo() {}

RevisionInfo::RevisionInfo(FileInfoCR fileInfo, Utf8StringCR description, uint32_t index, Utf8StringCR revisionId, Utf8StringCR parentId, Utf8StringCR mergedFromId,
                           Utf8StringCR masterFileId, uint32_t briefcaseId, Utf8StringCR userCreated, DateTimeCR pushDate) :
    m_fileInfo(fileInfo), m_description(description), m_index(index), m_revisionId(revisionId), m_parentId(parentId), m_mergedFromId(mergedFromId),
    m_masterFileId(masterFileId), m_briefcaseId(briefcaseId), m_userCreated(userCreated), m_pushDate(pushDate)
    {}

FileInfoCR RevisionInfo::GetFileInfo() const
    {
    return m_fileInfo;
    }

Utf8StringCR RevisionInfo::GetDescription() const
    {
    return m_description;
    }

uint32_t RevisionInfo::GetIndex() const
    {
    return m_index;
    }

Utf8StringCR RevisionInfo::GetRevisionId() const
    {
    return m_revisionId;
    }

Utf8StringCR RevisionInfo::GetParentId() const
    {
    return m_parentId;
    }

Utf8StringCR RevisionInfo::GetMergedFromId() const
    {
    return m_mergedFromId;
    }

Utf8StringCR RevisionInfo::GetMasterFileId() const
    {
    return m_masterFileId;
    }

uint32_t RevisionInfo::GetBriefcaseId() const
    {
    return m_briefcaseId;
    }

Utf8StringCR RevisionInfo::GetUserCreated() const
    {
    return m_userCreated;
    }

DateTimeCR RevisionInfo::GetPushDate() const
    {
    return m_pushDate;
    }
