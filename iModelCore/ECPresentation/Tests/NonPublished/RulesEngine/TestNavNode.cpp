/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
void TestNavNode::InitNode(IConnectionCR connection, Utf8CP type)
    {
    SetLabel("TestLabel");
    SetType("TestType");

    NavNodeExtendedData extendedData(*this);
    extendedData.SetSpecificationHash(""); // "" is an invalid Hash so it will never match anything unless specifically set
    extendedData.SetRulesetId("Invalid ruleset ID");
    extendedData.SetConnectionId(connection.GetId());

    if (nullptr != type)
        {
        SetType(type);

        if (0 == strcmp(NAVNODE_TYPE_ECInstanceNode, type) || 0 == strcmp(NAVNODE_TYPE_ECClassGroupingNode, type) || 0 == strcmp(NAVNODE_TYPE_ECPropertyGroupingNode, type))
            extendedData.SetECClassId(ECClassId((uint64_t)1));
        if (0 == strcmp(NAVNODE_TYPE_ECPropertyGroupingNode, type))
            extendedData.SetPropertyName("SomePropertyName");
        }

    SetNodeKey(*NavNodesHelper::CreateNodeKey(connection, *this, bvector<Utf8String>()));
    }

static JsonNavNodesFactory s_testNodesFactory;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TestNavNodePtr TestNodesHelper::CreateInstanceNode(IConnectionCR connection, ECClassCR ecClass, ECInstanceId instanceId)
    {
    IECInstancePtr instance = ecClass.GetDefaultStandaloneEnabler()->CreateInstance();
    instance->SetInstanceId(instanceId.ToString().c_str());
    return CreateInstanceNode(connection, *instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TestNavNodePtr TestNodesHelper::CreateInstanceNode(IConnectionCR connection, IECInstanceR instance)
    {
    TestNavNodePtr node = TestNavNode::Create(connection);
    s_testNodesFactory.InitECInstanceNode(*node, connection.GetId(), "locale", instance, "label");
    node->SetNodeKey(*NavNodesHelper::CreateNodeKey(connection, *node, bvector<Utf8String>()));
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TestNavNodePtr TestNodesHelper::CreateClassGroupingNode(IConnectionCR connection, ECClassCR ecClass, Utf8CP label)
    {
    TestNavNodePtr node = TestNavNode::Create(connection);
    s_testNodesFactory.InitECClassGroupingNode(*node, connection.GetId(), "locale", ecClass, label, GroupedInstanceKeysList());
    node->SetNodeKey(*NavNodesHelper::CreateNodeKey(connection, *node, bvector<Utf8String>()));
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TestNavNodePtr TestNodesHelper::CreateRelationshipGroupingNode(IConnectionCR connection, ECRelationshipClassCR rel, Utf8CP label)
    {
    TestNavNodePtr node = TestNavNode::Create(connection);
    s_testNodesFactory.InitECRelationshipGroupingNode(*node, connection.GetId(), "locale", rel, label, GroupedInstanceKeysList());
    node->SetNodeKey(*NavNodesHelper::CreateNodeKey(connection, *node, bvector<Utf8String>()));
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TestNavNodePtr TestNodesHelper::CreatePropertyGroupingNode(IConnectionCR connection, ECClassCR ecClass, ECPropertyCR ecProperty, Utf8CP label, RapidJsonValueCR groupingValue, bool isRangeGrouping)
    {
    TestNavNodePtr node = TestNavNode::Create(connection);
    s_testNodesFactory.InitECPropertyGroupingNode(*node, connection.GetId(), "locale", ecClass, ecProperty, label, "", groupingValue, isRangeGrouping, GroupedInstanceKeysList());
    node->SetNodeKey(*NavNodesHelper::CreateNodeKey(connection, *node, bvector<Utf8String>()));
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TestNavNodePtr TestNodesHelper::CreateLabelGroupingNode(IConnectionCR connection, Utf8CP label)
    {
    TestNavNodePtr node = TestNavNode::Create(connection);
    s_testNodesFactory.InitDisplayLabelGroupingNode(*node, connection.GetId(), "locale", label, GroupedInstanceKeysList());
    node->SetNodeKey(*NavNodesHelper::CreateNodeKey(connection, *node, bvector<Utf8String>()));
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TestNavNodePtr TestNodesHelper::CreateCustomNode(IConnectionCR connection, Utf8CP type, Utf8CP label, Utf8CP description)
    {
    TestNavNodePtr node = TestNavNode::Create(connection);
    s_testNodesFactory.InitCustomNode(*node, connection.GetId(), "locale", label, description, "", type);
    node->SetNodeKey(*NavNodesHelper::CreateNodeKey(connection, *node, bvector<Utf8String>()));
    return node;
    }
