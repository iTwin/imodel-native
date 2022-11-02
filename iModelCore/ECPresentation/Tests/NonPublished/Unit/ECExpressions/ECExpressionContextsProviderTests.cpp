/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentation/ECPresentationManager.h>
#include "../../../../Source/Shared/ECExpressions/ECExpressionContextsProvider.h"
#include "../../Helpers/TestNavNode.h"
#include "../../Helpers/TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECInstanceTestsHelper
    {
    static IECInstancePtr CreateInstance(Utf8CP classname, ECSchemaCR schema)
        {
        ECClassCP ecClass = schema.GetClassCP(classname);
        if (nullptr == ecClass)
            return nullptr;

        IECInstancePtr instance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
        instance->SetInstanceId("999");
        return instance;
        }
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECExpressionContextsProviderTests : ECPresentationTest
    {
    static ECDbTestProject* s_project;
    static IConnectionPtr s_connection;
    ECSqlStatementCache m_statementCache;
    RuntimeJsonLocalState m_localState;
    RulesetVariables m_rulesetVariables;
    TestNodeLocater m_nodeLocater;

    ECExpressionContextsProviderTests() : m_statementCache(10) {}

    ECSchemaCR GetSchema() {return *s_project->GetECDb().Schemas().GetSchema("TestSchema");}

    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("ECExpressionContextsProviderTests");
        s_connection = new TestConnection(s_project->GetECDb());

        ECSchemaPtr schema;
        Utf8String schemaXMLString = GetTestSchemaXMLString ();
        ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
        EXPECT_EQ (SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXMLString.c_str(), *schemaContext));

        BentleyStatus importStatus = s_project->GetECDb().Schemas().ImportSchemas({schema.get()});
        UNUSED_VARIABLE(importStatus);
        BeAssert(SUCCESS == importStatus);

        BeSQLite::DbResult result = s_project->GetECDb().SaveChanges();
        UNUSED_VARIABLE(result);
        BeAssert(BeSQLite::DbResult::BE_SQLITE_OK == result);
        }

    static void TearDownTestCase()
        {
        s_connection = nullptr;
        DELETE_AND_CLEAR(s_project);
        }

    void SetUp() override
        {
        ECPresentationTest::SetUp();
        s_project->GetECDb().AbandonChanges();
        }

    static Utf8String GetTestSchemaXMLString()
        {
        return R"XML(
            <ECSchema schemaName="TestSchema" alias="test" version="9.3" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECStructClass typeName="Struct1">
                    <ECArrayProperty propertyName="Ints" typeName="int" />
                    <ECProperty propertyName="Int" typeName="int" />
                </ECStructClass>
                <ECStructClass typeName="Struct2">
                    <ECStructArrayProperty propertyName="Structs" typeName="Struct1" />
                    <ECStructProperty propertyName="Struct" typeName="Struct1" />
                </ECStructClass>
                <ECEntityClass typeName="ClassA">
                    <ECStructProperty propertyName="Struct" typeName="Struct2" />
                    <ECStructArrayProperty propertyName="Structs" typeName="Struct2" />
                    <ECProperty propertyName="Int" typeName="int" />
                    <ECArrayProperty propertyName="Ints" typeName="int" />
                    <ECProperty propertyName="String" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="DerivedA">
                    <BaseClass>ClassA</BaseClass>
                    <ECProperty propertyName="DerivedInt" typeName="int" />
                    <ECNavigationProperty propertyName="A" relationshipName="ClassAHasDerivedClasses" direction="Backward" />
                </ECEntityClass>
                <ECEntityClass typeName="TestB">
                    <ECProperty propertyName="PropB" typeName="int" />
                </ECEntityClass>
                <ECRelationshipClass typeName="ClassAHasDerivedClasses" strength="referencing" strengthDirection="forward" modifier="None">
                    <Source multiplicity="(0..1)" polymorphic="False" roleLabel="Class1 Has Derived Class">
                        <Class class="ClassA" />
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="False" roleLabel="Class1 Has Derived Class (reversed)">
                        <Class class="DerivedA" />
                    </Target>
                </ECRelationshipClass>
                <ECEntityClass typeName="Struct3">
                    <ECArrayProperty propertyName="GetNodeRulesContext_ChildInstanceNode_GetProperty_Array" typeName="int" />
                    <ECProperty propertyName="GetNodeRulesContext_ChildInstanceNode_GetProperty_Int" typeName="int" />
                </ECEntityClass>
            </ECSchema>
        )XML";
        }
    };
