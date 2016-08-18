/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/FileInfo.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/FileInfo.h>
#include <DgnPlatform/TxnManager.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
FileInfo::FileInfo()
    {
    m_index = -1;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
FileInfo::FileInfo(Dgn::DgnDbCR db, Utf8StringCR description) : m_description(description)
    {
    BeFileName fileName = db.GetFileName();
    BeStringUtilities::WCharToUtf8(m_fileName, BeFileName::GetFileNameAndExtension(fileName).c_str());
    m_fileId = db.GetDbGuid();
    m_mergedRevisionId = db.Revisions().GetParentRevisionId();
    fileName.GetFileSize(m_fileSize);
    m_index = -1;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
FileInfo::FileInfo(int32_t index, Utf8StringCR fileName, Utf8StringCR fileId, Utf8StringCR mergedRevisionId,
                   Utf8StringCR description, Utf8StringCR url, uint64_t size, Utf8StringCR user, DateTimeCR date) :
    m_index(index), m_fileName(fileName), m_mergedRevisionId(mergedRevisionId), m_description(description),
    m_fileUrl(url), m_fileSize(size), m_userUploaded(user), m_uploadedDate(date)
    {
    m_fileId.FromString(fileId.c_str());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
int32_t FileInfo::GetIndex() const
    {
    return m_index;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
Utf8StringCR FileInfo::GetFileName() const
    {
    return m_fileName;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
BeGuid FileInfo::GetFileId() const
    {
    return m_fileId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
Utf8StringCR FileInfo::GetMergedRevisionId() const
    {
    return m_mergedRevisionId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
Utf8StringCR FileInfo::GetDescription() const
    {
    return m_description;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
Utf8StringCR FileInfo::GetFileURL() const
    {
    return m_fileUrl;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
uint64_t FileInfo::GetSize() const
    {
    return m_fileSize;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
Utf8StringCR FileInfo::GetUserUploaded() const
    {
    return m_userUploaded;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
DateTimeCR FileInfo::GetUploadedDate() const
    {
    return m_uploadedDate;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
FileInfoPtr FileInfo::FromJson(JsonValueCR json, FileInfoCR fileInfo)
    {
    auto info = std::make_shared<FileInfo>(fileInfo);
    JsonValueCR properties = json[ServerSchema::Properties];

    info->m_index = -1;
    if (properties[ServerSchema::Property::Index].isInt())
        info->m_index = properties[ServerSchema::Property::Index].asInt();
    if (-1 == info->m_index)
        {
        Utf8String indexStr = json[ServerSchema::InstanceId].asString();
        uint64_t index64;
        BeStringUtilities::ParseUInt64(index64, indexStr.c_str());
        info->m_index = (int32_t) index64;
        }

    Utf8String fileName = properties[ServerSchema::Property::FileName].asString();
    if (!fileName.empty())
        info->m_fileName = fileName;

    Utf8String fileId = properties[ServerSchema::Property::FileId].asString();
    if (!fileId.empty())
        info->m_fileId.FromString(fileId.c_str());

    Utf8String mergedRevisionId = properties[ServerSchema::Property::MergedRevisionId].asString();
    if (!mergedRevisionId.empty())
        info->m_mergedRevisionId = mergedRevisionId;

    Utf8String description = properties[ServerSchema::Property::FileDescription].asString();
    if (!description.empty())
        info->m_description = description;

    Utf8String url = properties[ServerSchema::Property::URL].asString();
    if (!url.empty())
        info->m_fileUrl = url;

    if (properties[ServerSchema::Property::FileSize].isUInt())
        info->m_fileSize = properties[ServerSchema::Property::FileSize].asUInt64();

    Utf8String userUploaded = properties[ServerSchema::Property::UserUploaded].asString();
    if (!userUploaded.empty())
        info->m_userUploaded = userUploaded;

    Utf8String dateStr = properties[ServerSchema::Property::UploadedDate].asString();
    if (!dateStr.empty())
        DateTime::FromString(info->m_uploadedDate, dateStr.c_str());

    return info;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
void FileInfo::ToPropertiesJson(JsonValueR json) const
    {
    if (0 <= GetIndex())
        json[ServerSchema::Property::Index] = GetIndex();
    json[ServerSchema::Property::FileId] = GetFileId().ToString();
    json[ServerSchema::Property::FileDescription] = GetDescription();
    json[ServerSchema::Property::FileName] = GetFileName();
    json[ServerSchema::Property::FileSize] = GetSize();
    json[ServerSchema::Property::MergedRevisionId] = GetMergedRevisionId();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
WebServices::ObjectId FileInfo::GetObjectId() const
    {
    Utf8String index = "";
    if (-1 != GetIndex())
        index.Sprintf("%d", GetIndex());
    return WebServices::ObjectId(ServerSchema::Schema::Repository, ServerSchema::Class::File, index);
    }
