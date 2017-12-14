/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/StatisticsInfo.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "WebServices/iModelHub/Client/StatisticsInfo.h"
#include "Utils.h"
#include "Logging.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_SQLITE

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis               11/2017
//---------------------------------------------------------------------------------------
StatisticsInfoPtr StatisticsInfo::ParseFromRelated(WSObjectsReader::Instance instance)
    {
    auto relationshipInstances = instance.GetRelationshipInstances();

    if (0 == relationshipInstances.Size())
        return nullptr;

    for (auto relationshipInstance : relationshipInstances)
        {
        auto relatedInstance = relationshipInstance.GetRelatedInstance();
        if (!relatedInstance.IsValid())
            continue;

        if (relatedInstance.GetObjectId().className != ServerSchema::Class::Statistics)
            continue;

        RapidJsonValueCR properties = relatedInstance.GetProperties();
        auto briefcaseCount = properties.HasMember(ServerSchema::Property::BriefcasesCount) ? properties[ServerSchema::Property::BriefcasesCount].GetInt() : -1;
        auto locksCount = properties.HasMember(ServerSchema::Property::OwnedLocksCount) ? properties[ServerSchema::Property::OwnedLocksCount].GetInt() : -1;
        auto changeSetsCount = properties.HasMember(ServerSchema::Property::PushedChangeSetsCount) ? properties[ServerSchema::Property::PushedChangeSetsCount].GetInt() : -1;

        DateTime lastChangeSetPushDate = DateTime();
        if (properties.HasMember(ServerSchema::Property::LastChangeSetPushDate))
            {
            Utf8String stringValue = properties[ServerSchema::Property::LastChangeSetPushDate].GetString();
            if (SUCCESS != DateTime::FromString(lastChangeSetPushDate, stringValue.c_str()))
                {
                LogHelper::Log(SEVERITY::LOG_WARNING, "StatisticsInfo::ParseFromRelated",  "Failed to parse to DateTime from %s.", stringValue.c_str());
                return nullptr;
                }
            }

        return new StatisticsInfo(briefcaseCount, locksCount, changeSetsCount, lastChangeSetPushDate, UserInfo::Parse(instance));
        }

    return nullptr;
    }