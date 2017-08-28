/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/NavigationQueryTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include "../../../Source/RulesDriven/RulesEngine/NavigationQuery.h"
#include "ECSchemaHelperTests.h"
#include "TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2015
+===============+===============+===============+===============+===============+======*/
struct NavigationQueryTests : ::testing::Test
    {
    ECSchemaReadContextPtr m_schemaContext;

    void SetUp() override
        {
        BeFileName applicationSchemaDir, temporaryDir;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(applicationSchemaDir);
        BeTest::GetHost().GetOutputRoot(temporaryDir);
        ECDb::Initialize(temporaryDir, &applicationSchemaDir);

        m_schemaContext = ECSchemaReadContext::CreateContext();
        }

    void TearDown() override
        {
        }
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2015
+===============+===============+===============+===============+===============+======*/
struct ComplexNavigationQueryTests : NavigationQueryTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_InnerJoin_SingleClause_ForwardRelationship)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_2, *m_schemaContext));

    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(class1, true);
    query->Join(RelatedClass(class1, class2, relationship1, true, "target_alias", ""), false);

    Utf8String expected(
        " FROM [sc2].[Class1]"
        " INNER JOIN [sc2].[Class2] target_alias ON [target_alias].[C1].[Id] = [Class1].[ECInstanceId]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_InnerJoin_SingleClause_BackwardRelationship)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_2, *m_schemaContext));

    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(class2, true);
    query->Join(RelatedClass(class2, class1, relationship1, false, "target_alias", ""), false);

    Utf8String expected(
        " FROM [sc2].[Class2]"
        " INNER JOIN [sc2].[Class1] target_alias ON [target_alias].[ECInstanceId] = [Class2].[C1].[Id]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_InnerJoin_SingleClause_RespectsRelationshipDirection_Forward)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass1")->GetRelationshipClassCP();
    ECEntityClassCR entity = *schema->GetClassCP("Class1")->GetEntityClassCP();

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(entity, true, "source");
    query->Join(RelatedClass(entity, entity, relationship, true, "target"), false);

    Utf8String expected(
        " FROM [sc3].[Class1] source"
        " INNER JOIN [sc3].[Class1] target ON [target].[Parent].[Id] = [source].[ECInstanceId]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_InnerJoin_SingleClause_RespectsRelationshipDirection_Backward)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass1")->GetRelationshipClassCP();
    ECEntityClassCR entity = *schema->GetClassCP("Class1")->GetEntityClassCP();

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(entity, true, "target");
    query->Join(RelatedClass(entity, entity, relationship, false, "source"), false);

    Utf8String expected(
        " FROM [sc3].[Class1] target"
        " INNER JOIN [sc3].[Class1] source ON [source].[ECInstanceId] = [target].[Parent].[Id]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_InnerJoin_MultipleClauses)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_2, *m_schemaContext));

    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECRelationshipClassCR relationship2 = *schema->GetClassCP("Class3HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(class1, true);

    RelatedClassPath path;
    path.push_back(RelatedClass(class1, class2, relationship1, true, "target_alias1", ""));
    path.push_back(RelatedClass(class2, class3, relationship2, false, "target_alias2", ""));
    query->Join(path, false);

    Utf8String expected(
        " FROM [sc2].[Class1]"
        " INNER JOIN [sc2].[Class2] target_alias1 ON [target_alias1].[C1].[Id] = [Class1].[ECInstanceId]"
        " INNER JOIN [sc2].[Class3] target_alias2 ON [target_alias2].[ECInstanceId] = [target_alias1].[C3].[Id]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_InnerJoin_MultipleClauses_DoesntIncludeMultipleTimes)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_2, *m_schemaContext));

    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECRelationshipClassCR relationship2 = *schema->GetClassCP("Class3HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(class1, true);

    RelatedClassPath path1;
    path1.push_back(RelatedClass(class1, class2, relationship1, true, "target_alias1", ""));
    query->Join(path1, false);
    
    RelatedClassPath path2;
    path2.push_back(RelatedClass(class1, class2, relationship1, true, "target_alias1", ""));
    path2.push_back(RelatedClass(class2, class3, relationship2, false, "target_alias2", ""));
    query->Join(path2, false);

    Utf8String expected(
        " FROM [sc2].[Class1]"
        " INNER JOIN [sc2].[Class2] target_alias1 ON [target_alias1].[C1].[Id] = [Class1].[ECInstanceId]"
        " INNER JOIN [sc2].[Class3] target_alias2 ON [target_alias2].[ECInstanceId] = [target_alias1].[C3].[Id]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_InnerJoin_UsesAliases)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_2, *m_schemaContext));

    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(class1, true, "Class1Alias");
    query->Join(RelatedClass(class1, class2, relationship1, true, "Class2Alias", ""), false);

    Utf8String expected(
        " FROM [sc2].[Class1] Class1Alias"
        " INNER JOIN [sc2].[Class2] Class2Alias ON [Class2Alias].[C1].[Id] = [Class1Alias].[ECInstanceId]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_InnerForwardJoin_BackwardNavigationProperty)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(class1, true, "Class1Alias");
    query->Join(RelatedClass(class1, class2, relationship, true, "Class2Alias"), false);

    Utf8String expected(
        " FROM [sc3].[Class1] Class1Alias"
        " INNER JOIN [sc3].[Class2] Class2Alias ON [Class2Alias].[Parent].[Id] = [Class1Alias].[ECInstanceId]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_InnerBackwardJoin_BackwardNavigationProperty)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(class2, true, "Class2Alias");
    query->Join(RelatedClass(class2, class1, relationship, false, "Class1Alias"), false);

    Utf8String expected(
        " FROM [sc3].[Class2] Class2Alias"
        " INNER JOIN [sc3].[Class1] Class1Alias ON [Class1Alias].[ECInstanceId] = [Class2Alias].[Parent].[Id]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_InnerBackwardJoin_ForwardNavigationProperty)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class3IsInClass1")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(class1, true, "Class1Alias");
    query->Join(RelatedClass(class1, class3, relationship, false, "Class3Alias"), false);

    Utf8String expected(
        " FROM [sc3].[Class1] Class1Alias"
        " INNER JOIN [sc3].[Class3] Class3Alias ON [Class3Alias].[Parent].[Id] = [Class1Alias].[ECInstanceId]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_InnerForwardJoin_ForwardNavigationProperty)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class3IsInClass1")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(class3, true, "Class3Alias");
    query->Join(RelatedClass(class3, class1, relationship, true, "Class1Alias"), false);

    Utf8String expected(
        " FROM [sc3].[Class3] Class3Alias"
        " INNER JOIN [sc3].[Class1] Class1Alias ON [Class1Alias].[ECInstanceId] = [Class3Alias].[Parent].[Id]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_OuterJoin_SingleClause_ForwardRelationship)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_2, *m_schemaContext));

    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(class1, true);
    query->Join(RelatedClass(class1, class2, relationship1, true, "target_alias", ""), true);

    Utf8String expected(
        " FROM [sc2].[Class1]"
        " LEFT JOIN [sc2].[Class2] target_alias ON [target_alias].[C1].[Id] = [Class1].[ECInstanceId]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_OuterJoin_SingleClause_BackwardRelationship)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_2, *m_schemaContext));

    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(class2, true);
    query->Join(RelatedClass(class2, class1, relationship1, false, "target_alias", ""), true);

    Utf8String expected(
        " FROM [sc2].[Class2]"
        " LEFT JOIN [sc2].[Class1] target_alias ON [target_alias].[ECInstanceId] = [Class2].[C1].[Id]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_OuterJoin_SingleClause_RespectsRelationshipDirection_Forward)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass1")->GetRelationshipClassCP();
    ECEntityClassCR entity = *schema->GetClassCP("Class1")->GetEntityClassCP();

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(entity, true, "source");
    query->Join(RelatedClass(entity, entity, relationship, true, "target"), true);

    Utf8String expected(
        " FROM [sc3].[Class1] source"
        " LEFT JOIN [sc3].[Class1] target ON [target].[Parent].[Id] = [source].[ECInstanceId]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_OuterJoin_SingleClause_RespectsRelationshipDirection_Backward)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass1")->GetRelationshipClassCP();
    ECEntityClassCR entity = *schema->GetClassCP("Class1")->GetEntityClassCP();

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(entity, true, "target");
    query->Join(RelatedClass(entity, entity, relationship, false, "source"), true);

    Utf8String expected(
        " FROM [sc3].[Class1] target"
        " LEFT JOIN [sc3].[Class1] source ON [source].[ECInstanceId] = [target].[Parent].[Id]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_OuterJoin_MultipleClauses)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_2, *m_schemaContext));

    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECRelationshipClassCR relationship2 = *schema->GetClassCP("Class3HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(class1, true);

    RelatedClassPath path;
    path.push_back(RelatedClass(class1, class2, relationship1, true, "target_alias1", ""));
    path.push_back(RelatedClass(class2, class3, relationship2, false, "target_alias2", ""));
    query->Join(path, true);

    Utf8String expected(
        " FROM [sc2].[Class1]"
        " LEFT JOIN [sc2].[Class2] target_alias1 ON [target_alias1].[C1].[Id] = [Class1].[ECInstanceId]"
        " LEFT JOIN [sc2].[Class3] target_alias2 ON [target_alias2].[ECInstanceId] = [target_alias1].[C3].[Id]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_OuterJoin_MultipleClauses_DoesntIncludeMultipleTimes_WithNavigationProperty)
    {
    ECSchemaPtr schema;
    BeFileName ecSchemaPath;
    BeTest::GetHost().GetDocumentsRoot (ecSchemaPath);
    ecSchemaPath.AppendToPath(L"ECPresentationTestData");
    ecSchemaPath.AppendToPath(L"RulesEngineTest.01.00.ecschema.xml");

    BeFileName ecdbSchemaPath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(ecdbSchemaPath);
    ecdbSchemaPath.AppendToPath(L"ECSchemas\\ECDb");
    SearchPathSchemaFileLocaterPtr schemaLocater;
    bvector<WString> searchPaths;
    searchPaths.push_back(ecdbSchemaPath.GetName());
    schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
    m_schemaContext->AddSchemaLocater(*schemaLocater);
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlFile(schema, ecSchemaPath.c_str(), *m_schemaContext));

    ECRelationshipClassCR ret_WidgetHasGadget = *schema->GetClassCP("WidgetHasGadget")->GetRelationshipClassCP();
    ECRelationshipClassCR ret_GadgetHasSprockets = *schema->GetClassCP("GadgetHasSprockets")->GetRelationshipClassCP();
    ECEntityClassCR ret_Widget = *schema->GetClassCP("Widget")->GetEntityClassCP();
    ECEntityClassCR ret_Gadget = *schema->GetClassCP("Gadget")->GetEntityClassCP();
    ECEntityClassCR ret_Sprocket = *schema->GetClassCP("Sprocket")->GetEntityClassCP();

    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    query->From(ret_Widget, false, "this");

    RelatedClassPath gadgetRelationshipPath;
    gadgetRelationshipPath.push_back(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "target_alias1", "relationship_alias1"));
    query->Join(gadgetRelationshipPath, true);

    RelatedClassPath sprocketRelationshipPath;
    sprocketRelationshipPath.push_back(RelatedClass(ret_Widget, ret_Gadget, ret_WidgetHasGadget, true, "target_alias1", "relationship_alias1"));
    sprocketRelationshipPath.push_back(RelatedClass(ret_Gadget, ret_Sprocket, ret_GadgetHasSprockets, true, "target_alias2", "relationship_alias2"));
    query->Join(sprocketRelationshipPath, true);

    Utf8String expected(
        " FROM ONLY [RET].[Widget] this"
        " LEFT JOIN [RET].[WidgetHasGadget] relationship_alias1 ON [this].[ECInstanceId] = [relationship_alias1].[SourceECInstanceId] AND [this].[ECClassId] = [relationship_alias1].[SourceECClassId]"
        " LEFT JOIN [RET].[Gadget] target_alias1 ON [target_alias1].[ECInstanceId] = [relationship_alias1].[TargetECInstanceId] AND [target_alias1].[ECClassId] = [relationship_alias1].[TargetECClassId]"
        " LEFT JOIN [RET].[Sprocket] target_alias2 ON [target_alias2].[Gadget].[Id] = [relationship_alias1].[TargetECInstanceId]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    m_schemaContext->RemoveSchemaLocater(*schemaLocater);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_OuterJoin_UsesAliases)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_2, *m_schemaContext));

    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(class1, true, "Class1Alias");
    query->Join(RelatedClass(class1, class2, relationship1, true, "Class2Alias", "RelationshipAlias"), true);

    Utf8String expected(
        " FROM [sc2].[Class1] Class1Alias"
        " LEFT JOIN [sc2].[Class2] Class2Alias ON [Class2Alias].[C1].[Id] = [Class1Alias].[ECInstanceId]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_OuterForwardJoin_BackwardNavigationProperty)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(class1, true, "Class1Alias");
    query->Join(RelatedClass(class1, class2, relationship, true, "Class2Alias"), true);

    Utf8String expected(
        " FROM [sc3].[Class1] Class1Alias"
        " LEFT JOIN [sc3].[Class2] Class2Alias ON [Class2Alias].[Parent].[Id] = [Class1Alias].[ECInstanceId]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_OuterBackwardJoin_BackwardNavigationProperty)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(class2, true, "Class2Alias");
    query->Join(RelatedClass(class2, class1, relationship, false, "Class1Alias"), true);

    Utf8String expected(
        " FROM [sc3].[Class2] Class2Alias"
        " LEFT JOIN [sc3].[Class1] Class1Alias ON [Class1Alias].[ECInstanceId] = [Class2Alias].[Parent].[Id]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_OuterBackwardJoin_ForwardNavigationProperty)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class3IsInClass1")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(class1, true, "Class1Alias");
    query->Join(RelatedClass(class1, class3, relationship, false, "Class3Alias"), true);

    Utf8String expected(
        " FROM [sc3].[Class1] Class1Alias"
        " LEFT JOIN [sc3].[Class3] Class3Alias ON [Class3Alias].[Parent].[Id] = [Class1Alias].[ECInstanceId]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_OuterForwardJoin_ForwardNavigationProperty)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class3IsInClass1")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(class3, true, "Class3Alias");
    query->Join(RelatedClass(class3, class1, relationship, true, "Class1Alias"), true);

    Utf8String expected(
        " FROM [sc3].[Class3] Class3Alias"
        " LEFT JOIN [sc3].[Class1] Class1Alias ON [Class1Alias].[ECInstanceId] = [Class3Alias].[Parent].[Id]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2016
+===============+===============+===============+===============+===============+======*/
struct TestContract : NavigationQueryContract
{
public:
    static Utf8CP GeneralField;
    static Utf8CP InternalField;
    static Utf8CP OuterField;
    static Utf8CP AggregateField;

private:
    RefCountedPtr<PresentationQueryContractSimpleField> m_generalField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_internalField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_outerField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_aggregateField;

private:
    TestContract()
        : m_generalField(PresentationQueryContractSimpleField::Create(GeneralField, "GetGeneralField()", false, false, false, FieldVisibility::Both)),
        m_internalField(PresentationQueryContractSimpleField::Create(InternalField, "GetInternalField()", false, false, false, FieldVisibility::Inner)),
        m_outerField(PresentationQueryContractSimpleField::Create(OuterField, "GetOuterField()", false, false, false, FieldVisibility::Outer)),
        m_aggregateField(PresentationQueryContractSimpleField::Create(AggregateField, "GetAggregateField()", false, false, true, FieldVisibility::Both))
        {
        }

protected:
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::ECInstanceNodes;}
    bvector<PresentationQueryContractFieldCPtr> _GetFields() const override
        {
        bvector<PresentationQueryContractFieldCPtr> fields;
        fields.push_back(m_generalField);
        fields.push_back(m_internalField);
        fields.push_back(m_outerField);
        fields.push_back(m_aggregateField);
        return fields;
        }
    /*virtual uint8_t _GetIndex(Utf8CP lookup) const override
        {
        static bvector<Utf8CP> fieldNames;
        if (fieldNames.empty())
            {
            fieldNames.push_back(GeneralField);
            fieldNames.push_back(InternalField);
            fieldNames.push_back(OuterField);
            fieldNames.push_back(AggregateField);
            }

        for (size_t i = 0; i < fieldNames.size(); i++)
            {
            if (fieldNames[i] == lookup)
                return (int)i;
            }

        EXPECT_TRUE(false);
        return -1;
        }*/

public:
    static RefCountedPtr<TestContract> Create() {return new TestContract();}
};
Utf8CP TestContract::GeneralField = "GeneralField";
Utf8CP TestContract::InternalField = "InternalField";
Utf8CP TestContract::OuterField = "OuterField";
Utf8CP TestContract::AggregateField = "AggregateField";
typedef RefCountedPtr<TestContract> TestContractPtr;

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, SelectAllDoesntIncludeInternalFieldsInOuterQuery)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_BASIC_2, *m_schemaContext));
    ECClassCP ecClass = schema->GetClassCP("Class2");
    ASSERT_TRUE(nullptr != ecClass);

    TestContractPtr contract = TestContract::Create();

    ComplexNavigationQueryPtr innerQuery = ComplexNavigationQuery::Create();
    innerQuery->SelectContract(*contract);
    innerQuery->From(*ecClass, true);
    innerQuery->GetResultParametersR().SetResultType(NavigationQueryResultType::ECInstanceNodes);

    ComplexNavigationQueryPtr outerQuery = ComplexNavigationQuery::Create();
    outerQuery->SelectAll().From(*innerQuery);
    outerQuery->GroupByContract(*contract);

    Utf8String expected(
        "SELECT GeneralField, GetOuterField() AS [OuterField], GetAggregateField() AS [AggregateField] "
        "FROM ("
        "SELECT GetGeneralField() AS [GeneralField], GetInternalField() AS [InternalField] "
            "FROM [b2].[Class2]"
        ") "
        "GROUP BY GeneralField");
    Utf8String str = outerQuery->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }
