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

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis           09/2016
//---------------------------------------------------------------------------------------
bool GuidFromJson(BeGuid& guid, RapidJsonValueCR json)
    {
    Utf8String guidString = json.GetString();
    if (guidString.size() > 0)
        return BentleyStatus::SUCCESS == guid.FromString(guidString.c_str());
    else
        return false;
    }

// avoid collision of a static function with the same name in another CPP file in this compiland...
BEGIN_UNNAMED_NAMESPACE

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
BriefcaseInfoPtr ParseRapidJson(RapidJsonValueCR json)
    {
    BeSQLite::BeBriefcaseId id;
    id = BeBriefcaseId(json[ServerSchema::Property::BriefcaseId].GetUint());
    Utf8String  userOwned = json.HasMember(ServerSchema::Property::UserOwned) ? json[ServerSchema::Property::UserOwned].GetString() : "";
    BeGuid fileId;
    GuidFromJson(fileId, json[ServerSchema::Property::FileId]);
    bool isReadOnly = json[ServerSchema::Property::IsReadOnly].GetBool();

    return new BriefcaseInfo(id, userOwned, fileId, isReadOnly);
    }

END_UNNAMED_NAMESPACE

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
BriefcaseInfoPtr BriefcaseInfo::Parse(JsonValueCR json)
    {
    JsonValueCR properties      = json[ServerSchema::Properties]; 

    auto rapidJson = ToRapidJson(properties);

    return ParseRapidJson(rapidJson);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
BriefcaseInfoPtr BriefcaseInfo::Parse(WSObjectsReader::Instance instance)
    {
    RapidJsonValueCR properties = instance.GetProperties();
    return ParseRapidJson(properties);
    }
