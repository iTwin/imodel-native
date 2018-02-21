/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/TestNavNode.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <UnitTests/BackDoor/ECPresentation/ECPresentationTest.h>
#include "../../../Source/RulesDriven/RulesEngine/JsonNavNode.h"
#include "../../../Source/RulesDriven/RulesEngine/ExtendedData.h"
#include <functional>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_EC

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct TestNavNode : JsonNavNode
{
friend struct TestNodesHelper;

private:
    ECClassId m_classId;
private:
    TestNavNode(IConnectionP connection, Utf8CP type) {InitNode(connection, type);}
    void InitNode(IConnectionP, Utf8CP);
public:
    static RefCountedPtr<TestNavNode> Create(IConnectionP connection = nullptr, Utf8CP type = nullptr) {return new TestNavNode(connection, type);}
    Utf8CP GetRulesetId() const {return NavNodeExtendedData(*this).GetRulesetId();}
    Utf8CP GetConnectionId() const {return NavNodeExtendedData(*this).GetConnectionId();}
};
typedef RefCountedPtr<TestNavNode> TestNavNodePtr;

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct TestNodesHelper
    {
    static TestNavNodePtr CreateTreeNode(uint64_t nodeId, uint64_t parentId);
    static TestNavNodePtr CreateInstanceNode(ECClassCR ecClass, ECInstanceId instanceId = ECInstanceId((uint64_t)123));
    static TestNavNodePtr CreateInstanceNode(IECInstanceR instance);
    static TestNavNodePtr CreateClassGroupingNode(ECClassCR ecClass, Utf8CP label);
    static TestNavNodePtr CreateRelationshipGroupingNode(ECRelationshipClassCR ecClass, Utf8CP label);
    static TestNavNodePtr CreatePropertyGroupingNode(ECClassCR ecClass, ECPropertyCR ecProperty, Utf8CP label, RapidJsonValueCR groupingValue, bool isRangeGrouping);
    static TestNavNodePtr CreateLabelGroupingNode(Utf8CP label);
    static TestNavNodePtr CreateCustomNode(Utf8CP type, Utf8CP label, Utf8CP description);
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct TestNodesFactory : JsonNavNodesFactory
{
private:
    Utf8String m_rulesetId;

public:
    TestNodesFactory(Utf8String rulesetId)
        : m_rulesetId(rulesetId)
        {}
    JsonNavNodePtr CreateECInstanceNode(IConnectionCR connection, ECClassId classId, ECInstanceId instanceId, Utf8CP label) const
        {
        JsonNavNodePtr node = JsonNavNodesFactory::CreateECInstanceNode(connection, classId, instanceId, label);
        NavNodeExtendedData(*node).SetRulesetId(m_rulesetId.c_str());
        return node;
        }
    JsonNavNodePtr CreateECInstanceNode(Utf8StringCR connectionId, IECInstanceCR instance, Utf8CP label) const
        {
        JsonNavNodePtr node = JsonNavNodesFactory::CreateECInstanceNode(connectionId, instance, label);
        NavNodeExtendedData(*node).SetRulesetId(m_rulesetId.c_str());
        return node;
        }
    JsonNavNodePtr CreateECClassGroupingNode(Utf8StringCR connectionId, ECClassCR ecClass, Utf8CP label, GroupedInstanceKeysListCR list) const
        {
        JsonNavNodePtr node = JsonNavNodesFactory::CreateECClassGroupingNode(connectionId, ecClass, label, list);
        NavNodeExtendedData(*node).SetRulesetId(m_rulesetId.c_str());
        return node;
        }
    JsonNavNodePtr CreateECRelationshipGroupingNode(Utf8StringCR connectionId, ECRelationshipClassCR ecRel, Utf8CP label, GroupedInstanceKeysListCR list) const
        {
        JsonNavNodePtr node = JsonNavNodesFactory::CreateECRelationshipGroupingNode(connectionId, ecRel, label, list);
        NavNodeExtendedData(*node).SetRulesetId(m_rulesetId.c_str());
        return node;
        }
    JsonNavNodePtr CreateECPropertyGroupingNode(Utf8StringCR connectionId, ECClassCR ecClass, ECPropertyCR ecProp, Utf8CP label, Utf8CP imageId, RapidJsonValueCR groupingValue, bool isRangeGrouping, GroupedInstanceKeysListCR list) const
        {
        JsonNavNodePtr node = JsonNavNodesFactory::CreateECPropertyGroupingNode(connectionId, ecClass, ecProp, label, imageId, groupingValue, isRangeGrouping, list);
        NavNodeExtendedData(*node).SetRulesetId(m_rulesetId.c_str());
        return node;
        }
    JsonNavNodePtr CreateDisplayLabelGroupingNode(Utf8StringCR connectionId, Utf8CP label, GroupedInstanceKeysListCR list) const
        {
        JsonNavNodePtr node = JsonNavNodesFactory::CreateDisplayLabelGroupingNode(connectionId, label, list);
        NavNodeExtendedData(*node).SetRulesetId(m_rulesetId.c_str());
        return node;
        }
    JsonNavNodePtr CreateCustomNode(Utf8StringCR connectionId, Utf8CP label, Utf8CP description, Utf8CP imageId, Utf8CP type) const
        {
        JsonNavNodePtr node = JsonNavNodesFactory::CreateCustomNode(connectionId, label, description, imageId, type);
        NavNodeExtendedData(*node).SetRulesetId(m_rulesetId.c_str());
        return node;
        }
    JsonNavNodePtr CreateFromJson(IConnectionCR connection, rapidjson::Document&& json) const
        {
        JsonNavNodePtr node = JsonNavNodesFactory::CreateFromJson(connection, std::move(json));
        NavNodeExtendedData(*node).SetRulesetId(m_rulesetId.c_str());
        return node;
        }
};

END_ECPRESENTATIONTESTS_NAMESPACE
