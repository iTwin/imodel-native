/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/FileInfo.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/FileInfo.h>
#include <DgnPlatform/TxnManager.h>
#include "Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
FileInfo::FileInfo(Dgn::DgnDbCR db, Utf8StringCR description) : m_description(description), m_areFileDetailsAvailable(false)
    {
    BeFileName fileName = db.GetFileName();
    BeStringUtilities::WCharToUtf8(m_fileName, BeFileName::GetFileNameAndExtension(fileName).c_str());
    m_fileId = db.GetDbGuid();
    m_mergedChangeSetId = db.Revisions().GetParentRevisionId();
    fileName.GetFileSize(m_fileSize);
    m_index = -1;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
FileInfo::FileInfo(int32_t index, Utf8StringCR fileName, Utf8StringCR fileId, Utf8StringCR mergedChangeSetId,
    Utf8StringCR description, uint64_t size, Utf8StringCR user, DateTimeCR date) :
    m_index(index), m_fileName(fileName), m_mergedChangeSetId(mergedChangeSetId), m_description(description),
    m_fileSize(size), m_userUploaded(user), m_uploadedDate(date), m_areFileDetailsAvailable(false)
    {
    m_fileId.FromString(fileId.c_str());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             10/2016
//---------------------------------------------------------------------------------------
Utf8String GetProperty(RapidJsonValueCR properties, Utf8StringCR member)
    {
    if (properties.HasMember(member.c_str()))
        {
        if (properties[member.c_str()].IsString())
            {
            return properties[member.c_str()].GetString();
            }
        }
    return "";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
FileInfoPtr FileInfo::Parse(RapidJsonValueCR properties, Utf8StringCR instanceId, FileInfoCR fileInfo)
    {
    FileInfoPtr info = new FileInfo(fileInfo);

    info->m_index = -1;
    if (properties.HasMember(ServerSchema::Property::Index))
        {
        if (properties[ServerSchema::Property::Index].IsInt())
            info->m_index = properties[ServerSchema::Property::Index].GetInt();
        }

    Utf8String fileName = GetProperty(properties, ServerSchema::Property::FileName);

    if (!fileName.empty())
        info->m_fileName = fileName;

    Utf8String fileId = GetProperty(properties, ServerSchema::Property::FileId);

    if (!fileId.empty())
        info->m_fileId.FromString(fileId.c_str());

    Utf8String mergedChangeSetId = GetProperty(properties, ServerSchema::Property::MergedChangeSetId);

    if (!mergedChangeSetId.empty())
        info->m_mergedChangeSetId = mergedChangeSetId;

    Utf8String description = GetProperty(properties, ServerSchema::Property::FileDescription);

    if (!description.empty())
        info->m_description = description;

    Utf8String sizeString = GetProperty(properties, ServerSchema::Property::FileSize);

    if (!sizeString.empty())
        {
        uint64_t size;
        BeStringUtilities::ParseUInt64(size, sizeString.c_str());
        if (size > 0)
            {
            info->m_fileSize = size;
            info->m_areFileDetailsAvailable = true;
            }
        }

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
    return Parse(instance.GetProperties(), instance.GetObjectId().remoteId, fileInfo);;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
void FileInfo::ToPropertiesJson(JsonValueR json) const
    {
    if (0 <= GetIndex())
        json[ServerSchema::Property::Index] = GetIndex();
    json[ServerSchema::Property::FileId] = GetFileId().ToString();
    json[ServerSchema::Property::MergedChangeSetId] = GetMergedChangeSetId();
    if (m_description.size() > 0)
        json[ServerSchema::Property::FileDescription] = m_description;
    if (m_fileName.size() > 0)
        json[ServerSchema::Property::FileName] = GetFileName();
    if (m_fileSize > 0)
        json[ServerSchema::Property::FileSize] = GetSize();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
WebServices::ObjectId FileInfo::GetObjectId() const
    {
    return WebServices::ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::File, GetFileId().ToString());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
FileAccessKeyPtr FileAccessKey::ParseFromRelated(JsonValueCR json)
    {
    if (!json.isMember(ServerSchema::RelationshipInstances))
        return nullptr;

    auto relationships = json[ServerSchema::RelationshipInstances];
    for (auto relationshipInstance : relationships)
        {
        if (!relationshipInstance.isMember(ServerSchema::RelatedInstance))
            continue;

        auto relatedInstance = relationshipInstance[ServerSchema::RelatedInstance];
        if (!relatedInstance.isMember(ServerSchema::Properties) ||
            !relatedInstance.isMember(ServerSchema::ClassName) ||
            !relatedInstance[ServerSchema::ClassName].isString() ||
            relatedInstance[ServerSchema::ClassName].asString() != ServerSchema::Class::AccessKey)
            continue;

        JsonValueCR properties = relatedInstance[ServerSchema::Properties];
        auto rapidJson = ToRapidJson(properties);

        return Parse(rapidJson);
        }
    return nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
FileAccessKeyPtr FileAccessKey::ParseFromRelated(WSObjectsReader::Instance instance)
    {
    auto relationshipInstances = instance.GetRelationshipInstances();
    if (0 == relationshipInstances.Size())
        return FileAccessKeyPtr(new NotUsedFileAccessKey());

    for (auto relationshipInstance : relationshipInstances)
        {
        auto relatedInstance = relationshipInstance.GetRelatedInstance();
        if (!relatedInstance.IsValid())
            continue;

        if (relatedInstance.GetObjectId().className != ServerSchema::Class::AccessKey)
            continue;

        return Parse(relatedInstance.GetProperties());
        }

    return FileAccessKeyPtr(new NotUsedFileAccessKey());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
Utf8String FileAccessKey::GetProperty(RapidJsonValueCR properties, Utf8StringCR member)
    {
    if (properties.HasMember(member.c_str()))
        {
        if (properties[member.c_str()].IsString())
            {
            return properties[member.c_str()].GetString();
            }
        }
    return "";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
FileAccessKeyPtr FileAccessKey::Parse(RapidJsonValueCR properties)
    {
    FileAccessKeyPtr result = new FileAccessKey();

    Utf8String downloadUrl = GetProperty(properties, ServerSchema::Property::DownloadUrl);
    if (!downloadUrl.empty())
        result->m_downloadUrl = downloadUrl;

    Utf8String uploadUrl = GetProperty(properties, ServerSchema::Property::UploadUrl);
    if (!uploadUrl.empty())
        result->m_uploadUrl = uploadUrl;

    result->m_valueSet = true;
    return result;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
void FileAccessKey::AddDownloadAccessKeySelect(Utf8StringR selectString)
    {
    selectString.Sprintf("%s,%s-forward-%s.DownloadURL", selectString.c_str(), ServerSchema::Relationship::FileAccessKey, ServerSchema::Class::AccessKey);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
void FileAccessKey::AddUploadAccessKeySelect(Utf8StringR selectString)
    {
    selectString.Sprintf("%s,%s-forward-%s.UploadURL", selectString.c_str(), ServerSchema::Relationship::FileAccessKey, ServerSchema::Class::AccessKey);
    }

