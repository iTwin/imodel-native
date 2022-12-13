/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/Connection.h>
#include <UnitTests/ECPresentation/ECPresentationTest.h>
#include "../../../Source/Shared/ExtendedData.h"
#include "../../../Source/Hierarchies/NavNodesHelper.h"
#include "../../../Source/Hierarchies/NavNodesCache.h"
#include <functional>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_EC

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestNodesHelper
    {
    static NavNodePtr CreateInstancesNode(IConnectionCR, bvector<ECClassInstanceKey> const&);
    static NavNodePtr CreateInstancesNode(IConnectionCR, bvector<ECInstanceKey> const&);
    static NavNodePtr CreateInstanceNode(IConnectionCR, ECClassCR ecClass, ECInstanceId instanceId = ECInstanceId((uint64_t)123));
    static NavNodePtr CreateInstanceNode(IConnectionCR, IECInstanceR instance);
    static NavNodePtr CreateClassGroupingNode(IConnectionCR, ECClassCR ecClass, Utf8CP label);
    static NavNodePtr CreateRelationshipGroupingNode(IConnectionCR, ECRelationshipClassCR ecClass, Utf8CP label);
    static NavNodePtr CreatePropertyGroupingNode(IConnectionCR, ECClassCR ecClass, ECPropertyCR ecProperty, Utf8CP label, bvector<ECValue> const& groupingValues, bool isRangeGrouping);
    static NavNodePtr CreateLabelGroupingNode(IConnectionCR, Utf8CP label);
    static NavNodePtr CreateCustomNode(IConnectionCR, Utf8CP type, Utf8CP label, Utf8CP description);
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestNodesFactory : NavNodesFactory
{
private:
    IConnectionCR m_connection;
    Utf8String m_rulesetId;
    Utf8String m_specificationIdentifier;

public:
    TestNodesFactory(IConnectionCR connection, Utf8String specificationIdentifier, Utf8String rulesetId)
        : m_connection(connection), m_specificationIdentifier(specificationIdentifier), m_rulesetId(rulesetId)
        {}
    NavNodePtr CreateECInstanceNode(NavNodeKeyCP parentKey, ECClassId classId, ECInstanceId instanceId, Utf8CP label) const
        {
        NavNodePtr node = NavNodesFactory::CreateECInstanceNode(m_connection, m_specificationIdentifier, parentKey, classId, instanceId, *LabelDefinition::Create(label));
        NavNodeExtendedData(*node).SetRulesetId(m_rulesetId.c_str());
        return node;
        }
    NavNodePtr CreateECInstanceNode(NavNodeKeyCP parentKey, IECInstanceCR instance, Utf8CP label) const
        {
        NavNodePtr node = NavNodesFactory::CreateECInstanceNode(m_connection, m_specificationIdentifier, parentKey, instance, *LabelDefinition::Create(label));
        NavNodeExtendedData(*node).SetRulesetId(m_rulesetId.c_str());
        return node;
        }
    NavNodePtr CreateECClassGroupingNode(NavNodeKeyCP parentKey, ECClassCR ecClass, bool isPolymorphic, Utf8CP label, uint64_t groupedInstancesCount) const
        {
        NavNodePtr node = NavNodesFactory::CreateECClassGroupingNode(m_connection, m_specificationIdentifier, parentKey, ecClass, isPolymorphic, *LabelDefinition::Create(label), groupedInstancesCount, nullptr);
        NavNodeExtendedData(*node).SetRulesetId(m_rulesetId.c_str());
        return node;
        }
    NavNodePtr CreateECRelationshipGroupingNode(NavNodeKeyCP parentKey, ECRelationshipClassCR ecRel, Utf8CP label, uint64_t groupedInstancesCount) const
        {
        NavNodePtr node = NavNodesFactory::CreateECRelationshipGroupingNode(m_connection, m_specificationIdentifier, parentKey, ecRel, *LabelDefinition::Create(label), groupedInstancesCount, nullptr);
        NavNodeExtendedData(*node).SetRulesetId(m_rulesetId.c_str());
        return node;
        }
    NavNodePtr CreateECPropertyGroupingNode(NavNodeKeyCP parentKey, ECClassCR ecClass, ECPropertyCR ecProp, Utf8CP label, Utf8CP imageId, RapidJsonValueCR groupingValue, bool isRangeGrouping, uint64_t groupedInstancesCount) const
        {
        NavNodePtr node = NavNodesFactory::CreateECPropertyGroupingNode(m_connection, m_specificationIdentifier, parentKey, ecClass, ecProp, *LabelDefinition::Create(label), imageId, groupingValue, isRangeGrouping, groupedInstancesCount, nullptr);
        NavNodeExtendedData(*node).SetRulesetId(m_rulesetId.c_str());
        return node;
        }
    NavNodePtr CreateECPropertyGroupingNode(NavNodeKeyCP parentKey, ECClassCR ecClass, ECPropertyCR ecProp, Utf8CP label, Utf8CP imageId, bvector<ECValue> const& groupingValues, bool isRangeGrouping, uint64_t groupedInstancesCount) const;
    NavNodePtr CreateDisplayLabelGroupingNode(NavNodeKeyCP parentKey, Utf8CP label, uint64_t groupedInstancesCount) const
        {
        NavNodePtr node = NavNodesFactory::CreateDisplayLabelGroupingNode(m_connection, m_specificationIdentifier, parentKey, *LabelDefinition::Create(label), groupedInstancesCount, nullptr);
        NavNodeExtendedData(*node).SetRulesetId(m_rulesetId.c_str());
        return node;
        }
    NavNodePtr CreateDisplayLabelGroupingNode(NavNodeKeyCP parentKey, Utf8CP label, uint64_t groupedInstancesCount, bvector<ECInstanceKey> groupedInstanceKeys) const
        {
        NavNodePtr node = NavNodesFactory::CreateDisplayLabelGroupingNode(m_connection, m_specificationIdentifier, parentKey, *LabelDefinition::Create(label), groupedInstancesCount, nullptr, std::make_unique<bvector<ECInstanceKey>>(groupedInstanceKeys));
        NavNodeExtendedData(*node).SetRulesetId(m_rulesetId.c_str());
        return node;
        }
    NavNodePtr CreateCustomNode(NavNodeKeyCP parentKey, Utf8CP label, Utf8CP description, Utf8CP imageId, Utf8CP type) const
        {
        NavNodePtr node = NavNodesFactory::CreateCustomNode(m_connection, m_specificationIdentifier, parentKey, *LabelDefinition::Create(label), description, imageId, type, nullptr);
        NavNodeExtendedData(*node).SetRulesetId(m_rulesetId.c_str());
        return node;
        }
    NavNodePtr CreateFromJson(rapidjson::Document&& json) const
        {
        auto key = NavNodeKey::Create("type", m_specificationIdentifier, bvector<Utf8String>());
        NavNodePtr node = NavNodesFactory::CreateFromJson(m_connection, std::move(json), *key);
        NavNodeExtendedData(*node).SetRulesetId(m_rulesetId.c_str());
        return node;
        }
};

END_ECPRESENTATIONTESTS_NAMESPACE
