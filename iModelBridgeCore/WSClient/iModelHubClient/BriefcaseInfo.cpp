/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/BriefcaseInfo.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/BriefcaseInfo.h>
#include <DgnPlatform/TxnManager.h>
#include "Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_SQLITE

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
BriefcaseInfoPtr BriefcaseInfo::ParseRapidJson(RapidJsonValueCR json)
    {
    BeBriefcaseId id = BeBriefcaseId(json[ServerSchema::Property::BriefcaseId].GetUint());
    Utf8String userOwned = json.HasMember(ServerSchema::Property::UserOwned) ? json[ServerSchema::Property::UserOwned].GetString() : "";
    bool isReadOnly = json[ServerSchema::Property::IsReadOnly].GetBool();
    Utf8String mergedChangeSetId = json.HasMember(ServerSchema::Property::MergedChangeSetId) ? json[ServerSchema::Property::MergedChangeSetId].GetString() : "";

    return new BriefcaseInfo(json, id, userOwned, mergedChangeSetId, isReadOnly);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
BriefcaseInfoPtr BriefcaseInfo::Parse(WSObjectsReader::Instance instance)
    {
    RapidJsonValueCR properties = instance.GetProperties();
    return ParseRapidJson(properties);
    }
