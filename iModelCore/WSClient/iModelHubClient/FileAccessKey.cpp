/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/FileAccessKey.h>
#include "Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB

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

    return result;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
void FileAccessKey::AddDownloadAccessKeySelect(Utf8StringR selectString)
    {
    selectString.Sprintf("%s,%s-forward-%s.DownloadURL", selectString.c_str(), ServerSchema::Relationship::FileAccessKey, 
                         ServerSchema::Class::AccessKey);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
void FileAccessKey::AddUploadAccessKeySelect(Utf8StringR selectString)
    {
    selectString.Sprintf("%s,%s-forward-%s.UploadURL", selectString.c_str(), ServerSchema::Relationship::FileAccessKey, 
                         ServerSchema::Class::AccessKey);
    }