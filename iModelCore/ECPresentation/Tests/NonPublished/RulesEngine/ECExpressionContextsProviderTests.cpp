/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/ECExpressionContextsProviderTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "../../../Source/RulesDriven/RulesEngine/ECExpressionContextsProvider.h"
#include "TestNavNode.h"
#include "TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2015
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
* @bsiclass                                     Grigas.Petraitis                01/2016
+===============+===============+===============+===============+===============+======*/
struct ECExpressionContextsProviderTests : ECPresentationTest
    {
    static ECDbTestProject* s_project;
    static IConnectionPtr s_connection;
    StubLocalState m_localState;
    TestUserSettings m_userSettings;
    Utf8String m_locale;
    
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
        BeAssert(SUCCESS == importStatus);

        BeSQLite::DbResult result = s_project->GetECDb().SaveChanges();
        BeAssert(BeSQLite::DbResult::BE_SQLITE_OK == result);
        }

    static void TearDownTestCase()
        {
        DELETE_AND_CLEAR(s_project);
        }

    void SetUp() override
        {
        ECPresentationTest::SetUp();
        s_project->GetECDb().AbandonChanges();
        m_locale = "test locale";
        }

    static Utf8String GetTestSchemaXMLString()
        {
        return
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<ECSchema schemaName=\"TestSchema\" alias=\"test\" version=\"9.3\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"
            "    <ECStructClass typeName=\"Struct1\">"
            "        <ECArrayProperty propertyName=\"Ints\" typeName=\"int\" />"
            "        <ECProperty propertyName=\"Int\" typeName=\"int\" />"
            "    </ECStructClass>"
            "    <ECStructClass typeName=\"Struct2\">"
            "        <ECStructArrayProperty propertyName=\"Structs\" typeName=\"Struct1\" />"
            "        <ECStructProperty propertyName=\"Struct\" typeName=\"Struct1\" />"
            "    </ECStructClass>"
            "    <ECEntityClass typeName=\"ClassA\">"
            "        <ECStructProperty propertyName=\"Struct\" typeName=\"Struct2\" />"
            "        <ECStructArrayProperty propertyName=\"Structs\" typeName=\"Struct2\" />"
            "        <ECProperty propertyName=\"Int\" typeName=\"int\" />"
            "        <ECArrayProperty propertyName=\"Ints\" typeName=\"int\" />"
            "        <ECProperty propertyName=\"String\" typeName=\"string\" />"
            "    </ECEntityClass>"
            "    <ECEntityClass typeName=\"DerivedA\">"
            "        <BaseClass>ClassA</BaseClass>"
            "        <ECProperty propertyName=\"DerivedInt\" typeName=\"int\" />"
            "        <ECNavigationProperty propertyName=\"A\" relationshipName=\"ClassAHasDerivedClasses\" direction=\"Backward\" />"                  
            "    </ECEntityClass>"
            "    <ECRelationshipClass typeName=\"ClassAHasDerivedClasses\" strength=\"referencing\" strengthDirection=\"forward\" modifier=\"None\">"
            "        <Source multiplicity=\"(0..1)\" polymorphic=\"False\" roleLabel=\"Class1 Has Derived Class\">"
            "            <Class class=\"ClassA\" />"
            "        </Source>"
            "        <Target multiplicity=\"(0..*)\" polymorphic=\"False\" roleLabel=\"Class1 Has Derived Class (reversed)\">"
            "            <Class class=\"DerivedA\" />"
            "        </Target>"
            "    </ECRelationshipClass>"
            "    <ECEntityClass typeName=\"Struct3\">"
            "        <ECArrayProperty propertyName=\"GetNodeRulesContext_ChildInstanceNode_GetProperty_Array\" typeName=\"int\" />"
            "        <ECProperty propertyName=\"GetNodeRulesContext_ChildInstanceNode_GetProperty_Int\" typeName=\"int\" />"
            "    </ECEntityClass>"
            "</ECSchema>";
        }
    };
