/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentationManager.h>
#include "NavNodesHelper.h"
#include "NavNodesCache.h"
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
NavNodePtr NavNodesFactory::CreateECClassGroupingNode(IConnectionCR connection, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey, ECClassCR ecClass, bool isPolymorphic,
    LabelDefinitionCR label, uint64_t groupedInstancesCount, PresentationQueryCP instanceKeysSelectQuery) const
    {
    NavNodePtr node = NavNode::Create();
    node->SetLabelDefinition(label);
    node->SetDescription(ecClass.GetDescription().c_str());
    node->SetType(NAVNODE_TYPE_ECClassGroupingNode);
    node->SetNodeKey(*ECClassGroupingNodeKey::Create(connection, specificationIdentifier, parentKey, ecClass, isPolymorphic, groupedInstancesCount, instanceKeysSelectQuery));

    NavNodeExtendedData extendedData(*node);
    extendedData.SetRequestedSpecification(true);
    extendedData.SetECClassId(ecClass.GetId());
    extendedData.SetIsECClassPolymorphic(isPolymorphic);
    extendedData.SetIsLabelCustomized(true);

    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NavNodesFactory::CreateECRelationshipGroupingNode(IConnectionCR connection, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey, ECRelationshipClassCR relationshipClass,
    LabelDefinitionCR label, uint64_t groupedInstancesCount, PresentationQueryCP instanceKeysSelectQuery) const
    {
    NavNodePtr node = NavNode::Create();
    node->SetLabelDefinition(label);
    node->SetDescription(relationshipClass.GetDescription().c_str());
    node->SetType(NAVNODE_TYPE_ECRelationshipGroupingNode);
    node->SetNodeKey(*NavNodeKey::Create(connection, specificationIdentifier, parentKey, NAVNODE_TYPE_ECRelationshipGroupingNode, label.GetDisplayValue(), instanceKeysSelectQuery));

    NavNodeExtendedData extendedData(*node);
    extendedData.SetRequestedSpecification(true);
    extendedData.SetECClassId(relationshipClass.GetId());
    extendedData.SetIsLabelCustomized(true);

    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NavNodesFactory::CreateDisplayLabelGroupingNode(IConnectionCR connection, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey,
    LabelDefinitionCR label, uint64_t groupedInstancesCount, PresentationQueryCP instanceKeysSelectQuery, std::unique_ptr<bvector<ECInstanceKey>> groupedInstanceKeys) const
    {
    NavNodePtr node = NavNode::Create();
    node->SetLabelDefinition(label);
    node->SetType(NAVNODE_TYPE_DisplayLabelGroupingNode);
    node->SetNodeKey(*LabelGroupingNodeKey::Create(connection, specificationIdentifier, parentKey, label.GetDisplayValue(),
        groupedInstancesCount, instanceKeysSelectQuery, std::move(groupedInstanceKeys)));

    NavNodeExtendedData extendedData(*node);
    extendedData.SetRequestedSpecification(true);
    extendedData.SetIsLabelCustomized(true);

    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NavNodesFactory::CreateECPropertyGroupingNode(IConnectionCR connection, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey, ECClassCR ecClass, ECPropertyCR ecProperty,
    LabelDefinitionCR label, Utf8CP imageId, RapidJsonValueCR groupingValue, bool isRangeGrouping, uint64_t groupedInstancesCount, PresentationQueryCP instanceKeysSelectQuery) const
    {
    NavNodePtr node = NavNode::Create();
    node->SetLabelDefinition(label);
    node->SetDescription(ecProperty.GetDescription().c_str());
    node->SetType(NAVNODE_TYPE_ECPropertyGroupingNode);
    if (imageId && *imageId)
        node->SetImageId(imageId);
    node->SetNodeKey(*ECPropertyGroupingNodeKey::Create(connection, specificationIdentifier, parentKey,
        ecClass, ecProperty.GetName(), groupingValue, groupedInstancesCount, instanceKeysSelectQuery));

    NavNodeExtendedData extendedData(*node);
    extendedData.SetRequestedSpecification(true);
    extendedData.SetPropertyName(Utf8String(ecProperty.GetName().c_str()).c_str());
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
NavNodePtr NavNodesFactory::CreateCustomNode(IConnectionCR connection, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey, LabelDefinitionCR label,
    Utf8CP description, Utf8CP imageId, Utf8CP type, PresentationQueryCP instanceKeysSelectQuery) const
    {
    NavNodePtr node = NavNode::Create();
    node->SetLabelDefinition(label);
    node->SetDescription(description);
    node->SetType(type);
    node->SetImageId(imageId);
    node->SetNodeKey(*NavNodeKey::Create(connection, specificationIdentifier, parentKey, type, label.GetDisplayValue(), instanceKeysSelectQuery));
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NavNodesFactory::CreateFromJson(IConnectionCR connection, RapidJsonValueCR json, NavNodeKeyR key) const
    {
    auto node = NavNodesHelper::DeserializeNodeFromJson(json);
    node->SetNodeKey(key);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bset<unsigned> GetUsedParentInstanceLevels(Utf8StringCR instanceFilter)
    {
    bset<unsigned> levels;
    size_t startPos = 0, endPos = 0;
    while (Utf8String::npos != (endPos = instanceFilter.find("parent.", endPos)))
        {
        endPos += 7; // strlen("parent.") = 7
        startPos = instanceFilter.find("parent", startPos);
        Utf8String selector(instanceFilter.begin() + startPos, instanceFilter.begin() + endPos);
        startPos = endPos;
        unsigned count = 1;
        for (Utf8Char const& c : selector)
            {
            if (c == '_')
                count++;
            }
        levels.insert(count);
        }
    return levels;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<NavNodeCPtr> GetParentInstanceNodesByLevel(IHierarchyCacheCR nodesCache, bvector<NavNodeCPtr> currInstanceNodes, unsigned currNodeLevel, unsigned targetNodeLevel)
    {
    bvector<NavNodeCPtr> curr = currInstanceNodes;
    while (!curr.empty() && currNodeLevel < targetNodeLevel)
        {
        bset<BeGuid> parentIds;
        for (auto const& currNode : curr)
            ContainerHelpers::Push(parentIds, NavNodeExtendedData(*currNode).GetVirtualParentIds());

        curr.clear();
        for (auto const& parentId : parentIds)
            {
            NavNodeCPtr parentNode = nodesCache.GetNode(parentId);
            if (parentNode.IsNull())
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to find parent node by ID");
            curr.push_back(parentNode);
            }

        if (ContainerHelpers::Contains(curr, [](auto const& node) {return node->GetKey()->AsECInstanceNodeKey(); }))
            ++currNodeLevel;
        }
    return curr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bset<ECClassCP> GetInstanceKeyClasses(bvector<ECClassInstanceKey> const& keys)
    {
    bset<ECClassCP> classes;
    for (ECClassInstanceKey const& key : keys)
        classes.insert(key.GetClass());
    return classes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<ECInstanceId> GetInstanceIdsWithClass(bvector<ECClassInstanceKey> const& keys, ECClassCR ecClass)
    {
    bvector<ECInstanceId> ids;
    for (ECClassInstanceKeyCR key : keys)
        {
        if (key.GetClass() == &ecClass && key.GetId().IsValid())
            ids.push_back(key.GetId());
        }
    return ids;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String FormatInstanceFilter(Utf8StringCR filter)
    {
    Utf8String formattedFilter(filter);
    formattedFilter.ReplaceAll(".parent", "_parent");
    return formattedFilter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
HierarchiesInstanceFilteringHelper::ParentInstanceFilteringInfo HierarchiesInstanceFilteringHelper::CreateParentInstanceFilteringInfo(
    IHierarchyCacheCR nodesCache,
    NavNodeCP immediateParentInstanceNode,
    Utf8StringCR filterExpression
)
    {
    ParentInstanceFilteringInfo result;
    result.modifiedInstanceFilter = FormatInstanceFilter(filterExpression);
    bset<unsigned> usedParentInstanceLevels = GetUsedParentInstanceLevels(result.modifiedInstanceFilter);
    if (usedParentInstanceLevels.empty())
        return result;

    if (!immediateParentInstanceNode)
        {
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("Hierarchy specification is referencing parent instance in its instance filter ECExpression,"
            "but there is no parent instance. The expression: `%s`.", filterExpression.c_str()));
        }

    bvector<NavNodeCPtr> previousParents = { immediateParentInstanceNode };
    unsigned previousLevel = 1;
    Utf8String parentAlias = "parent";
    for (unsigned targetLevel : usedParentInstanceLevels)
        {
        bvector<NavNodeCPtr> parentInstanceNodes = GetParentInstanceNodesByLevel(nodesCache, previousParents, previousLevel, targetLevel);
        bvector<ECClassInstanceKey> parentInstanceKeys;
        for (NavNodeCPtr const& parentInstanceNode : parentInstanceNodes)
            {
            ECInstancesNodeKey const& parentNodeKey = *parentInstanceNode->GetKey()->AsECInstanceNodeKey();
            ContainerHelpers::Push(parentInstanceKeys, parentNodeKey.GetInstanceKeys());
            }
        if (parentInstanceKeys.empty())
            {
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("Didn't find any parent instance keys for requested parent `%s` and target level `%d`",
                BeRapidJsonUtilities::ToString(immediateParentInstanceNode->GetKey()->AsJson()).c_str(), (int)targetLevel));
            }

        bset<ECClassCP> parentClasses = GetInstanceKeyClasses(parentInstanceKeys);
        ECClassCP parentClass = *parentClasses.begin();
        if (parentClasses.size() > 1)
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_INFO, LOG_WARNING, Utf8PrintfString("Used parent instance node represents instances of more than 1 ECClass. "
                "Using just the first one: `%s`.", parentClass->GetFullName()));
            }
        for (unsigned i = previousLevel; i < targetLevel; ++i)
            parentAlias.append("_parent");

        result.classInstanceIds.push_back(
            {
            SelectClass<ECClass>(*parentClass, parentAlias, false),
            GetInstanceIdsWithClass(parentInstanceKeys, *parentClass)
            });
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeCPtr HierarchiesInstanceFilteringHelper::GetParentInstanceNode(IHierarchyCacheCR nodesCache, NavNodeCR node)
    {
    NavNodeCPtr parentInstanceNode = &node;
    while (parentInstanceNode.IsValid() && nullptr == parentInstanceNode->GetKey()->AsECInstanceNodeKey())
        {
        auto parentIds = NavNodeExtendedData(*parentInstanceNode).GetVirtualParentIds();
        if (parentIds.empty())
            {
            parentInstanceNode = nullptr;
            break;
            }
        parentInstanceNode = nodesCache.GetNode(parentIds.front()).get();
        }
    return parentInstanceNode;
    }