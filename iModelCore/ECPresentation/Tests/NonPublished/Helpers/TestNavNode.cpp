/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentation/Connection.h>
#include "../../../Source/Shared/ValueHelpers.h"
#include "TestNavNode.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static rapidjson::Document GetGroupingValuesListJson(bvector<ECValue> const& groupingValues)
    {
    rapidjson::Document groupingValuesJson(rapidjson::kArrayType);
    for (ECValueCR value : groupingValues)
        groupingValuesJson.PushBack(ValueHelpers::GetJsonFromECValue(value, &groupingValuesJson.GetAllocator()), groupingValuesJson.GetAllocator());
    return groupingValuesJson;
    }

static NavNodesFactory s_testNodesFactory;
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr TestNodesHelper::CreateInstancesNode(IConnectionCR connection, bvector<ECClassInstanceKey> const& instanceKeys)
    {
    return CreateInstancesNode(connection, ContainerHelpers::TransformContainer<GroupedInstanceKeysList>(instanceKeys,
        [](ECClassInstanceKey const& key){return ECInstanceKey(key.GetClass()->GetId(), key.GetId());}));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr TestNodesHelper::CreateInstancesNode(IConnectionCR connection, bvector<ECInstanceKey> const& instanceKeys)
    {
    return s_testNodesFactory.CreateECInstanceNode(connection, "", nullptr, instanceKeys, *LabelDefinition::Create("label"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr TestNodesHelper::CreateInstanceNode(IConnectionCR connection, ECClassCR ecClass, ECInstanceId instanceId)
    {
    return s_testNodesFactory.CreateECInstanceNode(connection, "", nullptr, ecClass.GetId(), instanceId, *LabelDefinition::Create("label"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr TestNodesHelper::CreateInstanceNode(IConnectionCR connection, IECInstanceR instance)
    {
    return s_testNodesFactory.CreateECInstanceNode(connection, "", nullptr, instance, *LabelDefinition::Create("label"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr TestNodesHelper::CreateClassGroupingNode(IConnectionCR connection, ECClassCR ecClass, Utf8CP label)
    {
    return s_testNodesFactory.CreateECClassGroupingNode(connection, "", nullptr, ecClass, false, *LabelDefinition::Create(label), 0, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr TestNodesHelper::CreateRelationshipGroupingNode(IConnectionCR connection, ECRelationshipClassCR rel, Utf8CP label)
    {
    return s_testNodesFactory.CreateECRelationshipGroupingNode(connection, "", nullptr, rel, *LabelDefinition::Create(label), 0, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr TestNodesHelper::CreatePropertyGroupingNode(IConnectionCR connection, ECClassCR ecClass, ECPropertyCR ecProperty, Utf8CP label, bvector<ECValue> const& groupingValues, bool isRangeGrouping)
    {
    return s_testNodesFactory.CreateECPropertyGroupingNode(connection, "", nullptr, ecClass, ecProperty, *LabelDefinition::Create(label), "",
        GetGroupingValuesListJson(groupingValues), isRangeGrouping, 0, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr TestNodesHelper::CreateLabelGroupingNode(IConnectionCR connection, Utf8CP label)
    {
    return s_testNodesFactory.CreateDisplayLabelGroupingNode(connection, "", nullptr, *LabelDefinition::Create(label), 0, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr TestNodesHelper::CreateCustomNode(IConnectionCR connection, Utf8CP type, Utf8CP label, Utf8CP description)
    {
    NavNodePtr node = s_testNodesFactory.CreateCustomNode(connection, "", nullptr, *LabelDefinition::Create(label), description, "", type, nullptr);
    node->GetKey()->SetInstanceKeysSelectQuery(std::make_unique<PresentationQuery>(""));
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr TestNodesFactory::CreateECPropertyGroupingNode(NavNodeKeyCP parentKey, ECClassCR ecClass, ECPropertyCR ecProp, Utf8CP label, Utf8CP imageId, bvector<ECValue> const& groupingValues, bool isRangeGrouping, uint64_t groupedInstancesCount) const
    {
    return CreateECPropertyGroupingNode(parentKey, ecClass, ecProp, label, imageId,
        GetGroupingValuesListJson(groupingValues), isRangeGrouping, groupedInstancesCount);
    }
