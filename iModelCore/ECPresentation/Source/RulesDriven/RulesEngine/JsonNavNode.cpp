/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "JsonNavNode.h"
#include "ExtendedData.h"
#include "ImageHelper.h"
#include "ECSchemaHelper.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Pranciskus.Ambrazas            06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNode::JsonNavNode()
    : m_allocator(NAVNODE_JSON_CHUNK_SIZE), m_internalExtendedData(&m_allocator)
    {
    m_internalExtendedData.SetObject();
    m_usersExtendedData = nullptr;
    m_parentNodeId = 0;
    m_instanceId = 0;
    m_nodeId = 0;
    m_determinedChildren = false;
    m_hasChildren = false;
    m_isSelectable = true;
    m_isEditable = false;
    m_isChecked = false;
    m_isCheckboxVisible = false;
    m_isCheckboxEnabled = false;
    m_isExpanded = false;
    m_fontStyle = "Regular";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNode::JsonNavNode(JsonNavNode const& other)
    : m_allocator(NAVNODE_JSON_CHUNK_SIZE), m_internalExtendedData(&m_allocator)
    {
    m_internalExtendedData.CopyFrom(other.m_internalExtendedData, m_allocator);
    if (other.m_usersExtendedData != nullptr)
        InitUsersExtendedData(other.m_usersExtendedData);
    else
        m_usersExtendedData = nullptr;
    m_parentNodeId = other.m_parentNodeId;
    m_instanceId = other.m_instanceId;
    m_nodeId = other.m_nodeId;
    m_nodeKey = other.m_nodeKey;
    m_label = other.m_label;
    m_description = other.m_description;
    m_imageId = other.m_imageId;
    m_foreColor = other.m_foreColor;
    m_backColor = other.m_backColor;
    m_fontStyle = other.m_fontStyle;
    m_type = other.m_type;
    m_determinedChildren = other.m_determinedChildren;
    m_hasChildren = other.m_hasChildren;
    m_isSelectable = other.m_isSelectable;
    m_isEditable = other.m_isEditable;
    m_isChecked = other.m_isChecked;
    m_isCheckboxVisible = other.m_isCheckboxVisible;
    m_isCheckboxEnabled = other.m_isCheckboxEnabled;
    m_isExpanded = other.m_isExpanded;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNode::~JsonNavNode()
    {
    DELETE_AND_CLEAR(m_usersExtendedData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::InitUsersExtendedData(rapidjson::Value const* source)
    {
    m_usersExtendedData = new rapidjson::Document(&m_allocator);
    if (source)
        m_usersExtendedData->CopyFrom(*source, m_allocator);
    else
        m_usersExtendedData->SetObject();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::AddUsersExtendedData(Utf8CP key, ECValueCR value)
    {
    if (nullptr == m_usersExtendedData)
        InitUsersExtendedData();
    m_usersExtendedData->AddMember(rapidjson::Value(key, m_allocator), ValueHelpers::GetJsonFromECValue(value, &m_allocator), m_allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document JsonNavNode::GetJson() const
    {
    rapidjson::Document json;
    json.SetObject();
    if (m_internalExtendedData.IsObject() && m_internalExtendedData.MemberCount() > 0)
        {
        rapidjson::Document internalExtendedData(&json.GetAllocator());
        internalExtendedData.CopyFrom(m_internalExtendedData, internalExtendedData.GetAllocator());
        json.AddMember(NAVNODE_InternalData, internalExtendedData, json.GetAllocator());
        }
    if (nullptr != m_usersExtendedData && m_usersExtendedData->IsObject() && m_usersExtendedData->MemberCount() > 0)
        {
        rapidjson::Document usersExtendedData(&json.GetAllocator());
        usersExtendedData.CopyFrom(*m_usersExtendedData, usersExtendedData.GetAllocator());
        json.AddMember(NAVNODE_UsersExtendedData, usersExtendedData, json.GetAllocator());
        }
    json.AddMember(NAVNODE_NodeId, m_nodeId, json.GetAllocator());
    json.AddMember(NAVNODE_ParentNodeId, m_parentNodeId, json.GetAllocator());
    json.AddMember(NAVNODE_InstanceId, m_instanceId, json.GetAllocator());
    if (!m_label.empty())
        json.AddMember(NAVNODE_Label, rapidjson::Value(m_label.c_str(), json.GetAllocator()), json.GetAllocator());
    if (!m_description.empty())
        json.AddMember(NAVNODE_Description, rapidjson::Value(m_description.c_str(), json.GetAllocator()), json.GetAllocator());
    if (!m_imageId.empty())
        json.AddMember(NAVNODE_CollapsedImageId, rapidjson::Value(m_imageId.c_str(), json.GetAllocator()), json.GetAllocator());
    if (!m_foreColor.empty())
        json.AddMember(NAVNODE_ForeColor, rapidjson::Value(m_foreColor.c_str(), json.GetAllocator()), json.GetAllocator());
    if (!m_backColor.empty())
        json.AddMember(NAVNODE_BackColor, rapidjson::Value(m_backColor.c_str(), json.GetAllocator()), json.GetAllocator());
    if (!m_fontStyle.empty() && !m_fontStyle.EqualsI("Regular"))
        json.AddMember(NAVNODE_FontStyle, rapidjson::Value(m_fontStyle.c_str(), json.GetAllocator()), json.GetAllocator());
    if (!m_type.empty())
        json.AddMember(NAVNODE_Type, rapidjson::Value(m_type.c_str(), json.GetAllocator()), json.GetAllocator());
    if (m_determinedChildren)
        json.AddMember(NAVNODE_HasChildren, m_hasChildren, json.GetAllocator());
    if (!m_isSelectable)
        json.AddMember(NAVNODE_IsSelectable, m_isSelectable, json.GetAllocator());
    if (m_isEditable)
        json.AddMember(NAVNODE_IsEditable, m_isEditable, json.GetAllocator());
    if (m_isChecked)
        json.AddMember(NAVNODE_IsChecked, m_isChecked, json.GetAllocator());
    if (m_isCheckboxEnabled)
        json.AddMember(NAVNODE_IsCheckboxEnabled, m_isCheckboxEnabled, json.GetAllocator());
    if (m_isCheckboxVisible)
        json.AddMember(NAVNODE_IsCheckboxVisible, m_isCheckboxVisible, json.GetAllocator());
    if (m_isExpanded)
        json.AddMember(NAVNODE_IsExpanded, m_isExpanded, json.GetAllocator());    
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::InitFromJson(RapidJsonValueCR json)
    {
    if (json.HasMember(NAVNODE_InternalData))
        m_internalExtendedData.CopyFrom(json[NAVNODE_InternalData], m_allocator);
    if (json.HasMember(NAVNODE_UsersExtendedData))
        InitUsersExtendedData(&json[NAVNODE_UsersExtendedData]);

    m_nodeId = json.HasMember(NAVNODE_NodeId) ? json[NAVNODE_NodeId].GetUint64() : 0;
    m_parentNodeId = json.HasMember(NAVNODE_ParentNodeId) ? json[NAVNODE_ParentNodeId].GetUint64() : 0;
    m_instanceId = json.HasMember(NAVNODE_InstanceId) ? json[NAVNODE_InstanceId].GetUint64() : 0;
    if (json.HasMember(NAVNODE_HasChildren))
        {
        m_determinedChildren = true;
        m_hasChildren = json[NAVNODE_HasChildren].GetBool();
        }
    m_isSelectable = json.HasMember(NAVNODE_IsSelectable) ? json[NAVNODE_IsSelectable].GetBool() : true;
    m_isEditable = json.HasMember(NAVNODE_IsEditable) ? json[NAVNODE_IsEditable].GetBool() : false;
    m_isChecked = json.HasMember(NAVNODE_IsChecked) ? json[NAVNODE_IsChecked].GetBool() : false;
    m_isCheckboxVisible = json.HasMember(NAVNODE_IsCheckboxVisible) ? json[NAVNODE_IsCheckboxVisible].GetBool() : false;
    m_isCheckboxEnabled = json.HasMember(NAVNODE_IsCheckboxEnabled) ? json[NAVNODE_IsCheckboxEnabled].GetBool() : false;
    m_isExpanded = json.HasMember(NAVNODE_IsExpanded) ? json[NAVNODE_IsExpanded].GetBool() : false;

    if (json.HasMember(NAVNODE_Label))
        m_label = json[NAVNODE_Label].GetString();
    if (json.HasMember(NAVNODE_Description))
        m_description = json[NAVNODE_Description].GetString();
    if (json.HasMember(NAVNODE_CollapsedImageId))
        m_imageId = json[NAVNODE_CollapsedImageId].GetString();
    if (json.HasMember(NAVNODE_ForeColor))
        m_foreColor = json[NAVNODE_ForeColor].GetString();
    if (json.HasMember(NAVNODE_BackColor))
        m_backColor = json[NAVNODE_BackColor].GetString();
    if (json.HasMember(NAVNODE_FontStyle))
        m_fontStyle = json[NAVNODE_FontStyle].GetString();
    if (json.HasMember(NAVNODE_Type))
        m_type = json[NAVNODE_Type].GetString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodePtr JsonNavNodesFactory::CreateECInstanceNode(IConnectionCR connection, Utf8StringCR locale, ECClassId classId, ECInstanceId instanceId, Utf8CP label) const
    {
    JsonNavNodePtr node = JsonNavNode::Create();
    InitECInstanceNode(*node, connection, locale, classId, instanceId, label);
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodePtr JsonNavNodesFactory::CreateECInstanceNode(Utf8StringCR connectionId, Utf8StringCR locale, IECInstanceCR instance, Utf8CP label) const
    {
    JsonNavNodePtr node = JsonNavNode::Create();
    InitECInstanceNode(*node, connectionId, locale, instance, label);
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodePtr JsonNavNodesFactory::CreateECClassGroupingNode(Utf8StringCR connectionId, Utf8StringCR locale, ECClassCR ecClass, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const
    {
    JsonNavNodePtr node = JsonNavNode::Create();
    InitECClassGroupingNode(*node, connectionId, locale, ecClass, label, groupedInstanceKeys);
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodePtr JsonNavNodesFactory::CreateECRelationshipGroupingNode(Utf8StringCR connectionId, Utf8StringCR locale, ECRelationshipClassCR relationshipClass, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const
    {
    JsonNavNodePtr node = JsonNavNode::Create();
    InitECRelationshipGroupingNode(*node, connectionId, locale, relationshipClass, label, groupedInstanceKeys);
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodePtr JsonNavNodesFactory::CreateDisplayLabelGroupingNode(Utf8StringCR connectionId, Utf8StringCR locale, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const
    {
    JsonNavNodePtr node = JsonNavNode::Create();
    InitDisplayLabelGroupingNode(*node, connectionId, locale, label, groupedInstanceKeys);
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodePtr JsonNavNodesFactory::CreateECPropertyGroupingNode(Utf8StringCR connectionId, Utf8StringCR locale, ECClassCR ecClass, ECPropertyCR ecProperty, Utf8CP label, Utf8CP imageId, RapidJsonValueCR groupingValue, bool isRangeGrouping, GroupedInstanceKeysListCR groupedInstanceKeys) const
    {
    JsonNavNodePtr node = JsonNavNode::Create();
    InitECPropertyGroupingNode(*node, connectionId, locale, ecClass, ecProperty, label, imageId, groupingValue, isRangeGrouping, groupedInstanceKeys);
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodePtr JsonNavNodesFactory::CreateCustomNode(Utf8StringCR connectionId, Utf8StringCR locale, Utf8CP label, Utf8CP description, Utf8CP imageId, Utf8CP type) const
    {
    JsonNavNodePtr node = JsonNavNode::Create();
    InitCustomNode(*node, connectionId, locale, label, description, imageId, type);
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodePtr JsonNavNodesFactory::CreateFromJson(IConnectionCR connection, RapidJsonValueCR json) const
    {
    JsonNavNodePtr node = JsonNavNode::Create();
    InitFromJson(*node, connection, json);
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNodesFactory::InitECInstanceNode(JsonNavNodeR node, IConnectionCR connection, Utf8StringCR locale, ECClassId classId, ECInstanceId instanceId, Utf8CP label) const
    {
    ECClassCP ecClass = nullptr;
    if (nullptr == (ecClass = connection.GetECDb().Schemas().GetClass(classId)))
        {
        BeAssert(false);
        return;
        }

    node.SetInstanceId(instanceId.GetValueUnchecked());
    node.SetLabel(label);
    node.SetType(NAVNODE_TYPE_ECInstanceNode);
    node.SetExpandedImageId(ImageHelper::GetImageId(*ecClass, true, true).c_str());
    node.SetCollapsedImageId(ImageHelper::GetImageId(*ecClass, true, false).c_str());

    NavNodeExtendedData extendedData(node);
    extendedData.SetGroupedInstanceKey(ECInstanceKey(classId, instanceId));
    extendedData.SetConnectionId(connection.GetId());
    extendedData.SetLocale(locale.c_str());
    extendedData.SetECClassId(ecClass->GetId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNodesFactory::InitECInstanceNode(JsonNavNodeR node, Utf8StringCR connectionId, Utf8StringCR locale, IECInstanceCR instance, Utf8CP label) const
    {
    ECInstanceId instanceId;
    ECInstanceId::FromString(instanceId, instance.GetInstanceId().c_str());

    node.SetInstanceId(instanceId.GetValueUnchecked());
    node.SetLabel(label);
    node.SetType(NAVNODE_TYPE_ECInstanceNode);
    node.SetExpandedImageId(ImageHelper::GetImageId(instance.GetClass(), true, true).c_str());
    node.SetCollapsedImageId(ImageHelper::GetImageId(instance.GetClass(), true, false).c_str());

    NavNodeExtendedData extendedData(node);
    extendedData.SetGroupedInstanceKey(ECInstanceKey(instance.GetClass().GetId(), instanceId));
    extendedData.SetConnectionId(connectionId);
    extendedData.SetLocale(locale.c_str());
    extendedData.SetECClassId(instance.GetClass().GetId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNodesFactory::InitECClassGroupingNode(JsonNavNodeR node, Utf8StringCR connectionId, Utf8StringCR locale, ECClassCR ecClass, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const
    {
    node.SetLabel(label);
    node.SetDescription(ecClass.GetDescription().c_str());
    node.SetType(NAVNODE_TYPE_ECClassGroupingNode);
    node.SetHasChildren(!groupedInstanceKeys.empty());
    node.SetExpandedImageId(ImageHelper::GetImageId(ecClass, false, true).c_str());
    node.SetCollapsedImageId(ImageHelper::GetImageId(ecClass, false, false).c_str());

    NavNodeExtendedData extendedData(node);
    extendedData.SetRequestedSpecification(true);
    extendedData.SetGroupingType((int)GroupingType::Class);
    extendedData.SetGroupedInstanceKeys(groupedInstanceKeys);
    extendedData.SetConnectionId(connectionId);
    extendedData.SetLocale(locale.c_str());
    extendedData.SetECClassId(ecClass.GetId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNodesFactory::InitDisplayLabelGroupingNode(JsonNavNodeR node, Utf8StringCR connectionId, Utf8StringCR locale, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const
    {
    node.SetLabel(label);
    node.SetType(NAVNODE_TYPE_DisplayLabelGroupingNode);
    node.SetHasChildren(!groupedInstanceKeys.empty());
    node.SetExpandedImageId(ImageHelper::GetLabelGroupingNodeImageId(true).c_str());
    node.SetCollapsedImageId(ImageHelper::GetLabelGroupingNodeImageId(false).c_str());

    NavNodeExtendedData extendedData(node);
    extendedData.SetRequestedSpecification(true);
    extendedData.SetGroupingType((int)GroupingType::DisplayLabel);
    extendedData.SetGroupedInstanceKeys(groupedInstanceKeys);
    extendedData.SetConnectionId(connectionId);
    extendedData.SetLocale(locale.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNodesFactory::InitECPropertyGroupingNode(JsonNavNodeR node, Utf8StringCR connectionId, Utf8StringCR locale, ECClassCR ecClass, ECPropertyCR ecProperty, Utf8CP label, Utf8CP imageId, RapidJsonValueCR groupingValue, bool isRangeGrouping, GroupedInstanceKeysListCR groupedInstanceKeys) const
    {
    node.SetLabel(label);
    node.SetDescription(ecProperty.GetDescription().c_str());
    node.SetType(NAVNODE_TYPE_ECPropertyGroupingNode);
    node.SetHasChildren(!groupedInstanceKeys.empty());
    node.SetExpandedImageId((nullptr != imageId && 0 != *imageId) ? imageId : ImageHelper::GetImageId(ecProperty, true).c_str());
    node.SetCollapsedImageId((nullptr != imageId && 0 != *imageId) ? imageId : ImageHelper::GetImageId(ecProperty, false).c_str());

    NavNodeExtendedData extendedData(node);
    extendedData.SetRequestedSpecification(true);
    extendedData.SetPropertyName(Utf8String(ecProperty.GetName().c_str()).c_str());
    extendedData.SetGroupingType((int)GroupingType::Property);
    extendedData.SetGroupedInstanceKeys(groupedInstanceKeys);
    extendedData.SetConnectionId(connectionId);
    extendedData.SetLocale(locale.c_str());
    extendedData.SetECClassId(ecClass.GetId());

    if (isRangeGrouping)
        extendedData.SetPropertyValueRangeIndex(groupingValue.GetInt());
    else
        extendedData.SetPropertyValue(groupingValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNodesFactory::InitECRelationshipGroupingNode(JsonNavNodeR node, Utf8StringCR connectionId, Utf8StringCR locale, ECRelationshipClassCR ecRelationshipClass, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const
    {
    node.SetLabel(label);
    node.SetDescription(ecRelationshipClass.GetDescription().c_str());
    node.SetType(NAVNODE_TYPE_ECRelationshipGroupingNode);
    node.SetHasChildren(!groupedInstanceKeys.empty());
    node.SetExpandedImageId(ImageHelper::GetImageId(ecRelationshipClass, false, true).c_str());
    node.SetCollapsedImageId(ImageHelper::GetImageId(ecRelationshipClass, false, false).c_str());

    NavNodeExtendedData extendedData(node);
    extendedData.SetRequestedSpecification(true);
    extendedData.SetGroupingType((int)GroupingType::Relationship);
    extendedData.SetGroupedInstanceKeys(groupedInstanceKeys);
    extendedData.SetConnectionId(connectionId);
    extendedData.SetLocale(locale.c_str());
    extendedData.SetECClassId(ecRelationshipClass.GetId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNodesFactory::InitCustomNode(JsonNavNodeR node, Utf8StringCR connectionId, Utf8StringCR locale, Utf8CP label, Utf8CP description, Utf8CP imageId, Utf8CP type) const
    {
    node.SetLabel(label);
    node.SetDescription(description);
    node.SetType(type);
    node.SetExpandedImageId(imageId);
    node.SetCollapsedImageId(imageId);

    NavNodeExtendedData extendedData(node);
    extendedData.SetConnectionId(connectionId);
    extendedData.SetLocale(locale.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNodesFactory::InitFromJson(JsonNavNodeR node, IConnectionCR connection, RapidJsonValueCR json) const
    {
    node.InitFromJson(json);
    NavNodeExtendedData extendedData(node);
    extendedData.SetConnectionId(connection.GetId());
    }

#define COMPARE_PROPERTY(lhs,rhs,prop,json_name) \
    if (lhs.prop != rhs.prop) \
        changes.push_back(JsonChange(json_name, lhs.prop, rhs.prop));
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<JsonChange> NavNodesHelper::GetChanges(JsonNavNode const& lhs, JsonNavNode const& rhs)
    {
    bvector<JsonChange> changes;
    COMPARE_PROPERTY(lhs, rhs, m_nodeId, NAVNODE_NodeId);
    COMPARE_PROPERTY(lhs, rhs, m_parentNodeId, NAVNODE_ParentNodeId);
    COMPARE_PROPERTY(lhs, rhs, m_instanceId, NAVNODE_InstanceId);
    COMPARE_PROPERTY(lhs, rhs, m_hasChildren, NAVNODE_HasChildren);
    COMPARE_PROPERTY(lhs, rhs, m_isSelectable, NAVNODE_IsSelectable);
    COMPARE_PROPERTY(lhs, rhs, m_isEditable, NAVNODE_IsEditable);
    COMPARE_PROPERTY(lhs, rhs, m_isChecked, NAVNODE_IsChecked);
    COMPARE_PROPERTY(lhs, rhs, m_isCheckboxVisible, NAVNODE_IsCheckboxVisible);
    COMPARE_PROPERTY(lhs, rhs, m_isCheckboxEnabled, NAVNODE_IsCheckboxEnabled);
    COMPARE_PROPERTY(lhs, rhs, m_isExpanded, NAVNODE_IsExpanded);
    COMPARE_PROPERTY(lhs, rhs, m_label, NAVNODE_Label);
    COMPARE_PROPERTY(lhs, rhs, m_description, NAVNODE_Description);
    COMPARE_PROPERTY(lhs, rhs, m_imageId, NAVNODE_CollapsedImageId);
    COMPARE_PROPERTY(lhs, rhs, m_foreColor, NAVNODE_ForeColor);
    COMPARE_PROPERTY(lhs, rhs, m_backColor, NAVNODE_BackColor);
    COMPARE_PROPERTY(lhs, rhs, m_fontStyle, NAVNODE_FontStyle);
    COMPARE_PROPERTY(lhs, rhs, m_type, NAVNODE_Type);
    return changes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesHelper::AddRelatedInstanceInfo(NavNodeR node, Utf8CP serializedJson)
    {
    NavNodeExtendedData(node).SetRelatedInstanceKeys(serializedJson);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesHelper::SetSkippedInstanceKeys(NavNodeR node, Utf8CP serializedJson)
    {
    if (nullptr == serializedJson || 0 == *serializedJson)
        return;

    rapidjson::Document doc(&node.GetExtendedDataAllocator());
    doc.Parse(serializedJson);
    if (doc.IsArray())
        NavNodeExtendedData(node).SetSkippedInstanceKeys(std::move(doc));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodesHelper::IsGroupingNode(NavNodeCR node)
    {
    return node.GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode)
        || node.GetType().Equals(NAVNODE_TYPE_ECClassGroupingNode)
        || node.GetType().Equals(NAVNODE_TYPE_ECPropertyGroupingNode)
        || node.GetType().Equals(NAVNODE_TYPE_ECRelationshipGroupingNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodesHelper::IsCustomNode(NavNodeCR node)
    {
    return !node.GetType().Equals(NAVNODE_TYPE_ECInstanceNode)
        && !node.GetType().Equals(NAVNODE_TYPE_ECClassGroupingNode)
        && !node.GetType().Equals(NAVNODE_TYPE_ECRelationshipGroupingNode)
        && !node.GetType().Equals(NAVNODE_TYPE_ECPropertyGroupingNode)
        && !node.GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr NavNodesHelper::CreateNodeKey(IConnectionCR connection, JsonNavNodeCR node, bvector<Utf8String> const& path)
    {
    NavNodeExtendedData extendedData(node);
    if (node.GetType().Equals(NAVNODE_TYPE_ECInstanceNode))
        {
        ECClassCP ecClass = connection.GetECDb().Schemas().GetClass(extendedData.GetECClassId());
        return ECInstanceNodeKey::Create(ECClassInstanceKey(ecClass, ECInstanceId(node.GetInstanceId())), path);
        }
    if (node.GetType().Equals(NAVNODE_TYPE_ECClassGroupingNode))
        {
        uint64_t groupedInstancesCount = (uint64_t)extendedData.GetGroupedInstanceKeysCount();
        ECClassCP ecClass = connection.GetECDb().Schemas().GetClass(extendedData.GetECClassId());
        return ECClassGroupingNodeKey::Create(*ecClass, path, groupedInstancesCount);
        }
    if (node.GetType().Equals(NAVNODE_TYPE_ECPropertyGroupingNode))
        {
        uint64_t groupedInstancesCount = (uint64_t)extendedData.GetGroupedInstanceKeysCount();
        ECClassCP ecClass = connection.GetECDb().Schemas().GetClass(extendedData.GetECClassId());
        return ECPropertyGroupingNodeKey::Create(*ecClass, extendedData.GetPropertyName(), extendedData.GetPropertyValue(), path, groupedInstancesCount);
        }
    if (node.GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode))
        {
        uint64_t groupedInstancesCount = (uint64_t)extendedData.GetGroupedInstanceKeysCount();
        return LabelGroupingNodeKey::Create(node.GetLabel(), path, groupedInstancesCount);
        }
    return NavNodeKey::Create(node.GetType(), path);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr NavNodesHelper::CreateNodeKey(IConnectionCR connection, JsonNavNodeCR node, Utf8CP pathFromRootString)
    {
    bvector<Utf8String> path;
    CharP context;
    Utf8CP token = BeStringUtilities::Strtok(const_cast<CharP>(pathFromRootString), ",", &context);
    while (nullptr != token)
        {
        path.push_back(token);
        token = BeStringUtilities::Strtok(nullptr, ",", &context);
        }
    return CreateNodeKey(connection, node, path);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr NavNodesHelper::CreateNodeKey(IConnectionCR connection, JsonNavNodeCR node, NavNodeKeyCP parentNodeKey)
    {
    MD5 h;
    NavNodeExtendedData extendedData(node);
    Utf8String type = node.GetType();
    Utf8String dbGuid = connection.GetDb().GetDbGuid().ToString();
    Utf8CP specHash = extendedData.GetSpecificationHash();
    Utf8CP rulesetId = extendedData.GetRulesetId();
    h.Add(type.c_str(), type.SizeInBytes());
    h.Add(rulesetId, strlen(rulesetId));
    h.Add(specHash, strlen(specHash));
    h.Add(dbGuid.c_str(), dbGuid.SizeInBytes());

    if (0 == strcmp(NAVNODE_TYPE_ECInstanceNode, type.c_str()))
        {
        uint64_t instanceId = node.GetInstanceId();
        uint64_t classId = extendedData.GetECClassId().GetValueUnchecked();
        h.Add(&instanceId, sizeof(instanceId));
        h.Add(&classId, sizeof(instanceId));
        }
    else if (0 == strcmp(NAVNODE_TYPE_ECClassGroupingNode, type.c_str()))
        {
        uint64_t classId = extendedData.GetECClassId().GetValueUnchecked();
        h.Add(&classId, sizeof(classId));
        }
    else if (0 == strcmp(NAVNODE_TYPE_ECPropertyGroupingNode, type.c_str()))
        {
        uint64_t classId = extendedData.GetECClassId().GetValueUnchecked();
        Utf8CP propertyName = extendedData.GetPropertyName();
        int rangeIndex = extendedData.GetPropertyValueRangeIndex();
        if (extendedData.HasPropertyValue())
            {
            rapidjson::Value const* propertyValue = extendedData.GetPropertyValue();
            Utf8String valueString = BeRapidJsonUtilities::ToString(*propertyValue);
            h.Add(valueString.c_str(), valueString.SizeInBytes());
            }
        h.Add(&classId, sizeof(classId));
        h.Add(propertyName, strlen(propertyName));
        h.Add(&rangeIndex, sizeof(rangeIndex));
        }
    else
        {
        // CustomNode and DisplayLabelGroupingNode
        Utf8String nodeLabel = node.GetLabel();
        h.Add(nodeLabel.c_str(), nodeLabel.SizeInBytes());
        }

    // create path from root to this node
    bvector<Utf8String> parentPath = parentNodeKey ? parentNodeKey->GetPathFromRoot() : bvector<Utf8String>();
    Utf8String nodeHash = h.GetHashString();
    parentPath.push_back(nodeHash);
    return NavNodesHelper::CreateNodeKey(connection, node, parentPath);
    }
