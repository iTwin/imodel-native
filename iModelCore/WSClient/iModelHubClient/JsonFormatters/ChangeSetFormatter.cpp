/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ChangeSetFormatter.h"
#include "../Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2019
//---------------------------------------------------------------------------------------
Json::Value ChangeSetFormatter::Format
(
Dgn::DgnRevisionPtr       changeSet,
BeSQLite::BeBriefcaseId   briefcaseId,
PushChangeSetArgumentsPtr pushArguments
)
    {
    Json::Value pushChangeSetJson = Json::objectValue;
    JsonValueR instance = pushChangeSetJson[ServerSchema::Instance] = Json::objectValue;
    instance[ServerSchema::SchemaName] = ServerSchema::Schema::iModel;
    instance[ServerSchema::ClassName] = ServerSchema::Class::ChangeSet;
    instance[ServerSchema::Properties] = Json::objectValue;

    JsonValueR properties = instance[ServerSchema::Properties];
    properties[ServerSchema::Property::Id] = changeSet->GetId();
    properties[ServerSchema::Property::Description] = changeSet->GetSummary();
    uint64_t size;
    changeSet->GetRevisionChangesFile().GetFileSize(size);
    properties[ServerSchema::Property::FileSize] = Json::Value(size);
    properties[ServerSchema::Property::ParentId] = changeSet->GetParentId();
    properties[ServerSchema::Property::SeedFileId] = changeSet->GetDbGuid();
    properties[ServerSchema::Property::BriefcaseId] = briefcaseId.GetValue();
    properties[ServerSchema::Property::IsUploaded] = false;
    properties[ServerSchema::Property::ContainingChanges] = static_cast<int>(pushArguments->GetContainingChanges());

    BridgePropertiesPtr bridgeProperties = pushArguments->GetBridgeProperties();
    if (bridgeProperties.IsValid())
        BridgeProperties::FormatRelated(instance, bridgeProperties);

    return pushChangeSetJson;
    }