ECDbTestProject* ECExpressionContextsProviderTests::s_project = nullptr;
IConnectionPtr ECExpressionContextsProviderTests::s_connection = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
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
* @bsimethod                                    Grigas.Petraitis                03/2017
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
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionContextsProviderTests, GetNodeRulesContext_RootNode_ParentNodeIsNull)
    {
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(nullptr, *s_connection, m_locale, m_userSettings, nullptr));
    Utf8CP expression = "ParentNode.IsNull";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_TRUE(resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildNode_ParentNodeIsNotNull)
    {
    TestNavNodePtr navNode = TestNodesHelper::CreateCustomNode(*s_connection, "", "TestLabel", "");
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection, m_locale, m_userSettings, nullptr));
    Utf8CP expression = "ParentNode.IsNull";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildNode_LabelAndDescriptionAreValid)
    {
    TestNavNodePtr navNode = TestNodesHelper::CreateCustomNode(*s_connection, "", "MyLabel", "MyDescription");
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection, m_locale, m_userSettings, nullptr));
    
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
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildNode_ClassNameIsValid)
    {
    TestNavNodePtr navNode = TestNodesHelper::CreateCustomNode(*s_connection, "", "MyLabel", "MyDescription");
    NavNodeExtendedData(*navNode).SetECClassId(GetSchema().GetClassCP("ClassA")->GetId());

    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection, m_locale, m_userSettings, nullptr));
    Utf8CP expression = "ParentNode.ClassName";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsString());
    ASSERT_STREQ("ClassA", resultValue.GetUtf8CP());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildNode_SchemaNameIsValid)
    {
    TestNavNodePtr navNode = TestNodesHelper::CreateCustomNode(*s_connection, "", "MyLabel", "MyDescription");
    NavNodeExtendedData(*navNode).SetECClassId(GetSchema().GetClassCP("ClassA")->GetId());

    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection, m_locale, m_userSettings, nullptr));
    Utf8CP expression = "ParentNode.SchemaName";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsString());
    ASSERT_STREQ("TestSchema", resultValue.GetUtf8CP());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildNode_SchemaMajorVersionIsValid)
    {
    TestNavNodePtr navNode = TestNodesHelper::CreateCustomNode(*s_connection, "", "MyLabel", "MyDescription");
    NavNodeExtendedData(*navNode).SetECClassId(GetSchema().GetClassCP("ClassA")->GetId());

    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection, m_locale, m_userSettings, nullptr));
    Utf8CP expression = "ParentNode.SchemaMajorVersion";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsInteger());
    ASSERT_EQ(9, resultValue.GetInteger());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildNode_SchemaMinorVersionIsValid)
    {
    TestNavNodePtr navNode = TestNodesHelper::CreateCustomNode(*s_connection, "", "MyLabel", "MyDescription");
    NavNodeExtendedData(*navNode).SetECClassId(GetSchema().GetClassCP("ClassA")->GetId());

    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection, m_locale, m_userSettings, nullptr));
    Utf8CP expression = "ParentNode.SchemaMinorVersion";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsInteger());
    ASSERT_EQ(3, resultValue.GetInteger());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildNode_InstanceId)
    {
    IECInstancePtr instance = ECInstanceTestsHelper::CreateInstance("ClassA", GetSchema());
    instance->SetInstanceId("456");
    TestNavNodePtr navNode = TestNodesHelper::CreateInstanceNode(*s_connection, *instance);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection, m_locale, m_userSettings, nullptr));    
    Utf8CP expression = "ParentNode.InstanceId";
    NodePtr node = ECEvaluator::ParseValueExpressionAndCreateTree (expression);
    ASSERT_TRUE(node.IsValid ());
    ValueResultPtr result;
    ASSERT_EQ(ExpressionStatus::Success, node->GetValue(result, *ctx));
    ECValue resultValue;
    ASSERT_EQ(ExpressionStatus::Success, result->GetECValue(resultValue));
    ASSERT_TRUE(resultValue.IsString());
    ASSERT_STREQ("456", resultValue.GetUtf8CP());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildNonInstanceNode_IsNotInstanceNode)
    {
    TestNavNodePtr navNode = TestNodesHelper::CreateCustomNode(*s_connection, "", "MyLabel", "MyDescription");
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection, m_locale, m_userSettings, nullptr));
    Utf8CP expression = "ParentNode.IsInstanceNode";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildInstanceNode_IsInstanceNode)
    {
    IECInstancePtr instance = ECInstanceTestsHelper::CreateInstance("ClassA", GetSchema());
    TestNavNodePtr navNode = TestNodesHelper::CreateInstanceNode(*s_connection, *instance);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection, m_locale, m_userSettings, nullptr));
    Utf8CP expression = "ParentNode.IsInstanceNode";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_TRUE(resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildNode_TypeIsValid)
    {
    TestNavNodePtr navNode = TestNodesHelper::CreateCustomNode(*s_connection, "TestType", "MyLabel", "MyDescription");
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection, m_locale, m_userSettings, nullptr));
    Utf8CP expression = "ParentNode.Type";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsString());
    ASSERT_STREQ("TestType", resultValue.GetUtf8CP());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildInstanceNode_IsNodeType)
    {
    IECInstancePtr instance = ECInstanceTestsHelper::CreateInstance("ClassA", GetSchema());
    TestNavNodePtr navNode = TestNodesHelper::CreateInstanceNode(*s_connection, *instance);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection, m_locale, m_userSettings, nullptr));
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
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildRelationshipClassNode_IsNodeType)
    {
    ECRelationshipClassCP rel = GetSchema().GetClassCP("ClassAHasDerivedClasses")->GetRelationshipClassCP();
    TestNavNodePtr navNode = TestNodesHelper::CreateRelationshipGroupingNode(*s_connection, *rel, "TestLabel");
    NavNodeExtendedData navNodeExtendedData(*navNode);
    navNodeExtendedData.SetParentECClassId(GetSchema().GetClassCP("ClassA")->GetId());
    navNodeExtendedData.SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection, m_locale, m_userSettings, nullptr));

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
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildClassGroupingNode_IsNodeType)
    {
    TestNavNodePtr navNode = TestNodesHelper::CreateClassGroupingNode(*s_connection, *GetSchema().GetClassCP("ClassA"), "TestLabel");
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection, m_locale, m_userSettings, nullptr));

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
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildPropertyGroupingNode_IsNodeType)
    {
    ECClassCP classA = GetSchema().GetClassCP("ClassA");
    ECPropertyCP prop = classA->GetPropertyP("String");
    TestNavNodePtr navNode = TestNodesHelper::CreatePropertyGroupingNode(*s_connection, *classA, *prop, "TestLabel", rapidjson::Value(), false);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection, m_locale, m_userSettings, nullptr));

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
* @betest                                       Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_InstanceNode_IsOfClass)
    {
    IECInstancePtr instance = ECInstanceTestsHelper::CreateInstance("DerivedA", GetSchema());
    TestNavNodePtr navNode = TestNodesHelper::CreateInstanceNode(*s_connection, *instance);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection, m_locale, m_userSettings, nullptr));
    ECValue resultValue = EvaluateAndGetResult("ParentNode.IsOfClass(\"ClassA\", \"TestSchema\")", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_TRUE(resultValue.GetBoolean());
 
    resultValue = EvaluateAndGetResult("ParentNode.IsOfClass(\"Struct1\", \"TestSchema\")", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_RelationshipClassNode_IsOfClass)
    {
    ECRelationshipClassCP rel = GetSchema().GetClassCP("ClassAHasDerivedClasses")->GetRelationshipClassCP();
    TestNavNodePtr navNode = TestNodesHelper::CreateRelationshipGroupingNode(*s_connection, *rel, "TestLabel");
    NavNodeExtendedData navNodeExtendedData(*navNode);
    navNodeExtendedData.SetParentECClassId(GetSchema().GetClassCP("ClassA")->GetId());
    navNodeExtendedData.SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection, m_locale, m_userSettings, nullptr));

    ECValue resultValue = EvaluateAndGetResult("ParentNode.IsOfClass(\"ClassAHasDerivedClasses\", \"TestSchema\")", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_TRUE(resultValue.GetBoolean());
    
    resultValue = EvaluateAndGetResult("ParentNode.IsOfClass(\"ClassA\", \"TestSchema\")", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ClassGroupingNode_IsOfClass)
    {
    TestNavNodePtr navNode = TestNodesHelper::CreateClassGroupingNode(*s_connection, *GetSchema().GetClassCP("DerivedA"), "TestLabel");
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection, m_locale, m_userSettings, nullptr));

    ECValue resultValue = EvaluateAndGetResult("ParentNode.IsOfClass(\"ClassA\", \"TestSchema\")", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_TRUE(resultValue.GetBoolean());
    
    resultValue = EvaluateAndGetResult("ParentNode.IsOfClass(\"Struct1\", \"TestSchema\")", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_PropertyGroupingNode_IsOfClass)
    {
    ECClassCP classA = GetSchema().GetClassCP("DerivedA");
    ECPropertyCP prop = classA->GetPropertyP("String");
    TestNavNodePtr navNode = TestNodesHelper::CreatePropertyGroupingNode(*s_connection, *classA, *prop, "TestLabel", rapidjson::Value(), false);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection, m_locale, m_userSettings, nullptr));

    ECValue resultValue = EvaluateAndGetResult("ParentNode.IsOfClass(\"ClassA\", \"TestSchema\")", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_TRUE(resultValue.GetBoolean());
    
    resultValue = EvaluateAndGetResult("ParentNode.IsOfClass(\"Struct1\", \"TestSchema\")", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildRelationshipClassNode_ParentClassNameIsValid)
    {
    ECRelationshipClassCP rel = GetSchema().GetClassCP("ClassAHasDerivedClasses")->GetRelationshipClassCP();
    TestNavNodePtr navNode = TestNodesHelper::CreateRelationshipGroupingNode(*s_connection, *rel, "TestLabel");
    NavNodeExtendedData navNodeExtendedData(*navNode);
    navNodeExtendedData.SetParentECClassId(GetSchema().GetClassCP("ClassA")->GetId());
    navNodeExtendedData.SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection, m_locale, m_userSettings, nullptr));

    ECValue resultValue = EvaluateAndGetResult("ParentNode.ParentClassName", *ctx);
    ASSERT_TRUE(resultValue.IsString());
    ASSERT_STREQ("ClassA", resultValue.GetUtf8CP());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildRelationshipClassNode_ParentSchemaNameIsValid)
    {
    ECRelationshipClassCP rel = GetSchema().GetClassCP("ClassAHasDerivedClasses")->GetRelationshipClassCP();
    TestNavNodePtr navNode = TestNodesHelper::CreateRelationshipGroupingNode(*s_connection, *rel, "TestLabel");
    NavNodeExtendedData navNodeExtendedData(*navNode);
    navNodeExtendedData.SetParentECClassId(GetSchema().GetClassCP("ClassA")->GetId());
    navNodeExtendedData.SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection, m_locale, m_userSettings, nullptr));

    ECValue resultValue = EvaluateAndGetResult("ParentNode.ParentSchemaName", *ctx);
    ASSERT_TRUE(resultValue.IsString());
    ASSERT_STREQ("TestSchema", resultValue.GetUtf8CP());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_ChildRelationshipClassNode_RelationshipDirectionIsValid)
    {
    ECRelationshipClassCP rel = GetSchema().GetClassCP("ClassAHasDerivedClasses")->GetRelationshipClassCP();
    TestNavNodePtr navNode = TestNodesHelper::CreateRelationshipGroupingNode(*s_connection, *rel, "TestLabel");
    NavNodeExtendedData navNodeExtendedData(*navNode);
    navNodeExtendedData.SetParentECClassId(GetSchema().GetClassCP("ClassA")->GetId());
    navNodeExtendedData.SetRelationshipDirection(ECRelatedInstanceDirection::Backward);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection, m_locale, m_userSettings, nullptr));

    ECValue resultValue = EvaluateAndGetResult("ParentNode.RelationshipDirection", *ctx);
    ASSERT_TRUE(resultValue.IsString());
    ASSERT_STREQ("Backward", resultValue.GetUtf8CP());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, GetNodeRulesContext_GroupingNode_GroupedInstancesCount)
    {
    ECClassCP classA = GetSchema().GetClassCP("ClassA");
    ECPropertyCP prop = classA->GetPropertyP("Int");
    TestNavNodePtr navNode = TestNodesHelper::CreatePropertyGroupingNode(*s_connection, *classA, *prop, "TestLabel", rapidjson::Value(123), false);
    NavNodeExtendedData navNodeExtendedData(*navNode);
    navNodeExtendedData.SetGroupedInstanceKeys(
        {
        ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)2)), 
        ECInstanceKey(ECClassId((uint64_t)3), ECInstanceId((uint64_t)4)), 
        ECInstanceKey(ECClassId((uint64_t)5), ECInstanceId((uint64_t)6))
        });
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection, m_locale, m_userSettings, nullptr));

    ECValue resultValue = EvaluateAndGetResult("ParentNode.GroupedInstancesCount", *ctx);
    ASSERT_TRUE(resultValue.IsLong());
    ASSERT_EQ(3, resultValue.GetLong());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2015
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

    TestNavNodePtr navNode = TestNodesHelper::CreateInstanceNode(*s_connection, *instance);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(navNode.get(), *s_connection, m_locale, m_userSettings, nullptr));

    ECValue value = EvaluateAndGetResult("ParentNode.ECInstance.GetNodeRulesContext_ChildInstanceNode_GetProperty_Int", *ctx);
    ASSERT_TRUE(value.IsInteger());
    ASSERT_EQ(999, value.GetInteger());

    value = EvaluateAndGetResult("ParentNode.ECInstance.GetNodeRulesContext_ChildInstanceNode_GetProperty_Array[1]", *ctx);
    ASSERT_TRUE(value.IsInteger());
    ASSERT_EQ(11, value.GetInteger());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, UserSettings_GetSettingValue)
    {
    m_userSettings.SetSettingValue("MySetting", "UserSettings_GetSettingValue");
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(nullptr, *s_connection, m_locale, m_userSettings, nullptr));

    ECValue resultValue = EvaluateAndGetResult("GetSettingValue(\"MySetting\")", *ctx);
    ASSERT_TRUE(resultValue.IsString());
    ASSERT_STREQ("UserSettings_GetSettingValue", resultValue.GetUtf8CP());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, UserSettings_GetSettingIntValue)
    {
    m_userSettings.SetSettingIntValue("MySetting", 789);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(nullptr, *s_connection, m_locale, m_userSettings, nullptr));

    ECValue resultValue = EvaluateAndGetResult("GetSettingIntValue(\"MySetting\")", *ctx);
    ASSERT_TRUE(resultValue.IsLong());
    ASSERT_EQ(789, resultValue.GetLong());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, UserSettings_GetSettingIntValues)
    {
    m_userSettings.SetSettingIntValues("MySetting", {123, 456, 789});
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(nullptr, *s_connection, m_locale, m_userSettings, nullptr));

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
* @betest                                       Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, UserSettings_GetSettingIntValues_Lambda)
    {
    m_userSettings.SetSettingIntValues("MySetting", {123, 456, 789});
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(nullptr, *s_connection, m_locale, m_userSettings, nullptr));

    ECValue value = EvaluateAndGetResult("GetSettingIntValues(\"MySetting\").AnyMatches(x => x = 456)", *ctx);
    ASSERT_TRUE(value.IsBoolean());
    ASSERT_TRUE(value.GetBoolean());

    value = EvaluateAndGetResult("GetSettingIntValues(\"MySetting\").AnyMatches(x => x > 789)", *ctx);
    ASSERT_TRUE(value.IsBoolean());
    ASSERT_FALSE(value.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, UserSettings_GetSettingBoolValue)
    {
    m_userSettings.SetSettingBoolValue("MySetting", true);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(nullptr, *s_connection, m_locale, m_userSettings, nullptr));

    ECValue resultValue = EvaluateAndGetResult("GetSettingBoolValue(\"MySetting\")", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_EQ(true, resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, UserSettings_HasSetting)
    {
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(nullptr, *s_connection, m_locale, m_userSettings, nullptr));

    ECValue resultValue = EvaluateAndGetResult("HasSetting(\"MySetting\")", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_EQ(false, resultValue.GetBoolean());

    m_userSettings.SetSettingIntValue("MySetting", 99);
    resultValue = EvaluateAndGetResult("HasSetting(\"MySetting\")", *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_EQ(true, resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, UserSettings_ReactsToChangesInStore)
    {
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(nullptr, *s_connection, m_locale, m_userSettings, nullptr));

    m_userSettings.SetSettingIntValue("MySetting", 123);
    ECValue resultValue = EvaluateAndGetResult("GetSettingIntValue(\"MySetting\")", *ctx);
    ASSERT_EQ(123, resultValue.GetLong());

    m_userSettings.SetSettingIntValue("MySetting", 456);
    resultValue = EvaluateAndGetResult("GetSettingIntValue(\"MySetting\")", *ctx);
    ASSERT_EQ(456, resultValue.GetLong());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionContextsProviderTests, GetContentRulesContext_ContentDisplayTypeMatches)
    {
    TestNodeLocater nodeLocater;
    ECExpressionContextsProvider::ContentRulesContextParameters params("MyContentType", "", false, *s_connection, m_locale, &nodeLocater, nullptr, m_userSettings, nullptr);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetContentRulesContext(params);
    Utf8CP expression = "ContentDisplayType";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsUtf8());
    ASSERT_STREQ("MyContentType", resultValue.GetUtf8CP());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionContextsProviderTests, GetContentRulesContext_SelectionProviderNameMatches)
    {
    TestNodeLocater nodeLocater;
    ECExpressionContextsProvider::ContentRulesContextParameters params("", "MySelectionProvider", false, *s_connection, m_locale, &nodeLocater, nullptr, m_userSettings, nullptr);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetContentRulesContext(params);
    Utf8CP expression = "SelectionProviderName";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsUtf8());
    ASSERT_STREQ("MySelectionProvider", resultValue.GetUtf8CP());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECExpressionContextsProviderTests, GetContentRulesContext_IsSubSelectionMatches)
    {
    TestNodeLocater nodeLocater;
    ECExpressionContextsProvider::ContentRulesContextParameters params1("", "", false, *s_connection, m_locale, &nodeLocater, nullptr, m_userSettings, nullptr);
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetContentRulesContext(params1);
    Utf8CP expression = "IsSubSelection";
    ECValue resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_FALSE(resultValue.GetBoolean());

    ECExpressionContextsProvider::ContentRulesContextParameters params2("", "", true, *s_connection, m_locale, &nodeLocater, nullptr, m_userSettings, nullptr);
    ctx = ECExpressionContextsProvider::GetContentRulesContext(params2);
    resultValue = EvaluateAndGetResult(expression, *ctx);
    ASSERT_TRUE(resultValue.IsBoolean());
    ASSERT_TRUE(resultValue.GetBoolean());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECExpressionContextsProviderTests, Common_Set)
    {
    ExpressionContextPtr ctx = ECExpressionContextsProvider::GetNodeRulesContext(ECExpressionContextsProvider::NodeRulesContextParameters(nullptr, *s_connection, m_locale, m_userSettings, nullptr));

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