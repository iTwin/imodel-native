/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/BriefcaseInfo.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbServerBriefcaseInfo.h>
#include <DgnPlatform/TxnManager.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

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
DgnDbServerBriefcaseInfoPtr ParseRapidJson(RapidJsonValueCR json)
    {
    BeSQLite::BeBriefcaseId id;
    id = BeBriefcaseId(json[ServerSchema::Property::BriefcaseId].GetUint());
    Utf8String  userOwned = json.HasMember(ServerSchema::Property::UserOwned) ? json[ServerSchema::Property::UserOwned].GetString() : "";
    BeGuid fileId;
    GuidFromJson(fileId, json[ServerSchema::Property::FileId]);
    bool isReadOnly = json[ServerSchema::Property::IsReadOnly].GetBool();

    return new DgnDbServerBriefcaseInfo(id, userOwned, fileId, isReadOnly);
    }

END_UNNAMED_NAMESPACE

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerBriefcaseInfoPtr DgnDbServerBriefcaseInfo::Parse(JsonValueCR json)
    {
    JsonValueCR properties      = json[ServerSchema::Properties]; 

    auto rapidJson = ToRapidJson(properties);

    return ParseRapidJson(rapidJson);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
DgnDbServerBriefcaseInfoPtr DgnDbServerBriefcaseInfo::Parse(WSObjectsReader::Instance instance)
    {
    RapidJsonValueCR properties = instance.GetProperties();
    return ParseRapidJson(properties);
    }
