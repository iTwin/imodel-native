/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/TestNavNode.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TestNavNode.h"
#include <ECPresentation/Connection.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void TestNavNode::InitNode(IConnectionP connection, Utf8CP type)
    {
    SetLabel("TestLabel");
    SetType("TestType");

    NavNodeExtendedData extendedData(*this);
    extendedData.SetSpecificationHash(""); // "" is an invalid Hash so it will never match anything unless specifically set
    extendedData.SetRulesetId("Invalid ruleset ID");

    if (nullptr != connection)
        extendedData.SetConnectionId(connection->GetId());

    if (nullptr != type)
        {
        SetType(type);
        extendedData.SetECClassId(ECClassId((uint64_t)1));
        }

    SetNodeKey(*NavNodesHelper::CreateNodeKey(*this, bvector<Utf8String>()));
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
    s_testNodesFactory.InitECInstanceNode(*node, "undefined connection id", instance, "label");
    node->m_classId = instance.GetClass().GetId();
    node->SetNodeKey(*NavNodesHelper::CreateNodeKey(*node, bvector<Utf8String>()));
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TestNavNodePtr TestNodesHelper::CreateClassGroupingNode(ECClassCR ecClass, Utf8CP label)
    {
    TestNavNodePtr node = TestNavNode::Create();
    s_testNodesFactory.InitECClassGroupingNode(*node, "undefined connection id", ecClass, label, GroupedInstanceKeysList());
    node->m_classId = ecClass.GetId();
    node->SetNodeKey(*NavNodesHelper::CreateNodeKey(*node, bvector<Utf8String>()));
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TestNavNodePtr TestNodesHelper::CreateRelationshipGroupingNode(ECRelationshipClassCR rel, Utf8CP label)
    {
    TestNavNodePtr node = TestNavNode::Create();
    s_testNodesFactory.InitECRelationshipGroupingNode(*node, "undefined connection id", rel, label, GroupedInstanceKeysList());
    node->m_classId = rel.GetId();
    node->SetNodeKey(*NavNodesHelper::CreateNodeKey(*node, bvector<Utf8String>()));
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TestNavNodePtr TestNodesHelper::CreatePropertyGroupingNode(ECClassCR ecClass, ECPropertyCR ecProperty, Utf8CP label, RapidJsonValueCR groupingValue, bool isRangeGrouping)
    {
    TestNavNodePtr node = TestNavNode::Create();
    s_testNodesFactory.InitECPropertyGroupingNode(*node, "undefined connection id", ecClass, ecProperty, label, "", groupingValue, isRangeGrouping, GroupedInstanceKeysList());
    node->m_classId = ecClass.GetId();
    node->SetNodeKey(*NavNodesHelper::CreateNodeKey(*node, bvector<Utf8String>()));
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TestNavNodePtr TestNodesHelper::CreateLabelGroupingNode(Utf8CP label)
    {
    TestNavNodePtr node = TestNavNode::Create();
    s_testNodesFactory.InitDisplayLabelGroupingNode(*node, "undefined connection id", label, GroupedInstanceKeysList());
    node->SetNodeKey(*NavNodesHelper::CreateNodeKey(*node, bvector<Utf8String>()));
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TestNavNodePtr TestNodesHelper::CreateCustomNode(Utf8CP type, Utf8CP label, Utf8CP description)
    {
    TestNavNodePtr node = TestNavNode::Create();
    s_testNodesFactory.InitCustomNode(*node, "undefined connection id", label, description, "", type);
    node->SetNodeKey(*NavNodesHelper::CreateNodeKey(*node, bvector<Utf8String>()));
    return node;
    }
