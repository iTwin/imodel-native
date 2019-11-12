/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/BridgeProperties.h>
#include "Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2019
//---------------------------------------------------------------------------------------
BridgePropertiesPtr BridgeProperties::ParseFromRelated(WSObjectsReader::Instance instance)
    {
    WSObjectsReader::Instance relatedInstance(nullptr);
    if (!TryGetRelatedInstance(relatedInstance, instance, ServerSchema::Class::BridgeProperties))
        return nullptr;

    return Parse(relatedInstance.GetProperties());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2019
//---------------------------------------------------------------------------------------
BridgePropertiesPtr BridgeProperties::Parse(RapidJsonValueCR properties)
    {
    BridgePropertiesPtr result = new BridgeProperties();

    BeSQLite::BeGuid jobId;
    if (TryParseGuidProperty(jobId, properties, ServerSchema::Property::JobId))
        result->m_jobId = jobId;

    bvector<Utf8String> users;
    if (TryParseStringArrayProperty(users, properties, ServerSchema::Property::Users))
        result->m_users = users;

    bvector<Utf8String> changedFiles;
    if (TryParseStringArrayProperty(changedFiles, properties, ServerSchema::Property::ChangedFiles))
        result->m_changedFiles = changedFiles;

    return result;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2019
//---------------------------------------------------------------------------------------
bool BridgeProperties::IsVectorValid(bvector<Utf8String> vector) const
    {
    if (vector.size() > 100)
        return false;

    for(auto vectorItem : vector)
        if (vectorItem.length() > 255)
            return false;

    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2019
//---------------------------------------------------------------------------------------
StatusResult BridgeProperties::Validate() const
    {
    if (!IsVectorValid(m_users))
        return StatusResult::Error(Error::Id::InvalidBridgePropertiesUsersValue);

    if (!IsVectorValid(m_changedFiles))
        return StatusResult::Error(Error::Id::InvalidBridgePropertiesChangedFilesValue);

    return StatusResult::Success();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2019
//---------------------------------------------------------------------------------------
void BridgeProperties::FormatRelated
(
JsonValueR          mainInstance,
BridgePropertiesPtr bridgeProperties
)
    {
    if (bridgeProperties->IsEmpty())
        return;

    JsonValueR relationshipInstances = mainInstance[ServerSchema::RelationshipInstances] = Json::arrayValue;
    JsonValueR relationshipInstance = relationshipInstances[0] = Json::objectValue;
    relationshipInstance[ServerSchema::SchemaName] = ServerSchema::Schema::iModel;
    relationshipInstance[ServerSchema::ClassName] = ServerSchema::Relationship::HasBridgeProperties;
    relationshipInstance[ServerSchema::Direction] = ServerSchema::RelationshipDirection::Forward;

    JsonValueR relatedInstance = relationshipInstance[ServerSchema::RelatedInstance] = Json::objectValue;
    relatedInstance[ServerSchema::SchemaName] = ServerSchema::Schema::iModel;
    relatedInstance[ServerSchema::ClassName] = ServerSchema::Class::BridgeProperties;

    JsonValueR relatedInstanceProperties = relatedInstance[ServerSchema::Properties] = Json::objectValue;

    if (bridgeProperties->GetJobId().IsValid())
        relatedInstanceProperties[ServerSchema::Property::JobId] = bridgeProperties->GetJobId().ToString().c_str();

    FillArrayPropertyJson(relatedInstanceProperties, ServerSchema::Property::ChangedFiles, bridgeProperties->GetChangedFiles());
    FillArrayPropertyJson(relatedInstanceProperties, ServerSchema::Property::Users, bridgeProperties->GetUsers());
    }