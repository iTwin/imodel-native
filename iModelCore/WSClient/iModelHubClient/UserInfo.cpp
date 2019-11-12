/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/UserInfo.h>
//#include <DgnPlatform/TxnManager.h>
#include "Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_SQLITE

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               07/2017
//---------------------------------------------------------------------------------------
UserInfoPtr UserInfo::ParseRapidJson(RapidJsonValueCR json)
    {
    auto id = json.HasMember(ServerSchema::Property::Id) ? json[ServerSchema::Property::Id].GetString() : "";
    auto name = json.HasMember(ServerSchema::Property::Name) ? json[ServerSchema::Property::Name].GetString() : "";
    auto surname = json.HasMember(ServerSchema::Property::Surname) ? json[ServerSchema::Property::Surname].GetString() : "";
    auto email = json.HasMember(ServerSchema::Property::Email) ? json[ServerSchema::Property::Email].GetString() : "";

    return new UserInfo(id, name, surname, email);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               07/2017
//---------------------------------------------------------------------------------------
UserInfoPtr UserInfo::Parse(WSObjectsReader::Instance instance)
    {
    RapidJsonValueCR properties = instance.GetProperties();
    return ParseRapidJson(properties);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Paulius.Valiunas               08/2017
//---------------------------------------------------------------------------------------
UserInfoPtr UserInfo::ParseFromRelated(WSObjectsReader::Instance *instance)
    {
    auto relationshipInstance = *instance->GetRelationshipInstances().begin();

    auto relatedInstance = relationshipInstance.GetRelatedInstance();
    if (!relatedInstance.IsValid())
        return nullptr;

    if (relatedInstance.GetObjectId().className != ServerSchema::Class::UserInfo)
        return nullptr;

    return Parse(relatedInstance);
    }
