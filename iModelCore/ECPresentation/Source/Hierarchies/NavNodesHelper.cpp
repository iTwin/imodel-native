/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentationManager.h>
#include "NavNodesHelper.h"
#include "../Shared/ExtendedData.h"
#include "../Shared/ValueHelpers.h"

// Member names of the serialized NavNode JSON object
#define NAVNODE_NodeId              "NodeId"
#define NAVNODE_ParentNodeId        "ParentNodeId"
#define NAVNODE_Key                 "Key"
#define NAVNODE_InstanceId          "ECInstanceId"
#define NAVNODE_ImageId             "ImageId"
#define NAVNODE_ForeColor           "ForeColor"
#define NAVNODE_BackColor           "BackColor"
#define NAVNODE_FontStyle           "FontStyle"
#define NAVNODE_Type                "Type"
#define NAVNODE_HasChildren         "HasChildren"
#define NAVNODE_IsChecked           "IsChecked"
#define NAVNODE_IsCheckboxVisible   "IsCheckboxVisible"
#define NAVNODE_IsCheckboxEnabled   "IsCheckboxEnabled"
#define NAVNODE_AutoExpand          "AutoExpand"
#define NAVNODE_Description         "Description"
#define NAVNODE_InternalData        "InternalData"
#define NAVNODE_UsersExtendedData   "ExtendedData"
#define NAVNODE_LabelDefinition     "LabelDefinition"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document NavNodesHelper::SerializeNodeToJson(NavNodeCR node)
    {
    rapidjson::Document json;
    json.SetObject();
    if (node.m_internalExtendedData.IsObject() && node.m_internalExtendedData.MemberCount() > 0)
        {
        rapidjson::Document internalExtendedData(&json.GetAllocator());
        internalExtendedData.CopyFrom(node.m_internalExtendedData, internalExtendedData.GetAllocator());
        json.AddMember(NAVNODE_InternalData, internalExtendedData, json.GetAllocator());
        }
    if (!node.m_description.empty())
        json.AddMember(NAVNODE_Description, rapidjson::Value(node.m_description.c_str(), json.GetAllocator()), json.GetAllocator());
    if (!node.m_imageId.empty())
        json.AddMember(NAVNODE_ImageId, rapidjson::Value(node.m_imageId.c_str(), json.GetAllocator()), json.GetAllocator());
    if (!node.m_type.empty())
        json.AddMember(NAVNODE_Type, rapidjson::Value(node.m_type.c_str(), json.GetAllocator()), json.GetAllocator());
    if (node.m_shouldAutoExpand)
        json.AddMember(NAVNODE_AutoExpand, node.m_shouldAutoExpand, json.GetAllocator());
    if (node.m_labelDefinition.IsValid())
        json.AddMember(NAVNODE_LabelDefinition, node.m_labelDefinition->ToInternalJson(&json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NavNodesHelper::DeserializeNodeFromJson(RapidJsonValueCR json)
    {
    auto node = NavNode::Create();

    if (json.HasMember(NAVNODE_InternalData))
        node->m_internalExtendedData.CopyFrom(json[NAVNODE_InternalData], node->m_allocator);
    node->m_shouldAutoExpand = json.HasMember(NAVNODE_AutoExpand) ? json[NAVNODE_AutoExpand].GetBool() : false;

    if (json.HasMember(NAVNODE_Description))
        node->m_description = json[NAVNODE_Description].GetString();
    if (json.HasMember(NAVNODE_ImageId))
        node->m_imageId = json[NAVNODE_ImageId].GetString();
    if (json.HasMember(NAVNODE_Type))
        node->m_type = json[NAVNODE_Type].GetString();
    if (json.HasMember(NAVNODE_LabelDefinition))
        node->m_labelDefinition = LabelDefinition::FromInternalJson(json[NAVNODE_LabelDefinition]);

    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NavNodesFactory::CreateECInstanceNode(IConnectionCR connection, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey, bvector<ECClassInstanceKey> const& classInstanceKeys, LabelDefinitionCR label) const
    {
    NavNodePtr node = NavNode::Create();
    node->SetType(NAVNODE_TYPE_ECInstancesNode);
    node->SetLabelDefinition(label);
    node->SetNodeKey(*ECInstancesNodeKey::Create(connection, specificationIdentifier, parentKey, classInstanceKeys));

    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(connection.GetId());
    extendedData.SetIsLabelCustomized(true);

    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NavNodesFactory::CreateECInstanceNode(IConnectionCR connection, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey, bvector<ECInstanceKey> const& instanceKeys, LabelDefinitionCR label) const
    {
    auto classInstanceKeys = ContainerHelpers::TransformContainer<bvector<ECClassInstanceKey>>(instanceKeys,
        [&connection](auto const& key){return ValueHelpers::GetECClassInstanceKey(connection.GetECDb().Schemas(), key);});
    return CreateECInstanceNode(connection, specificationIdentifier, parentKey, classInstanceKeys, label);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NavNodesFactory::CreateECInstanceNode(IConnectionCR connection, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey, ECClassId classId, ECInstanceId instanceId, LabelDefinitionCR label) const
    {
    return CreateECInstanceNode(connection, specificationIdentifier, parentKey, {ECInstanceKey(classId, instanceId)}, label);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NavNodesFactory::CreateECInstanceNode(IConnectionCR connection, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey, IECInstanceCR instance, LabelDefinitionCR label) const
    {
    ECInstanceId instanceId;
    ECInstanceId::FromString(instanceId, instance.GetInstanceId().c_str());
    return CreateECInstanceNode(connection, specificationIdentifier, parentKey, instance.GetClass().GetId(), instanceId, label);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NavNodesFactory::CreateECClassGroupingNode(IConnectionCR connection, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey, ECClassCR ecClass, bool isPolymorphic, LabelDefinitionCR label, uint64_t groupedInstancesCount) const
    {
    NavNodePtr node = NavNode::Create();
    node->SetLabelDefinition(label);
    node->SetDescription(ecClass.GetDescription().c_str());
    node->SetType(NAVNODE_TYPE_ECClassGroupingNode);
    node->SetNodeKey(*ECClassGroupingNodeKey::Create(connection, specificationIdentifier, parentKey, ecClass, isPolymorphic, groupedInstancesCount));

    NavNodeExtendedData extendedData(*node);
    extendedData.SetRequestedSpecification(true);
    extendedData.SetConnectionId(connection.GetId());
    extendedData.SetECClassId(ecClass.GetId());
    extendedData.SetIsECClassPolymorphic(isPolymorphic);
    extendedData.SetIsLabelCustomized(true);

    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NavNodesFactory::CreateECRelationshipGroupingNode(IConnectionCR connection, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey, ECRelationshipClassCR relationshipClass, LabelDefinitionCR label, uint64_t groupedInstancesCount) const
    {
    NavNodePtr node = NavNode::Create();
    node->SetLabelDefinition(label);
    node->SetDescription(relationshipClass.GetDescription().c_str());
    node->SetType(NAVNODE_TYPE_ECRelationshipGroupingNode);
    node->SetNodeKey(*NavNodeKey::Create(connection, specificationIdentifier, parentKey, NAVNODE_TYPE_ECRelationshipGroupingNode, label.GetDisplayValue()));

    NavNodeExtendedData extendedData(*node);
    extendedData.SetRequestedSpecification(true);
    extendedData.SetConnectionId(connection.GetId());
    extendedData.SetECClassId(relationshipClass.GetId());
    extendedData.SetIsLabelCustomized(true);

    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NavNodesFactory::CreateDisplayLabelGroupingNode(IConnectionCR connection, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey, LabelDefinitionCR label, uint64_t groupedInstancesCount, std::unique_ptr<bvector<ECInstanceKey>> groupedInstanceKeys) const
    {
    NavNodePtr node = NavNode::Create();
    node->SetLabelDefinition(label);
    node->SetType(NAVNODE_TYPE_DisplayLabelGroupingNode);
    node->SetNodeKey(*LabelGroupingNodeKey::Create(connection, specificationIdentifier, parentKey, label.GetDisplayValue(), groupedInstancesCount, std::move(groupedInstanceKeys)));

    NavNodeExtendedData extendedData(*node);
    extendedData.SetRequestedSpecification(true);
    extendedData.SetConnectionId(connection.GetId());
    extendedData.SetIsLabelCustomized(true);

    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NavNodesFactory::CreateECPropertyGroupingNode(IConnectionCR connection, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey, ECClassCR ecClass, ECPropertyCR ecProperty, LabelDefinitionCR label, Utf8CP imageId, RapidJsonValueCR groupingValue, bool isRangeGrouping, uint64_t groupedInstancesCount) const
    {
    NavNodePtr node = NavNode::Create();
    node->SetLabelDefinition(label);
    node->SetDescription(ecProperty.GetDescription().c_str());
    node->SetType(NAVNODE_TYPE_ECPropertyGroupingNode);
    if (imageId && *imageId)
        node->SetImageId(imageId);
    node->SetNodeKey(*ECPropertyGroupingNodeKey::Create(connection, specificationIdentifier, parentKey,
        ecClass, ecProperty.GetName(), groupingValue, groupedInstancesCount));

    NavNodeExtendedData extendedData(*node);
    extendedData.SetRequestedSpecification(true);
    extendedData.SetPropertyName(Utf8String(ecProperty.GetName().c_str()).c_str());
    extendedData.SetConnectionId(connection.GetId());
    extendedData.SetECClassId(ecClass.GetId());
    if (isRangeGrouping)
        extendedData.SetPropertyValueRangeIndexes(groupingValue);
    else
        extendedData.SetPropertyValues(groupingValue);
    extendedData.SetIsLabelCustomized(true);

    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NavNodesFactory::CreateCustomNode(IConnectionCR connection, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey, LabelDefinitionCR label, Utf8CP description, Utf8CP imageId, Utf8CP type) const
    {
    NavNodePtr node = NavNode::Create();
    node->SetLabelDefinition(label);
    node->SetDescription(description);
    node->SetType(type);
    node->SetImageId(imageId);
    node->SetNodeKey(*NavNodeKey::Create(connection, specificationIdentifier, parentKey, type, label.GetDisplayValue()));

    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(connection.GetId());

    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NavNodesFactory::CreateFromJson(IConnectionCR connection, RapidJsonValueCR json, NavNodeKeyR key) const
    {
    auto node = NavNodesHelper::DeserializeNodeFromJson(json);
    node->SetNodeKey(key);

    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(connection.GetId());

    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesHelper::AddRelatedInstanceInfo(NavNodeR node, Utf8CP serializedJson)
    {
    NavNodeExtendedData(node).SetRelatedInstanceKeys(serializedJson);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodesHelper::IsGroupingNode(NavNodeCR node)
    {
    return node.GetKey()->AsGroupingNodeKey();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodesHelper::IsCustomNode(NavNodeCR node)
    {
    return !node.GetKey()->AsECInstanceNodeKey()
        && !node.GetKey()->AsGroupingNodeKey();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String NavNodesHelper::NodeKeyHashPathToString(NavNodeKeyCR key) { return BeStringUtilities::Join(key.GetHashPath(), "/"); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> NavNodesHelper::NodeKeyHashPathFromString(Utf8CP str)
    {
    bvector<Utf8String> path;
    CharP context;
    Utf8CP token = BeStringUtilities::Strtok(const_cast<CharP>(str), "/", &context);
    while (nullptr != token)
        {
        path.push_back(token);
        token = BeStringUtilities::Strtok(nullptr, "/", &context);
        }
    return path;
    }
