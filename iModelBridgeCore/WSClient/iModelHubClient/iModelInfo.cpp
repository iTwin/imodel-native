/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/iModelInfo.h>
#include <DgnPlatform/TxnManager.h>
#include "Utils.h"
#include "Logging.h"

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
Utf8String iModelInfo::GetWSRepositoryName() const
    {
    return ServerSchema::Plugin::iModel + ("--" + m_id);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2017
//---------------------------------------------------------------------------------------
iModelResult iModelInfo::ReadFromLocalValues(Dgn::DgnDbCR db)
    {
    Utf8String serverUrl;
    Utf8String id;
    BeSQLite::DbResult status;
    status = db.QueryBriefcaseLocalValue(serverUrl, Db::Properties::iModelURL);
    if (BeSQLite::DbResult::BE_SQLITE_ROW == status)
        status = db.QueryBriefcaseLocalValue(id, Db::Properties::iModelId);
    if (BeSQLite::DbResult::BE_SQLITE_ROW == status)
        {
        return iModelResult::Success(iModelInfo::Create(serverUrl, id));
        }
    auto error = Error(db, status);
    return iModelResult::Error(error);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2017
//---------------------------------------------------------------------------------------
iModelResult iModelInfo::ReadiModelInfo(Dgn::DgnDbCR db)
    {
    const Utf8String methodName = "iModelInfo::ReadiModelInfo";
    Utf8String id;
    DbResult status = db.QueryProperty(id, iModelHubProperties::iModelId());
    if (BeSQLite::DbResult::BE_SQLITE_ROW != status)
        {
        return ReadFromLocalValues(db);
        }
    Utf8String serverUrl;
    if (BeSQLite::DbResult::BE_SQLITE_ROW == db.QueryProperty(serverUrl, iModelHubProperties::iModelURL()))
        {
        return iModelResult::Success(iModelInfo::Create(serverUrl, id));
        }
    auto error = Error(db, status);
    return iModelResult::Error(error);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
BeSQLite::DbResult iModelInfo::WriteiModelProperties(Dgn::DgnDbR db) const
    {
    BeSQLite::DbResult status = db.SavePropertyString(iModelHubProperties::iModelURL(), GetServerURL());
    if (BeSQLite::DbResult::BE_SQLITE_OK == status)
        status = db.SavePropertyString(iModelHubProperties::iModelId(), GetId());
    return status;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
StatusResult iModelInfo::WriteiModelInfo(Dgn::DgnDbR db, BeSQLite::BeBriefcaseId const& briefcaseId) const
    {
    const Utf8String methodName = "iModelInfo::WriteiModelInfo";
    BeSQLite::DbResult status;
    status = db.SetAsBriefcase(briefcaseId);

    //Write the iModelInfo properties to the file
    if (BeSQLite::DbResult::BE_SQLITE_OK == status)
        status = WriteiModelProperties(db);

    if (BeSQLite::DbResult::BE_SQLITE_OK == status)
        return StatusResult::Success();

    auto error = Error(db, status);
    LogHelper::Log(SEVERITY::LOG_ERROR, methodName, error.GetMessage().c_str());
    return StatusResult::Error(error);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
iModelInfoPtr iModelInfo::Parse(RapidJsonValueCR properties, Utf8StringCR iModelInstanceId, UserInfoPtr ownerInfo, Utf8StringCR url)
    {
    Utf8String name = properties[ServerSchema::Property::iModelName].GetString();
    Utf8String description = (properties.HasMember(ServerSchema::Property::iModelDescription) && properties[ServerSchema::Property::iModelDescription].IsString()) ?
        properties[ServerSchema::Property::iModelDescription].GetString() : "";
    Utf8String userUploaded = (properties.HasMember(ServerSchema::Property::UserCreated) && properties[ServerSchema::Property::UserCreated].IsString()) ?
        properties[ServerSchema::Property::UserCreated].GetString() : "";
    DateTime createdDate;
    Utf8String dateStr = (properties.HasMember(ServerSchema::Property::CreatedDate) && properties[ServerSchema::Property::CreatedDate].IsString()) ?
        properties[ServerSchema::Property::CreatedDate].GetString() : "";
    if (!dateStr.empty())
        DateTime::FromString(createdDate, dateStr.c_str());
    bool isInitialized = (properties.HasMember(ServerSchema::Property::Initialized) && properties[ServerSchema::Property::Initialized].IsBool()) ?
        properties[ServerSchema::Property::Initialized].GetBool() : false;
    return new iModelInfo(url, iModelInstanceId, name, description, userUploaded, createdDate, ownerInfo, isInitialized);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2016
//---------------------------------------------------------------------------------------
iModelInfoPtr iModelInfo::Parse(WSObjectsReader::Instance instance, Utf8StringCR url)
    {
    Utf8String iModelInstanceId = instance.GetObjectId().remoteId;
    RapidJsonValueCR properties = instance.GetProperties();
    return Parse(properties, iModelInstanceId, UserInfo::ParseFromRelated(&instance), url);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Paulius.Valiunas               08/2017
//---------------------------------------------------------------------------------------
void iModelInfo::AddHasCreatorInfoSelect(Utf8StringR selectString)
    {
    selectString.Sprintf("%s,%s-forward-%s.*", selectString.c_str(), ServerSchema::Relationship::HasCreatorInfo, ServerSchema::Class::UserInfo);
    }