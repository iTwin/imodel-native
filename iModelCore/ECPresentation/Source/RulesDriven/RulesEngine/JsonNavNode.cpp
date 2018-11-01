/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/JsonNavNode.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
    : m_allocator(NAVNODE_JSON_CHUNK_SIZE), m_json(rapidjson::Document(&m_allocator))//, m_ecdb(nullptr)
    {
    m_json.SetObject();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RapidJsonValueCR JsonNavNode::GetJson() const {return m_json;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Pranciskus.Ambrazas            06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::MemoryPoolAllocator<>& JsonNavNode::_GetExtendedDataAllocator() const {return m_json.GetAllocator();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t JsonNavNode::_GetNodeId() const
    {
    if (!m_json.HasMember(NAVNODE_NodeId))
        return 0;
    return m_json[NAVNODE_NodeId].GetUint64();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::_SetNodeId(uint64_t id) {AddMember(NAVNODE_NodeId, rapidjson::Value(id).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t JsonNavNode::_GetParentNodeId() const {return m_json.HasMember(NAVNODE_ParentNodeId) ? m_json[NAVNODE_ParentNodeId].GetUint64() : 0;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::_SetParentNodeId(uint64_t id)
    {
    AddMember(NAVNODE_ParentNodeId, rapidjson::Value(id).Move());
    NavNodeExtendedData extendedData(*this);
    if (!extendedData.HasVirtualParentId())
        extendedData.SetVirtualParentId(id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyCPtr JsonNavNode::_GetNodeKey() const
    {
    if (m_nodeKey.IsNull())
        {
        BeAssert(false);
        return NavNodeKey::Create("", bvector<Utf8String>());
        }
    return m_nodeKey;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String JsonNavNode::_GetLabel() const {return m_json.HasMember(NAVNODE_Label) ? m_json[NAVNODE_Label].GetString() : "";}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String JsonNavNode::_GetDescription() const {return m_json.HasMember(NAVNODE_Description) ? m_json[NAVNODE_Description].GetString() : "";}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RapidJsonValueR JsonNavNode::_GetExtendedData() const 
    {
    if (!m_json.IsObject() || !m_json.HasMember(NAVNODE_ExtendedData))
        m_json.AddMember(NAVNODE_ExtendedData, rapidjson::Value(rapidjson::kObjectType), m_json.GetAllocator());
    return m_json[NAVNODE_ExtendedData];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String JsonNavNode::_GetExpandedImageId() const {return m_json.HasMember(NAVNODE_ExpandedImageId) ? m_json[NAVNODE_ExpandedImageId].GetString() : "";}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String JsonNavNode::_GetCollapsedImageId() const {return m_json.HasMember(NAVNODE_CollapsedImageId) ? m_json[NAVNODE_CollapsedImageId].GetString() : "";}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String JsonNavNode::_GetForeColor() const {return m_json.HasMember(NAVNODE_ForeColor) ? m_json[NAVNODE_ForeColor].GetString() : "";}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String JsonNavNode::_GetBackColor() const {return m_json.HasMember(NAVNODE_BackColor) ? m_json[NAVNODE_BackColor].GetString() : "";}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String JsonNavNode::_GetFontStyle() const {return m_json.HasMember(NAVNODE_FontStyle) ? m_json[NAVNODE_FontStyle].GetString() : "Regular";}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String JsonNavNode::_GetType() const {return m_json.HasMember(NAVNODE_Type) ? m_json[NAVNODE_Type].GetString() : "";}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonNavNode::_HasChildren() const
    {
    if (m_json.HasMember(NAVNODE_HasChildren) && m_json[NAVNODE_HasChildren].GetBool())
        return true;

    NavNodeExtendedData extendedData(*this);
    return extendedData.GetAlwaysReturnsChildren();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonNavNode::_IsSelectable() const {return m_json.HasMember(NAVNODE_IsSelectable) ? m_json[NAVNODE_IsSelectable].GetBool() : true;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonNavNode::_IsEditable() const {return m_json.HasMember(NAVNODE_IsEditable) ? m_json[NAVNODE_IsEditable].GetBool() : false;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonNavNode::_IsChecked() const {return m_json.HasMember(NAVNODE_IsChecked) ? m_json[NAVNODE_IsChecked].GetBool() : false;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonNavNode::_IsCheckboxVisible() const {return m_json.HasMember(NAVNODE_IsCheckboxVisible) ? m_json[NAVNODE_IsCheckboxVisible].GetBool() : false;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonNavNode::_IsCheckboxEnabled() const {return m_json.HasMember(NAVNODE_IsCheckboxEnabled) ? m_json[NAVNODE_IsCheckboxEnabled].GetBool() : false;} 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonNavNode::_IsExpanded() const {return m_json.HasMember(NAVNODE_IsExpanded) ? m_json[NAVNODE_IsExpanded].GetBool() : false;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Pranciskus.Ambrazas             05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::AddMember(Utf8CP name, rapidjson::Value& value)
    {
    rapidjson::Value::MemberIterator iterator = m_json.FindMember(name);
    if (iterator == m_json.MemberEnd())
        m_json.AddMember(rapidjson::GenericStringRef<Utf8Char>(name), value, m_json.GetAllocator());
    else
        iterator->value = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::_SetLabel(Utf8CP label) {AddMember(NAVNODE_Label, rapidjson::Value(label, m_json.GetAllocator()).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Pranciskus.Ambrazas             05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::_SetInstanceId(uint64_t instanceId) {AddMember(NAVNODE_InstanceId, rapidjson::Value(instanceId).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t JsonNavNode::_GetInstanceId() const {return m_json.HasMember(NAVNODE_InstanceId) ? m_json[NAVNODE_InstanceId].GetUint64() : 0;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::_SetDescription(Utf8CP description) {AddMember(NAVNODE_Description, rapidjson::Value(description, m_json.GetAllocator()).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::SetImageId(Utf8CP imageId)
    {
    AddMember(NAVNODE_ExpandedImageId, rapidjson::Value(imageId, m_json.GetAllocator()).Move());
    AddMember(NAVNODE_CollapsedImageId, rapidjson::Value(imageId, m_json.GetAllocator()).Move());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::_SetForeColor(Utf8CP color) {AddMember(NAVNODE_ForeColor, rapidjson::Value(color, m_json.GetAllocator()).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::_SetBackColor(Utf8CP color) {AddMember(NAVNODE_BackColor, rapidjson::Value(color, m_json.GetAllocator()).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Pranciskus.Ambrazas             05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::_SetExpandedImageId(Utf8CP imageId) {AddMember(NAVNODE_ExpandedImageId, rapidjson::Value(imageId, m_json.GetAllocator()).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Pranciskus.Ambrazas             05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::_SetCollapsedImageId(Utf8CP imageId) {AddMember(NAVNODE_CollapsedImageId, rapidjson::Value(imageId, m_json.GetAllocator()).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Pranciskus.Ambrazas             05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::_SetType(Utf8CP type) {AddMember(NAVNODE_Type, rapidjson::Value(type, m_json.GetAllocator()).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::_SetFontStyle(Utf8CP style) {AddMember(NAVNODE_FontStyle, rapidjson::Value(style, m_json.GetAllocator()).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::_SetHasChildren(bool value) {AddMember(NAVNODE_HasChildren, rapidjson::Value(value).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::_SetIsChecked(bool value) {AddMember(NAVNODE_IsChecked, rapidjson::Value(value).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::_SetIsCheckboxVisible(bool value) {AddMember(NAVNODE_IsCheckboxVisible, rapidjson::Value(value).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::_SetIsCheckboxEnabled(bool value) {AddMember(NAVNODE_IsCheckboxEnabled, rapidjson::Value(value).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::_SetIsExpanded(bool value) {AddMember(NAVNODE_IsExpanded, rapidjson::Value(value).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::_SetIsSelectable(bool value) {AddMember(NAVNODE_IsSelectable, rapidjson::Value(value).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::_SetIsEditable(bool value) {AddMember(NAVNODE_IsEditable, rapidjson::Value(value).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr JsonNavNode::_Clone() const
    {
    JsonNavNode* node = new JsonNavNode();
    node->m_json.CopyFrom(m_json, node->m_allocator);
    node->m_nodeKey = m_nodeKey;
    return node;
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
JsonNavNodePtr JsonNavNodesFactory::CreateFromJson(IConnectionCR connection, rapidjson::Document&& json) const
    {
    JsonNavNodePtr node = JsonNavNode::Create();
    InitFromJson(*node, connection, std::move(json));
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
void JsonNavNodesFactory::InitFromJson(JsonNavNodeR node, IConnectionCR connection, rapidjson::Document&& json) const
    {
    node.m_json = std::move(json);    
    NavNodeExtendedData extendedData(node);
    extendedData.SetConnectionId(connection.GetId());
    }

typedef bool(*IsMemberSignificantFunc)(Utf8StringCR);
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsJsonNodeMemberSignificant(Utf8StringCR name)
    {
    if (name.Equals(NAVNODE_Key))
        return false;
    if (name.Equals(NAVNODE_ExtendedData))
        return false;
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsJsonNodeExtendedDataMemberSignificant(Utf8StringCR name)
    {
    if (name.Equals(ITEM_EXTENDEDDATA_IsCustomized))
        return false;
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Pranciskus.Ambrazas             05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<Utf8String> GetMemberNames(RapidJsonValueCR json)
    {
    bvector<Utf8String> names;
    rapidjson::Value::ConstMemberIterator iterator = json.MemberBegin();
    rapidjson::Value::ConstMemberIterator iteratorEnd = json.MemberEnd();
    for ( ; iterator != iteratorEnd; ++iterator)
        names.push_back(iterator->name.GetString());
    return names;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void GetJsonNodeChanges(bvector<JsonChange>& changes, RapidJsonValueCR oldJson, RapidJsonValueCR newJson, bool isExtendedData = false, Utf8CP prefix = nullptr)
    {
    IsMemberSignificantFunc IsMemberSignificant = isExtendedData ? &IsJsonNodeExtendedDataMemberSignificant : &IsJsonNodeMemberSignificant;
    std::function<Utf8String(Utf8StringCR)> GetPrefixedName = [prefix](Utf8StringCR name) -> Utf8String
        {
        if (nullptr == prefix || 0 == *prefix)
            return name;
        return Utf8PrintfString("%s.%s", prefix, name.c_str());
        };

    bvector<Utf8String> oldMembers = GetMemberNames(oldJson);
    bvector<Utf8String> newMembers = GetMemberNames(newJson);
    bvector<Utf8String> oldToNewMembersDiff, newToOldMembersDiff, commonMembers;
    std::set_difference(oldMembers.begin(), oldMembers.end(), newMembers.begin(), newMembers.end(), std::back_inserter(oldToNewMembersDiff));
    std::set_difference(newMembers.begin(), newMembers.end(), oldMembers.begin(), oldMembers.end(), std::back_inserter(newToOldMembersDiff));
    std::set_intersection(oldMembers.begin(), oldMembers.end(), newMembers.begin(), newMembers.end(), std::back_inserter(commonMembers));

    for (Utf8StringCR member : oldToNewMembersDiff)
        {
        if (IsMemberSignificant(member))
            changes.push_back(JsonChange(GetPrefixedName(member).c_str(), oldJson[member.c_str()], rapidjson::Value()));
        }
    
    for (Utf8StringCR member : newToOldMembersDiff)
        {
        if (IsMemberSignificant(member))
            changes.push_back(JsonChange(GetPrefixedName(member).c_str(), rapidjson::Value(), newJson[member.c_str()]));
        }
    
    for (Utf8StringCR member : commonMembers)
        {
        if (!IsMemberSignificant(member))
            continue;

        if (member.Equals(NAVNODE_ExtendedData))
            GetJsonNodeChanges(changes, oldJson[member.c_str()], newJson[member.c_str()], true);
        else if (oldJson[member.c_str()] != newJson[member.c_str()])
            changes.push_back(JsonChange(GetPrefixedName(member).c_str(), oldJson[member.c_str()], newJson[member.c_str()]));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<JsonChange> NavNodesHelper::GetChanges(JsonNavNode const& oldNode, JsonNavNode const& newNode)
    {
    bvector<JsonChange> changes;
    GetJsonNodeChanges(changes, oldNode.GetJson(), newNode.GetJson());
    return changes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesHelper::SwapData(JsonNavNode& lhs, JsonNavNode& rhs) {lhs.m_json.Swap(rhs.m_json);}

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
NavNodeKeyPtr NavNodesHelper::CreateNodeKey(IConnectionCR connection, JsonNavNodeCR node, Utf8CP pathFromRootJsonString)
    {
    rapidjson::Document json;
    json.Parse(pathFromRootJsonString);
    bvector<Utf8String> path;
    for (RapidJsonValueCR pathElement : json.GetArray())
        path.push_back(pathElement.GetString());
    return CreateNodeKey(connection, node, path);
    }
