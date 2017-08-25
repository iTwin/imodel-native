/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/TestNavNode.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TestNavNode.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void TestNavNode::InitNode(ECDb const* db)
    {
    SetLabel("TestLabel");
    SetType("TestType");

    NavNodeExtendedData extendedData(*this);
    extendedData.SetSpecificationId(-1); // -1 is an invalid ID so it will never match anything unless specifically set
    extendedData.SetRulesetId("Invalid ruleset ID");

    if (nullptr != db)
        {
        SetECDb(*db);
        extendedData.SetConnectionId(db->GetDbGuid());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyCPtr TestNavNode::_CreateKey() const
    {
    if (GetType().Equals(NAVNODE_TYPE_ECInstanceNode))
        return ECInstanceNodeKey::Create(m_classId, ECInstanceId(GetJson()[NAVNODE_InstanceId].GetUint64()));
    if (GetType().Equals(NAVNODE_TYPE_ECClassGroupingNode))
        return ECClassGroupingNodeKey::Create(GetNodeId(), m_classId);
    if (GetType().Equals(NAVNODE_TYPE_ECPropertyGroupingNode))
        {
        NavNodeExtendedData extendedData(*this);
        return ECPropertyGroupingNodeKey::Create(GetNodeId(), m_classId, extendedData.GetPropertyName(), 
            extendedData.GetPropertyValueRangeIndex(), extendedData.GetPropertyValue());
        }
    return JsonNavNode::_CreateKey();
    }

static JsonNavNodesFactory s_testNodesFactory;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TestNavNodePtr TestNodesHelper::CreateInstanceNode(ECClassCR ecClass, ECInstanceId instanceId)
    {
    IECInstancePtr instance = ecClass.GetDefaultStandaloneEnabler()->CreateInstance();
    instance->SetInstanceId(instanceId.ToString().c_str());
    return CreateInstanceNode(*instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TestNavNodePtr TestNodesHelper::CreateInstanceNode(IECInstanceR instance)
    {
    TestNavNodePtr node = TestNavNode::Create();
    s_testNodesFactory.InitECInstanceNode(*node, BeGuid(true), instance, "label");
    node->m_classId = instance.GetClass().GetId();
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TestNavNodePtr TestNodesHelper::CreateClassGroupingNode(ECClassCR ecClass, Utf8CP label)
    {
    TestNavNodePtr node = TestNavNode::Create();
    s_testNodesFactory.InitECClassGroupingNode(*node, BeGuid(true), ecClass, label, GroupedInstanceKeysList());
    node->m_classId = ecClass.GetId();
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TestNavNodePtr TestNodesHelper::CreateRelationshipGroupingNode(ECRelationshipClassCR rel, Utf8CP label)
    {
    TestNavNodePtr node = TestNavNode::Create();
    s_testNodesFactory.InitECRelationshipGroupingNode(*node, BeGuid(true), rel, label, GroupedInstanceKeysList());
    node->m_classId = rel.GetId();
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TestNavNodePtr TestNodesHelper::CreatePropertyGroupingNode(ECClassCR ecClass, ECPropertyCR ecProperty, Utf8CP label, RapidJsonValueCR groupingValue, bool isRangeGrouping)
    {
    TestNavNodePtr node = TestNavNode::Create();
    s_testNodesFactory.InitECPropertyGroupingNode(*node, BeGuid(true), ecClass, ecProperty, label, "", groupingValue, isRangeGrouping, GroupedInstanceKeysList());
    node->m_classId = ecClass.GetId();
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TestNavNodePtr TestNodesHelper::CreateLabelGroupingNode(Utf8CP label)
    {
    TestNavNodePtr node = TestNavNode::Create();
    s_testNodesFactory.InitDisplayLabelGroupingNode(*node, BeGuid(true), label, GroupedInstanceKeysList());
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TestNavNodePtr TestNodesHelper::CreateCustomNode(Utf8CP type, Utf8CP label, Utf8CP description)
    {
    TestNavNodePtr node = TestNavNode::Create();
    s_testNodesFactory.InitCustomNode(*node, BeGuid(true), label, description, "", type);
    return node;
    }
