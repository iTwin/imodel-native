/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/FileInfo.h>
#include <DgnPlatform/TxnManager.h>
#include "Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
FileInfo::FileInfo(Dgn::DgnDbCR db, Utf8StringCR description) : BaseFileInfo(db, description)
    {
    m_mergedChangeSetId = db.Revisions().GetParentRevisionId();
    m_index = -1;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
FileInfoPtr FileInfo::Parse(RapidJsonValueCR properties, FileInfoCR fileInfo)
    {
    FileInfoPtr info = new FileInfo(fileInfo);

    info->UpdateBaseFileInfo (properties);

    info->m_index = -1;
    if (properties.HasMember(ServerSchema::Property::Index))
        {
        if (properties[ServerSchema::Property::Index].IsInt())
            info->m_index = properties[ServerSchema::Property::Index].GetInt();
        }

    Utf8String mergedChangeSetId = GetProperty(properties, ServerSchema::Property::MergedChangeSetId);

    if (!mergedChangeSetId.empty())
        info->m_mergedChangeSetId = mergedChangeSetId;

    Utf8String userUploaded = GetProperty(properties, ServerSchema::Property::UserUploaded);

    if (!userUploaded.empty())
        info->m_userUploaded = userUploaded;

    Utf8String dateStr = GetProperty(properties, ServerSchema::Property::UploadedDate);

    if (!dateStr.empty())
        DateTime::FromString(info->m_uploadedDate, dateStr.c_str());

    info->m_initialized = properties.HasMember(ServerSchema::Property::InitializationState) ?
        (InitializationState)properties[ServerSchema::Property::InitializationState].GetInt() : InitializationState::NotStarted;

    return info;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             10/2016
//---------------------------------------------------------------------------------------
FileInfoPtr FileInfo::Parse(WSObjectsReader::Instance instance, FileInfoCR fileInfo)
    {
    return Parse(instance.GetProperties(), fileInfo);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
void FileInfo::ToPropertiesJson(JsonValueR json) const
    {
    if (0 <= GetIndex())
        json[ServerSchema::Property::Index] = GetIndex();
    json[ServerSchema::Property::MergedChangeSetId] = GetMergedChangeSetId();
    if (m_description.size() > 0)
        json[ServerSchema::Property::FileDescription] = m_description;
    if (m_fileName.size() > 0)
        json[ServerSchema::Property::FileName] = GetFileName();
    if (m_fileSize > 0)
        json[ServerSchema::Property::FileSize] = Json::Value(GetSize());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
WebServices::ObjectId FileInfo::GetObjectId() const
    {
    return WebServices::ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::File, GetFileId().ToString());
    }
