/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/FileInfo.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/FileInfo.h>
#include <DgnPlatform/TxnManager.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_WEBSERVICES
//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
FileInfo::FileInfo() : m_areFileDetailsAvailable(false)
    {
    m_index = -1;
    m_fileSize = 0;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
FileInfo::FileInfo(BeGuid fileId) : m_fileId(fileId), m_areFileDetailsAvailable(false)
    {
    m_index = -1;
    m_fileSize = 0;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
FileInfo::FileInfo(Dgn::DgnDbCR db, Utf8StringCR description) : m_description(description), m_areFileDetailsAvailable(false)
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
    Utf8StringCR description, uint64_t size, Utf8StringCR user, DateTimeCR date) :
    m_index(index), m_fileName(fileName), m_mergedRevisionId(mergedRevisionId), m_description(description),
    m_fileSize(size), m_userUploaded(user), m_uploadedDate(date), m_areFileDetailsAvailable(false)
    {
    m_fileId.FromString(fileId.c_str());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                  10/2016
//---------------------------------------------------------------------------------------
FileInfoPtr FileInfo::Create(BeGuid fileId)
    {
    return FileInfoPtr(new FileInfo(fileId));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                  10/2016
//---------------------------------------------------------------------------------------
FileInfoPtr FileInfo::Create(Dgn::DgnDbCR db, Utf8StringCR description)
    {
    return FileInfoPtr(new FileInfo(db, description));
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
DateTimeCR FileInfo::GetUploadedDate() const
    {
    return m_uploadedDate;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
bool FileInfo::GetInitialized() const
    {
    return m_initialized;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
FileInfoPtr FileInfo::Parse(RapidJsonValueCR properties, Utf8StringCR instanceId, FileInfoCR fileInfo)
    {
    auto info = std::make_shared<FileInfo>(fileInfo);

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

    Utf8String mergedRevisionId = GetProperty(properties, ServerSchema::Property::MergedRevisionId);

    if (!mergedRevisionId.empty())
        info->m_mergedRevisionId = mergedRevisionId;

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

    info->m_initialized = properties.HasMember(ServerSchema::Property::Initialized) ? properties[ServerSchema::Property::Initialized].GetBool() : false;

    return info;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             10/2016
//---------------------------------------------------------------------------------------
FileInfoPtr FileInfo::Parse(JsonValueCR json, FileInfoCR fileInfo)
    {
    JsonValueCR properties = json[ServerSchema::Properties];
    auto rapidJson = ToRapidJson(properties);

    return Parse(rapidJson, json[ServerSchema::InstanceId].asString(), fileInfo);
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
    json[ServerSchema::Property::MergedRevisionId] = GetMergedRevisionId();
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
    return WebServices::ObjectId(ServerSchema::Schema::Repository, ServerSchema::Class::File, GetFileId().ToString());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
bool FileInfo::AreFileDetailsAvailable() const
    {
    return m_areFileDetailsAvailable;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas            02/2017
//---------------------------------------------------------------------------------------
bool FileInfo::GetContainsFileAccessKey()
    {
    return m_containsFileAccessKey;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas            02/2017
//---------------------------------------------------------------------------------------
DgnDbServerFileAccessKeyPtr FileInfo::GetFileAccessKey() const
    {
    return m_fileAccessKey;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas            02/2017
//---------------------------------------------------------------------------------------
void FileInfo::SetFileAccessKey(DgnDbServerFileAccessKeyPtr fileAccessKey)
    {
    m_containsFileAccessKey = true;
    m_fileAccessKey = fileAccessKey;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
DgnDbServerFileAccessKeyPtr DgnDbServerFileAccessKey::ParseFromRelated(JsonValueCR json)
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
DgnDbServerFileAccessKeyPtr DgnDbServerFileAccessKey::ParseFromRelated(WSObjectsReader::Instance instance)
    {
    auto relationshipInstances = instance.GetRelationshipInstances();
    if (0 == relationshipInstances.Size())
        return nullptr;

    for (auto relationshipInstance : relationshipInstances)
        {
        auto relatedInstance = relationshipInstance.GetRelatedInstance();
        if (!relatedInstance.IsValid())
            continue;

        if (relatedInstance.GetObjectId().className != ServerSchema::Class::AccessKey)
            continue;

        return Parse(relatedInstance.GetProperties());
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerFileAccessKey::GetProperty(RapidJsonValueCR properties, Utf8StringCR member)
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
DgnDbServerFileAccessKeyPtr DgnDbServerFileAccessKey::Parse(RapidJsonValueCR properties)
    {
    auto result = std::make_shared<DgnDbServerFileAccessKey>();

    Utf8String downloadUrl = GetProperty(properties, ServerSchema::Property::DownloadUrl);
    if (!downloadUrl.empty())
        result->m_downloadUrl = downloadUrl;

    Utf8String uploadUrl = GetProperty(properties, ServerSchema::Property::UploadUrl);
    if (!uploadUrl.empty())
        result->m_uploadUrl = uploadUrl;

    return result;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
void DgnDbServerFileAccessKey::AddDownloadAccessKeySelect(Utf8StringR selectString)
    {
    selectString.Sprintf("%s,%s-forward-%s.DownloadURL", selectString.c_str(), ServerSchema::Relationship::FileAccessKey, ServerSchema::Class::AccessKey);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
void DgnDbServerFileAccessKey::AddUploadAccessKeySelect(Utf8StringR selectString)
    {
    selectString.Sprintf("%s,%s-forward-%s.UploadURL", selectString.c_str(), ServerSchema::Relationship::FileAccessKey, ServerSchema::Class::AccessKey);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
Utf8StringCR DgnDbServerFileAccessKey::GetDownloadUrl() const
    {
    return m_downloadUrl;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
Utf8StringCR DgnDbServerFileAccessKey::GetUploadUrl() const
    {
    return m_uploadUrl;
    }
