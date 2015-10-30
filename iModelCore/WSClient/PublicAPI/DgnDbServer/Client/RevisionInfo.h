/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/RevisionInfo.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/FileInfo.h>
#include <Bentley/DateTime.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
struct RevisionInfo
{
//__PUBLISH_SECTION_END__
private:
    FileInfo    m_fileInfo;
    Utf8String  m_description;
    uint32_t    m_index;
    Utf8String  m_revisionId;
    Utf8String  m_parentId;
    Utf8String  m_mergedFromId;
    Utf8String  m_masterFileId;
    uint32_t    m_briefcaseId;
    Utf8String  m_userCreated;
    DateTime    m_pushDate;
//__PUBLISH_SECTION_START__
public:
    RevisionInfo();
    DGNDBSERVERCLIENT_EXPORT RevisionInfo(FileInfoCR fileInfo, Utf8StringCR description, uint32_t index, Utf8StringCR revisionId, Utf8StringCR parentId, Utf8StringCR mergedFromId,
                 Utf8StringCR masterFileId, uint32_t briefcaseId, Utf8StringCR userCreated, DateTimeCR pushDate);

    DGNDBSERVERCLIENT_EXPORT FileInfoCR   GetFileInfo() const;
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetDescription() const;
    DGNDBSERVERCLIENT_EXPORT uint32_t     GetIndex() const;
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetRevisionId() const;
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetParentId() const;
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetMergedFromId() const;
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetMasterFileId() const;
    DGNDBSERVERCLIENT_EXPORT uint32_t     GetBriefcaseId() const;
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetUserCreated() const;
    DGNDBSERVERCLIENT_EXPORT DateTimeCR   GetPushDate() const;
};

typedef RevisionInfo& RevisionInfoR;
typedef const RevisionInfo& RevisionInfoCR;
END_BENTLEY_DGNDBSERVER_NAMESPACE
