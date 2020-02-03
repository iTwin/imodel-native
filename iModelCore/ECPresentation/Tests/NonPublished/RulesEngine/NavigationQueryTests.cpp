/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include "../../../Source/RulesDriven/RulesEngine/NavigationQuery.h"
#include "ECSchemaHelperTests.h"
#include "TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2019
+===============+===============+===============+===============+===============+======*/
struct IdsFilteringHelperTests : ::testing::Test
    {};

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IdsFilteringHelperTests, CreatesConstantFalseClauseWhenSetIsEmpty)
    {
    bvector<BeInt64Id> ids;
    IdsFilteringHelper<bvector<BeInt64Id>> helper(ids);
    EXPECT_STREQ("FALSE", helper.CreateWhereClause("any").c_str());
    EXPECT_TRUE(helper.CreateBoundValues().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IdsFilteringHelperTests, CreatesConstantTrueClauseWhenSetIsEmptyAndClauseIsInversed)
    {
    bvector<BeInt64Id> ids;
    IdsFilteringHelper<bvector<BeInt64Id>> helper(ids);
    EXPECT_STREQ("TRUE", helper.CreateWhereClause("any", true).c_str());
    EXPECT_TRUE(helper.CreateBoundValues().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IdsFilteringHelperTests, CreatesSqlInClauseWhenSetIsNotEmptyButSmallerThan100Items)
    {
    bvector<BeInt64Id> ids;
    for (uint64_t i = 0; i < 3; ++i)
        ids.push_back(BeInt64Id(i + 1));
    IdsFilteringHelper<bvector<BeInt64Id>> helper(ids);
    EXPECT_STREQ("test_id IN (?,?,?)", helper.CreateWhereClause("test_id").c_str());
    BoundQueryValuesList bindings = helper.CreateBoundValues();
    ASSERT_EQ(3, bindings.size());
    EXPECT_TRUE(bindings[0]->Equals(BoundQueryId(ids[0])));
    EXPECT_TRUE(bindings[1]->Equals(BoundQueryId(ids[1])));
    EXPECT_TRUE(bindings[2]->Equals(BoundQueryId(ids[2])));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IdsFilteringHelperTests, CreatesSqlNotInClauseWhenSetIsNotEmptyButSmallerThan100ItemsAndClauseIsInversed)
    {
    bvector<BeInt64Id> ids;
    for (uint64_t i = 0; i < 3; ++i)
        ids.push_back(BeInt64Id(i + 1));
    IdsFilteringHelper<bvector<BeInt64Id>> helper(ids);
    EXPECT_STREQ("test_id NOT IN (?,?,?)", helper.CreateWhereClause("test_id", true).c_str());
    BoundQueryValuesList bindings = helper.CreateBoundValues();
    ASSERT_EQ(3, bindings.size());
    EXPECT_TRUE(bindings[0]->Equals(BoundQueryId(ids[0])));
    EXPECT_TRUE(bindings[1]->Equals(BoundQueryId(ids[1])));
    EXPECT_TRUE(bindings[2]->Equals(BoundQueryId(ids[2])));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IdsFilteringHelperTests, CreatesInVirtualSetClauseWhenSetLargerThan100Items)
    {
    bvector<BeInt64Id> ids;
    for (uint64_t i = 0; i < 101; ++i)
        ids.push_back(BeInt64Id(i + 1));
    IdsFilteringHelper<bvector<BeInt64Id>> helper(ids);
    EXPECT_STREQ("InVirtualSet(?, test_id)", helper.CreateWhereClause("test_id").c_str());
    BoundQueryValuesList bindings = helper.CreateBoundValues();
    ASSERT_EQ(1, bindings.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IdsFilteringHelperTests, CreatesNotInVirtualSetClauseWhenSetLargerThan100ItemsAndClauseIsInversed)
    {
    bvector<BeInt64Id> ids;
    for (uint64_t i = 0; i < 101; ++i)
        ids.push_back(BeInt64Id(i + 1));
    IdsFilteringHelper<bvector<BeInt64Id>> helper(ids);
    EXPECT_STREQ("NOT InVirtualSet(?, test_id)", helper.CreateWhereClause("test_id", true).c_str());
    BoundQueryValuesList bindings = helper.CreateBoundValues();
    ASSERT_EQ(1, bindings.size());
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2015
+===============+===============+===============+===============+===============+======*/
struct NavigationQueryTests : ECPresentationTest
    {
    ECSchemaReadContextPtr m_schemaContext;

    void SetUp() override
        {
        ECPresentationTest::SetUp();
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
    DECLARE_SCHEMA_REGISTRY(ComplexNavigationQueryTests)
    static ECDbTestProject* s_project;
    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("ComplexNavigationQueryTests");
        INIT_SCHEMA_REGISTRY(s_project->GetECDb())
        }
    static void TearDownTestCase()
        {
        DELETE_AND_CLEAR(s_project);
        }
    };
ECDbTestProject* ComplexNavigationQueryTests::s_project = nullptr;
DEFINE_SCHEMA_REGISTRY(ComplexNavigationQueryTests)
#define DEFINE_NAVIGATION_QUERY_TEST_SCHEMA(name, schema_xml) DEFINE_REGISTRY_SCHEMA(ComplexNavigationQueryTests, name, schema_xml)

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_Where_WrapsConditionWithBraces)
    {
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->Where("Test1", BoundQueryValuesList());
    query->Where("Test2", BoundQueryValuesList(), true);

    Utf8String str = query->ToString();
    ASSERT_STREQ(" WHERE (Test1) AND (Test2)", str.c_str());
    }

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
    query->Join(RelatedClass(class1, SelectClass(class2, true), relationship1, true, "target_alias", "rel_alias", false));

    Utf8String expected(
        " FROM [sc2].[Class1]"
        " INNER JOIN [sc2].[Class2] [target_alias] ON [target_alias].[C1].[Id] = [Class1].[ECInstanceId]");
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
    query->Join(RelatedClass(class2, SelectClass(class1, true), relationship1, false, "target_alias", "rel_alias", false));

    Utf8String expected(
        " FROM [sc2].[Class2]"
        " INNER JOIN [sc2].[Class1] [target_alias] ON [target_alias].[ECInstanceId] = [Class2].[C1].[Id]");
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
    query->Join(RelatedClass(entity, SelectClass(entity, true), relationship, true, "target", "rel_alias", false));

    Utf8String expected(
        " FROM [sc3].[Class1] [source]"
        " INNER JOIN [sc3].[Class1] [target] ON [target].[Parent].[Id] = [source].[ECInstanceId]");
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
    query->Join(RelatedClass(entity, SelectClass(entity, true), relationship, false, "source", "rel_alias", false));

    Utf8String expected(
        " FROM [sc3].[Class1] [target]"
        " INNER JOIN [sc3].[Class1] [source] ON [source].[ECInstanceId] = [target].[Parent].[Id]");
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
    path.push_back(RelatedClass(class1, class2, relationship1, true, "target_alias1", "rel_alias1"));
    path.push_back(RelatedClass(class2, class3, relationship2, false, "target_alias2", "rel_alias2"));
    query->Join(path, false);

    Utf8String expected(
        " FROM [sc2].[Class1]"
        " LEFT JOIN [sc2].[Class2] [target_alias1] ON [target_alias1].[C1].[Id] = [Class1].[ECInstanceId]"
        " LEFT JOIN [sc2].[Class3] [target_alias2] ON [target_alias2].[ECInstanceId] = [target_alias1].[C3].[Id]");
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
    path1.push_back(RelatedClass(class1, class2, relationship1, true, "target_alias1", "rel_alias1"));
    query->Join(path1, false);
    
    RelatedClassPath path2;
    path2.push_back(RelatedClass(class1, class2, relationship1, true, "target_alias1", "rel_alias1"));
    path2.push_back(RelatedClass(class2, class3, relationship2, false, "target_alias2", "rel_alias2"));
    query->Join(path2, false);

    Utf8String expected(
        " FROM [sc2].[Class1]"
        " LEFT JOIN [sc2].[Class2] [target_alias1] ON [target_alias1].[C1].[Id] = [Class1].[ECInstanceId]"
        " LEFT JOIN [sc2].[Class3] [target_alias2] ON [target_alias2].[ECInstanceId] = [target_alias1].[C3].[Id]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_NAVIGATION_QUERY_TEST_SCHEMA(ToString_InnerJoin_MultipleClauses_DoesntIncludeRelationshipMultipleTimesEvenIfJoinedClassIsDifferent, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="Child">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_Children" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Child" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="B">
        <BaseClass>Child</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>Child</BaseClass>
    </ECEntityClass>
)*");
TEST_F(ComplexNavigationQueryTests, ToString_InnerJoin_MultipleClauses_DoesntIncludeRelationshipMultipleTimesEvenIfJoinedClassIsDifferent)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECRelationshipClassCP relationship = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_Has_Children")->GetRelationshipClassCP();

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(*classA, "a", true);
    query->Join(RelatedClass(*classA, *classB, *relationship, true, "b", "rel"));
    query->Join(RelatedClass(*classA, *classC, *relationship, true, "c", "rel"));

    Utf8String expected(
        " FROM [alias_ToString_InnerJoin_MultipleClauses_DoesntIncludeRelationshipMultipleTimesEvenIfJoinedClassIsDifferent].[A] [a]"
        " LEFT JOIN [alias_ToString_InnerJoin_MultipleClauses_DoesntIncludeRelationshipMultipleTimesEvenIfJoinedClassIsDifferent].[A_Has_Children] [rel] ON [a].[ECInstanceId] = [rel].[SourceECInstanceId]"
        " LEFT JOIN [alias_ToString_InnerJoin_MultipleClauses_DoesntIncludeRelationshipMultipleTimesEvenIfJoinedClassIsDifferent].[B] [b] ON [b].[ECInstanceId] = [rel].[TargetECInstanceId]"
        " LEFT JOIN [alias_ToString_InnerJoin_MultipleClauses_DoesntIncludeRelationshipMultipleTimesEvenIfJoinedClassIsDifferent].[C] [c] ON [c].[ECInstanceId] = [rel].[TargetECInstanceId]");
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
    query->Join(RelatedClass(class1, SelectClass(class2, true), relationship1, true, "Class2Alias", "rel_alias", false));

    Utf8String expected(
        " FROM [sc2].[Class1] [Class1Alias]"
        " INNER JOIN [sc2].[Class2] [Class2Alias] ON [Class2Alias].[C1].[Id] = [Class1Alias].[ECInstanceId]");
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
    query->Join(RelatedClass(class1, SelectClass(class2, true), relationship, true, "Class2Alias", "rel_alias", false));

    Utf8String expected(
        " FROM [sc3].[Class1] [Class1Alias]"
        " INNER JOIN [sc3].[Class2] [Class2Alias] ON [Class2Alias].[Parent].[Id] = [Class1Alias].[ECInstanceId]");
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
    query->Join(RelatedClass(class2, SelectClass(class1, true), relationship, false, "Class1Alias", "rel_alias", false));

    Utf8String expected(
        " FROM [sc3].[Class2] [Class2Alias]"
        " INNER JOIN [sc3].[Class1] [Class1Alias] ON [Class1Alias].[ECInstanceId] = [Class2Alias].[Parent].[Id]");
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
    query->Join(RelatedClass(class1, SelectClass(class3, true), relationship, false, "Class3Alias", "rel_alias", false));

    Utf8String expected(
        " FROM [sc3].[Class1] [Class1Alias]"
        " INNER JOIN [sc3].[Class3] [Class3Alias] ON [Class3Alias].[Parent].[Id] = [Class1Alias].[ECInstanceId]");
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
    query->Join(RelatedClass(class3, SelectClass(class1, true), relationship, true, "Class1Alias", "rel_alias", false));

    Utf8String expected(
        " FROM [sc3].[Class3] [Class3Alias]"
        " INNER JOIN [sc3].[Class1] [Class1Alias] ON [Class1Alias].[ECInstanceId] = [Class3Alias].[Parent].[Id]");
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
    query->Join(RelatedClass(class1, class2, relationship1, true, "target_alias", "rel_alias"));

    Utf8String expected(
        " FROM [sc2].[Class1]"
        " LEFT JOIN [sc2].[Class2] [target_alias] ON [target_alias].[C1].[Id] = [Class1].[ECInstanceId]");
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
    query->Join(RelatedClass(class2, class1, relationship1, false, "target_alias", "rel_alias"));

    Utf8String expected(
        " FROM [sc2].[Class2]"
        " LEFT JOIN [sc2].[Class1] [target_alias] ON [target_alias].[ECInstanceId] = [Class2].[C1].[Id]");
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
    query->Join(RelatedClass(entity, entity, relationship, true, "target", "rel_alias"));

    Utf8String expected(
        " FROM [sc3].[Class1] [source]"
        " LEFT JOIN [sc3].[Class1] [target] ON [target].[Parent].[Id] = [source].[ECInstanceId]");
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
    query->Join(RelatedClass(entity, entity, relationship, false, "source", "rel_alias"));

    Utf8String expected(
        " FROM [sc3].[Class1] [target]"
        " LEFT JOIN [sc3].[Class1] [source] ON [source].[ECInstanceId] = [target].[Parent].[Id]");
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
    path.push_back(RelatedClass(class1, class2, relationship1, true, "target_alias1", "rel_alias"));
    path.push_back(RelatedClass(class2, class3, relationship2, false, "target_alias2", "rel_alias"));
    query->Join(path, true);

    Utf8String expected(
        " FROM [sc2].[Class1]"
        " LEFT JOIN [sc2].[Class2] [target_alias1] ON [target_alias1].[C1].[Id] = [Class1].[ECInstanceId]"
        " LEFT JOIN [sc2].[Class3] [target_alias2] ON [target_alias2].[ECInstanceId] = [target_alias1].[C3].[Id]");
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
        " FROM ONLY [RET].[Widget] [this]"
        " LEFT JOIN [RET].[WidgetHasGadget] [relationship_alias1] ON [this].[ECInstanceId] = [relationship_alias1].[SourceECInstanceId]"
        " LEFT JOIN [RET].[Gadget] [target_alias1] ON [target_alias1].[ECInstanceId] = [relationship_alias1].[TargetECInstanceId]"
        " LEFT JOIN [RET].[Sprocket] [target_alias2] ON [target_alias2].[Gadget].[Id] = [relationship_alias1].[TargetECInstanceId]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    m_schemaContext->RemoveSchemaLocater(*schemaLocater);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_NAVIGATION_QUERY_TEST_SCHEMA(ToString_OuterJoin_MultiStepPath_WithNavigationProperty, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C">
        <ECNavigationProperty propertyName="B" relationshipName="B_Has_C" direction="Backward" />        
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_Has_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ComplexNavigationQueryTests, ToString_OuterJoin_MultiStepPath_WithNavigationProperty)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema(BeTest::GetNameOfCurrentTest());
    ECEntityClassCR classA = *schema->GetClassCP("A")->GetEntityClassCP();
    ECEntityClassCR classB = *schema->GetClassCP("B")->GetEntityClassCP();
    ECEntityClassCR classC = *schema->GetClassCP("C")->GetEntityClassCP();
    ECRelationshipClassCR relAB = *schema->GetClassCP("A_Has_B")->GetRelationshipClassCP();
    ECRelationshipClassCR relBC = *schema->GetClassCP("B_Has_C")->GetRelationshipClassCP();

    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    query->From(classA, false, "this");

    RelatedClassPath joinPath{
        RelatedClass(classA, classB, relAB, true, "b_alias", "rel_ab_alias"),
        RelatedClass(classB, classC, relBC, true, "c_alias", "rel_bc_alias"),
        };
    query->Join(joinPath, true);

    Utf8String expected(
        " FROM ONLY [alias_ToString_OuterJoin_MultiStepPath_WithNavigationProperty].[A] [this]"
        " LEFT JOIN [alias_ToString_OuterJoin_MultiStepPath_WithNavigationProperty].[A_Has_B] [rel_ab_alias] ON [this].[ECInstanceId] = [rel_ab_alias].[SourceECInstanceId]"
        " LEFT JOIN [alias_ToString_OuterJoin_MultiStepPath_WithNavigationProperty].[B] [b_alias] ON [b_alias].[ECInstanceId] = [rel_ab_alias].[TargetECInstanceId]"
        " LEFT JOIN [alias_ToString_OuterJoin_MultiStepPath_WithNavigationProperty].[C] [c_alias] ON [c_alias].[B].[Id] = [b_alias].[ECInstanceId]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
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
    query->Join(RelatedClass(class1, class2, relationship1, true, "Class2Alias", "RelationshipAlias"));

    Utf8String expected(
        " FROM [sc2].[Class1] [Class1Alias]"
        " LEFT JOIN [sc2].[Class2] [Class2Alias] ON [Class2Alias].[C1].[Id] = [Class1Alias].[ECInstanceId]");
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
    query->Join(RelatedClass(class1, class2, relationship, true, "Class2Alias", "rel_alias"));

    Utf8String expected(
        " FROM [sc3].[Class1] [Class1Alias]"
        " LEFT JOIN [sc3].[Class2] [Class2Alias] ON [Class2Alias].[Parent].[Id] = [Class1Alias].[ECInstanceId]");
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
    query->Join(RelatedClass(class2, class1, relationship, false, "Class1Alias", "rel_alias"));

    Utf8String expected(
        " FROM [sc3].[Class2] [Class2Alias]"
        " LEFT JOIN [sc3].[Class1] [Class1Alias] ON [Class1Alias].[ECInstanceId] = [Class2Alias].[Parent].[Id]");
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
    query->Join(RelatedClass(class1, class3, relationship, false, "Class3Alias", "rel_alias"));

    Utf8String expected(
        " FROM [sc3].[Class1] [Class1Alias]"
        " LEFT JOIN [sc3].[Class3] [Class3Alias] ON [Class3Alias].[Parent].[Id] = [Class1Alias].[ECInstanceId]");
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
    query->Join(RelatedClass(class3, class1, relationship, true, "Class1Alias", "rel_alias"));

    Utf8String expected(
        " FROM [sc3].[Class3] [Class3Alias]"
        " LEFT JOIN [sc3].[Class1] [Class1Alias] ON [Class1Alias].[ECInstanceId] = [Class3Alias].[Parent].[Id]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_NAVIGATION_QUERY_TEST_SCHEMA(ToString_Join_ClassWithTargetIds, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_Has_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ComplexNavigationQueryTests, ToString_Join_ClassWithTargetIds)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECRelationshipClassCP relAB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_Has_B")->GetRelationshipClassCP();

    bset<ECInstanceId> targetIds;
    targetIds.insert(ECInstanceId((uint64_t)123));
    targetIds.insert(ECInstanceId((uint64_t)456));
    
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(*classA, true, "src");
    query->Join(RelatedClass(*classA, *relAB, true, "rel", *classB, targetIds, "tgt", true));

    Utf8String expected(
        " FROM [alias_ToString_Join_ClassWithTargetIds].[A] [src]"
        " LEFT JOIN [alias_ToString_Join_ClassWithTargetIds].[A_Has_B] [rel] ON [src].[ECInstanceId] = [rel].[SourceECInstanceId]"
        " LEFT JOIN [alias_ToString_Join_ClassWithTargetIds].[B] [tgt] ON [tgt].[ECInstanceId] = [rel].[TargetECInstanceId] AND [tgt].[ECInstanceId] IN (?,?)");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_NAVIGATION_QUERY_TEST_SCHEMA(ToString_Join_PathWithTargetIds, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_Has_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_Has_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ComplexNavigationQueryTests, ToString_Join_PathWithTargetIds)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECRelationshipClassCP relAB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_Has_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B_Has_C")->GetRelationshipClassCP();

    bset<ECInstanceId> targetIds;
    targetIds.insert(ECInstanceId((uint64_t)123));

    RelatedClassPath path = {
        RelatedClass(*classA, *relAB, true, "rel_ab", *classB, targetIds, "tgt1", false),
        RelatedClass(*classB, *relBC, true, "rel_bc", *classC, "tgt2", false),
        };

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(*classA, true, "src");
    query->Join(path);

    Utf8String expected(
        " FROM [alias_ToString_Join_PathWithTargetIds].[A] [src]"
        " INNER JOIN [alias_ToString_Join_PathWithTargetIds].[A_Has_B] [rel_ab] ON [src].[ECInstanceId] = [rel_ab].[SourceECInstanceId]"
        " INNER JOIN [alias_ToString_Join_PathWithTargetIds].[B] [tgt1] ON [tgt1].[ECInstanceId] = [rel_ab].[TargetECInstanceId] AND [tgt1].[ECInstanceId] IN (?)"
        " INNER JOIN [alias_ToString_Join_PathWithTargetIds].[B_Has_C] [rel_bc] ON [tgt1].[ECInstanceId] = [rel_bc].[SourceECInstanceId]"
        " INNER JOIN [alias_ToString_Join_PathWithTargetIds].[C] [tgt2] ON [tgt2].[ECInstanceId] = [rel_bc].[TargetECInstanceId]");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_NAVIGATION_QUERY_TEST_SCHEMA(ToString_Join_ClassWithJoinClause, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(ComplexNavigationQueryTests, ToString_Join_ClassWithJoinClause)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");

    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->From(SelectClass(*classA), "a");
    query->Join(SelectClass(*classB, false), "b", QueryClauseAndBindings("[b].[Prop] > ?", { new BoundQueryECValue(ECValue(123)) }), true);

    Utf8String expected(
        " FROM [alias_ToString_Join_ClassWithJoinClause].[A] [a]"
        " LEFT JOIN ONLY [alias_ToString_Join_ClassWithJoinClause].[B] [b] ON [b].[Prop] > ?");
    Utf8String str = query->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2016
+===============+===============+===============+===============+===============+======*/
struct TestContract : NavigationQueryContract
{
private:
    bvector<PresentationQueryContractFieldCPtr> m_fields;
protected:
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::ECInstanceNodes;}
    bvector<PresentationQueryContractFieldCPtr> _GetFields() const override {return m_fields;}
public:
    static RefCountedPtr<TestContract> Create() {return new TestContract();}
    void AddField(PresentationQueryContractFieldCPtr field) {m_fields.push_back(field);}
};
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
    contract->AddField(PresentationQueryContractSimpleField::Create("GeneralField", "GetGeneralField()", false, false, FieldVisibility::Both));
    contract->AddField(PresentationQueryContractSimpleField::Create("InternalField", "GetInternalField()", false, false, FieldVisibility::Inner));
    contract->AddField(PresentationQueryContractSimpleField::Create("OuterField", "GetOuterField()", false, false, FieldVisibility::Outer));
    contract->AddField(PresentationQueryContractSimpleField::Create("AggregateField", "GetAggregateField()", false, true, FieldVisibility::Both));

    ComplexNavigationQueryPtr innerQuery = ComplexNavigationQuery::Create();
    innerQuery->SelectContract(*contract);
    innerQuery->From(*ecClass, true);
    innerQuery->GetResultParametersR().SetResultType(NavigationQueryResultType::ECInstanceNodes);

    ComplexNavigationQueryPtr outerQuery = ComplexNavigationQuery::Create();
    outerQuery->SelectAll().From(*innerQuery);
    outerQuery->GroupByContract(*contract);

    Utf8String expected(
        "SELECT [GeneralField], GetOuterField() AS [OuterField], GetAggregateField() AS [AggregateField] "
        "FROM ("
        "SELECT GetGeneralField() AS [GeneralField], GetInternalField() AS [InternalField] "
            "FROM [b2].[Class2]"
        ") "
        "GROUP BY [GeneralField]");
    Utf8String str = outerQuery->ToString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_SelectingUniqueValues_GroupsByFieldName)
    {
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    TestContractPtr contract = TestContract::Create();
    contract->AddField(PresentationQueryContractSimpleField::Create("DistinctField", "GetDistinctField()", false, false, FieldVisibility::Both));
    query->GroupByContract(*contract);

    ASSERT_FALSE(contract->IsAggregating());
    ASSERT_STREQ(" GROUP BY [DistinctField]", query->ToString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_SelectingUniqueValues_GroupsByGroupingClause)
    {
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    TestContractPtr contract = TestContract::Create();
    PresentationQueryContractFieldPtr field = PresentationQueryContractSimpleField::Create("DistinctField", "GetDistinctField()", false, false, FieldVisibility::Both);
    field->SetGroupingClause("GroupingClause");
    contract->AddField(field);
    query->GroupByContract(*contract);

    ASSERT_FALSE(contract->IsAggregating());
    ASSERT_STREQ(" GROUP BY [GroupingClause]", query->ToString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_SelectingAggregateField_AggregateFieldIsNotGroupedWhenNoOtherFieldsAreSelected)
    {
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    TestContractPtr contract = TestContract::Create();   
    contract->AddField(PresentationQueryContractSimpleField::Create("AggregateField", "GetAggregateField()", false, true, FieldVisibility::Both));
    query->SelectContract(*contract, "this");
    query->GroupByContract(*contract);

    ASSERT_TRUE(contract->IsAggregating());
    ASSERT_STREQ("SELECT GetAggregateField() AS [AggregateField]", query->ToString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_SelectingAggregateField_AggregateFieldIsGroupedByNotAggregateField)
    {
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    TestContractPtr contract = TestContract::Create();   
    contract->AddField(PresentationQueryContractSimpleField::Create("GeneralField", "GetGeneralField()", false, false, FieldVisibility::Both));
    contract->AddField(PresentationQueryContractSimpleField::Create("AggregateField", "GetAggregateField()", false, true, FieldVisibility::Both));
    query->SelectContract(*contract, "this");
    query->GroupByContract(*contract);

    ASSERT_TRUE(contract->IsAggregating());
    Utf8String expected(
        "SELECT GetGeneralField() AS [GeneralField], GetAggregateField() AS [AggregateField]"
        " GROUP BY [GeneralField]");
    ASSERT_STREQ(expected.c_str(), query->ToString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexNavigationQueryTests, ToString_SelectingMultipleAggregateFields_AggregateFieldsAreGroupedByMultipleNotAggregateFields)
    {
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    TestContractPtr contract = TestContract::Create();   
    contract->AddField(PresentationQueryContractSimpleField::Create("GeneralField1", "GetGeneralField1()", false, false, FieldVisibility::Both));
    contract->AddField(PresentationQueryContractSimpleField::Create("GeneralField2", "GetGeneralField2()", false, false, FieldVisibility::Both));
    contract->AddField(PresentationQueryContractSimpleField::Create("AggregateField1", "GetAggregateField1()", false, true, FieldVisibility::Both));
    contract->AddField(PresentationQueryContractSimpleField::Create("AggregateField2", "GetAggregateField2()", false, true, FieldVisibility::Both));
    query->SelectContract(*contract, "this");
    query->GroupByContract(*contract);

    ASSERT_TRUE(contract->IsAggregating());
    Utf8String expected(
        "SELECT GetGeneralField1() AS [GeneralField1], GetGeneralField2() AS [GeneralField2], GetAggregateField1() AS [AggregateField1], GetAggregateField2() AS [AggregateField2]"
        " GROUP BY [GeneralField1], [GeneralField2]");
    ASSERT_STREQ(expected.c_str(), query->ToString().c_str());
    }