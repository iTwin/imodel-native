/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentation/Rules/PresentationRules.h>
#include "../../../../Source/Hierarchies/NavigationQuery.h"
#include "../../Helpers/TestHelpers.h"
#include "../ECSchemaHelperTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IdsFilteringHelperTests : ::testing::Test
    {};

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IdsFilteringHelperTests, CreatesConstantFalseClauseWhenSetIsEmpty)
    {
    bvector<BeInt64Id> ids;
    ValuesFilteringHelper helper(ids);
    EXPECT_STREQ("FALSE", helper.CreateWhereClause("any").c_str());
    EXPECT_TRUE(helper.CreateBoundValues().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IdsFilteringHelperTests, CreatesConstantTrueClauseWhenSetIsEmptyAndClauseIsInversed)
    {
    bvector<BeInt64Id> ids;
    ValuesFilteringHelper helper(ids);
    EXPECT_STREQ("TRUE", helper.CreateWhereClause("any", true).c_str());
    EXPECT_TRUE(helper.CreateBoundValues().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IdsFilteringHelperTests, CreatesSqlInClauseWhenSetIsNotEmptyButSmallerThan100Items)
    {
    bvector<BeInt64Id> ids;
    for (uint64_t i = 0; i < 3; ++i)
        ids.push_back(BeInt64Id(i + 1));
    ValuesFilteringHelper helper(ids);
    EXPECT_STREQ("test_id IN (?,?,?)", helper.CreateWhereClause("test_id").c_str());
    BoundQueryValuesList bindings = helper.CreateBoundValues();
    ASSERT_EQ(3, bindings.size());
    EXPECT_TRUE(bindings[0]->Equals(BoundQueryId(ids[0])));
    EXPECT_TRUE(bindings[1]->Equals(BoundQueryId(ids[1])));
    EXPECT_TRUE(bindings[2]->Equals(BoundQueryId(ids[2])));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IdsFilteringHelperTests, CreatesSqlNotInClauseWhenSetIsNotEmptyButSmallerThan100ItemsAndClauseIsInversed)
    {
    bvector<BeInt64Id> ids;
    for (uint64_t i = 0; i < 3; ++i)
        ids.push_back(BeInt64Id(i + 1));
    ValuesFilteringHelper helper(ids);
    EXPECT_STREQ("test_id NOT IN (?,?,?)", helper.CreateWhereClause("test_id", true).c_str());
    BoundQueryValuesList bindings = helper.CreateBoundValues();
    ASSERT_EQ(3, bindings.size());
    EXPECT_TRUE(bindings[0]->Equals(BoundQueryId(ids[0])));
    EXPECT_TRUE(bindings[1]->Equals(BoundQueryId(ids[1])));
    EXPECT_TRUE(bindings[2]->Equals(BoundQueryId(ids[2])));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IdsFilteringHelperTests, CreatesInVirtualSetClauseWhenSetLargerThan100Items)
    {
    bvector<BeInt64Id> ids;
    for (uint64_t i = 0; i < 101; ++i)
        ids.push_back(BeInt64Id(i + 1));
    ValuesFilteringHelper helper(ids);
    EXPECT_STREQ("InVirtualSet(?, test_id)", helper.CreateWhereClause("test_id").c_str());
    BoundQueryValuesList bindings = helper.CreateBoundValues();
    ASSERT_EQ(1, bindings.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IdsFilteringHelperTests, CreatesNotInVirtualSetClauseWhenSetLargerThan100ItemsAndClauseIsInversed)
    {
    bvector<BeInt64Id> ids;
    for (uint64_t i = 0; i < 101; ++i)
        ids.push_back(BeInt64Id(i + 1));
    ValuesFilteringHelper helper(ids);
    EXPECT_STREQ("NOT InVirtualSet(?, test_id)", helper.CreateWhereClause("test_id", true).c_str());
    BoundQueryValuesList bindings = helper.CreateBoundValues();
    ASSERT_EQ(1, bindings.size());
    }

/*=================================================================================**//**
* @bsiclass
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
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ComplexQueryBuilderTests : NavigationQueryTests
    {
    DECLARE_SCHEMA_REGISTRY(ComplexQueryBuilderTests)
    static ECDbTestProject* s_project;
    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("ComplexQueryBuilderTests");
        INIT_SCHEMA_REGISTRY(s_project->GetECDb())
        }
    static void TearDownTestCase()
        {
        DELETE_AND_CLEAR(s_project);
        }
    };
ECDbTestProject* ComplexQueryBuilderTests::s_project = nullptr;
DEFINE_SCHEMA_REGISTRY(ComplexQueryBuilderTests)
#define DEFINE_NAVIGATION_QUERY_TEST_SCHEMA(name, schema_xml) DEFINE_REGISTRY_SCHEMA(ComplexQueryBuilderTests, name, schema_xml)

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_Where_WrapsConditionWithBraces)
    {
    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->Where("Test1", BoundQueryValuesList());
    query->Where("Test2", BoundQueryValuesList());

    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(" WHERE (Test1) AND (Test2)", str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_From_NoAliasPolymorphic)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_BASIC_1, *m_schemaContext));
    ECClassCR class1 = *schema->GetClassCP("Class1");

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(SelectClass<ECClass>(class1, "", true));

    ASSERT_STREQ(" FROM [b1].[Class1]", query->GetQuery()->GetQueryString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_From_WithAliasPolymorphic)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_BASIC_1, *m_schemaContext));
    ECClassCR class1 = *schema->GetClassCP("Class1");

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(SelectClass<ECClass>(class1, "test", true));

    ASSERT_STREQ(" FROM [b1].[Class1] [test]", query->GetQuery()->GetQueryString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_From_NonPolymorphic)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_BASIC_1, *m_schemaContext));
    ECClassCR class1 = *schema->GetClassCP("Class1");

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(SelectClass<ECClass>(class1, "", false));

    ASSERT_STREQ(" FROM ONLY [b1].[Class1]", query->GetQuery()->GetQueryString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_From_Disqualified)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_BASIC_1, *m_schemaContext));
    ECClassCR class1 = *schema->GetClassCP("Class1");

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(SelectClass<ECClass>(class1, "", true, true));

    ASSERT_STREQ(" FROM +[b1].[Class1]", query->GetQuery()->GetQueryString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_NAVIGATION_QUERY_TEST_SCHEMA(ToString_From_WithExcludes, R"*(
    <ECEntityClass typeName="A">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
)*");
TEST_F(ComplexQueryBuilderTests, ToString_From_WithExcludes)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    Utf8CP schemaAlias = classA->GetSchema().GetAlias().c_str();

    SelectClassWithExcludes<ECClass> selectClass(*classA, "a");
    selectClass.GetDerivedExcludedClasses().push_back(SelectClass<ECClass>(*classB, "b", true));
    selectClass.GetDerivedExcludedClasses().push_back(SelectClass<ECClass>(*classC, "c", false));

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(selectClass);

    ASSERT_STREQ(
        Utf8PrintfString(" FROM (SELECT * FROM [%s].[%s] [a] WHERE ([a].[ECClassId] IS NOT ([%s].[%s], ONLY [%s].[%s]))) [a]",
            schemaAlias, classA->GetName().c_str(), schemaAlias, classB->GetName().c_str(), schemaAlias, classC->GetName().c_str()).c_str(),
        query->GetQuery()->GetQueryString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_InnerJoin_SingleClause_ForwardRelationship)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_2, *m_schemaContext));

    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(SelectClass<ECClass>(class1, "", true));
    query->Join(RelatedClass(class1, SelectClass<ECRelationshipClass>(relationship1, "rel_alias"), true, SelectClass<ECClass>(class2, "target_alias", true), false));

    Utf8String expected(
        " FROM [sc2].[Class1]"
        " INNER JOIN [sc2].[Class2] [target_alias] ON [target_alias].[C1].[Id] = [Class1].[ECInstanceId]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_InnerJoin_SingleClause_BackwardRelationship)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_2, *m_schemaContext));

    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(SelectClass<ECClass>(class2, "", true));
    query->Join(RelatedClass(class2, SelectClass<ECRelationshipClass>(relationship1, "rel_alias"), false, SelectClass<ECClass>(class1, "target_alias", true), false));

    Utf8String expected(
        " FROM [sc2].[Class2]"
        " INNER JOIN [sc2].[Class1] [target_alias] ON [target_alias].[ECInstanceId] = [Class2].[C1].[Id]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_InnerJoin_SingleClause_RespectsRelationshipDirection_Forward)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass1")->GetRelationshipClassCP();
    ECEntityClassCR entity = *schema->GetClassCP("Class1")->GetEntityClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(entity, true, "source");
    query->Join(RelatedClass(entity, SelectClass<ECRelationshipClass>(relationship, "rel_alias"), true, SelectClass<ECClass>(entity, "target", true), false));

    Utf8String expected(
        " FROM [sc3].[Class1] [source]"
        " INNER JOIN [sc3].[Class1] [target] ON [target].[Parent].[Id] = [source].[ECInstanceId]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_InnerJoin_SingleClause_RespectsRelationshipDirection_Backward)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass1")->GetRelationshipClassCP();
    ECEntityClassCR entity = *schema->GetClassCP("Class1")->GetEntityClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(entity, true, "target");
    query->Join(RelatedClass(entity, SelectClass<ECRelationshipClass>(relationship, "rel_alias"), false, SelectClass<ECClass>(entity, "source", true), false));

    Utf8String expected(
        " FROM [sc3].[Class1] [target]"
        " INNER JOIN [sc3].[Class1] [source] ON [source].[ECInstanceId] = [target].[Parent].[Id]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_InnerJoin_MultipleClauses)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_2, *m_schemaContext));

    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECRelationshipClassCR relationship2 = *schema->GetClassCP("Class3HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(SelectClass<ECClass>(class1, "", true));

    RelatedClassPath path;
    path.push_back(RelatedClass(class1, SelectClass<ECRelationshipClass>(relationship1, "rel_alias1"), true, SelectClass<ECClass>(class2, "target_alias1"), false));
    path.push_back(RelatedClass(class2, SelectClass<ECRelationshipClass>(relationship2, "rel_alias2"), false, SelectClass<ECClass>(class3, "target_alias2"), false));
    query->Join(path);

    Utf8String expected(
        " FROM [sc2].[Class1]"
        " INNER JOIN [sc2].[Class2] [target_alias1] ON [target_alias1].[C1].[Id] = [Class1].[ECInstanceId]"
        " INNER JOIN [sc2].[Class3] [target_alias2] ON [target_alias2].[ECInstanceId] = [target_alias1].[C3].[Id]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_InnerJoin_MultipleClauses_DoesntIncludeMultipleTimes)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_2, *m_schemaContext));

    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECRelationshipClassCR relationship2 = *schema->GetClassCP("Class3HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(SelectClass<ECClass>(class1, "", true));

    RelatedClassPath path1;
    path1.push_back(RelatedClass(class1, SelectClass<ECRelationshipClass>(relationship1, "rel_alias1"), true, SelectClass<ECClass>(class2, "target_alias1"), false));
    query->Join(path1);

    RelatedClassPath path2;
    path2.push_back(RelatedClass(class1, SelectClass<ECRelationshipClass>(relationship1, "rel_alias1"), true, SelectClass<ECClass>(class2, "target_alias1"), false));
    path2.push_back(RelatedClass(class2, SelectClass<ECRelationshipClass>(relationship2, "rel_alias2"), false, SelectClass<ECClass>(class3, "target_alias2"), false));
    query->Join(path2);

    Utf8String expected(
        " FROM [sc2].[Class1]"
        " INNER JOIN [sc2].[Class2] [target_alias1] ON [target_alias1].[C1].[Id] = [Class1].[ECInstanceId]"
        " INNER JOIN [sc2].[Class3] [target_alias2] ON [target_alias2].[ECInstanceId] = [target_alias1].[C3].[Id]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_NAVIGATION_QUERY_TEST_SCHEMA(ToString_InnerJoin_MultipleClauses_DoesntIncludeRelationshipMultipleTimesEvenIfJoinedClassIsDifferent, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="Child">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
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
TEST_F(ComplexQueryBuilderTests, ToString_InnerJoin_MultipleClauses_DoesntIncludeRelationshipMultipleTimesEvenIfJoinedClassIsDifferent)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECRelationshipClassCP relationship = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_Has_Children")->GetRelationshipClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(*classA, true, "a");
    query->Join(RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relationship, "rel"), true, SelectClass<ECClass>(*classB, "b"), false));
    query->Join(RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relationship, "rel"), true, SelectClass<ECClass>(*classC, "c"), false));

    Utf8String expected(
        " FROM [alias_ToString_InnerJoin_MultipleClauses_DoesntIncludeRelationshipMultipleTimesEvenIfJoinedClassIsDifferent].[A] [a]"
        " INNER JOIN [alias_ToString_InnerJoin_MultipleClauses_DoesntIncludeRelationshipMultipleTimesEvenIfJoinedClassIsDifferent].[A_Has_Children] [rel] ON [a].[ECInstanceId] = [rel].[SourceECInstanceId]"
        " INNER JOIN [alias_ToString_InnerJoin_MultipleClauses_DoesntIncludeRelationshipMultipleTimesEvenIfJoinedClassIsDifferent].[B] [b] ON [b].[ECInstanceId] = [rel].[TargetECInstanceId]"
        " INNER JOIN [alias_ToString_InnerJoin_MultipleClauses_DoesntIncludeRelationshipMultipleTimesEvenIfJoinedClassIsDifferent].[C] [c] ON [c].[ECInstanceId] = [rel].[TargetECInstanceId]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_InnerJoin_UsesAliases)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_2, *m_schemaContext));

    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(class1, true, "Class1Alias");
    query->Join(RelatedClass(class1, SelectClass<ECRelationshipClass>(relationship1, "rel_alias"), true, SelectClass<ECClass>(class2, "Class2Alias", true), false));

    Utf8String expected(
        " FROM [sc2].[Class1] [Class1Alias]"
        " INNER JOIN [sc2].[Class2] [Class2Alias] ON [Class2Alias].[C1].[Id] = [Class1Alias].[ECInstanceId]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_InnerForwardJoin_BackwardNavigationProperty)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(class1, true, "Class1Alias");
    query->Join(RelatedClass(class1, SelectClass<ECRelationshipClass>(relationship, "rel_alias"), true, SelectClass<ECClass>(class2, "Class2Alias", true), false));

    Utf8String expected(
        " FROM [sc3].[Class1] [Class1Alias]"
        " INNER JOIN [sc3].[Class2] [Class2Alias] ON [Class2Alias].[Parent].[Id] = [Class1Alias].[ECInstanceId]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_InnerBackwardJoin_BackwardNavigationProperty)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(class2, true, "Class2Alias");
    query->Join(RelatedClass(class2, SelectClass<ECRelationshipClass>(relationship, "rel_alias"), false, SelectClass<ECClass>(class1, "Class1Alias", true), false));

    Utf8String expected(
        " FROM [sc3].[Class2] [Class2Alias]"
        " INNER JOIN [sc3].[Class1] [Class1Alias] ON [Class1Alias].[ECInstanceId] = [Class2Alias].[Parent].[Id]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_InnerBackwardJoin_ForwardNavigationProperty)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class3IsInClass1")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(class1, true, "Class1Alias");
    query->Join(RelatedClass(class1, SelectClass<ECRelationshipClass>(relationship, "rel_alias"), false, SelectClass<ECClass>(class3, "Class3Alias", true), false));

    Utf8String expected(
        " FROM [sc3].[Class1] [Class1Alias]"
        " INNER JOIN [sc3].[Class3] [Class3Alias] ON [Class3Alias].[Parent].[Id] = [Class1Alias].[ECInstanceId]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_InnerForwardJoin_ForwardNavigationProperty)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class3IsInClass1")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(class3, true, "Class3Alias");
    query->Join(RelatedClass(class3, SelectClass<ECRelationshipClass>(relationship, "rel_alias"), true, SelectClass<ECClass>(class1, "Class1Alias", true), false));

    Utf8String expected(
        " FROM [sc3].[Class3] [Class3Alias]"
        " INNER JOIN [sc3].[Class1] [Class1Alias] ON [Class1Alias].[ECInstanceId] = [Class3Alias].[Parent].[Id]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_OuterJoin_SingleClause_ForwardRelationship)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_2, *m_schemaContext));

    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(SelectClass<ECClass>(class1, "", true));
    query->Join(RelatedClass(class1, SelectClass<ECRelationshipClass>(relationship1, "rel_alias"), true, SelectClass<ECClass>(class2, "target_alias")));

    Utf8String expected(
        " FROM [sc2].[Class1]"
        " LEFT JOIN [sc2].[Class2] [target_alias] ON [target_alias].[C1].[Id] = [Class1].[ECInstanceId]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_OuterJoin_SingleClause_BackwardRelationship)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_2, *m_schemaContext));

    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(SelectClass<ECClass>(class2, "", true));
    query->Join(RelatedClass(class2, SelectClass<ECRelationshipClass>(relationship1, "rel_alias"), false, SelectClass<ECClass>(class1, "target_alias")));

    Utf8String expected(
        " FROM [sc2].[Class2]"
        " LEFT JOIN [sc2].[Class1] [target_alias] ON [target_alias].[ECInstanceId] = [Class2].[C1].[Id]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_OuterJoin_SingleClause_RespectsRelationshipDirection_Forward)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass1")->GetRelationshipClassCP();
    ECEntityClassCR entity = *schema->GetClassCP("Class1")->GetEntityClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(entity, true, "source");
    query->Join(RelatedClass(entity, SelectClass<ECRelationshipClass>(relationship, "rel_alias"), true, SelectClass<ECClass>(entity, "target")));

    Utf8String expected(
        " FROM [sc3].[Class1] [source]"
        " LEFT JOIN [sc3].[Class1] [target] ON [target].[Parent].[Id] = [source].[ECInstanceId]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_OuterJoin_SingleClause_RespectsRelationshipDirection_Backward)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass1")->GetRelationshipClassCP();
    ECEntityClassCR entity = *schema->GetClassCP("Class1")->GetEntityClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(entity, true, "target");
    query->Join(RelatedClass(entity, SelectClass<ECRelationshipClass>(relationship, "rel_alias"), false, SelectClass<ECClass>(entity, "source")));

    Utf8String expected(
        " FROM [sc3].[Class1] [target]"
        " LEFT JOIN [sc3].[Class1] [source] ON [source].[ECInstanceId] = [target].[Parent].[Id]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_OuterJoin_MultipleClauses)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_2, *m_schemaContext));

    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECRelationshipClassCR relationship2 = *schema->GetClassCP("Class3HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(SelectClass<ECClass>(class1, "", true));

    RelatedClassPath path;
    path.push_back(RelatedClass(class1, SelectClass<ECRelationshipClass>(relationship1, "rel_alias"), true, SelectClass<ECClass>(class2, "target_alias1")));
    path.push_back(RelatedClass(class2, SelectClass<ECRelationshipClass>(relationship2, "rel_alias"), false, SelectClass<ECClass>(class3, "target_alias2")));
    query->Join(path);

    Utf8String expected(
        " FROM [sc2].[Class1]"
        " LEFT JOIN [sc2].[Class2] [target_alias1] ON [target_alias1].[C1].[Id] = [Class1].[ECInstanceId]"
        " LEFT JOIN [sc2].[Class3] [target_alias2] ON [target_alias2].[ECInstanceId] = [target_alias1].[C3].[Id]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_OuterJoin_MultipleClauses_DoesntIncludeMultipleTimes_WithNavigationProperty)
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

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(ret_Widget, false, "this");

    RelatedClassPath gadgetRelationshipPath;
    gadgetRelationshipPath.push_back(RelatedClass(ret_Widget, SelectClass<ECRelationshipClass>(ret_WidgetHasGadget, "relationship_alias1"), true, SelectClass<ECClass>(ret_Gadget, "target_alias1")));
    query->Join(gadgetRelationshipPath);

    RelatedClassPath sprocketRelationshipPath;
    sprocketRelationshipPath.push_back(RelatedClass(ret_Widget, SelectClass<ECRelationshipClass>(ret_WidgetHasGadget, "relationship_alias1"), true, SelectClass<ECClass>(ret_Gadget, "target_alias1")));
    sprocketRelationshipPath.push_back(RelatedClass(ret_Gadget, SelectClass<ECRelationshipClass>(ret_GadgetHasSprockets, "relationship_alias2"), true, SelectClass<ECClass>(ret_Sprocket, "target_alias2")));
    query->Join(sprocketRelationshipPath);

    Utf8String expected(
        " FROM ONLY [RET].[Widget] [this]"
        " LEFT JOIN ("
            "SELECT [relationship_alias1].* "
            "FROM [RET].[WidgetHasGadget] [relationship_alias1] "
            "INNER JOIN [RET].[Gadget] [target_alias1] ON [target_alias1].[ECInstanceId] = [relationship_alias1].[TargetECInstanceId]"
        ") [relationship_alias1] ON [this].[ECInstanceId] = [relationship_alias1].[SourceECInstanceId]"
        " LEFT JOIN [RET].[Gadget] [target_alias1] ON [target_alias1].[ECInstanceId] = [relationship_alias1].[TargetECInstanceId]"
        " LEFT JOIN [RET].[Sprocket] [target_alias2] ON [target_alias2].[Gadget].[Id] = [relationship_alias1].[TargetECInstanceId]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    m_schemaContext->RemoveSchemaLocater(*schemaLocater);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
TEST_F(ComplexQueryBuilderTests, ToString_OuterJoin_MultiStepPath_WithNavigationProperty)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema(BeTest::GetNameOfCurrentTest());
    ECEntityClassCR classA = *schema->GetClassCP("A")->GetEntityClassCP();
    ECEntityClassCR classB = *schema->GetClassCP("B")->GetEntityClassCP();
    ECEntityClassCR classC = *schema->GetClassCP("C")->GetEntityClassCP();
    ECRelationshipClassCR relAB = *schema->GetClassCP("A_Has_B")->GetRelationshipClassCP();
    ECRelationshipClassCR relBC = *schema->GetClassCP("B_Has_C")->GetRelationshipClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(classA, false, "this");

    RelatedClassPath joinPath{
        RelatedClass(classA, SelectClass<ECRelationshipClass>(relAB, "rel_ab_alias"), true, SelectClass<ECClass>(classB, "b_alias")),
        RelatedClass(classB, SelectClass<ECRelationshipClass>(relBC, "rel_bc_alias"), true, SelectClass<ECClass>(classC, "c_alias")),
        };
    query->Join(joinPath);

    Utf8String expected(
        " FROM ONLY [alias_ToString_OuterJoin_MultiStepPath_WithNavigationProperty].[A] [this]"
        " LEFT JOIN ("
            "SELECT [rel_ab_alias].* "
            "FROM [alias_ToString_OuterJoin_MultiStepPath_WithNavigationProperty].[A_Has_B] [rel_ab_alias] "
            "INNER JOIN [alias_ToString_OuterJoin_MultiStepPath_WithNavigationProperty].[B] [b_alias] ON [b_alias].[ECInstanceId] = [rel_ab_alias].[TargetECInstanceId]"
        ") [rel_ab_alias] ON [this].[ECInstanceId] = [rel_ab_alias].[SourceECInstanceId]"
        " LEFT JOIN [alias_ToString_OuterJoin_MultiStepPath_WithNavigationProperty].[B] [b_alias] ON [b_alias].[ECInstanceId] = [rel_ab_alias].[TargetECInstanceId]"
        " LEFT JOIN [alias_ToString_OuterJoin_MultiStepPath_WithNavigationProperty].[C] [c_alias] ON [c_alias].[B].[Id] = [b_alias].[ECInstanceId]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_OuterJoin_UsesAliases)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_2, *m_schemaContext));

    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(class1, true, "Class1Alias");
    query->Join(RelatedClass(class1, SelectClass<ECRelationshipClass>(relationship1, "rel_alias"), true, SelectClass<ECClass>(class2, "Class2Alias")));

    Utf8String expected(
        " FROM [sc2].[Class1] [Class1Alias]"
        " LEFT JOIN [sc2].[Class2] [Class2Alias] ON [Class2Alias].[C1].[Id] = [Class1Alias].[ECInstanceId]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_OuterForwardJoin_BackwardNavigationProperty)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(class1, true, "Class1Alias");
    query->Join(RelatedClass(class1, SelectClass<ECRelationshipClass>(relationship, "rel_alias"), true, SelectClass<ECClass>(class2, "Class2Alias")));

    Utf8String expected(
        " FROM [sc3].[Class1] [Class1Alias]"
        " LEFT JOIN [sc3].[Class2] [Class2Alias] ON [Class2Alias].[Parent].[Id] = [Class1Alias].[ECInstanceId]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_OuterBackwardJoin_BackwardNavigationProperty)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(class2, true, "Class2Alias");
    query->Join(RelatedClass(class2, SelectClass<ECRelationshipClass>(relationship, "rel_alias"), false, SelectClass<ECClass>(class1, "Class1Alias")));

    Utf8String expected(
        " FROM [sc3].[Class2] [Class2Alias]"
        " LEFT JOIN [sc3].[Class1] [Class1Alias] ON [Class1Alias].[ECInstanceId] = [Class2Alias].[Parent].[Id]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_OuterBackwardJoin_ForwardNavigationProperty)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class3IsInClass1")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(class1, true, "Class1Alias");
    query->Join(RelatedClass(class1, SelectClass<ECRelationshipClass>(relationship, "rel_alias"), false, SelectClass<ECClass>(class3, "Class3Alias")));

    Utf8String expected(
        " FROM [sc3].[Class1] [Class1Alias]"
        " LEFT JOIN [sc3].[Class3] [Class3Alias] ON [Class3Alias].[Parent].[Id] = [Class1Alias].[ECInstanceId]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_OuterForwardJoin_ForwardNavigationProperty)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *m_schemaContext));

    ECRelationshipClassCR relationship = *schema->GetClassCP("Class3IsInClass1")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(class3, true, "Class3Alias");
    query->Join(RelatedClass(class3, SelectClass<ECRelationshipClass>(relationship, "rel_alias"), true, SelectClass<ECClass>(class1, "Class1Alias")));

    Utf8String expected(
        " FROM [sc3].[Class3] [Class3Alias]"
        " LEFT JOIN [sc3].[Class1] [Class1Alias] ON [Class1Alias].[ECInstanceId] = [Class3Alias].[Parent].[Id]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_NAVIGATION_QUERY_TEST_SCHEMA(ToString_Join_WithTargetInstanceFilter, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="PropC" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="bc" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ComplexQueryBuilderTests, ToString_Join_WithTargetInstanceFilter)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECRelationshipClassCP relAB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B_C")->GetRelationshipClassCP();

    RelatedClass rc1(*classA, SelectClass<ECRelationshipClass>(*relAB, "rel_ab"), true, SelectClass<ECClass>(*classB, "tgt1"), false);
    rc1.SetTargetInstanceFilter("tgt1.PropB > 10 AND tgt1.PropB <= 20");
    RelatedClass rc2(*classB, SelectClass<ECRelationshipClass>(*relBC, "rel_bc"), true, SelectClass<ECClass>(*classC, "tgt2"), true);
    rc2.SetTargetInstanceFilter("tgt2.PropC LIKE 'xxx%'");
    RelatedClassPath path = { rc1, rc2 };

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(*classA, true, "src");
    query->Join(path);

    Utf8String expected(
        " FROM [alias_ToString_Join_WithTargetInstanceFilter].[A] [src]"
        " INNER JOIN [alias_ToString_Join_WithTargetInstanceFilter].[A_B] [rel_ab] ON [src].[ECInstanceId] = [rel_ab].[SourceECInstanceId]"
        " INNER JOIN [alias_ToString_Join_WithTargetInstanceFilter].[B] [tgt1] ON [tgt1].[ECInstanceId] = [rel_ab].[TargetECInstanceId] AND (tgt1.PropB > 10 AND tgt1.PropB <= 20)"
        " LEFT JOIN ("
        "SELECT [rel_bc].* "
        "FROM [alias_ToString_Join_WithTargetInstanceFilter].[B_C] [rel_bc] "
        "INNER JOIN [alias_ToString_Join_WithTargetInstanceFilter].[C] [tgt2] ON [tgt2].[ECInstanceId] = [rel_bc].[TargetECInstanceId] AND (tgt2.PropC LIKE 'xxx%')"
        ") [rel_bc] ON [tgt1].[ECInstanceId] = [rel_bc].[SourceECInstanceId]"
        " LEFT JOIN [alias_ToString_Join_WithTargetInstanceFilter].[C] [tgt2] ON [tgt2].[ECInstanceId] = [rel_bc].[TargetECInstanceId] AND (tgt2.PropC LIKE 'xxx%')"
    );
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_NAVIGATION_QUERY_TEST_SCHEMA(ToString_Join_NavigationProperty_WithTargetInstanceFilter, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="int" />
        <ECNavigationProperty propertyName="NavPropA" relationshipName="A_B" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ComplexQueryBuilderTests, ToString_Join_NavigationProperty_WithTargetInstanceFilter)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECRelationshipClassCP relAB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();

    RelatedClass rc(*classA, SelectClass<ECRelationshipClass>(*relAB, "rel_ab"), true, SelectClass<ECClass>(*classB, "tgt"), false);
    rc.SetTargetInstanceFilter("tgt.PropB = 999");
    RelatedClassPath path = { rc };

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(*classA, true, "src");
    query->Join(path);

    Utf8String expected(
        " FROM [alias_ToString_Join_NavigationProperty_WithTargetInstanceFilter].[A] [src]"
        " INNER JOIN [alias_ToString_Join_NavigationProperty_WithTargetInstanceFilter].[B] [tgt] ON [tgt].[NavPropA].[Id] = [src].[ECInstanceId] AND (tgt.PropB = 999)"
    );
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
TEST_F(ComplexQueryBuilderTests, ToString_Join_ClassWithTargetIds)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECRelationshipClassCP relAB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_Has_B")->GetRelationshipClassCP();

    bvector<ECInstanceId> targetIds;
    targetIds.push_back(ECInstanceId((uint64_t)123));
    targetIds.push_back(ECInstanceId((uint64_t)456));

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(*classA, true, "src");
    query->Join(RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, "rel"), true, SelectClass<ECClass>(*classB, "tgt"), targetIds));

    Utf8String expected(
        " FROM [alias_ToString_Join_ClassWithTargetIds].[A] [src]"
        " LEFT JOIN ("
            "SELECT [rel].* "
            "FROM [alias_ToString_Join_ClassWithTargetIds].[A_Has_B] [rel] "
            "INNER JOIN [alias_ToString_Join_ClassWithTargetIds].[B] [tgt] ON [tgt].[ECInstanceId] = [rel].[TargetECInstanceId] AND ([tgt].[ECInstanceId] IN (?,?))"
        ") [rel] ON [src].[ECInstanceId] = [rel].[SourceECInstanceId]"
        " LEFT JOIN [alias_ToString_Join_ClassWithTargetIds].[B] [tgt] ON [tgt].[ECInstanceId] = [rel].[TargetECInstanceId] AND ([tgt].[ECInstanceId] IN (?,?))");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
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
TEST_F(ComplexQueryBuilderTests, ToString_Join_PathWithTargetIds)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECRelationshipClassCP relAB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_Has_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B_Has_C")->GetRelationshipClassCP();

    bvector<ECInstanceId> targetIds = { ECInstanceId((uint64_t)123) };

    RelatedClassPath path = {
        RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, "rel_ab"), true, SelectClass<ECClass>(*classB, "tgt1"), targetIds, false),
        RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relBC, "rel_bc"), true, SelectClass<ECClass>(*classC, "tgt2"), false),
        };

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(*classA, true, "src");
    query->Join(path);

    Utf8String expected(
        " FROM [alias_ToString_Join_PathWithTargetIds].[A] [src]"
        " INNER JOIN [alias_ToString_Join_PathWithTargetIds].[A_Has_B] [rel_ab] ON [src].[ECInstanceId] = [rel_ab].[SourceECInstanceId]"
        " INNER JOIN [alias_ToString_Join_PathWithTargetIds].[B] [tgt1] ON [tgt1].[ECInstanceId] = [rel_ab].[TargetECInstanceId] AND ([tgt1].[ECInstanceId] IN (?))"
        " INNER JOIN [alias_ToString_Join_PathWithTargetIds].[B_Has_C] [rel_bc] ON [tgt1].[ECInstanceId] = [rel_bc].[SourceECInstanceId]"
        " INNER JOIN [alias_ToString_Join_PathWithTargetIds].[C] [tgt2] ON [tgt2].[ECInstanceId] = [rel_bc].[TargetECInstanceId]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_NAVIGATION_QUERY_TEST_SCHEMA(ToString_Join_ClassWithJoinClause, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(ComplexQueryBuilderTests, ToString_Join_ClassWithJoinClause)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(SelectClass<ECClass>(*classA, "a"));
    query->Join(SelectClass<ECClass>(*classB, "b", false), QueryClauseAndBindings("[b].[Prop] > ?", { std::make_shared<BoundQueryECValue>(ECValue(123)) }), true);

    Utf8String expected(
        " FROM [alias_ToString_Join_ClassWithJoinClause].[A] [a]"
        " LEFT JOIN ONLY [alias_ToString_Join_ClassWithJoinClause].[B] [b] ON ([b].[Prop] > ?)");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_NAVIGATION_QUERY_TEST_SCHEMA(ToString_Join_InnerToOuter, R"*(
    <ECEntityClass typeName="A" />
    <ECRelationshipClass typeName="A_A" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="A" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ComplexQueryBuilderTests, ToString_Join_InnerToOuter)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECRelationshipClassCP relAA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_A")->GetRelationshipClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(SelectClass<ECClass>(*classA, "a0"));

    query->Join({
        RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAA, "rel1"), true, SelectClass<ECClass>(*classA, "a1"), false),
        RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAA, "rel2"), true, SelectClass<ECClass>(*classA, "a2"), true),
        });

    Utf8PrintfString schemaName("alias_%s", BeTest::GetNameOfCurrentTest());
    Utf8PrintfString expected(
        " FROM [%s].[A] [a0]"

        " INNER JOIN [%s].[A_A] [rel1] ON [a0].[ECInstanceId] = [rel1].[SourceECInstanceId]"
        " INNER JOIN [%s].[A] [a1] ON [a1].[ECInstanceId] = [rel1].[TargetECInstanceId]"

        " LEFT JOIN ("
            "SELECT [rel2].* "
            "FROM [%s].[A_A] [rel2] "
            "INNER JOIN [%s].[A] [a2] ON [a2].[ECInstanceId] = [rel2].[TargetECInstanceId]"
        ") [rel2] ON [a1].[ECInstanceId] = [rel2].[SourceECInstanceId]"
        " LEFT JOIN [%s].[A] [a2] ON [a2].[ECInstanceId] = [rel2].[TargetECInstanceId]"
        ,
        schemaName.c_str(),
        schemaName.c_str(), schemaName.c_str(),
        schemaName.c_str(), schemaName.c_str(), schemaName.c_str()
    );
    ASSERT_STREQ(expected.c_str(), query->GetQuery()->GetQueryString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_NAVIGATION_QUERY_TEST_SCHEMA(ToString_Join_WithExcludes, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ComplexQueryBuilderTests, ToString_Join_WithExcludes)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECClassCP classD = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "D");
    ECRelationshipClassCP relAB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();
    Utf8CP schemaAlias = classA->GetSchema().GetAlias().c_str();

    SelectClassWithExcludes<ECClass> joinTarget(*classB, "b");
    joinTarget.GetDerivedExcludedClasses().push_back(SelectClass<ECClass>(*classC, "c", true));
    joinTarget.GetDerivedExcludedClasses().push_back(SelectClass<ECClass>(*classD, "d", false));

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(SelectClass<ECClass>(*classA, "a"));
    query->Join({
        RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, "r"), true, joinTarget, false)
        });

    Utf8PrintfString expected(
        " FROM [%s].[%s] [a]"
        " INNER JOIN [%s].[%s] [r] ON [a].[ECInstanceId] = [r].[SourceECInstanceId]"
        " INNER JOIN ("
            "SELECT * "
            "FROM [%s].[%s] [b] "
            "WHERE ([b].[ECClassId] IS NOT ([%s].[%s], ONLY [%s].[%s]))"
        ") [b] ON [b].[ECInstanceId] = [r].[TargetECInstanceId]"
        ,
        schemaAlias, classA->GetName().c_str(),
        schemaAlias, relAB->GetName().c_str(),
        schemaAlias, classB->GetName().c_str(),
        schemaAlias, classC->GetName().c_str(),
        schemaAlias, classD->GetName().c_str()
    );
    ASSERT_STREQ(expected.c_str(), query->GetQuery()->GetQueryString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_NAVIGATION_QUERY_TEST_SCHEMA(ToString_Join_DisqualifiedTarget_NoRelationship, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F(ComplexQueryBuilderTests, ToString_Join_DisqualifiedTarget_NoRelationship)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(SelectClass<ECClass>(*classA, "a"));
    query->Join(SelectClass<ECClass>(*classB, "b", true, true), QueryClauseAndBindings(), false);

    Utf8String expected(
        " FROM [alias_ToString_Join_DisqualifiedTarget_NoRelationship].[A] [a]"
        " INNER JOIN +[alias_ToString_Join_DisqualifiedTarget_NoRelationship].[B] [b]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_NAVIGATION_QUERY_TEST_SCHEMA(ToString_Join_DisqualifiedTarget_SingleStepWithNavigationProperty, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECNavigationProperty propertyName="NavPropA" relationshipName="A_B" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="a" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="b" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ComplexQueryBuilderTests, ToString_Join_DisqualifiedTarget_SingleStepWithNavigationProperty)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(SelectClass<ECClass>(*classA, "a"));
    query->Join(RelatedClass(*classA, SelectClass<ECRelationshipClass>(*rel, "r", true), true, SelectClass<ECClass>(*classB, "b", true, true)));

    Utf8String expected(
        " FROM [alias_ToString_Join_DisqualifiedTarget_SingleStepWithNavigationProperty].[A] [a]"
        " LEFT JOIN +[alias_ToString_Join_DisqualifiedTarget_SingleStepWithNavigationProperty].[B] [b] ON [b].[NavPropA].[Id] = [a].[ECInstanceId]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_NAVIGATION_QUERY_TEST_SCHEMA(ToString_OuterJoin_DisqualifiedTarget_SingleStep, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="a" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="b" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ComplexQueryBuilderTests, ToString_OuterJoin_DisqualifiedTarget_SingleStep)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(SelectClass<ECClass>(*classA, "a"));
    query->Join(RelatedClass(*classA, SelectClass<ECRelationshipClass>(*rel, "r", true), true, SelectClass<ECClass>(*classB, "b", true, true), true));

    Utf8String expected(
        " FROM [alias_ToString_OuterJoin_DisqualifiedTarget_SingleStep].[A] [a]"
        " LEFT JOIN ("
        "SELECT [r].* "
        "FROM [alias_ToString_OuterJoin_DisqualifiedTarget_SingleStep].[A_B] [r] "
        "INNER JOIN +[alias_ToString_OuterJoin_DisqualifiedTarget_SingleStep].[B] [b] ON [b].[ECInstanceId] = [r].[TargetECInstanceId]) [r]"
        " ON [a].[ECInstanceId] = [r].[SourceECInstanceId]"
        " LEFT JOIN +[alias_ToString_OuterJoin_DisqualifiedTarget_SingleStep].[B] [b] ON [b].[ECInstanceId] = [r].[TargetECInstanceId]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_NAVIGATION_QUERY_TEST_SCHEMA(ToString_InnerJoin_DisqualifiedTarget_SingleStep, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="a" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="b" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ComplexQueryBuilderTests, ToString_InnerJoin_DisqualifiedTarget_SingleStep)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(SelectClass<ECClass>(*classA, "a"));
    query->Join(RelatedClass(*classA, SelectClass<ECRelationshipClass>(*rel, "r", true), true, SelectClass<ECClass>(*classB, "b", true, true), false));

    Utf8String expected(
        " FROM [alias_ToString_InnerJoin_DisqualifiedTarget_SingleStep].[A] [a]"
        " INNER JOIN [alias_ToString_InnerJoin_DisqualifiedTarget_SingleStep].[A_B] [r] ON [a].[ECInstanceId] = [r].[SourceECInstanceId]"
        " INNER JOIN +[alias_ToString_InnerJoin_DisqualifiedTarget_SingleStep].[B] [b] ON [b].[ECInstanceId] = [r].[TargetECInstanceId]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_NAVIGATION_QUERY_TEST_SCHEMA(ToString_InnerJoin_DisqualifiedRelationship_SingleStep, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="a" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="b" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ComplexQueryBuilderTests, ToString_InnerJoin_DisqualifiedRelationship_SingleStep)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(SelectClass<ECClass>(*classA, "a"));
    query->Join(RelatedClass(*classA, SelectClass<ECRelationshipClass>(*rel, "r", true, true), true, SelectClass<ECClass>(*classB, "b", true), false));

    Utf8String expected(
        " FROM [alias_ToString_InnerJoin_DisqualifiedRelationship_SingleStep].[A] [a]"
        " INNER JOIN +[alias_ToString_InnerJoin_DisqualifiedRelationship_SingleStep].[A_B] [r] ON [a].[ECInstanceId] = [r].[SourceECInstanceId]"
        " INNER JOIN [alias_ToString_InnerJoin_DisqualifiedRelationship_SingleStep].[B] [b] ON [b].[ECInstanceId] = [r].[TargetECInstanceId]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_NAVIGATION_QUERY_TEST_SCHEMA(ToString_InnerJoin_DisqualifiedTarget_MultiStep, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="a" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="b" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="b" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="c" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ComplexQueryBuilderTests, ToString_InnerJoin_DisqualifiedTarget_MultiStep)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECRelationshipClassCP relAB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B_C")->GetRelationshipClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(SelectClass<ECClass>(*classA, "a"));
    query->Join({
        RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, "rAB", true), true, SelectClass<ECClass>(*classB, "b", true, true), false),
        RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relBC, "rBC", true), true, SelectClass<ECClass>(*classC, "c", true, false), false)
        });

    Utf8String expected(
        " FROM [alias_ToString_InnerJoin_DisqualifiedTarget_MultiStep].[A] [a]"
        " INNER JOIN [alias_ToString_InnerJoin_DisqualifiedTarget_MultiStep].[A_B] [rAB] ON [a].[ECInstanceId] = [rAB].[SourceECInstanceId]"
        " INNER JOIN +[alias_ToString_InnerJoin_DisqualifiedTarget_MultiStep].[B] [b] ON [b].[ECInstanceId] = [rAB].[TargetECInstanceId]"
        " INNER JOIN [alias_ToString_InnerJoin_DisqualifiedTarget_MultiStep].[B_C] [rBC] ON [b].[ECInstanceId] = [rBC].[SourceECInstanceId]"
        " INNER JOIN [alias_ToString_InnerJoin_DisqualifiedTarget_MultiStep].[C] [c] ON [c].[ECInstanceId] = [rBC].[TargetECInstanceId]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_NAVIGATION_QUERY_TEST_SCHEMA(ToString_InnerJoin_DisqualifiedRelationship_MultiStep, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="a" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="b" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="b" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="c" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ComplexQueryBuilderTests, ToString_InnerJoin_DisqualifiedRelationship_MultiStep)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECRelationshipClassCP relAB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B_C")->GetRelationshipClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(SelectClass<ECClass>(*classA, "a"));
    query->Join({
        RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, "rAB", true, false), true, SelectClass<ECClass>(*classB, "b", true, false), false),
        RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relBC, "rBC", true, true), true, SelectClass<ECClass>(*classC, "c", true, false), false)
        });

    Utf8String expected(
        " FROM [alias_ToString_InnerJoin_DisqualifiedRelationship_MultiStep].[A] [a]"
        " INNER JOIN [alias_ToString_InnerJoin_DisqualifiedRelationship_MultiStep].[A_B] [rAB] ON [a].[ECInstanceId] = [rAB].[SourceECInstanceId]"
        " INNER JOIN [alias_ToString_InnerJoin_DisqualifiedRelationship_MultiStep].[B] [b] ON [b].[ECInstanceId] = [rAB].[TargetECInstanceId]"
        " INNER JOIN +[alias_ToString_InnerJoin_DisqualifiedRelationship_MultiStep].[B_C] [rBC] ON [b].[ECInstanceId] = [rBC].[SourceECInstanceId]"
        " INNER JOIN [alias_ToString_InnerJoin_DisqualifiedRelationship_MultiStep].[C] [c] ON [c].[ECInstanceId] = [rBC].[TargetECInstanceId]");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_NAVIGATION_QUERY_TEST_SCHEMA(ToString_Join_NestedQuery, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F(ComplexQueryBuilderTests, ToString_Join_NestedQuery)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    SelectClass<ECClass> selectClass(*classA, "a");

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(selectClass);

    ComplexQueryBuilderPtr joinQuery = ComplexQueryBuilder::Create();
    joinQuery->SelectAll();
    joinQuery->From(selectClass);
    query->Join(*joinQuery, "x", QueryClauseAndBindings("x.ECInstanceId = a.ECInstanceId"), false);

    Utf8String expected(
        " FROM [alias_ToString_Join_NestedQuery].[A] [a]"
        " INNER JOIN (SELECT * FROM [alias_ToString_Join_NestedQuery].[A] [a]) x ON (x.ECInstanceId = a.ECInstanceId)");
    Utf8String str = query->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestNavigationContract : NavigationQueryContract
{
private:
    bvector<PresentationQueryContractFieldCPtr> m_fields;
protected:
    TestNavigationContract() : NavigationQueryContract("") {}
    RefCountedPtr<NavigationQueryContract> _Clone() const override {return new TestNavigationContract(*this);}
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::ECInstanceNodes;}
    bvector<PresentationQueryContractFieldCPtr> _GetFields() const override {return m_fields;}
public:
    static RefCountedPtr<TestNavigationContract> Create() {return new TestNavigationContract();}
    void AddField(PresentationQueryContractFieldCPtr field) {m_fields.push_back(field);}
};
typedef RefCountedPtr<TestNavigationContract> TestNavigationContractPtr;

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, SelectAllDoesntIncludeInternalFieldsInOuterQuery)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_BASIC_2, *m_schemaContext));
    ECClassCP ecClass = schema->GetClassCP("Class2");
    ASSERT_TRUE(nullptr != ecClass);

    TestNavigationContractPtr contract = TestNavigationContract::Create();
    contract->AddField(PresentationQueryContractSimpleField::Create("GeneralField", "GetGeneralField()", false, false, FieldVisibility::Both));
    contract->AddField(PresentationQueryContractSimpleField::Create("InternalField", "GetInternalField()", false, false, FieldVisibility::Inner));
    contract->AddField(PresentationQueryContractSimpleField::Create("OuterField", "GetOuterField()", false, false, FieldVisibility::Outer));
    contract->AddField(PresentationQueryContractSimpleField::Create("AggregateField", "GetAggregateField()", false, true, FieldVisibility::Both));

    ComplexQueryBuilderPtr innerQuery = ComplexQueryBuilder::Create();
    innerQuery->SelectContract(*contract);
    innerQuery->From(SelectClass<ECClass>(*ecClass, "", true));
    innerQuery->GetNavigationResultParameters().SetResultType(NavigationQueryResultType::ECInstanceNodes);

    ComplexQueryBuilderPtr outerQuery = ComplexQueryBuilder::Create();
    outerQuery->SelectAll().From(*innerQuery);
    outerQuery->GroupByContract(*contract);

    Utf8String expected(
        "SELECT [GeneralField], GetOuterField() AS [OuterField], GetAggregateField() AS [AggregateField] "
        "FROM ("
        "SELECT GetGeneralField() AS [GeneralField], GetInternalField() AS [InternalField] "
            "FROM [b2].[Class2]"
        ") "
        "GROUP BY [GeneralField]");
    Utf8String str = outerQuery->GetQuery()->GetQueryString();
    ASSERT_STREQ(expected.c_str(), str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_SelectingUniqueValues_GroupsByFieldName)
    {
    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    TestNavigationContractPtr contract = TestNavigationContract::Create();
    contract->AddField(PresentationQueryContractSimpleField::Create("DistinctField", "GetDistinctField()", false, false, FieldVisibility::Both));
    query->GroupByContract(*contract);

    ASSERT_FALSE(contract->IsAggregating());
    ASSERT_STREQ(" GROUP BY [DistinctField]", query->GetQuery()->GetQueryString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_SelectingUniqueValues_GroupsByGroupingClause)
    {
    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    TestNavigationContractPtr contract = TestNavigationContract::Create();
    PresentationQueryContractFieldPtr field = PresentationQueryContractSimpleField::Create("DistinctField", "GetDistinctField()", false, false, FieldVisibility::Both);
    field->SetGroupingClause("GroupingClause");
    contract->AddField(field);
    query->GroupByContract(*contract);

    ASSERT_FALSE(contract->IsAggregating());
    ASSERT_STREQ(" GROUP BY [GroupingClause]", query->GetQuery()->GetQueryString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_GroupsByDisqualifiedWrappedGroupingClause)
    {
    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    TestNavigationContractPtr contract = TestNavigationContract::Create();
    PresentationQueryContractFieldPtr field = PresentationQueryContractSimpleField::Create("DistinctField", "GetDistinctField()", false, false, FieldVisibility::Both);
    field->SetGroupingClause("+[GroupingClause]");
    contract->AddField(field);
    query->GroupByContract(*contract);

    ASSERT_FALSE(contract->IsAggregating());
    ASSERT_STREQ(" GROUP BY +[GroupingClause]", query->GetQuery()->GetQueryString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_SelectingAggregateField_AggregateFieldIsNotGroupedWhenNoOtherFieldsAreSelected)
    {
    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    TestNavigationContractPtr contract = TestNavigationContract::Create();
    contract->AddField(PresentationQueryContractSimpleField::Create("AggregateField", "GetAggregateField()", false, true, FieldVisibility::Both));
    query->SelectContract(*contract, "this");
    query->GroupByContract(*contract);

    ASSERT_TRUE(contract->IsAggregating());
    ASSERT_STREQ("SELECT GetAggregateField() AS [AggregateField]", query->GetQuery()->GetQueryString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_SelectingAggregateField_AggregateFieldIsGroupedByNotAggregateField)
    {
    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    TestNavigationContractPtr contract = TestNavigationContract::Create();
    contract->AddField(PresentationQueryContractSimpleField::Create("GeneralField", "GetGeneralField()", false, false, FieldVisibility::Both));
    contract->AddField(PresentationQueryContractSimpleField::Create("AggregateField", "GetAggregateField()", false, true, FieldVisibility::Both));
    query->SelectContract(*contract, "this");
    query->GroupByContract(*contract);

    ASSERT_TRUE(contract->IsAggregating());
    Utf8String expected(
        "SELECT GetGeneralField() AS [GeneralField], GetAggregateField() AS [AggregateField]"
        " GROUP BY [GeneralField]");
    ASSERT_STREQ(expected.c_str(), query->GetQuery()->GetQueryString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComplexQueryBuilderTests, ToString_SelectingMultipleAggregateFields_AggregateFieldsAreGroupedByMultipleNotAggregateFields)
    {
    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    TestNavigationContractPtr contract = TestNavigationContract::Create();
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
    ASSERT_STREQ(expected.c_str(), query->GetQuery()->GetQueryString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_NAVIGATION_QUERY_TEST_SCHEMA(ToString_DoNotJoinLastTargetClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="bc" polymorphic="true">
            <Class class="B"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ComplexQueryBuilderTests, ToString_DoNotJoinLastTargetClass)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECRelationshipClassCP relAB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B_C")->GetRelationshipClassCP();

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    query->From(SelectClass<ECClass>(*classA, "a"));
    query->Join({
        RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, "rAB", true, false), true, SelectClass<ECClass>(*classB, "b", true, false), false),
        RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relBC, "rBC", true, false), true, SelectClass<ECClass>(*classC, "c", true, false), false)
        }, false);

    Utf8String expected(
        " FROM [alias_ToString_DoNotJoinLastTargetClass].[A] [a]"
        " INNER JOIN [alias_ToString_DoNotJoinLastTargetClass].[A_B] [rAB] ON [a].[ECInstanceId] = [rAB].[SourceECInstanceId]"
        " INNER JOIN [alias_ToString_DoNotJoinLastTargetClass].[B] [b] ON [b].[ECInstanceId] = [rAB].[TargetECInstanceId]"
        " INNER JOIN [alias_ToString_DoNotJoinLastTargetClass].[B_C] [rBC] ON [b].[ECInstanceId] = [rBC].[SourceECInstanceId]");
    ASSERT_STREQ(expected.c_str(), query->GetQuery()->GetQueryString().c_str());
    }
