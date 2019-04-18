/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
    TestNavNode(IConnectionCR connection, Utf8CP type) {InitNode(connection, type);}
    void InitNode(IConnectionCR, Utf8CP);
public:
    static RefCountedPtr<TestNavNode> Create(IConnectionCR connection, Utf8CP type = nullptr) {return new TestNavNode(connection, type);}
    Utf8CP GetRulesetId() const {return NavNodeExtendedData(*this).GetRulesetId();}
    Utf8CP GetConnectionId() const {return NavNodeExtendedData(*this).GetConnectionId();}
};
typedef RefCountedPtr<TestNavNode> TestNavNodePtr;

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct TestNodesHelper
    {
    //static TestNavNodePtr CreateTreeNode(uint64_t nodeId, uint64_t parentId);
    static TestNavNodePtr CreateInstanceNode(IConnectionCR, ECClassCR ecClass, ECInstanceId instanceId = ECInstanceId((uint64_t)123));
    static TestNavNodePtr CreateInstanceNode(IConnectionCR, IECInstanceR instance);
    static TestNavNodePtr CreateClassGroupingNode(IConnectionCR, ECClassCR ecClass, Utf8CP label);
    static TestNavNodePtr CreateRelationshipGroupingNode(IConnectionCR, ECRelationshipClassCR ecClass, Utf8CP label);
    static TestNavNodePtr CreatePropertyGroupingNode(IConnectionCR, ECClassCR ecClass, ECPropertyCR ecProperty, Utf8CP label, RapidJsonValueCR groupingValue, bool isRangeGrouping);
    static TestNavNodePtr CreateLabelGroupingNode(IConnectionCR, Utf8CP label);
    static TestNavNodePtr CreateCustomNode(IConnectionCR, Utf8CP type, Utf8CP label, Utf8CP description);
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct TestNodesFactory : JsonNavNodesFactory
{
private:
    Utf8String m_rulesetId;
    Utf8String m_locale;

public:
    TestNodesFactory(Utf8String rulesetId, Utf8String locale = "")
        : m_rulesetId(rulesetId), m_locale(locale)
        {}
    JsonNavNodePtr CreateECInstanceNode(IConnectionCR connection, ECClassId classId, ECInstanceId instanceId, Utf8CP label) const
        {
        JsonNavNodePtr node = JsonNavNodesFactory::CreateECInstanceNode(connection, m_locale, classId, instanceId, label);
        NavNodeExtendedData(*node).SetRulesetId(m_rulesetId.c_str());
        return node;
        }
    JsonNavNodePtr CreateECInstanceNode(Utf8StringCR connectionId, IECInstanceCR instance, Utf8CP label) const
        {
        JsonNavNodePtr node = JsonNavNodesFactory::CreateECInstanceNode(connectionId, m_locale, instance, label);
        NavNodeExtendedData(*node).SetRulesetId(m_rulesetId.c_str());
        return node;
        }
    JsonNavNodePtr CreateECClassGroupingNode(Utf8StringCR connectionId, ECClassCR ecClass, Utf8CP label, GroupedInstanceKeysListCR list) const
        {
        JsonNavNodePtr node = JsonNavNodesFactory::CreateECClassGroupingNode(connectionId, m_locale, ecClass, label, list);
        NavNodeExtendedData(*node).SetRulesetId(m_rulesetId.c_str());
        return node;
        }
    JsonNavNodePtr CreateECRelationshipGroupingNode(Utf8StringCR connectionId, ECRelationshipClassCR ecRel, Utf8CP label, GroupedInstanceKeysListCR list) const
        {
        JsonNavNodePtr node = JsonNavNodesFactory::CreateECRelationshipGroupingNode(connectionId, m_locale, ecRel, label, list);
        NavNodeExtendedData(*node).SetRulesetId(m_rulesetId.c_str());
        return node;
        }
    JsonNavNodePtr CreateECPropertyGroupingNode(Utf8StringCR connectionId, ECClassCR ecClass, ECPropertyCR ecProp, Utf8CP label, Utf8CP imageId, RapidJsonValueCR groupingValue, bool isRangeGrouping, GroupedInstanceKeysListCR list) const
        {
        JsonNavNodePtr node = JsonNavNodesFactory::CreateECPropertyGroupingNode(connectionId, m_locale, ecClass, ecProp, label, imageId, groupingValue, isRangeGrouping, list);
        NavNodeExtendedData(*node).SetRulesetId(m_rulesetId.c_str());
        return node;
        }
    JsonNavNodePtr CreateDisplayLabelGroupingNode(Utf8StringCR connectionId, Utf8CP label, GroupedInstanceKeysListCR list) const
        {
        JsonNavNodePtr node = JsonNavNodesFactory::CreateDisplayLabelGroupingNode(connectionId, m_locale, label, list);
        NavNodeExtendedData(*node).SetRulesetId(m_rulesetId.c_str());
        return node;
        }
    JsonNavNodePtr CreateCustomNode(Utf8StringCR connectionId, Utf8CP label, Utf8CP description, Utf8CP imageId, Utf8CP type) const
        {
        JsonNavNodePtr node = JsonNavNodesFactory::CreateCustomNode(connectionId, m_locale, label, description, imageId, type);
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