ECDbTestProject* ECExpressionContextsProviderTests::s_project = nullptr;
IConnectionPtr ECExpressionContextsProviderTests::s_connection = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECValue EvaluateAndGetResult(Utf8CP expression, ExpressionContextR context)
    {
    NodePtr node = ECEvaluator::ParseValueExpressionAndCreateTree(expression);
    EXPECT_TRUE(node.IsValid());

    ValueResultPtr result;
    EXPECT_EQ(ExpressionStatus::Success, node->GetValue(result, context));

    ECValue resultValue;
    EXPECT_EQ(ExpressionStatus::Success, result->GetECValue(resultValue));

    return resultValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static RefCountedPtr<IValueListResult const> EvaluateAndGetValueListResult(Utf8CP expression, ExpressionContextR context)
    {
    NodePtr node = ECEvaluator::ParseValueExpressionAndCreateTree(expression);
    EXPECT_TRUE(node.IsValid());

    EvaluationResult result;
    EXPECT_EQ(ExpressionStatus::Success, node->GetValue(result, context));

    EXPECT_TRUE(result.IsValueList());
    return result.GetValueList();
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionContextsProviderTests, GetNodeRulesContext_RootNode_ParentNodeIsNull)
    {
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(nullptr, *s_connection,
        m_rulesetVariables, nullptr));
    Utf8CP expression = "ParentNode.IsNull";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_TRUE(resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildNode_ParentNodeIsNotNull)
    {
    auto navNode = TestNodesHelper::CreateCustomNode(*s_connection, "", "TestLabel", "");
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection,
        m_rulesetVariables, nullptr));
    Utf8CP expression = "ParentNode.IsNull";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildNode_LabelAndDescriptionAreValid)
    {
    auto navNode = TestNodesHelper::CreateCustomNode(*s_connection, "", "MyLabel", "MyDescription");
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection,
        m_rulesetVariables, nullptr));

    Utf8CP expression = "ParentNode.Label";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsString());
    ASSERT_STREQ("MyLabel", resultValue.GetUtf8CP());

    expression = "ParentNode.Description";
    resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsString());
    ASSERT_STREQ("MyDescription", resultValue.GetUtf8CP());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildNode_ClassNameIsValid)
    {
    auto navNode = TestNodesHelper::CreateCustomNode(*s_connection, "", "MyLabel", "MyDescription");
    NavNodeExtendedData(*navNode).SetECClassId(GetSchema().GetClassCP("ClassA")->GetId());

    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection,
        m_rulesetVariables, nullptr));
    Utf8CP expression = "ParentNode.ClassName";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsString());
    ASSERT_STREQ("ClassA", resultValue.GetUtf8CP());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildNode_SchemaNameIsValid)
    {
    auto navNode = TestNodesHelper::CreateCustomNode(*s_connection, "", "MyLabel", "MyDescription");
    NavNodeExtendedData(*navNode).SetECClassId(GetSchema().GetClassCP("ClassA")->GetId());

    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection,
        m_rulesetVariables, nullptr));
    Utf8CP expression = "ParentNode.SchemaName";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsString());
    ASSERT_STREQ("TestSchema", resultValue.GetUtf8CP());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildNode_SchemaMajorVersionIsValid)
    {
    auto navNode = TestNodesHelper::CreateCustomNode(*s_connection, "", "MyLabel", "MyDescription");
    NavNodeExtendedData(*navNode).SetECClassId(GetSchema().GetClassCP("ClassA")->GetId());

    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection,
        m_rulesetVariables, nullptr));
    Utf8CP expression = "ParentNode.SchemaMajorVersion";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsInteger());
    ASSERT_EQ(9, resultValue.GetInteger());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildNode_SchemaMinorVersionIsValid)
    {
    auto navNode = TestNodesHelper::CreateCustomNode(*s_connection, "", "MyLabel", "MyDescription");
    NavNodeExtendedData(*navNode).SetECClassId(GetSchema().GetClassCP("ClassA")->GetId());

    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection,
        m_rulesetVariables, nullptr));
    Utf8CP expression = "ParentNode.SchemaMinorVersion";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsInteger());
    ASSERT_EQ(3, resultValue.GetInteger());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildNode_InstanceId)
    {
    IECInstancePtr instance = ECInstanceTestsHelper::CreateInstance("ClassA", GetSchema());
    instance->SetInstanceId("456");
    auto navNode = TestNodesHelper::CreateInstanceNode(*s_connection, *instance);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection,
        m_rulesetVariables, nullptr));
    Utf8CP expression = "ParentNode.InstanceId";
    NodePtr node = ECEvaluator::ParseValueExpressionAndCreateTree (expression);
    ASSERT_TRUE(node.IsValid ());
    ValueResultPtr result;
    ASSERT_EQ(ExpressionStatus::Success, node->GetValue(result, *ctx));
    ECValue resultValue;
    ASSERT_EQ(ExpressionStatus::Success, result->GetECValue(resultValue));
    ASSERT_TRUE(resultValue.IsLong());
    ASSERT_EQ(456, resultValue.GetLong());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildNonInstanceNode_IsNotInstanceNode)
    {
    auto navNode = TestNodesHelper::CreateCustomNode(*s_connection, "", "MyLabel", "MyDescription");
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection,
        m_rulesetVariables, nullptr));
    Utf8CP expression = "ParentNode.IsInstanceNode";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildInstanceNode_IsInstanceNode)
    {
    IECInstancePtr instance = ECInstanceTestsHelper::CreateInstance("ClassA", GetSchema());
    auto navNode = TestNodesHelper::CreateInstanceNode(*s_connection, *instance);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection,
        m_rulesetVariables, nullptr));
    Utf8CP expression = "ParentNode.IsInstanceNode";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_TRUE(resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildNode_TypeIsValid)
    {
    auto navNode = TestNodesHelper::CreateCustomNode(*s_connection, "TestType", "MyLabel", "MyDescription");
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection,
        m_rulesetVariables, nullptr));
    Utf8CP expression = "ParentNode.Type";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsString());
    ASSERT_STREQ("TestType", resultValue.GetUtf8CP());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildInstanceNode_IsNodeType)
    {
    IECInstancePtr instance = ECInstanceTestsHelper::CreateInstance("ClassA", GetSchema());
    auto navNode = TestNodesHelper::CreateInstanceNode(*s_connection, *instance);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection,
        m_rulesetVariables, nullptr));
    ECValue resultValue = EvaluateAndGetResult("ParentNode.IsInstanceNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_TRUE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsClassNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsRelationshipClassNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsSchemaNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsSearchNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsClassGroupingNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());
    resultValue = EvaluateAndGetResult("ParentNode.IsPropertyGroupingNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildRelationshipClassNode_IsNodeType)
    {
    ECRelationshipClassCP rel = GetSchema().GetClassCP("ClassAHasDerivedClasses")->GetRelationshipClassCP();
    auto navNode = TestNodesHelper::CreateRelationshipGroupingNode(*s_connection, *rel, "TestLabel");
    NavNodeExtendedData navNodeExtendedData(*navNode);
    navNodeExtendedData.SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection,
        m_rulesetVariables, nullptr));

    ECValue resultValue = EvaluateAndGetResult("ParentNode.IsInstanceNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsClassNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsRelationshipClassNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_TRUE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsSchemaNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsSearchNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsClassGroupingNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsPropertyGroupingNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildClassGroupingNode_IsNodeType)
    {
    auto navNode = TestNodesHelper::CreateClassGroupingNode(*s_connection, *GetSchema().GetClassCP("ClassA"), "TestLabel");
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection,
        m_rulesetVariables, nullptr));

    ECValue resultValue = EvaluateAndGetResult("ParentNode.IsInstanceNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsClassNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsRelationshipClassNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsSchemaNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsSearchNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsClassGroupingNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_TRUE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsPropertyGroupingNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildPropertyGroupingNode_IsNodeType)
    {
    ECClassCP classA = GetSchema().GetClassCP("ClassA");
    ECPropertyCP prop = classA->GetPropertyP("String");
    auto navNode = TestNodesHelper::CreatePropertyGroupingNode(*s_connection, *classA, *prop, "TestLabel", bvector<ECValue>(), false);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection,
        m_rulesetVariables, nullptr));

    ECValue resultValue = EvaluateAndGetResult("ParentNode.IsInstanceNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsClassNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsRelationshipClassNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsSchemaNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsSearchNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsClassGroupingNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsPropertyGroupingNode", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_TRUE(resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_InstanceNode_IsOfClass)
    {
    IECInstancePtr instance = ECInstanceTestsHelper::CreateInstance("DerivedA", GetSchema());
    auto navNode = TestNodesHelper::CreateInstanceNode(*s_connection, *instance);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection,
        m_rulesetVariables, nullptr));
    ECValue resultValue = EvaluateAndGetResult("ParentNode.IsOfClass (\"ClassA\", \"TestSchema\")", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_TRUE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsOfClass(\"Struct1\", \"TestSchema\")", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionContextsProviderTests, GetNodeRulesContext_InstanceNodeMergedFromMultipleInstances_IsOfClass)
    {
    IECInstancePtr instanceA = ECInstanceTestsHelper::CreateInstance("DerivedA", GetSchema());
    IECInstancePtr instanceB = ECInstanceTestsHelper::CreateInstance("TestB", GetSchema());
    auto navNode = TestNodesHelper::CreateInstancesNode(*s_connection,
        {
        RulesEngineTestHelpers::GetInstanceKey(*instanceA),
        RulesEngineTestHelpers::GetInstanceKey(*instanceB),
        });

    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection,
        m_rulesetVariables, nullptr));
    ECValue resultValue = EvaluateAndGetResult(R"expr( ParentNode.IsOfClass("ClassA", "TestSchema") )expr", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_TRUE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult(R"expr( ParentNode.IsOfClass("TestB", "TestSchema") )expr", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_TRUE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult(R"expr( ParentNode.IsOfClass("Struct1", "TestSchema") )expr", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionContextsProviderTests, GetNodeRulesContext_Instance_IsOfClass)
    {
    ECClassCP instanceClass = GetSchema().GetClassCP("DerivedA");
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_connection->GetECDb(), *instanceClass, nullptr, true);

    auto navNode = TestNodesHelper::CreateInstanceNode(*s_connection, *instance);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection,
        m_rulesetVariables, nullptr));
    ECValue resultValue = EvaluateAndGetResult("ParentNode.ECInstance.IsOfClass (\"ClassA\", \"TestSchema\")", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_TRUE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.ECInstance.IsOfClass(\"Struct1\", \"TestSchema\")", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ClassGroupingNode_IsOfClass)
    {
    auto navNode = TestNodesHelper::CreateClassGroupingNode(*s_connection, *GetSchema().GetClassCP("DerivedA"), "TestLabel");
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection,
        m_rulesetVariables, nullptr));

    ECValue resultValue = EvaluateAndGetResult("ParentNode.IsOfClass(\"ClassA\", \"TestSchema\")", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_TRUE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsOfClass(\"Struct1\", \"TestSchema\")", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_PropertyGroupingNode_IsOfClass)
    {
    ECClassCP classA = GetSchema().GetClassCP("DerivedA");
    ECPropertyCP prop = classA->GetPropertyP("String");
    auto navNode = TestNodesHelper::CreatePropertyGroupingNode(*s_connection, *classA, *prop, "TestLabel", bvector<ECValue>(), false);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection,
        m_rulesetVariables, nullptr));

    ECValue resultValue = EvaluateAndGetResult("ParentNode.IsOfClass(\"ClassA\", \"TestSchema\")", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_TRUE(resultValue.GetBoolean());

    resultValue = EvaluateAndGetResult("ParentNode.IsOfClass(\"Struct1\", \"TestSchema\")", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_GroupingNode_GroupedInstancesCount)
    {
    ECClassCP classA = GetSchema().GetClassCP("ClassA");
    ECPropertyCP prop = classA->GetPropertyP("Int");
    auto navNode = TestNodesHelper::CreatePropertyGroupingNode(*s_connection, *classA, *prop, "TestLabel", bvector<ECValue>(), false);
    navNode->SetNodeKey(*ECPropertyGroupingNodeKey::Create(*classA, "TestLabel", navNode->GetKey()->AsECPropertyGroupingNodeKey()->GetGroupingValuesArray(), "", navNode->GetKey()->GetHashPath(), 3));
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection,
        m_rulesetVariables, nullptr));

    ECValue resultValue = EvaluateAndGetResult("ParentNode.GroupedInstancesCount", *ctx);
    ASSERT_TRUE(resultValue.IsLong());
    ASSERT_EQ(3, resultValue.GetLong());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildInstanceNode_GetProperty)
    {
    ECClassCP struct3 = GetSchema().GetClassCP("Struct3");
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *struct3, [](IECInstanceR instance)
        {
        instance.SetValue("GetNodeRulesContext_ChildInstanceNode_GetProperty_Int", ECValue(999));
        instance.AddArrayElements("GetNodeRulesContext_ChildInstanceNode_GetProperty_Array", 3);
        instance.SetValue("GetNodeRulesContext_ChildInstanceNode_GetProperty_Array", ECValue(0), 0);
        instance.SetValue("GetNodeRulesContext_ChildInstanceNode_GetProperty_Array", ECValue(11), 1);
        instance.SetValue("GetNodeRulesContext_ChildInstanceNode_GetProperty_Array", ECValue(22), 2);
        });

    auto navNode = TestNodesHelper::CreateInstanceNode(*s_connection, *instance);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection,
        m_rulesetVariables, nullptr));

    ECValue value = EvaluateAndGetResult("ParentNode.ECInstance.GetNodeRulesContext_ChildInstanceNode_GetProperty_Int", *ctx);
    ASSERT_TRUE(value.IsInteger());
    ASSERT_EQ(999, value.GetInteger());

    value = EvaluateAndGetResult("ParentNode.ECInstance.GetNodeRulesContext_ChildInstanceNode_GetProperty_Array[1]", *ctx);
    ASSERT_TRUE(value.IsInteger());
    ASSERT_EQ(11, value.GetInteger());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionContextsProviderTests, GetNodeRulesContext_GetNavigationPropertyId)
    {
    ECClassCP classA = GetSchema().GetClassCP("ClassA");
    ECClassCP classDerivedA = GetSchema().GetClassCP("DerivedA");
    ECRelationshipClassCP rel = GetSchema().GetClassCP("ClassAHasDerivedClasses")->GetRelationshipClassCP();
    IECInstancePtr instanceA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr instanceDerivedA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classDerivedA);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *instanceA, *instanceDerivedA);

    auto navNode = TestNodesHelper::CreateInstanceNode(*s_connection, *instanceDerivedA);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection,
        m_rulesetVariables, nullptr));

    ECValue value = EvaluateAndGetResult("ParentNode.ECInstance.A.Id", *ctx);
    ASSERT_TRUE(value.IsLong());
    ASSERT_EQ(ECInstanceId::FromString(instanceA->GetInstanceId().c_str()).GetValue(), value.GetLong());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionContextsProviderTests, GetNodeRulesContext_ChildrenArtifacts)
    {
    IECInstancePtr instance = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *GetSchema().GetClassCP("ClassA"));
    auto navNode = TestNodesHelper::CreateInstanceNode(*s_connection, *instance);

    bvector<NodeArtifacts> childrenArtifacts;
    NodeArtifacts childArtifacts;
    childArtifacts.Insert("IsModel", ECValue(true));
    childArtifacts.Insert("SomeId", ECValue("test"));
    childrenArtifacts.push_back(childArtifacts);
    NavNodeExtendedData(*navNode).SetChildrenArtifacts(childrenArtifacts);

    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection,
        m_rulesetVariables, nullptr));

    ECValue value = EvaluateAndGetResult("ParentNode.ChildrenArtifacts.AnyMatches(x => x.IsModel)", *ctx);
    EXPECT_TRUE(value.IsBoolean());
    EXPECT_EQ(true, value.GetBoolean());

    value = EvaluateAndGetResult("ParentNode.ChildrenArtifacts.AnyMatches(x => x.SomeId = \"test\")", *ctx);
    EXPECT_TRUE(value.IsBoolean());
    EXPECT_EQ(true, value.GetBoolean());

    value = EvaluateAndGetResult("ParentNode.ChildrenArtifacts.AnyMatches(x => x.SomeId = \"fail\")", *ctx);
    EXPECT_TRUE(value.IsBoolean());
    EXPECT_EQ(false, value.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, UserSettings_GetSettingValue)
    {
    m_rulesetVariables.SetStringValue("MySetting", "UserSettings_GetSettingValue");
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(nullptr, *s_connection,
        m_rulesetVariables, nullptr));

    ECValue resultValue = EvaluateAndGetResult("GetSettingValue(\"MySetting\")", *ctx);
    ASSERT_TRUE(resultValue.IsString());
    ASSERT_STREQ("UserSettings_GetSettingValue", resultValue.GetUtf8CP());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, UserSettings_GetSettingIntValue)
    {
    m_rulesetVariables.SetIntValue("MySetting", 789);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(nullptr, *s_connection,
        m_rulesetVariables, nullptr));

    ECValue resultValue = EvaluateAndGetResult("GetSettingIntValue(\"MySetting\")", *ctx);
    ASSERT_TRUE(resultValue.IsLong());
    ASSERT_EQ(789, resultValue.GetLong());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, UserSettings_GetSettingIntValues)
    {
    m_rulesetVariables.SetIntValues("MySetting", {123, 456, 789});
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(nullptr, *s_connection,
        m_rulesetVariables, nullptr));

    EvaluationResult valueResult;
    RefCountedPtr<IValueListResult const> result = EvaluateAndGetValueListResult("GetSettingIntValues(\"MySetting\")", *ctx);
    ASSERT_EQ(3, result->GetCount());

    EXPECT_EQ(ExpressionStatus::Success, result->GetValueAt(valueResult, 0));
    ASSERT_TRUE(valueResult.IsECValue());
    EXPECT_TRUE(valueResult.GetECValue()->IsLong());
    EXPECT_EQ(123, valueResult.GetECValue()->GetLong());

    EXPECT_EQ(ExpressionStatus::Success, result->GetValueAt(valueResult, 1));
    ASSERT_TRUE(valueResult.IsECValue());
    EXPECT_TRUE(valueResult.GetECValue()->IsLong());
    EXPECT_EQ(456, valueResult.GetECValue()->GetLong());

    EXPECT_EQ(ExpressionStatus::Success, result->GetValueAt(valueResult, 2));
    ASSERT_TRUE(valueResult.IsECValue());
    EXPECT_TRUE(valueResult.GetECValue()->IsLong());
    EXPECT_EQ(789, valueResult.GetECValue()->GetLong());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, UserSettings_GetSettingIntValues_Lambda)
    {
    m_rulesetVariables.SetIntValues("MySetting", {123, 456, 789});
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(nullptr, *s_connection,
        m_rulesetVariables, nullptr));

    ECValue value = EvaluateAndGetResult("GetSettingIntValues(\"MySetting\").AnyMatches(x => x = 456)", *ctx);
    ASSERT_TRUE(value.IsBoolean());
    ASSERT_TRUE(value.GetBoolean());

    value = EvaluateAndGetResult("GetSettingIntValues(\"MySetting\").AnyMatches(x => x > 789)", *ctx);
    ASSERT_TRUE(value.IsBoolean());
    ASSERT_FALSE(value.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, UserSettings_GetSettingBoolValue)
    {
    m_rulesetVariables.SetBoolValue("MySetting", true);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(nullptr, *s_connection,
        m_rulesetVariables, nullptr));

    ECValue resultValue = EvaluateAndGetResult("GetSettingBoolValue(\"MySetting\")", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_EQ(true, resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, UserSettings_HasSetting)
    {
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(nullptr, *s_connection,
        m_rulesetVariables, nullptr));

    ECValue resultValue = EvaluateAndGetResult("HasSetting(\"MySetting\")", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_EQ(false, resultValue.GetBoolean());

    m_rulesetVariables.SetIntValue("MySetting", 99);
    resultValue = EvaluateAndGetResult("HasSetting(\"MySetting\")", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_EQ(true, resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, UserSettings_ReactsToChangesInStore)
    {
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(nullptr, *s_connection,
        m_rulesetVariables, nullptr));

    m_rulesetVariables.SetIntValue("MySetting", 123);
    ECValue resultValue = EvaluateAndGetResult("GetSettingIntValue(\"MySetting\")", *ctx);
    ASSERT_EQ(123, resultValue.GetLong());

    m_rulesetVariables.SetIntValue("MySetting", 456);
    resultValue = EvaluateAndGetResult("GetSettingIntValue(\"MySetting\")", *ctx);
    ASSERT_EQ(456, resultValue.GetLong());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionContextsProviderTests, GetContentRulesContext_ContentDisplayTypeMatches)
    {
    TestNodeLabelCalculator testNodeLabelCalculator("test");
    ECExpressionContextsProvider::ContentRulesContextParameters params("MyContentType", "", false, *s_connection,
        "ruleset_id", &m_nodeLocater, nullptr, m_rulesetVariables, nullptr, testNodeLabelCalculator);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetContentRulesContext(params);
    Utf8CP expression = "ContentDisplayType";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsUtf8());
    ASSERT_STREQ("MyContentType", resultValue.GetUtf8CP());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionContextsProviderTests, GetContentRulesContext_SelectionProviderNameMatches)
    {
    TestNodeLabelCalculator testNodeLabelCalculator("test");
    ECExpressionContextsProvider::ContentRulesContextParameters params("", "MySelectionProvider", false, *s_connection,
        "ruleset_id", &m_nodeLocater, nullptr, m_rulesetVariables, nullptr, testNodeLabelCalculator);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetContentRulesContext(params);
    Utf8CP expression = "SelectionProviderName";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsUtf8());
    ASSERT_STREQ("MySelectionProvider", resultValue.GetUtf8CP());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionContextsProviderTests, GetContentRulesContext_IsSubSelectionMatches)
    {
    TestNodeLabelCalculator testNodeLabelCalculator("test");
    ECExpressionContextsProvider::ContentRulesContextParameters params1("", "", false, *s_connection,
        "ruleset_id", &m_nodeLocater, nullptr, m_rulesetVariables, nullptr, testNodeLabelCalculator);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetContentRulesContext(params1);
    Utf8CP expression = "IsSubSelection";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());

    ECExpressionContextsProvider::ContentRulesContextParameters params2("", "", true, *s_connection,
        "ruleset_id", &m_nodeLocater, nullptr, m_rulesetVariables, nullptr, testNodeLabelCalculator);
    ctx = ECExpressionContextsProvider::GetContentRulesContext(params2);
    resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_TRUE(resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, Common_Set)
    {
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(nullptr, *s_connection,
        m_rulesetVariables, nullptr));

    EvaluationResult valueResult;
    RefCountedPtr<IValueListResult const> result = EvaluateAndGetValueListResult("Set(1, \"2\")", *ctx);
    ASSERT_EQ(2, result->GetCount());

    EXPECT_EQ(ExpressionStatus::Success, result->GetValueAt(valueResult, 0));
    ASSERT_TRUE(valueResult.IsECValue());
    EXPECT_TRUE(valueResult.GetECValue()->IsInteger());
    EXPECT_EQ(1, valueResult.GetECValue()->GetInteger());

    EXPECT_EQ(ExpressionStatus::Success, result->GetValueAt(valueResult, 1));
    ASSERT_TRUE(valueResult.IsECValue());
    EXPECT_TRUE(valueResult.GetECValue()->IsString());
    EXPECT_STREQ("2", valueResult.GetECValue()->GetUtf8CP());
    }
