/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/VersionInfo.h>
#include "Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_SQLITE

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite           02/2017
//---------------------------------------------------------------------------------------
Json::Value VersionInfo::GenerateJson() const
    {
    Json::Value versionJson = Json::objectValue;
    versionJson[ServerSchema::Instance] = Json::objectValue;
    versionJson[ServerSchema::Instance][ServerSchema::SchemaName] = ServerSchema::Schema::iModel;
    versionJson[ServerSchema::Instance][ServerSchema::ClassName] = ServerSchema::Class::Version;
    JsonValueR propertiesJson = versionJson[ServerSchema::Instance][ServerSchema::Properties] = Json::objectValue;

    propertiesJson[ServerSchema::Property::ChangeSetId] = m_changeSetId;
    propertiesJson[ServerSchema::Property::Name] = m_name;
    propertiesJson[ServerSchema::Property::Description] = m_description;

    return versionJson;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite           02/2017
//---------------------------------------------------------------------------------------
VersionInfoPtr VersionInfo::ParseRapidJson(RapidJsonValueCR properties, Utf8String smallThumbnailId, Utf8String largeThumbnailId)
    {
    auto id = properties[ServerSchema::Property::Id].GetString();
    auto name = properties[ServerSchema::Property::Name].GetString();
    auto changeSetId = properties[ServerSchema::Property::ChangeSetId].GetString();
    auto description = properties.HasMember(ServerSchema::Property::Description) ? properties[ServerSchema::Property::Description].GetString() : "";
    auto userCreated = properties.HasMember(ServerSchema::Property::UserCreated) ? properties[ServerSchema::Property::UserCreated].GetString() : "";
    auto createdDate = properties.HasMember(ServerSchema::Property::CreatedDate) ? 
        BeJsonUtilities::DateTimeFromValue(properties[ServerSchema::Property::CreatedDate].GetString()) : DateTime();

    return new VersionInfo(id, name, description, changeSetId, userCreated, createdDate, smallThumbnailId, largeThumbnailId);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite           02/2017
//---------------------------------------------------------------------------------------
VersionInfoPtr VersionInfo::Parse(WSObjectsReader::Instance instance)
    {
    RapidJsonValueCR properties = instance.GetProperties();
    return ParseRapidJson(properties, Thumbnail::ParseFromRelated(instance, Thumbnail::Size::Small), Thumbnail::ParseFromRelated(instance, Thumbnail::Size::Large));
    }
