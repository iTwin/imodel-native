/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/JsonNavNode.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
    : m_allocator(NAVNODE_JSON_CHUNK_SIZE), m_json(rapidjson::Document(&m_allocator)), m_ecdb(nullptr)
    {
    m_json.SetObject();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RapidJsonValueCR JsonNavNode::GetJson() const {return m_json;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document JsonNavNode::_AsJson(rapidjson::MemoryPoolAllocator<>* allocator) const
    {
    rapidjson::Document json(allocator);
    json.CopyFrom(m_json, json.GetAllocator());
    json.RemoveMember(NAVNODE_ExtendedData);
    json.AddMember(NAVNODE_Key, GetKey().AsJson(&json.GetAllocator()), json.GetAllocator());
    return json;
    }

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
void JsonNavNode::SetNodeId(uint64_t id) {AddMember(NAVNODE_NodeId, rapidjson::Value(id).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t JsonNavNode::_GetParentNodeId() const {return m_json.HasMember(NAVNODE_ParentNodeId) ? m_json[NAVNODE_ParentNodeId].GetUint64() : 0;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::SetParentNodeId(uint64_t id)
    {
    AddMember(NAVNODE_ParentNodeId, rapidjson::Value(id).Move());
    NavNodeExtendedData extendedData(*this);
    if (!extendedData.HasVirtualParentId())
        extendedData.SetVirtualParentId(id);
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
RefCountedPtr<IECInstance const> JsonNavNode::_GetInstance() const
    {
    ECInstanceNodeKey const* key = GetKey().AsECInstanceNodeKey();
    if (nullptr == key)
        return nullptr;
    
    if (m_instance.IsNull() && key->GetInstanceKey().IsValid())
        LoadECInstance();

    return m_instance;
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
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyCPtr JsonNavNode::_CreateKey() const
    {
    NavNodeExtendedData extendedData(*this);

    if (GetType().Equals(NAVNODE_TYPE_ECInstanceNode))
        {
        BeAssert(m_json.HasMember(NAVNODE_InstanceId));
        BeAssert(extendedData.HasECClassId());
        return ECInstanceNodeKey::Create(extendedData.GetECClassId(), ECInstanceId(m_json[NAVNODE_InstanceId].GetUint64()));
        }

    if (GetType().Equals(NAVNODE_TYPE_ECClassGroupingNode))
        {
        BeAssert(extendedData.HasECClassId());
        BeAssert(extendedData.HasGroupingType());
        GroupingType type = (GroupingType)extendedData.GetGroupingType();
        if (GroupingType::Class == type)
            return ECClassGroupingNodeKey::Create(GetNodeId(), extendedData.GetECClassId());
        else if (GroupingType::BaseClass == type)
            return ECClassGroupingNodeKey::Create(GetNodeId(), extendedData.GetECClassId(), "BaseECClassGroupingNode");
        else
            {
            BeAssert(false);
            return nullptr;
            }
        }

    if (GetType().Equals(NAVNODE_TYPE_ECPropertyGroupingNode))
        {
        BeAssert(extendedData.HasECClassId());
        BeAssert(extendedData.HasPropertyName());
        BeAssert(extendedData.HasPropertyValue() || extendedData.HasPropertyValueRangeIndex());        
        return ECPropertyGroupingNodeKey::Create(GetNodeId(), extendedData.GetECClassId(), extendedData.GetPropertyName(), 
            extendedData.GetPropertyValueRangeIndex(), extendedData.GetPropertyValue());
        }

    // for label grouping nodes and all other nodes (custom nodes) we use DisplayLabelGroupingNodeKey
    return DisplayLabelGroupingNodeKey::Create(GetNodeId(), m_json[NAVNODE_Label].GetString(), m_json[NAVNODE_Type].GetString());
    }

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
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::LoadECInstance() const
    {
    if (nullptr == m_ecdb)
        {
        // ECDb must be set to load the ECInstance
        BeAssert(false);
        return;
        }

    BeAssert(0 == strcmp(NAVNODE_TYPE_ECInstanceNode, m_json[NAVNODE_Type].GetString()));
    BeAssert(m_json.HasMember(NAVNODE_InstanceId));
    
    NavNodeExtendedData extendedData(*this);
    BeAssert(extendedData.HasECClassId());

    m_instance = ECInstancesHelper::LoadInstance(*m_ecdb, ECInstanceKey(extendedData.GetECClassId(), ECInstanceId(m_json[NAVNODE_InstanceId].GetUint64())));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::SetLabel(Utf8CP label) {AddMember(NAVNODE_Label, rapidjson::Value(label, m_json.GetAllocator()).Move()); InvalidateKey();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Pranciskus.Ambrazas             05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::SetInstanceId(uint64_t instanceId) {AddMember(NAVNODE_InstanceId, rapidjson::Value(instanceId).Move()); InvalidateKey();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::SetDescription(Utf8CP descr) {AddMember(NAVNODE_Description, rapidjson::Value(descr, m_json.GetAllocator()).Move());}

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
void JsonNavNode::SetForeColor(Utf8CP color) {AddMember(NAVNODE_ForeColor, rapidjson::Value(color, m_json.GetAllocator()).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::SetBackColor(Utf8CP color) {AddMember(NAVNODE_BackColor, rapidjson::Value(color, m_json.GetAllocator()).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Pranciskus.Ambrazas             05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::SetExpandedImageId(Utf8CP imageId) {AddMember(NAVNODE_ExpandedImageId, rapidjson::Value(imageId, m_json.GetAllocator()).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Pranciskus.Ambrazas             05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::SetCollapsedImageId(Utf8CP imageId) {AddMember(NAVNODE_CollapsedImageId, rapidjson::Value(imageId, m_json.GetAllocator()).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Pranciskus.Ambrazas             05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::SetType(Utf8CP type) {AddMember(NAVNODE_Type, rapidjson::Value(type, m_json.GetAllocator()).Move()); InvalidateKey();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::SetFontStyle(Utf8CP style) {AddMember(NAVNODE_FontStyle, rapidjson::Value(style, m_json.GetAllocator()).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::SetHasChildren(bool value) {AddMember(NAVNODE_HasChildren, rapidjson::Value(value).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::SetIsChecked(bool value) {AddMember(NAVNODE_IsChecked, rapidjson::Value(value).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::SetIsCheckboxVisible(bool value) {AddMember(NAVNODE_IsCheckboxVisible, rapidjson::Value(value).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::SetIsCheckboxEnabled(bool value) {AddMember(NAVNODE_IsCheckboxEnabled, rapidjson::Value(value).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNode::SetIsExpanded(bool value) {AddMember(NAVNODE_IsExpanded, rapidjson::Value(value).Move());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodePtr JsonNavNodesFactory::CreateECInstanceNode(ECDbCR db, ECClassId classId, ECInstanceId instanceId, Utf8CP label) const
    {
    JsonNavNodePtr node = JsonNavNode::Create();
    InitECInstanceNode(*node, db, classId, instanceId, label);
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodePtr JsonNavNodesFactory::CreateECInstanceNode(BeGuidCR connectionId, IECInstanceCR instance, Utf8CP label) const
    {
    JsonNavNodePtr node = JsonNavNode::Create();
    InitECInstanceNode(*node, connectionId, instance, label);
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodePtr JsonNavNodesFactory::CreateECClassGroupingNode(BeGuidCR connectionId, ECClassCR ecClass, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const
    {
    JsonNavNodePtr node = JsonNavNode::Create();
    InitECClassGroupingNode(*node, connectionId, ecClass, label, groupedInstanceKeys);
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodePtr JsonNavNodesFactory::CreateECRelationshipGroupingNode(BeGuidCR connectionId, ECRelationshipClassCR relationshipClass, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const
    {
    JsonNavNodePtr node = JsonNavNode::Create();
    InitECRelationshipGroupingNode(*node, connectionId, relationshipClass, label, groupedInstanceKeys);
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodePtr JsonNavNodesFactory::CreateDisplayLabelGroupingNode(BeGuidCR connectionId, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const
    {
    JsonNavNodePtr node = JsonNavNode::Create();
    InitDisplayLabelGroupingNode(*node, connectionId, label, groupedInstanceKeys);
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodePtr JsonNavNodesFactory::CreateECPropertyGroupingNode(BeGuidCR connectionId, ECClassCR ecClass, ECPropertyCR ecProperty, Utf8CP label, Utf8CP imageId, RapidJsonValueCR groupingValue, bool isRangeGrouping, GroupedInstanceKeysListCR groupedInstanceKeys) const
    {
    JsonNavNodePtr node = JsonNavNode::Create();
    InitECPropertyGroupingNode(*node, connectionId, ecClass, ecProperty, label, imageId, groupingValue, isRangeGrouping, groupedInstanceKeys);
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodePtr JsonNavNodesFactory::CreateCustomNode(BeGuidCR connectionId, Utf8CP label, Utf8CP description, Utf8CP imageId, Utf8CP type) const
    {
    JsonNavNodePtr node = JsonNavNode::Create();
    InitCustomNode(*node, connectionId, label, description, imageId, type);
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodePtr JsonNavNodesFactory::CreateFromJson(ECDbCR db, rapidjson::Document&& json) const
    {
    JsonNavNodePtr node = JsonNavNode::Create();
    InitFromJson(*node, db, std::move(json));
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNodesFactory::InitECInstanceNode(JsonNavNodeR node, ECDbCR db, ECClassId classId, ECInstanceId instanceId, Utf8CP label) const
    {
    ECClassCP ecClass = nullptr;
    if (nullptr == (ecClass = db.Schemas().GetClass(classId)))
        {
        BeAssert(false);
        return;
        }

    node.SetInstanceId(instanceId.GetValueUnchecked());
    node.SetLabel(label);
    node.SetType(NAVNODE_TYPE_ECInstanceNode);
    node.SetExpandedImageId(ImageHelper::GetImageId(*ecClass, true, true).c_str());
    node.SetCollapsedImageId(ImageHelper::GetImageId(*ecClass, true, false).c_str());
    node.SetECDb(db);

    NavNodeExtendedData extendedData(node);
    extendedData.SetGroupedInstanceKey(ECInstanceKey(classId, instanceId));
    extendedData.SetConnectionId(db.GetDbGuid());
    extendedData.SetECClassId(ecClass->GetId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNodesFactory::InitECInstanceNode(JsonNavNodeR node, BeGuidCR connectionId, IECInstanceCR instance, Utf8CP label) const
    {
    ECInstanceId instanceId;
    ECInstanceId::FromString(instanceId, instance.GetInstanceId().c_str());

    node.SetInstanceId(instanceId.GetValueUnchecked());
    node.SetLabel(label);
    node.SetType(NAVNODE_TYPE_ECInstanceNode);
    node.SetExpandedImageId(ImageHelper::GetImageId(instance.GetClass(), true, true).c_str());
    node.SetCollapsedImageId(ImageHelper::GetImageId(instance.GetClass(), true, false).c_str());
    node.SetInstance(instance);

    NavNodeExtendedData extendedData(node);
    extendedData.SetGroupedInstanceKey(ECInstanceKey(instance.GetClass().GetId(), instanceId));
    extendedData.SetConnectionId(connectionId);
    extendedData.SetECClassId(instance.GetClass().GetId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNodesFactory::InitECClassGroupingNode(JsonNavNodeR node, BeGuidCR connectionId, ECClassCR ecClass, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const
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
    extendedData.SetECClassId(ecClass.GetId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNodesFactory::InitDisplayLabelGroupingNode(JsonNavNodeR node, BeGuidCR connectionId, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const
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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNodesFactory::InitECPropertyGroupingNode(JsonNavNodeR node, BeGuidCR connectionId, ECClassCR ecClass, ECPropertyCR ecProperty, Utf8CP label, Utf8CP imageId, RapidJsonValueCR groupingValue, bool isRangeGrouping, GroupedInstanceKeysListCR groupedInstanceKeys) const
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
    extendedData.SetECClassId(ecClass.GetId());

    if (isRangeGrouping)
        extendedData.SetPropertyValueRangeIndex(groupingValue.GetInt());
    else
        extendedData.SetPropertyValue(groupingValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNodesFactory::InitECRelationshipGroupingNode(JsonNavNodeR node, BeGuidCR connectionId, ECRelationshipClassCR ecRelationshipClass, Utf8CP label, GroupedInstanceKeysListCR groupedInstanceKeys) const
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
    extendedData.SetECClassId(ecRelationshipClass.GetId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNodesFactory::InitCustomNode(JsonNavNodeR node, BeGuidCR connectionId, Utf8CP label, Utf8CP description, Utf8CP imageId, Utf8CP type) const
    {
    node.SetLabel(label);
    node.SetDescription(description);
    node.SetType(type);
    node.SetExpandedImageId(imageId);
    node.SetCollapsedImageId(imageId);
    
    NavNodeExtendedData extendedData(node);
    extendedData.SetConnectionId(connectionId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonNavNodesFactory::InitFromJson(JsonNavNodeR node, ECDbCR db, rapidjson::Document&& json) const
    {
    node.m_json = std::move(json);
    node.SetECDb(db);
    
    NavNodeExtendedData extendedData(node);
    extendedData.SetConnectionId(db.GetDbGuid());
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
    if (nullptr == serializedJson || 0 == *serializedJson)
        return;
    
    rapidjson::Document doc(&node.GetExtendedDataAllocator());
    doc.Parse(serializedJson);
    if (doc.IsArray())
        NavNodeExtendedData(node).SetRelatedInstanceKeys(std::move(doc));
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
    if (nullptr != node.GetKey().AsECClassGroupingNodeKey())
        return true;
    if (nullptr != node.GetKey().AsECPropertyGroupingNodeKey())
        return true;
    if (nullptr != node.GetKey().AsDisplayLabelGroupingNodeKey() && node.GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode))
        return true;
    return false;
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
