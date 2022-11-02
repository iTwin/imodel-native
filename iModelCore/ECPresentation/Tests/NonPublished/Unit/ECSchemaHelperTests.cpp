/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECSchemaHelperTests.h"

ECDbTestProject* ECSchemaHelperTests::s_project = nullptr;
DEFINE_SCHEMA_REGISTRY(ECSchemaHelperTests)

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaHelperTests::SetUpTestCase()
    {
    s_project = new ECDbTestProject();
    s_project->Create("ECSchemaHelperTests", "RulesEngineTest.01.00.ecschema.xml");

    INIT_SCHEMA_REGISTRY(s_project->GetECDb())

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaReadContext = ECSchemaReadContext::CreateContext();
    schemaReadContext->AddSchemaLocater(s_project->GetECDb().GetSchemaLocater());
    ASSERT_TRUE(SchemaReadStatus::Success == ECSchema::ReadFromXmlString(schema, SCHEMA_BASIC_1, *schemaReadContext));
    ASSERT_TRUE(SchemaReadStatus::Success == ECSchema::ReadFromXmlString(schema, SCHEMA_BASIC_2, *schemaReadContext));
    ASSERT_TRUE(SchemaReadStatus::Success == ECSchema::ReadFromXmlString(schema, SCHEMA_BASIC_3, *schemaReadContext));
    ASSERT_TRUE(SchemaReadStatus::Success == ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_1, *schemaReadContext));
    ASSERT_TRUE(SchemaReadStatus::Success == ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_2, *schemaReadContext));
    ASSERT_TRUE(SchemaReadStatus::Success == ECSchema::ReadFromXmlString(schema, SCHEMA_COMPLEX_3, *schemaReadContext));
    ASSERT_TRUE(SchemaReadStatus::Success == ECSchema::ReadFromXmlString(schema, HIDDEN_SCHEMA, *schemaReadContext));
    ASSERT_TRUE(SchemaReadStatus::Success == ECSchema::ReadFromXmlString(schema, VISIBLE_SCHEMA, *schemaReadContext));
    BentleyStatus status = s_project->GetECDb().Schemas().ImportSchemas(schemaReadContext->GetCache().GetSchemas());
    ASSERT_TRUE(SUCCESS == status);
    ASSERT_TRUE(BeSQLite::DbResult::BE_SQLITE_OK == s_project->GetECDb().SaveChanges());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaHelperTests::TearDownTestCase()
    {
    delete s_project;
    s_project = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ECSchemaHelperTests::SetUp()
    {
    ECPresentationTest::SetUp();
    m_connection = m_connections.CreateConnection(s_project->GetECDb());
    m_helper = new ECSchemaHelper(*m_connection, nullptr, nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ECSchemaHelperTests::TearDown()
    {
    s_project->GetECDb().AbandonChanges();
    DELETE_AND_CLEAR(m_helper);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static SupportedClassInfo<ECClass> const* Find(SupportedClassInfos const& classInfos, ECClassCR ecClass)
    {
    auto iter = classInfos.find(SupportedClassInfo<ECClass>(ecClass));
    if (classInfos.end() != iter)
        return &*iter;
    return nullptr;
    }


/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SupportedClassesParserTests : ECSchemaHelperTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SupportedClassesParserTests, Parse_IncludedClassInfosFromSingleSchema)
    {
    SupportedClassesParser parser(*m_helper, "Basic2:Class2", true);
    SupportedClassInfos const& classInfos = parser.GetClassInfos();
    ASSERT_EQ(1, classInfos.size());

    SupportedClassInfo<ECClass> const* info = Find(classInfos, *m_helper->GetECClass("Basic2", "Class2")->GetEntityClassCP());
    ASSERT_TRUE(nullptr != info);
    ASSERT_TRUE(info->IsInclude());
    ASSERT_TRUE(info->IsPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SupportedClassesParserTests, Parse_IncludedClassInfosFromMultipleSchemas)
    {
    SupportedClassesParser parser(*m_helper, "Basic2:Class2;Basic3:Class3", true);
    SupportedClassInfos const& classInfos = parser.GetClassInfos();
    ASSERT_EQ(2, classInfos.size());

    SupportedClassInfo<ECClass> const* info1 = Find(classInfos, *m_helper->GetECClass("Basic2", "Class2")->GetEntityClassCP());
    ASSERT_TRUE(nullptr != info1);
    ASSERT_TRUE(info1->IsInclude());
    ASSERT_TRUE(info1->IsPolymorphic());

    SupportedClassInfo<ECClass> const* info2 = Find(classInfos, *m_helper->GetECClass("Basic3", "Class3")->GetEntityClassCP());
    ASSERT_TRUE(nullptr != info2);
    ASSERT_TRUE(info2->IsInclude());
    ASSERT_TRUE(info2->IsPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SupportedClassesParserTests, Parse_ExcludedClassInfosFromSingleSchema)
    {
    SupportedClassesParser parser(*m_helper, "E:Basic2:Class2", true);
    SupportedClassInfos const& classInfos = parser.GetClassInfos();
    ASSERT_EQ(1, classInfos.size());

    SupportedClassInfo<ECClass> const* info = Find(classInfos, *m_helper->GetECClass("Basic2", "Class2")->GetEntityClassCP());
    ASSERT_TRUE(nullptr != info);
    ASSERT_TRUE(info->IsExclude());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SupportedClassesParserTests, Parse_ExcludedClassInfosFromMultipleSchemas)
    {
    SupportedClassesParser parser(*m_helper, "E:Basic2:Class2;Basic3:Class3", true);
    SupportedClassInfos const& classInfos = parser.GetClassInfos();
    ASSERT_EQ(2, classInfos.size());

    SupportedClassInfo<ECClass> const* info1 = Find(classInfos, *m_helper->GetECClass("Basic2", "Class2")->GetEntityClassCP());
    ASSERT_TRUE(nullptr != info1);
    ASSERT_TRUE(info1->IsExclude());

    SupportedClassInfo<ECClass> const* info2 = Find(classInfos, *m_helper->GetECClass("Basic3", "Class3")->GetEntityClassCP());
    ASSERT_TRUE(nullptr != info2);
    ASSERT_TRUE(info2->IsExclude());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SupportedClassesParserTests, Parse_PolymorphicallyExcludedClassInfosFromSingleSchema)
    {
    SupportedClassesParser parser(*m_helper, "PE:Basic2:Class2", true);
    SupportedClassInfos const& classInfos = parser.GetClassInfos();
    ASSERT_EQ(1, classInfos.size());

    SupportedClassInfo<ECClass> const* info = Find(classInfos, *m_helper->GetECClass("Basic2", "Class2")->GetEntityClassCP());
    ASSERT_TRUE(nullptr != info);
    ASSERT_TRUE(info->IsExclude());
    ASSERT_TRUE(info->IsPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SupportedClassesParserTests, Parse_PolymorphicallyExcludedClassInfosFromMultipleSchemas)
    {
    SupportedClassesParser parser(*m_helper, "PE:Basic2:Class2;Basic3:Class3", true);
    SupportedClassInfos const& classInfos = parser.GetClassInfos();
    ASSERT_EQ(2, classInfos.size());

    SupportedClassInfo<ECClass> const* info1 = Find(classInfos, *m_helper->GetECClass("Basic2", "Class2")->GetEntityClassCP());
    ASSERT_TRUE(nullptr != info1);
    ASSERT_TRUE(info1->IsExclude());
    ASSERT_TRUE(info1->IsPolymorphic());

    SupportedClassInfo<ECClass> const* info2 = Find(classInfos, *m_helper->GetECClass("Basic3", "Class3")->GetEntityClassCP());
    ASSERT_TRUE(nullptr != info2);
    ASSERT_TRUE(info2->IsExclude());
    ASSERT_TRUE(info2->IsPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SupportedClassesParserTests, Parse_IncludedAndExcluded)
    {
    SupportedClassesParser parser(*m_helper, "Basic1:Class1;E:Basic1:Class1B;PE:Basic2:Class2", true);
    SupportedClassInfos const& classInfos = parser.GetClassInfos();
    ASSERT_EQ(3, classInfos.size());

    SupportedClassInfo<ECClass> const* info1 = Find(classInfos, *m_helper->GetECClass("Basic1", "Class1")->GetEntityClassCP());
    ASSERT_TRUE(nullptr != info1);
    ASSERT_TRUE(info1->IsInclude());
    ASSERT_TRUE(info1->IsPolymorphic());

    SupportedClassInfo<ECClass> const* info2 = Find(classInfos, *m_helper->GetECClass("Basic1", "Class1B")->GetEntityClassCP());
    ASSERT_TRUE(nullptr != info2);
    ASSERT_TRUE(info2->IsExclude());

    SupportedClassInfo<ECClass> const* info3 = Find(classInfos, *m_helper->GetECClass("Basic2", "Class2")->GetEntityClassCP());
    ASSERT_TRUE(nullptr != info3);
    ASSERT_TRUE(info3->IsExclude());
    ASSERT_TRUE(info3->IsPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SupportedClassesParserTests, GetECClassInfosFromClassList_IncludedAndExcluded)
    {
    SupportedClassesParser parser(*m_helper, "Basic1:Class1;E:Basic1:Class1", true);
    SupportedClassInfos const& classInfos = parser.GetClassInfos();
    ASSERT_EQ(1, classInfos.size());

    SupportedClassInfo<ECClass> const* info = Find(classInfos, *m_helper->GetECClass("Basic1", "Class1")->GetEntityClassCP());
    ASSERT_TRUE(nullptr != info);
    ASSERT_TRUE(info->IsInclude());
    ASSERT_TRUE(info->IsExclude());
    ASSERT_TRUE(info->IsPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetSchema)
    {
    ASSERT_TRUE(nullptr == m_helper->GetSchema("does_not_exist", false));

    ECSchemaCP schema = m_helper->GetSchema("Basic1", false);
    ASSERT_TRUE(nullptr != schema);
    ASSERT_STREQ("Basic1", schema->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetECClass)
    {
    ASSERT_TRUE(nullptr == m_helper->GetECClass("Basic1", "does_not_exist"));

    ECClassCP ecClass = m_helper->GetECClass("Basic1", "Class1");
    ASSERT_TRUE(nullptr != ecClass);
    ASSERT_STREQ("Basic1:Class1", ecClass->GetFullName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetECClassesFromSchemaList_SingleSchema)
    {
    ECClassSet classes = m_helper->GetECClassesFromSchemaList("Basic2");
    ASSERT_EQ(1, classes.size());
    ASSERT_STREQ("Basic2:Class2", (*classes.begin()).first->GetFullName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetECClassesFromSchemaList_MultipleSchemas)
    {
    ECClassSet classes = m_helper->GetECClassesFromSchemaList("Basic2,Basic3");
    ASSERT_EQ(2, classes.size());
    ASSERT_TRUE(classes.end() != classes.find(m_helper->GetECClass("Basic2", "Class2")->GetEntityClassCP()));
    ASSERT_TRUE(classes.end() != classes.find(m_helper->GetECClass("Basic3", "Class3")->GetEntityClassCP()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetECClassesFromSchemaList_Polymorphic)
    {
    ECClassSet classes = m_helper->GetECClassesFromSchemaList("Basic1");
    ASSERT_EQ(1, classes.size());
    auto iter = classes.find(m_helper->GetECClass("Basic1", "Class1")->GetEntityClassCP());
    ASSERT_TRUE(classes.end() != iter);
    ASSERT_TRUE(iter->second);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetOppositeRelationshipEnds_SingleEnd)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass2And3")->GetRelationshipClassCP();

    ECRelationshipConstraintClassList oppositeRelationshipEnds = m_helper->GetRelationshipConstraintClasses(relationship, ECRelatedInstanceDirection::Backward, "");
    ASSERT_EQ(1, oppositeRelationshipEnds.size());
    ASSERT_STREQ("SchemaComplex:Class1", oppositeRelationshipEnds[0]->GetFullName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetOppositeRelationshipEnds_MultipleEnds)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass2And3")->GetRelationshipClassCP();

    ECRelationshipConstraintClassList oppositeRelationshipEnds = m_helper->GetRelationshipConstraintClasses(relationship, ECRelatedInstanceDirection::Forward, "");
    ASSERT_EQ(2, oppositeRelationshipEnds.size());
    ASSERT_STREQ("SchemaComplex:Class2", oppositeRelationshipEnds[0]->GetFullName());
    ASSERT_STREQ("SchemaComplex:Class3", oppositeRelationshipEnds[1]->GetFullName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetOppositeRelationshipEnds_UnsupportedSchema)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass2And3")->GetRelationshipClassCP();

    ECRelationshipConstraintClassList oppositeRelationshipEnds = m_helper->GetRelationshipConstraintClasses(relationship, ECRelatedInstanceDirection::Forward, "Basic1");
    ASSERT_EQ(0, oppositeRelationshipEnds.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, Deprecated_GetRelationshipClassPaths_FromSupportedSchemas)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2And3")->GetRelationshipClassCP();
    ECRelationshipClassCR relationship2 = *schema->GetClassCP("Class1HasClass3")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECClassCR class2 = *schema->GetClassCP("Class2");
    ECClassCR class3 = *schema->GetClassCP("Class3");

    ECClassUseCounter relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptionsDeprecated options(class1, (int)ECRelatedInstanceDirection::Forward, 0, "SchemaComplex", "", "", true, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPathsDeprecated(options);
    ASSERT_EQ(3, paths.size());
    for (size_t i = 0; i < paths.size(); i++)
        ASSERT_EQ(1, paths[i].size());

    EXPECT_EQ(&class1, paths[0][0].GetSourceClass());
    EXPECT_EQ(&class2, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship1, &paths[0][0].GetRelationship().GetClass());
    EXPECT_TRUE(paths[0][0].IsForwardRelationship());

    EXPECT_EQ(&class1, paths[1][0].GetSourceClass());
    EXPECT_EQ(&class3, &paths[1][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[1][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship1, &paths[1][0].GetRelationship().GetClass());
    EXPECT_TRUE(paths[1][0].IsForwardRelationship());

    EXPECT_EQ(&class1, paths[2][0].GetSourceClass());
    EXPECT_EQ(&class3, &paths[2][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[2][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship2, &paths[2][0].GetRelationship().GetClass());
    EXPECT_TRUE(paths[2][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, Deprecated_GetRelationshipClassPaths_FromRelatedClassNames)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2And3")->GetRelationshipClassCP();
    ECRelationshipClassCR relationship2 = *schema->GetClassCP("Class1HasClass3")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECClassCR class3 = *schema->GetClassCP("Class3");

    ECClassUseCounter relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptionsDeprecated options(class1, (int)ECRelatedInstanceDirection::Forward, 0, "", "", "SchemaComplex:Class3", true, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPathsDeprecated(options);
    ASSERT_EQ(2, paths.size());
    for (size_t i = 0; i < paths.size(); i++)
        ASSERT_EQ(1, paths[i].size());

    EXPECT_EQ(&class1, paths[0][0].GetSourceClass());
    EXPECT_EQ(&class3, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship1, &paths[0][0].GetRelationship().GetClass());
    EXPECT_TRUE(paths[0][0].IsForwardRelationship());

    EXPECT_EQ(&class1, paths[1][0].GetSourceClass());
    EXPECT_EQ(&class3, &paths[1][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[1][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship2, &paths[1][0].GetRelationship().GetClass());
    EXPECT_TRUE(paths[1][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, Deprecated_GetRelationshipClassPaths_FindsRelatedClassesPolymorphically)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECRelationshipClassCR relationship = *schema->GetClassCP("DerivingRelationship")->GetRelationshipClassCP();
    ECEntityClassCR class4 = *schema->GetClassCP("Class4")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    ECClassUseCounter relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptionsDeprecated options(class3, (int)ECRelatedInstanceDirection::Backward, 0, "", "SchemaComplex:DerivingRelationship", "SchemaComplex:Class4", true, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPathsDeprecated(options);
    ASSERT_EQ(1, paths.size());
    ASSERT_EQ(1, paths[0].size());
    EXPECT_EQ(&class3, paths[0][0].GetSourceClass());
    EXPECT_EQ(&class4, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship, &paths[0][0].GetRelationship().GetClass());
    EXPECT_FALSE(paths[0][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Deprecated_GetRelationshipClassPaths_DoesntIncludeDerivedRelatedClassesWhenBaseAlreadyIncludedAndRequestingPathsPolymorphically, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="Aspect">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="CustomAspect">
        <BaseClass>Aspect</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementHasAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Aspect" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ECSchemaHelperTests, Deprecated_GetRelationshipClassPaths_DoesntIncludeDerivedRelatedClassesWhenBaseAlreadyIncludedAndRequestingPathsPolymorphically)
    {
    ECEntityClassCP elementClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Element")->GetEntityClassCP();
    ECEntityClassCP aspectClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Aspect")->GetEntityClassCP();
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ElementHasAspect")->GetRelationshipClassCP();

    ECClassUseCounter relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptionsDeprecated options(*elementClass, (int)ECRelatedInstanceDirection::Forward, 0, "",
        rel->GetFullName(), aspectClass->GetFullName(), true, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPathsDeprecated(options);
    ASSERT_EQ(1, paths.size());
    ASSERT_EQ(1, paths[0].size());
    EXPECT_EQ(elementClass, paths[0][0].GetSourceClass());
    EXPECT_EQ(aspectClass, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(rel, &paths[0][0].GetRelationship().GetClass());
    EXPECT_TRUE(paths[0][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Deprecated_GetRelationshipClassPaths_IncludesBaseAndDerivedRelatedClassesWhenRequestingPathsNonPolymorphically,
    R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="Aspect">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="CustomAspect">
        <BaseClass>Aspect</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementHasAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Aspect" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ECSchemaHelperTests, Deprecated_GetRelationshipClassPaths_IncludesBaseAndDerivedRelatedClassesWhenRequestingPathsNonPolymorphically)
    {
    ECEntityClassCP elementClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Element")->GetEntityClassCP();
    ECEntityClassCP aspectClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Aspect")->GetEntityClassCP();
    ECEntityClassCP customAspectClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "CustomAspect")->GetEntityClassCP();
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ElementHasAspect")->GetRelationshipClassCP();

    ECClassUseCounter relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptionsDeprecated options(*elementClass, (int)ECRelatedInstanceDirection::Forward, 0, "",
        rel->GetFullName(), aspectClass->GetFullName(), false, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPathsDeprecated(options);
    ASSERT_EQ(2, paths.size());

    ASSERT_EQ(1, paths[0].size());
    EXPECT_EQ(elementClass, paths[0][0].GetSourceClass());
    EXPECT_EQ(aspectClass, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_FALSE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(rel, &paths[0][0].GetRelationship().GetClass());
    EXPECT_TRUE(paths[0][0].IsForwardRelationship());

    ASSERT_EQ(1, paths[1].size());
    EXPECT_EQ(elementClass, paths[1][0].GetSourceClass());
    EXPECT_EQ(customAspectClass, &paths[1][0].GetTargetClass().GetClass());
    EXPECT_FALSE(paths[1][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(rel, &paths[1][0].GetRelationship().GetClass());
    EXPECT_TRUE(paths[1][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, Deprecated_GetRelationshipClassPaths_FromSupportedRelationships)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass2And3")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECClassCR class2 = *schema->GetClassCP("Class2");
    ECClassCR class3 = *schema->GetClassCP("Class3");

    ECClassUseCounter relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptionsDeprecated options(class1, (int)ECRelatedInstanceDirection::Forward, 0, "", "SchemaComplex:Class1HasClass2And3", "", true, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPathsDeprecated(options);
    ASSERT_EQ(2, paths.size());
    for (size_t i = 0; i < paths.size(); i++)
        ASSERT_EQ(1, paths[i].size());

    EXPECT_EQ(&class1, paths[0][0].GetSourceClass());
    EXPECT_EQ(&class2, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship, &paths[0][0].GetRelationship().GetClass());
    EXPECT_TRUE(paths[0][0].IsForwardRelationship());

    EXPECT_EQ(&class1, paths[1][0].GetSourceClass());
    EXPECT_EQ(&class3, &paths[1][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[1][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship, &paths[1][0].GetRelationship().GetClass());
    EXPECT_TRUE(paths[1][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, Deprecated_GetRelationshipClassPaths_RespectsPolymorphicRelationshipConstraints)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class4 = *schema->GetClassCP("Class4")->GetEntityClassCP();

    ECClassUseCounter relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptionsDeprecated options1(class1, (int)ECRelatedInstanceDirection::Forward, 0, "", "SchemaComplex:Class1HasClass3", "", true, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPathsDeprecated(options1);
    ASSERT_EQ(1, paths.size());

    // Class4 derives from Class1, but the relationship is not followed because the source constraint is not polymorphic
    ECSchemaHelper::RelationshipClassPathOptionsDeprecated options2(class4, (int)ECRelatedInstanceDirection::Forward, 0, "", "SchemaComplex:Class1HasClass3", "", true, relationshipUseCounts, nullptr);
    paths = m_helper->GetRelationshipClassPathsDeprecated(options2);
    ASSERT_EQ(0, paths.size());

    ECSchemaHelper::RelationshipClassPathOptionsDeprecated options3(class1, (int)ECRelatedInstanceDirection::Forward, 0, "", "SchemaComplex:Class1HasClass2And3", "", true, relationshipUseCounts, nullptr);
    paths = m_helper->GetRelationshipClassPathsDeprecated(options3);
    ASSERT_EQ(2, paths.size());

    // the relationship constraint is polymorphic in this case, so following Class4 is the same as following Class1
    ECSchemaHelper::RelationshipClassPathOptionsDeprecated options4(class4, (int)ECRelatedInstanceDirection::Forward, 0, "", "SchemaComplex:Class1HasClass2And3", "", true, relationshipUseCounts, nullptr);
    paths = m_helper->GetRelationshipClassPathsDeprecated(options4);
    ASSERT_EQ(2, paths.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, Deprecated_GetRelationshipClassPaths_FollowsBothRelationshipsDirections)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex2");
    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECRelationshipClassCR relationship2 = *schema->GetClassCP("Class3HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    static int relationshipDirection = (int)ECRelatedInstanceDirection::Forward | (int)ECRelatedInstanceDirection::Backward;

    ECClassUseCounter relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptionsDeprecated options(class2, relationshipDirection, 0, "SchemaComplex2", "", "", true, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPathsDeprecated(options);
    ASSERT_EQ(2, paths.size());
    for (size_t i = 0; i < paths.size(); i++)
        ASSERT_EQ(1, paths[i].size());

    EXPECT_EQ(&class2, paths[0][0].GetSourceClass());
    EXPECT_EQ(&class1, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship1, &paths[0][0].GetRelationship().GetClass());
    EXPECT_FALSE(paths[0][0].IsForwardRelationship());

    EXPECT_EQ(&class2, paths[1][0].GetSourceClass());
    EXPECT_EQ(&class3, &paths[1][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[1][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship2, &paths[1][0].GetRelationship().GetClass());
    EXPECT_FALSE(paths[1][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, Deprecated_GetRelationshipClassPaths_FollowsBothRelationshipsDirectionsWithDepthMoreThan0)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex2");
    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECRelationshipClassCR relationship2 = *schema->GetClassCP("Class3HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    static int relationshipDirection = (int)ECRelatedInstanceDirection::Forward | (int)ECRelatedInstanceDirection::Backward;

    ECClassUseCounter relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptionsDeprecated options(class1, relationshipDirection, 1, "", "", "SchemaComplex2:Class3", true, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPathsDeprecated(options);
    ASSERT_EQ(1, paths.size());
    ASSERT_EQ(2, paths[0].size());

    EXPECT_EQ(&class1, paths[0][0].GetSourceClass());
    EXPECT_EQ(&class2, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship1, &paths[0][0].GetRelationship().GetClass());
    EXPECT_TRUE(paths[0][0].IsForwardRelationship());

    EXPECT_EQ(&class2, paths[0][1].GetSourceClass());
    EXPECT_EQ(&class3, &paths[0][1].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[0][1].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship2, &paths[0][1].GetRelationship().GetClass());
    EXPECT_FALSE(paths[0][1].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, Deprecated_GetRelationshipClassPaths_DoesntDuplicatePathsWithPolymorphicallySimilarClasses)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass2And3")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();

    ECClassUseCounter relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptionsDeprecated options(class2, (int)ECRelatedInstanceDirection::Backward,
        0, "SchemaComplex", "", "", true, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPathsDeprecated(options);
    ASSERT_EQ(1, paths.size());

    EXPECT_EQ(&class2, paths[0][0].GetSourceClass());
    EXPECT_EQ(&class1, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship, &paths[0][0].GetRelationship().GetClass());
    EXPECT_FALSE(paths[0][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, Deprecated_GetRelationshipClassPaths_DoesntDuplicatePathsWithPolymorphicallySimilarRelationships)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass3")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    ECClassUseCounter relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptionsDeprecated options(class1, (int)ECRelatedInstanceDirection::Forward,
        0, "", "SchemaComplex:Class1HasClass3,DerivingRelationship", "SchemaComplex:Class3", true, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPathsDeprecated(options);
    ASSERT_EQ(1, paths.size());

    EXPECT_EQ(&class1, paths[0][0].GetSourceClass());
    EXPECT_EQ(&class3, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship, &paths[0][0].GetRelationship().GetClass());
    EXPECT_TRUE(paths[0][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, Deprecated_GetRelationshipClassPaths_RecursivelyUsingSuppliedRelationshipsAndClasses)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("RulesEngineTest");
    ECEntityClassCR widget = *schema->GetClassCP("Widget")->GetEntityClassCP();
    ECEntityClassCR gadget = *schema->GetClassCP("Gadget")->GetEntityClassCP();
    ECEntityClassCR sprocket = *schema->GetClassCP("Sprocket")->GetEntityClassCP();
    ECRelationshipClassCR wiggetHasGadgets = *schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCR gadgetHasSprockets = *schema->GetClassCP("GadgetHasSprockets")->GetRelationshipClassCP();

    ECClassUseCounter relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptionsDeprecated options(widget, (int)ECRelatedInstanceDirection::Forward, -1, "",
        "RulesEngineTest:WidgetHasGadgets,GadgetHasSprockets", "RulesEngineTest:Gadget,Sprocket", true, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPathsDeprecated(options);
    ASSERT_EQ(2, paths.size());

    RelatedClassPath const& path1 = paths[0];
    ASSERT_EQ(1, path1.size());
    EXPECT_EQ(&widget, path1[0].GetSourceClass());
    EXPECT_EQ(&gadget, &path1[0].GetTargetClass().GetClass());
    EXPECT_TRUE(path1[0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&wiggetHasGadgets, &path1[0].GetRelationship().GetClass());
    EXPECT_TRUE(path1[0].IsForwardRelationship());

    RelatedClassPath const& path2 = paths[1];
    ASSERT_EQ(2, path2.size());
    EXPECT_EQ(&widget, path2[0].GetSourceClass());
    EXPECT_EQ(&gadget, &path2[0].GetTargetClass().GetClass());
    EXPECT_TRUE(path2[0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&wiggetHasGadgets, &path2[0].GetRelationship().GetClass());
    EXPECT_TRUE(path2[0].IsForwardRelationship());
    EXPECT_EQ(&gadget, path2[1].GetSourceClass());
    EXPECT_EQ(&sprocket, &path2[1].GetTargetClass().GetClass());
    EXPECT_TRUE(path2[0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&gadgetHasSprockets, &path2[1].GetRelationship().GetClass());
    EXPECT_TRUE(path2[1].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelationshipClassPaths_IncludesPathsForRelationshipTargetClassIfTargetNotSpecifiedInSpecification, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="Base">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>Base</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>Base</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_Target" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Base" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ECSchemaHelperTests, GetRelationshipClassPaths_IncludesPathsForRelationshipTargetClassIfTargetNotSpecifiedInSpecification)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP baseTargetClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Base");
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_Has_Target")->GetRelationshipClassCP();

    ECClassUseCounter relationshipUseCounts;
    RelationshipPathSpecification pathSpecification(*new RelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward, ""));
    ECSchemaHelper::MultiRelationshipPathOptions options(*classA, pathSpecification, true, relationshipUseCounts);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPaths(options);

    ASSERT_EQ(1, paths.size());

    ASSERT_EQ(1, paths[0].size());
    EXPECT_EQ(classA, paths[0][0].GetSourceClass());
    EXPECT_EQ(baseTargetClass, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(rel, &paths[0][0].GetRelationship().GetClass());
    EXPECT_TRUE(paths[0][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelationshipClassPaths_IncludesPathsOnlyForSpecifiedTargetClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="Base">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>Base</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>Base</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="A_Has_Target" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Base" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ECSchemaHelperTests, GetRelationshipClassPaths_IncludesPathsOnlyForSpecifiedTargetClass)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_Has_Target")->GetRelationshipClassCP();

    ECClassUseCounter relationshipUseCounts;
    RelationshipPathSpecification pathSpecification(*new RelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward, classC->GetFullName()));
    ECSchemaHelper::MultiRelationshipPathOptions options(*classA, pathSpecification, true, relationshipUseCounts);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPaths(options);
    ASSERT_EQ(1, paths.size());
    ASSERT_EQ(1, paths[0].size());
    EXPECT_EQ(classA, paths[0][0].GetSourceClass());
    EXPECT_EQ(classC, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(rel, &paths[0][0].GetRelationship().GetClass());
    EXPECT_TRUE(paths[0][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelationshipClassPaths_CreatesPathsForClassAndItsDerivedClassesIfSpecificallyAskedForWhenClassesAreHiddenThroughBaseClassCustomAttribute, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
            <HiddenClass xmlns="CoreCustomAttributes.01.00" />
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>C</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="E">
        <BaseClass>C</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ECSchemaHelperTests, GetRelationshipClassPaths_CreatesPathsForClassAndItsDerivedClassesIfSpecificallyAskedForWhenClassesAreHiddenThroughBaseClassCustomAttribute)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECClassCP classD = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "D");
    ECClassCP classE = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "E");
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();

    ECClassUseCounter relationshipUseCounts;
    RelationshipPathSpecification pathSpecification(*new RelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward, classC->GetFullName()));
    ECSchemaHelper::MultiRelationshipPathOptions options(*classA, pathSpecification, false, relationshipUseCounts);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPaths(options);

    ASSERT_EQ(3, paths.size());

    ASSERT_EQ(1, paths[0].size());
    EXPECT_EQ(classA, paths[0][0].GetSourceClass());
    EXPECT_EQ(classC, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_FALSE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(rel, &paths[0][0].GetRelationship().GetClass());
    EXPECT_TRUE(paths[0][0].IsForwardRelationship());

    ASSERT_EQ(1, paths[1].size());
    EXPECT_EQ(classA, paths[1][0].GetSourceClass());
    EXPECT_EQ(classD, &paths[1][0].GetTargetClass().GetClass());
    EXPECT_FALSE(paths[1][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(rel, &paths[1][0].GetRelationship().GetClass());
    EXPECT_TRUE(paths[1][0].IsForwardRelationship());

    ASSERT_EQ(1, paths[2].size());
    EXPECT_EQ(classA, paths[2][0].GetSourceClass());
    EXPECT_EQ(classE, &paths[2][0].GetTargetClass().GetClass());
    EXPECT_FALSE(paths[2][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(rel, &paths[2][0].GetRelationship().GetClass());
    EXPECT_TRUE(paths[2][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelationshipClassPaths_CreatesPathsForClassAndItsDerivedClassesIfSpecificallyAskedForWhenClassesAreHiddenThroughRequestedClassCustomAttribute, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
        <ECCustomAttributes>
            <HiddenClass xmlns="CoreCustomAttributes.01.00" />
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>C</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="E">
        <BaseClass>C</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ECSchemaHelperTests, GetRelationshipClassPaths_CreatesPathsForClassAndItsDerivedClassesIfSpecificallyAskedForWhenClassesAreHiddenThroughRequestedClassCustomAttribute)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECClassCP classD = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "D");
    ECClassCP classE = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "E");
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();

    ECClassUseCounter relationshipUseCounts;
    RelationshipPathSpecification pathSpecification(*new RelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward, classC->GetFullName()));
    ECSchemaHelper::MultiRelationshipPathOptions options(*classA, pathSpecification, false, relationshipUseCounts);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPaths(options);

    ASSERT_EQ(3, paths.size());

    ASSERT_EQ(1, paths[0].size());
    EXPECT_EQ(classA, paths[0][0].GetSourceClass());
    EXPECT_EQ(classC, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_FALSE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(rel, &paths[0][0].GetRelationship().GetClass());
    EXPECT_TRUE(paths[0][0].IsForwardRelationship());

    ASSERT_EQ(1, paths[1].size());
    EXPECT_EQ(classA, paths[1][0].GetSourceClass());
    EXPECT_EQ(classD, &paths[1][0].GetTargetClass().GetClass());
    EXPECT_FALSE(paths[1][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(rel, &paths[1][0].GetRelationship().GetClass());
    EXPECT_TRUE(paths[1][0].IsForwardRelationship());

    ASSERT_EQ(1, paths[2].size());
    EXPECT_EQ(classA, paths[2][0].GetSourceClass());
    EXPECT_EQ(classE, &paths[2][0].GetTargetClass().GetClass());
    EXPECT_FALSE(paths[2][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(rel, &paths[2][0].GetRelationship().GetClass());
    EXPECT_TRUE(paths[2][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelationshipClassPaths_OmitsDerivedClassesWhenTheyAreHiddenThroughLocalCustomAttribute, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>C</BaseClass>
        <ECCustomAttributes>
            <HiddenClass xmlns="CoreCustomAttributes.01.00" />
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="E">
        <BaseClass>D</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="F">
        <BaseClass>C</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ECSchemaHelperTests, GetRelationshipClassPaths_OmitsDerivedClassesWhenTheyAreHiddenThroughLocalCustomAttribute)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECClassCP classF = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "F");
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();

    ECClassUseCounter relationshipUseCounts;
    RelationshipPathSpecification pathSpecification(*new RelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection_Forward, classC->GetFullName()));
    ECSchemaHelper::MultiRelationshipPathOptions options(*classA, pathSpecification, false, relationshipUseCounts);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPaths(options);

    ASSERT_EQ(2, paths.size());

    ASSERT_EQ(1, paths[0].size());
    EXPECT_EQ(classA, paths[0][0].GetSourceClass());
    EXPECT_EQ(classC, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_FALSE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(rel, &paths[0][0].GetRelationship().GetClass());
    EXPECT_TRUE(paths[0][0].IsForwardRelationship());

    ASSERT_EQ(1, paths[1].size());
    EXPECT_EQ(classA, paths[1][0].GetSourceClass());
    EXPECT_EQ(classF, &paths[1][0].GetTargetClass().GetClass());
    EXPECT_FALSE(paths[1][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(rel, &paths[1][0].GetRelationship().GetClass());
    EXPECT_TRUE(paths[1][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetForeignKeyClass_FindsForeignClassByNavigationProperty)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex3");
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    EXPECT_EQ(&class1, &m_helper->GetForeignKeyClass(*class1.GetPropertyP("Parent")).GetTargetClass().GetClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetECClassesFromSchemaList_DoesNotReturnHiddenClasses)
    {
    ECClassSet classes = m_helper->GetECClassesFromSchemaList("VisibleSchema");
    EXPECT_EQ(1, classes.size());
    ECClassCP visibleClass = m_helper->GetECClass("VisibleSchema:VisibleClass");
    auto classIter = classes.find(visibleClass->GetEntityClassCP());
    EXPECT_TRUE(classes.end() != classIter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetECClassesFromSchemaList_DoesNotReturnClassesFromHiddenSchemasWhenAskingForAllSchemas)
    {
    ECClassSet classes = m_helper->GetECClassesFromSchemaList("");
    ECClassCP classFromHiddenSchema = m_helper->GetECClass("HiddenSchema:Class1");
    auto classIter = classes.find(classFromHiddenSchema->GetEntityClassCP());
    EXPECT_TRUE(classes.end() == classIter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetExtendedTypeNameOrTypeName_ReturnsExtendedTypeNameIfPropertyHasIt, R"*(
    <ECEntityClass typeName="Model">
        <ECProperty propertyName="Prop" typeName="int" extendedTypeName="URL" />
    </ECEntityClass>
)*");
TEST_F (ECSchemaHelperTests, GetExtendedTypeNameOrTypeName_ReturnsExtendedTypeNameIfPropertyHasIt)
    {
    ECClassCP modelClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Model");
    ECPropertyCR property = *modelClass->GetPropertyP("Prop");

    EXPECT_EQ(ECSchemaHelper::GetTypeName(property), "URL");
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetExtendedTypeNameOrTypeName_ReturnsTypeNameIfExtendedTypeNameIsNotDefined, R"*(
    <ECEntityClass typeName="Model">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(ECSchemaHelperTests, GetExtendedTypeNameOrTypeName_ReturnsTypeNameIfExtendedTypeNameIsNotDefined)
    {
    ECClassCP modelClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Model");
    ECPropertyCR property = *modelClass->GetPropertyP("Prop");

    EXPECT_EQ(ECSchemaHelper::GetTypeName(property), "int");
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelationshipPaths_ReturnsPathsWithTargetInstanceCheck, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ECSchemaHelperTests, GetRelationshipPaths_ReturnsPathsWithTargetInstanceCheck)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECRelationshipClassCP relAB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b);

    CustomFunctionsInjector incjectCustomFunctions(m_connections, *m_connection);

    ECClassUseCounter counter;
    RelationshipPathSpecification pathToRelatedInstanceSpec(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward));
    bvector<ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification> pathSpecs = { ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification(0, pathToRelatedInstanceSpec, true, {}) };
    ECSchemaHelper::RelationshipPathsRequestParams params(SelectClass<ECClass>(*classA, ""), pathSpecs, nullptr, bvector<RelatedClassPath>(), counter, true);

    ECSchemaHelper::RelationshipPathsResponse response = m_helper->GetRelationshipPaths(params);
    EXPECT_EQ(1, response.GetPaths(0).size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelationshipPaths_OmitsRelationshipPathsWithoutTargetInstances, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
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
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ECSchemaHelperTests, GetRelationshipPaths_OmitsRelationshipPathsWithoutTargetInstances)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *c);

    CustomFunctionsInjector incjectCustomFunctions(m_connections, *m_connection);

    ECClassUseCounter counter;
    RelationshipPathSpecification pathToRelatedInstanceSpec(*new RelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward));
    bvector<ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification> pathSpecs = { ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification(0, pathToRelatedInstanceSpec, true, {}) };
    ECSchemaHelper::RelationshipPathsRequestParams params(SelectClass<ECClass>(*classA, ""), pathSpecs, nullptr, bvector<RelatedClassPath>(), counter, true);
    ECSchemaHelper::RelationshipPathsResponse response = m_helper->GetRelationshipPaths(params);

    ASSERT_EQ(2, response.GetPaths(0).size());

    RelatedClassPathCR path1 = response.GetPaths(0)[0].m_path;
    EXPECT_EQ(classA, path1[0].GetSourceClass());
    EXPECT_EQ(classB, &path1[0].GetTargetClass().GetClass());
    EXPECT_TRUE(path1[0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(rel, &path1[0].GetRelationship().GetClass());
    EXPECT_TRUE(path1[0].IsForwardRelationship());
    EXPECT_EQ(2, path1.GetTargetsCount().Value());

    RelatedClassPathCR path2 = response.GetPaths(0)[1].m_path;
    EXPECT_EQ(classA, path2[0].GetSourceClass());
    EXPECT_EQ(classC, &path2[0].GetTargetClass().GetClass());
    EXPECT_TRUE(path2[0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(rel, &path2[0].GetRelationship().GetClass());
    EXPECT_TRUE(path2[0].IsForwardRelationship());
    EXPECT_EQ(1, path2.GetTargetsCount().Value());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelationshipPaths_OmitsRelationshipPathsWithoutTargetInstancesWhileNotCountingThem, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
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
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ECSchemaHelperTests, GetRelationshipPaths_OmitsRelationshipPathsWithoutTargetInstancesWhileNotCountingThem)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *c);

    CustomFunctionsInjector incjectCustomFunctions(m_connections, *m_connection);

    ECClassUseCounter counter;
    RelationshipPathSpecification pathToRelatedInstanceSpec(*new RelationshipStepSpecification(rel->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward));
    bvector<ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification> pathSpecs = { ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification(0, pathToRelatedInstanceSpec, true, {}) };
    ECSchemaHelper::RelationshipPathsRequestParams params(SelectClass<ECClass>(*classA, ""), pathSpecs, nullptr, bvector<RelatedClassPath>(), counter, false);
    ECSchemaHelper::RelationshipPathsResponse response = m_helper->GetRelationshipPaths(params);

    ASSERT_EQ(2, response.GetPaths(0).size());

    RelatedClassPathCR path1 = response.GetPaths(0)[0].m_path;
    EXPECT_EQ(classA, path1[0].GetSourceClass());
    EXPECT_EQ(classB, &path1[0].GetTargetClass().GetClass());
    EXPECT_TRUE(path1[0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(rel, &path1[0].GetRelationship().GetClass());
    EXPECT_TRUE(path1[0].IsForwardRelationship());
    EXPECT_TRUE(path1.GetTargetsCount().IsNull());

    RelatedClassPathCR path2 = response.GetPaths(0)[1].m_path;
    EXPECT_EQ(classA, path2[0].GetSourceClass());
    EXPECT_EQ(classC, &path2[0].GetTargetClass().GetClass());
    EXPECT_TRUE(path2[0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(rel, &path2[0].GetRelationship().GetClass());
    EXPECT_TRUE(path2[0].IsForwardRelationship());
    EXPECT_TRUE(path2.GetTargetsCount().IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelationshipPaths_ReturnsPathsWithNonPolymorphicTargetInstancesMatchingInstanceFilter, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ECSchemaHelperTests, GetRelationshipPaths_ReturnsPathsWithNonPolymorphicTargetInstancesMatchingInstanceFilter)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECRelationshipClassCP relAB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR inst){inst.SetValue("Prop", ECValue("abc")); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b);

    CustomFunctionsInjector incjectCustomFunctions(m_connections, *m_connection);

    ECClassUseCounter counter;
    RelationshipPathSpecification pathToRelatedInstanceSpec(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward));
    bvector<ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification> pathSpecs = { ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification(0, pathToRelatedInstanceSpec, false, {"this.Prop = \"abc\""}) };
    ECSchemaHelper::RelationshipPathsRequestParams params(SelectClass<ECClass>(*classA, ""), pathSpecs, nullptr, bvector<RelatedClassPath>(), counter, true);

    ECSchemaHelper::RelationshipPathsResponse response = m_helper->GetRelationshipPaths(params);
    EXPECT_EQ(1, response.GetPaths(0).size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelationshipPaths_DoesntReturnPathsWithNonPolymorphicTargetInstancesThatDontMatchInstanceFilter, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ECSchemaHelperTests, GetRelationshipPaths_DoesntReturnPathsWithNonPolymorphicTargetInstancesThatDontMatchInstanceFilter)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECRelationshipClassCP relAB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR inst) {inst.SetValue("Prop", ECValue(123)); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b);

    CustomFunctionsInjector incjectCustomFunctions(m_connections, *m_connection);

    ECClassUseCounter counter;
    RelationshipPathSpecification pathToRelatedInstanceSpec(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward));
    bvector<ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification> pathSpecs = { ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification(0, pathToRelatedInstanceSpec, false, {"this.Prop = 456"}) };
    ECSchemaHelper::RelationshipPathsRequestParams params(SelectClass<ECClass>(*classA, ""), pathSpecs, nullptr, bvector<RelatedClassPath>(), counter, true);

    ECSchemaHelper::RelationshipPathsResponse response = m_helper->GetRelationshipPaths(params);
    EXPECT_EQ(0, response.GetPaths(0).size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelationshipPaths_ReturnsPathsWithPolymorphicTargetInstancesMatchingInstanceFilter, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ECSchemaHelperTests, GetRelationshipPaths_ReturnsPathsWithPolymorphicTargetInstancesMatchingInstanceFilter)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECRelationshipClassCP relAB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR inst) {inst.SetValue("Prop", ECValue(123)); });
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR inst) {inst.SetValue("Prop", ECValue(123)); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *c);

    CustomFunctionsInjector incjectCustomFunctions(m_connections, *m_connection);

    ECClassUseCounter counter;
    RelationshipPathSpecification pathToRelatedInstanceSpec(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward));
    bvector<ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification> pathSpecs = { ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification(0, pathToRelatedInstanceSpec, true, {"this.Prop = 123"}) };
    ECSchemaHelper::RelationshipPathsRequestParams params(SelectClass<ECClass>(*classA, ""), pathSpecs, nullptr, bvector<RelatedClassPath>(), counter, true);

    ECSchemaHelper::RelationshipPathsResponse response = m_helper->GetRelationshipPaths(params);
    ASSERT_EQ(2, response.GetPaths(0).size());
    EXPECT_EQ(classB, &response.GetPaths(0)[0].m_path.back().GetTargetClass().GetClass());
    EXPECT_EQ(classC, &response.GetPaths(0)[1].m_path.back().GetTargetClass().GetClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelationshipPaths_DoesntReturnPathsWithPolymorphicTargetInstancesThatDontMatchInstanceFilter, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ECSchemaHelperTests, GetRelationshipPaths_DoesntReturnPathsWithPolymorphicTargetInstancesThatDontMatchInstanceFilter)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECRelationshipClassCP relAB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR inst) {inst.SetValue("Prop", ECValue(123)); });
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR inst) {inst.SetValue("Prop", ECValue(123)); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *c);

    CustomFunctionsInjector incjectCustomFunctions(m_connections, *m_connection);

    ECClassUseCounter counter;
    RelationshipPathSpecification pathToRelatedInstanceSpec(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward));
    bvector<ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification> pathSpecs = { ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification(0, pathToRelatedInstanceSpec, true, {"this.Prop = 456"}) };
    ECSchemaHelper::RelationshipPathsRequestParams params(SelectClass<ECClass>(*classA, ""), pathSpecs, nullptr, bvector<RelatedClassPath>(), counter, true);

    ECSchemaHelper::RelationshipPathsResponse response = m_helper->GetRelationshipPaths(params);
    EXPECT_EQ(0, response.GetPaths(0).size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelationshipPaths_ReturnsPathsWithTargetInstancesMatchingAllStepsInstanceFilters, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="string" />
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
TEST_F(ECSchemaHelperTests, GetRelationshipPaths_ReturnsPathsWithTargetInstancesMatchingAllStepsInstanceFilters)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECRelationshipClassCP relAB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B_C")->GetRelationshipClassCP();
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR inst){inst.SetValue("PropB", ECValue("aaa"));});
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR inst){inst.SetValue("PropB", ECValue("bbb"));});
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR inst){inst.SetValue("PropC", ECValue("ccc"));});
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC, [](IECInstanceR inst){inst.SetValue("PropC", ECValue("ddd"));});
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b1, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b2, *c2);

    CustomFunctionsInjector incjectCustomFunctions(m_connections, *m_connection);

    ECClassUseCounter counter;
    RelationshipPathSpecification pathToRelatedInstanceSpec(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward),
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)
        });
    bvector<ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification> pathSpecs = { ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification(0, pathToRelatedInstanceSpec,
        false, {"this.PropB = \"bbb\"", "this.PropC = \"ddd\""})};
    ECSchemaHelper::RelationshipPathsRequestParams params(SelectClass<ECClass>(*classA, ""), pathSpecs, nullptr, bvector<RelatedClassPath>(), counter, true);

    ECSchemaHelper::RelationshipPathsResponse response = m_helper->GetRelationshipPaths(params);
    EXPECT_EQ(1, response.GetPaths(0).size());
    EXPECT_EQ(2, response.GetPaths(0)[0].m_path.size());
    EXPECT_EQ(classB, &response.GetPaths(0)[0].m_path[0].GetTargetClass().GetClass());
    EXPECT_STREQ(Utf8PrintfString("[%s].[PropB] = 'bbb'", response.GetPaths(0)[0].m_path[0].GetTargetClass().GetAlias().c_str()).c_str(), response.GetPaths(0)[0].m_path[0].GetTargetInstanceFilter().c_str());
    EXPECT_EQ(classC, &response.GetPaths(0)[0].m_path[1].GetTargetClass().GetClass());
    EXPECT_STREQ(Utf8PrintfString("[%s].[PropC] = 'ddd'", response.GetPaths(0)[0].m_path[1].GetTargetClass().GetAlias().c_str()).c_str(), response.GetPaths(0)[0].m_path[1].GetTargetInstanceFilter().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelationshipPaths_ReturnsPathsWithTargetInstancesMatchingFirstStepInstanceFilter, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECProperty propertyName="PropB" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="C" />
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
TEST_F(ECSchemaHelperTests, GetRelationshipPaths_ReturnsPathsWithTargetInstancesMatchingFirstStepInstanceFilter)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECRelationshipClassCP relAB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B_C")->GetRelationshipClassCP();
    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR inst){inst.SetValue("PropB", ECValue("aaa"));});
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB, [](IECInstanceR inst){inst.SetValue("PropB", ECValue("bbb"));});
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b1, *c1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b2, *c2);

    CustomFunctionsInjector incjectCustomFunctions(m_connections, *m_connection);

    ECClassUseCounter counter;
    RelationshipPathSpecification pathToRelatedInstanceSpec(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward),
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)
        });
    bvector<ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification> pathSpecs = { ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification(0, pathToRelatedInstanceSpec,
        false, {"this.PropB = \"bbb\""}) };
    ECSchemaHelper::RelationshipPathsRequestParams params(SelectClass<ECClass>(*classA, ""), pathSpecs, nullptr, bvector<RelatedClassPath>(), counter, true);

    ECSchemaHelper::RelationshipPathsResponse response = m_helper->GetRelationshipPaths(params);
    EXPECT_EQ(1, response.GetPaths(0).size());
    EXPECT_EQ(2, response.GetPaths(0)[0].m_path.size());
    EXPECT_EQ(classB, &response.GetPaths(0)[0].m_path[0].GetTargetClass().GetClass());
    EXPECT_STREQ(Utf8PrintfString("[%s].[PropB] = 'bbb'", response.GetPaths(0)[0].m_path[0].GetTargetClass().GetAlias().c_str()).c_str(), response.GetPaths(0)[0].m_path[0].GetTargetInstanceFilter().c_str());
    EXPECT_EQ(classC, &response.GetPaths(0)[0].m_path[1].GetTargetClass().GetClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelationshipPaths_ReturnsPathsWithRequestedTargetWhenRequestingNonPolymorphically, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ECSchemaHelperTests, GetRelationshipPaths_ReturnsPathsWithRequestedTargetWhenRequestingNonPolymorphically)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECRelationshipClassCP relAB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a, *c);

    CustomFunctionsInjector injectCustomFunctions(m_connections, *m_connection);

    ECClassUseCounter counter;
    RelationshipPathSpecification pathToRelatedInstanceSpec(*new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward));
    bvector<ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification> pathSpecs = { ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification(0, pathToRelatedInstanceSpec, false, {}) };
    ECSchemaHelper::RelationshipPathsRequestParams params(SelectClass<ECClass>(*classA, ""), pathSpecs, nullptr, bvector<RelatedClassPath>(), counter, true);

    ECSchemaHelper::RelationshipPathsResponse response = m_helper->GetRelationshipPaths(params);
    ASSERT_EQ(1, response.GetPaths(0).size());
    ASSERT_EQ(1, response.GetPaths(0)[0].m_path.size());
    RelatedClassCR path = response.GetPaths(0)[0].m_path[0];
    EXPECT_EQ(classA, path.GetSourceClass());
    EXPECT_EQ(relAB, &path.GetRelationship().GetClass());
    EXPECT_TRUE(path.IsForwardRelationship());
    EXPECT_EQ(classB, &path.GetTargetClass().GetClass());
    EXPECT_TRUE(path.GetTargetClass().IsSelectPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelationshipPaths_ReturnsPathsWhenIntermediateClassesAreDifferent, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B1">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="B2">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C" />
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="B_C" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="bc" polymorphic="true">
            <Class class="B" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="cb" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ECSchemaHelperTests, GetRelationshipPaths_ReturnsPathsWhenIntermediateClassesAreDifferent)
    {
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB1 = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B1");
    ECClassCP classB2 = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B2");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECRelationshipClassCP relAB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A_B")->GetRelationshipClassCP();
    ECRelationshipClassCP relBC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B_C")->GetRelationshipClassCP();

    IECInstancePtr a1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB1);
    IECInstancePtr c1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a1, *b1);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b1, *c1);

    IECInstancePtr a2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB2);
    IECInstancePtr c2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relAB, *a2, *b2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relBC, *b2, *c2);

    CustomFunctionsInjector injectCustomFunctions(m_connections, *m_connection);

    ECClassUseCounter counter;
    RelationshipPathSpecification pathToRelatedInstanceSpec(
        {
        new RelationshipStepSpecification(relAB->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward),
        new RelationshipStepSpecification(relBC->GetFullName(), RequiredRelationDirection::RequiredRelationDirection_Forward)
        });
    bvector<ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification> pathSpecs = { ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification(0, pathToRelatedInstanceSpec, true, {}) };
    ECSchemaHelper::RelationshipPathsRequestParams params(SelectClass<ECClass>(*classA, ""), pathSpecs, nullptr, bvector<RelatedClassPath>(), counter, true);

    ECSchemaHelper::RelationshipPathsResponse response = m_helper->GetRelationshipPaths(params);
    ASSERT_EQ(2, response.GetPaths(0).size());

    ASSERT_EQ(2, response.GetPaths(0)[0].m_path.size());
    RelatedClassPath path1 = response.GetPaths(0)[0].m_path;
    EXPECT_EQ(relAB, &path1[0].GetRelationship().GetClass());
    EXPECT_TRUE(path1[0].IsForwardRelationship());
    EXPECT_EQ(classB1, &path1[0].GetTargetClass().GetClass());
    EXPECT_EQ(relBC, &path1[1].GetRelationship().GetClass());
    EXPECT_TRUE(path1[1].IsForwardRelationship());
    EXPECT_EQ(classC, &path1[1].GetTargetClass().GetClass());

    ASSERT_EQ(2, response.GetPaths(0)[1].m_path.size());
    RelatedClassPath path2 = response.GetPaths(0)[1].m_path;
    EXPECT_EQ(relAB, &path2[0].GetRelationship().GetClass());
    EXPECT_TRUE(path2[0].IsForwardRelationship());
    EXPECT_EQ(classB2, &path2[0].GetTargetClass().GetClass());
    EXPECT_EQ(relBC, &path2[1].GetRelationship().GetClass());
    EXPECT_TRUE(path2[1].IsForwardRelationship());
    EXPECT_EQ(classC, &path2[1].GetTargetClass().GetClass());
    }

#ifdef wip
/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelatedPropertyPathsWithInstances_ReturnsEmptyWhenEmptyPropertyPathIsGiven, R"*(
    <ECEntityClass typeName="Element" />
)*");
TEST_F(ECSchemaHelperTests, GetRelatedPropertyPathsWithInstances_ReturnsEmptyWhenEmptyPropertyPathIsGiven)
    {
    ECClassCP elementClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Element");
    EXPECT_TRUE(m_helper->GetRelatedPropertyPathsWithInstances(RelatedClassPath(), SelectClass<ECClass>(*elementClass, "this"), { }, nullptr).empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelatedPropertyPathsWithInstances_ReturnsPathsWithTargetClassInstances, R"*(
    <ECEntityClass typeName="Model" />
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Element" />
)*");
TEST_F(ECSchemaHelperTests, GetRelatedPropertyPathsWithInstances_ReturnsPathsWithTargetClassInstances)
    {
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ModelContainsElements")->GetRelationshipClassCP();
    ECClassCP modelClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Model");
    ECClassCP elementClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Element");

    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model, *element);

    RelatedClassPath path = {RelatedClass(*modelClass, SelectClass<ECRelationshipClass>(*rel, "r"), true, SelectClass<ECClass>(*elementClass, "e", true))};
    bvector<RelatedClassPath> pathsWithInstances = m_helper->GetRelatedPropertyPathsWithInstances(RelatedClassPath(), SelectClass<ECClass>(*elementClass, "this"), { path }, nullptr);
    EXPECT_EQ(1, pathsWithInstances.size());
    EXPECT_TRUE(pathsWithInstances[0] == path);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelatedPropertyPathsWithInstances_ReturnsOnlyPathsWithTargetClassInstances, R"*(
    <ECEntityClass typeName="Model" />
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="A">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
)*");
TEST_F(ECSchemaHelperTests, GetRelatedPropertyPathsWithInstances_ReturnsOnlyPathsWithTargetClassInstances)
    {
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ModelContainsElements")->GetRelationshipClassCP();
    ECClassCP modelClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Model");
    ECClassCP elementClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Element");
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");

    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr elementB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model, *elementB);

    bvector<RelatedClassPath> paths = {
        {RelatedClass(*modelClass, SelectClass<ECRelationshipClass>(*rel, "r"), true, SelectClass<ECClass>(*elementClass, "e", true))},
        {RelatedClass(*modelClass, SelectClass<ECRelationshipClass>(*rel, "r"), true, SelectClass<ECClass>(*classA, "a", true))},
        {RelatedClass(*modelClass, SelectClass<ECRelationshipClass>(*rel, "r"), true, SelectClass<ECClass>(*classB, "b", true))}
        };

    bvector<RelatedClassPath> pathsWithInstances = m_helper->GetRelatedPropertyPathsWithInstances(RelatedClassPath(), SelectClass<ECClass>(*modelClass, "this"), paths, nullptr);
    ASSERT_EQ(1, pathsWithInstances.size());
    EXPECT_EQ(pathsWithInstances[0], paths[2]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelatedPropertyPathsWithInstances_ReturnsMultiRelationshipPathsWithTargetClassInstances, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="A">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
)*");
TEST_F(ECSchemaHelperTests, GetRelatedPropertyPathsWithInstances_ReturnsMultiRelationshipPathsWithTargetClassInstances)
    {
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ElementOwnsChildElements")->GetRelationshipClassCP();
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA);
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *c);

    RelatedClassPath pathFromSelectClass = {
        RelatedClass(*classB, SelectClass<ECRelationshipClass>(*rel, "ba"), false, SelectClass<ECClass>(*classA, "a")),
        RelatedClass(*classA, SelectClass<ECRelationshipClass>(*rel, "ac"), true, SelectClass<ECClass>(*classC, "c")),
        };

    bvector<RelatedClassPath> pathsWithInstances = m_helper->GetRelatedPropertyPathsWithInstances(RelatedClassPath(), SelectClass<ECClass>(*classB, "this"), { pathFromSelectClass }, nullptr);
    ASSERT_EQ(1, pathsWithInstances.size());
    EXPECT_EQ(pathsWithInstances[0], pathFromSelectClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelatedPropertyPathsWithInstances_ReturnsPathsWithProvidedAnEmptyInstanceFilter, R"*(
    <ECEntityClass typeName="Model">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Element" />
)*");
TEST_F(ECSchemaHelperTests, GetRelatedPropertyPathsWithInstances_ReturnsPathsWithProvidedAnEmptyInstanceFilter)
    {
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ModelContainsElements")->GetRelationshipClassCP();
    ECClassCP modelClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Model");
    ECClassCP elementClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Element");

    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(1)); });
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model, *element);

    RelatedClassPath path = {RelatedClass(*modelClass, SelectClass<ECRelationshipClass>(*rel, "r"), true, SelectClass<ECClass>(*elementClass, "e", true))};
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), nullptr, SelectClassInfo(*modelClass, "this", true), nullptr, nullptr);

    bvector<RelatedClassPath> pathsWithInstances = m_helper->GetRelatedPropertyPathsWithInstances(RelatedClassPath(), SelectClass<ECClass>(*modelClass, "this"), { path }, &filteringParams);
    ASSERT_EQ(1, pathsWithInstances.size());
    EXPECT_EQ(pathsWithInstances[0], path);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelatedPropertyPathsWithInstances_ReturnsPathsWithInstancesMatchingInstanceFilter, R"*(
    <ECEntityClass typeName="Model">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Element" />
)*");
TEST_F(ECSchemaHelperTests, GetRelatedPropertyPathsWithInstances_ReturnsPathsWithInstancesMatchingInstanceFilter)
    {
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ModelContainsElements")->GetRelationshipClassCP();
    ECClassCP modelClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Model");
    ECClassCP elementClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Element");

    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model, *element);

    RelatedClassPath path = {RelatedClass(*modelClass, SelectClass<ECRelationshipClass>(*rel, "r"), true, SelectClass<ECClass>(*elementClass, "e", true))};
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), nullptr, SelectClassInfo(*modelClass, "this", true), nullptr, "this.Prop = 1");

    bvector<RelatedClassPath> pathsWithInstances = m_helper->GetRelatedPropertyPathsWithInstances(RelatedClassPath(), SelectClass<ECClass>(*modelClass, "this"), { path }, &filteringParams);
    ASSERT_EQ(1, pathsWithInstances.size());
    EXPECT_EQ(pathsWithInstances[0], path);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelatedPropertyPathsWithInstances_ReturnsEmptyIfNoInstancesMatchInstanceFilter, R"*(
    <ECEntityClass typeName="Model">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Element" />
)*");
TEST_F(ECSchemaHelperTests, GetRelatedPropertyPathsWithInstances_ReturnsEmptyIfNoInstancesMatchInstanceFilter)
    {
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ModelContainsElements")->GetRelationshipClassCP();
    ECClassCP modelClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Model");
    ECClassCP elementClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Element");

    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model, *element);

    RelatedClassPath path = {RelatedClass(*modelClass, SelectClass<ECRelationshipClass>(*rel, "r"), true, SelectClass<ECClass>(*elementClass, "e", true))};
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), nullptr, SelectClassInfo(*modelClass, "this", true), nullptr, "this.Prop = 2");

    bvector<RelatedClassPath> pathsWithInstances = m_helper->GetRelatedPropertyPathsWithInstances(RelatedClassPath(), SelectClass<ECClass>(*modelClass, "this"), { path }, &filteringParams);
    EXPECT_EQ(0, pathsWithInstances.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelatedPropertyPathsWithInstances_ReturnsPathsWithInstancesRelatedToGivenInput, R"*(
    <ECEntityClass typeName="Model" />
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Element" />
)*");
TEST_F(ECSchemaHelperTests, GetRelatedPropertyPathsWithInstances_ReturnsPathsWithInstancesRelatedToGivenInput)
    {
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ModelContainsElements")->GetRelationshipClassCP();
    ECClassCP modelClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Model");
    ECClassCP elementClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Element");

    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model, *element);

    RelatedClassPath path = {RelatedClass(*modelClass, SelectClass<ECRelationshipClass>(*rel, "r"), true, SelectClass<ECClass>(*elementClass, "e", true))};
    TestParsedInput input(*model);
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), &input, SelectClassInfo(*modelClass, "this", true), nullptr, "");

    bvector<RelatedClassPath> pathsWithInstances = m_helper->GetRelatedPropertyPathsWithInstances(RelatedClassPath(), SelectClass<ECClass>(*modelClass, "this"), { path }, &filteringParams);
    ASSERT_EQ(1, pathsWithInstances.size());
    EXPECT_EQ(pathsWithInstances[0], path);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelatedPropertyPathsWithInstances_ReturnsEmptyIfNoInstancesRelatedToGivenInput, R"*(
    <ECEntityClass typeName="Model" />
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Element" />
)*");
TEST_F(ECSchemaHelperTests, GetRelatedPropertyPathsWithInstances_ReturnsEmptyIfNoInstancesRelatedToGivenInput)
    {
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ModelContainsElements")->GetRelationshipClassCP();
    ECClassCP modelClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Model");
    ECClassCP elementClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Element");

    IECInstancePtr model1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr model2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model1, *element);

    RelatedClassPath path = {RelatedClass(*modelClass, SelectClass<ECRelationshipClass>(*rel, "r"), true, SelectClass<ECClass>(*elementClass, "e", true))};
    TestParsedInput input(*model2);
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), &input, SelectClassInfo(*modelClass, "this", true), nullptr, "");

    bvector<RelatedClassPath> pathsWithInstances = m_helper->GetRelatedPropertyPathsWithInstances(RelatedClassPath(), SelectClass<ECClass>(*modelClass, "this"), { path }, &filteringParams);
    EXPECT_EQ(0, pathsWithInstances.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelatedPropertyPathsWithInstances_ReturnsEmptyIfGivenInputIsEmpty, R"*(
    <ECEntityClass typeName="Model" />
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Element" />
)*");
TEST_F(ECSchemaHelperTests, GetRelatedPropertyPathsWithInstances_ReturnsEmptyIfGivenInputIsEmpty)
    {
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ModelContainsElements")->GetRelationshipClassCP();
    ECClassCP modelClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Model");
    ECClassCP elementClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Element");

    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model, *element);

    RelatedClassPath path = {RelatedClass(*modelClass, SelectClass<ECRelationshipClass>(*rel, "r"), true, SelectClass<ECClass>(*elementClass, "e", true))};
    TestParsedInput input;
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), &input, SelectClassInfo(*modelClass, "this", true), nullptr, "");

    bvector<RelatedClassPath> pathsWithInstances = m_helper->GetRelatedPropertyPathsWithInstances(RelatedClassPath(), SelectClass<ECClass>(*modelClass, "this"), { path }, &filteringParams);
    EXPECT_EQ(0, pathsWithInstances.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelatedPropertyPathsWithInstances_ReturnsInstancesRelatedToGivenClassIfInputContainsClassWithoutInstances, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementA">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ElementB">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="Aspect">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementHasAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Aspect" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="AspectA">
        <BaseClass>Aspect</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="AspectB">
        <BaseClass>Aspect</BaseClass>
    </ECEntityClass>
)*");
TEST_F(ECSchemaHelperTests, GetRelatedPropertyPathsWithInstances_ReturnsInstancesRelatedToGivenClassIfInputContainsClassWithoutInstances)
    {
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ElementHasAspect")->GetRelationshipClassCP();
    ECClassCP elementClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Element");
    ECClassCP elementAClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ElementA");
    ECClassCP elementBClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ElementB");
    ECClassCP aspectAClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "AspectA");
    ECClassCP aspectBClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "AspectB");

    IECInstancePtr elementA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementAClass);
    IECInstancePtr elementB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementBClass);
    IECInstancePtr aspectA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass);
    IECInstancePtr aspectB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *elementA, *aspectA);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *elementB, *aspectB);

    RelatedClassPath pathA = {RelatedClass(*elementClass, SelectClass<ECRelationshipClass>(*rel, "r"), true, SelectClass<ECClass>(*aspectAClass, "a", true))};
    RelatedClassPath pathB = {RelatedClass(*elementClass, SelectClass<ECRelationshipClass>(*rel, "r"), true, SelectClass<ECClass>(*aspectBClass, "b", true))};
    TestParsedInput input(*elementAClass, bvector<ECInstanceId>());
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), &input, SelectClassInfo(*elementAClass, "this", true), nullptr, "");

    bvector<RelatedClassPath> pathsWithInstances = m_helper->GetRelatedPropertyPathsWithInstances(RelatedClassPath(), SelectClass<ECClass>(*elementAClass, "this"), {pathA, pathB}, &filteringParams);
    EXPECT_EQ(1, pathsWithInstances.size());
    EXPECT_EQ(pathsWithInstances[0], pathA);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelatedPropertyPathsWithInstances_ReturnsInstancesRelatedToGivenClassIfThereIsNoInputKeys, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementA">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="ElementB">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="Aspect">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementHasAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Aspect" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="AspectA">
        <BaseClass>Aspect</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="AspectB">
        <BaseClass>Aspect</BaseClass>
    </ECEntityClass>
)*");
TEST_F(ECSchemaHelperTests, GetRelatedPropertyPathsWithInstances_ReturnsInstancesRelatedToGivenClassIfThereIsNoInputKeys)
    {
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ElementHasAspect")->GetRelationshipClassCP();
    ECClassCP elementClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Element");
    ECClassCP elementAClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ElementA");
    ECClassCP elementBClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ElementB");
    ECClassCP aspectAClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "AspectA");
    ECClassCP aspectBClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "AspectB");

    IECInstancePtr elementA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementAClass);
    IECInstancePtr elementB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementBClass);
    IECInstancePtr aspectA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass);
    IECInstancePtr aspectB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *elementA, *aspectA);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *elementB, *aspectB);

    RelatedClassPath pathA = {RelatedClass(*elementClass, SelectClass<ECRelationshipClass>(*rel, "r"), true, SelectClass<ECClass>(*aspectAClass, "a", true))};
    RelatedClassPath pathB = {RelatedClass(*elementClass, SelectClass<ECRelationshipClass>(*rel, "r"), true, SelectClass<ECClass>(*aspectBClass, "b", true))};
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), nullptr, SelectClassInfo(*elementAClass, "this", true), nullptr, "");

    bvector<RelatedClassPath> pathsWithInstances = m_helper->GetRelatedPropertyPathsWithInstances(RelatedClassPath(), SelectClass<ECClass>(*elementAClass, "this"), {pathA, pathB}, &filteringParams);
    EXPECT_EQ(1, pathsWithInstances.size());
    EXPECT_EQ(pathsWithInstances[0], pathA);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelatedPropertyPathsWithInstances_ReturnsPathsWithInstancesMatchingNestedInstanceFilter, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="A">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="E">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
)*");
TEST_F(ECSchemaHelperTests, GetRelatedPropertyPathsWithInstances_ReturnsPathsWithInstancesMatchingNestedInstanceFilter)
    {
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ElementOwnsChildElements")->GetRelationshipClassCP();
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECClassCP classD = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "D");
    ECClassCP classE = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "E");

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr d = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr e = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *b, *c);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *c, *d);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *d, *e);

    RelatedClassPath pathToSelectClass = {
        RelatedClass(*classA, SelectClass<ECRelationshipClass>(*rel, "ab"), true, SelectClass<ECClass>(*classB, "b")),
        RelatedClass(*classB, SelectClass<ECRelationshipClass>(*rel, "bc"), true, SelectClass<ECClass>(*classC, "c")),
        };
    RelatedClassPath pathFromSelectClass = {
        RelatedClass(*classC, SelectClass<ECRelationshipClass>(*rel, "cd"), true, SelectClass<ECClass>(*classD, "d")),
        RelatedClass(*classD, SelectClass<ECRelationshipClass>(*rel, "de"), true, SelectClass<ECClass>(*classE, "e")),
        };
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), nullptr, SelectClassInfo(*classA, "this", true), nullptr, "this.Prop = 1");

    bvector<RelatedClassPath> pathsWithInstances = m_helper->GetRelatedPropertyPathsWithInstances(pathToSelectClass, SelectClass<ECClass>(*classC, "this"), { pathFromSelectClass }, &filteringParams);
    ASSERT_EQ(1, pathsWithInstances.size());
    EXPECT_EQ(pathsWithInstances[0], pathFromSelectClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelatedPropertyPathsWithInstances_ReturnsEmptyIfNoInstancesMatchNestedInstanceFilter, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="A">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="E">
        <BaseClass>Element</BaseClass>
    </ECEntityClass>
)*");
TEST_F(ECSchemaHelperTests, GetRelatedPropertyPathsWithInstances_ReturnsEmptyIfNoInstancesMatchNestedInstanceFilter)
    {
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ElementOwnsChildElements")->GetRelationshipClassCP();
    ECClassCP classA = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "A");
    ECClassCP classB = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "B");
    ECClassCP classC = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "C");
    ECClassCP classD = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "D");
    ECClassCP classE = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "E");

    IECInstancePtr a = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classA, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(2)); });
    IECInstancePtr b = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classB);
    IECInstancePtr c = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classC);
    IECInstancePtr d = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classD);
    IECInstancePtr e = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classE);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *a, *b);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *b, *c);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *c, *d);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *d, *e);

    RelatedClassPath pathToSelectClass = {
        RelatedClass(*classA, SelectClass<ECRelationshipClass>(*rel, "ab"), true, SelectClass<ECClass>(*classB, "b")),
        RelatedClass(*classB, SelectClass<ECRelationshipClass>(*rel, "bc"), true, SelectClass<ECClass>(*classC, "c")),
        };
    RelatedClassPath pathFromSelectClass = {
        RelatedClass(*classC, SelectClass<ECRelationshipClass>(*rel, "cd"), true, SelectClass<ECClass>(*classD, "d")),
        RelatedClass(*classD, SelectClass<ECRelationshipClass>(*rel, "de"), true, SelectClass<ECClass>(*classE, "e")),
        };
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), nullptr, SelectClassInfo(*classA, "this", true), nullptr, "this.Prop = 1");

    bvector<RelatedClassPath> pathsWithInstances = m_helper->GetRelatedPropertyPathsWithInstances(pathToSelectClass, SelectClass<ECClass>(*classC, "this"), { pathFromSelectClass }, &filteringParams);
    EXPECT_EQ(0, pathsWithInstances.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelatedPropertyPathsWithInstances_ReturnsPathsWithInstancesMatchingRelatedInstanceFilter, R"*(
    <ECEntityClass typeName="Model" />
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Element" />
    <ECRelationshipClass typeName="ElementHasAspects" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Aspect" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Aspect">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(ECSchemaHelperTests, GetRelatedPropertyPathsWithInstances_ReturnsPathsWithInstancesMatchingRelatedInstanceFilter)
    {
    ECRelationshipClassCP relModelToElement = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ModelContainsElements")->GetRelationshipClassCP();
    ECRelationshipClassCP relElementToAspect = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ElementHasAspects")->GetRelationshipClassCP();
    ECClassCP classModel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Model");
    ECClassCP classElement = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Element");
    ECClassCP classAspect = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Aspect");

    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classModel);
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classElement);
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *classAspect, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(1)); });
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relModelToElement, *model, *element);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *relElementToAspect, *element, *aspect);

    RelatedClassPath pathFromSelectClass = {RelatedClass(*classElement, SelectClass<ECRelationshipClass>(*relModelToElement, "model_to_elements"), false, SelectClass<ECClass>(*classModel, "model"))};
    SelectClassInfo selectInfo(*classElement, "this", true);
    selectInfo.SetRelatedInstancePaths({RelatedClassPath{RelatedClass(*classElement, SelectClass<ECRelationshipClass>(*relElementToAspect, "element_to_aspect"), true, SelectClass<ECClass>(*classAspect, "aspect"))}});
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), nullptr, selectInfo, nullptr, "aspect.Prop = 1");

    bvector<RelatedClassPath> pathsWithInstances = m_helper->GetRelatedPropertyPathsWithInstances(RelatedClassPath(), SelectClass<ECClass>(*classElement, "this"), { pathFromSelectClass }, &filteringParams);
    ASSERT_EQ(1, pathsWithInstances.size());
    EXPECT_EQ(pathsWithInstances[0], pathFromSelectClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_GetRelatedPropertyPathsWithInstances_ReturnsCorrectResultForRelatedToRecursivelyFilteredInstances, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Aspect">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementHasAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Aspect" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="AspectA">
        <BaseClass>Aspect</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="AspectB">
        <BaseClass>Aspect</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="AspectC">
        <BaseClass>Aspect</BaseClass>
    </ECEntityClass>
)*");
TEST_F(ECSchemaHelperTests, DEPRECATED_GetRelatedPropertyPathsWithInstances_ReturnsCorrectResultForRelatedToRecursivelyFilteredInstances)
    {
    ECClassCP elementClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Element");
    ECRelationshipClassCP elementToElementRel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ElementOwnsChildElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementToAspectRel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ElementHasAspect")->GetRelationshipClassCP();
    ECClassCP aspectAClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "AspectA");
    ECClassCP aspectBClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "AspectB");
    ECClassCP aspectCClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "AspectC");

    IECInstancePtr e1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr e2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementToElementRel, *e1, *e2);
    IECInstancePtr e3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementToElementRel, *e2, *e3);
    IECInstancePtr e4 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspectA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementToAspectRel, *e2, *aspectA);
    IECInstancePtr aspectB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementToAspectRel, *e3, *aspectB);
    IECInstancePtr aspectC = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectCClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementToAspectRel, *e4, *aspectC);

    SelectClassInfo selectInfo(*elementClass, "", true);
    RelatedClassPath pathToSelectClass{RelatedClass(*elementClass, SelectClass<ECRelationshipClass>(*elementToElementRel, "e_to_e"), true, SelectClass<ECClass>(*elementClass, "e"))};
    selectInfo.SetPathFromInputToSelectClass(pathToSelectClass);
    InstanceFilteringParams::RecursiveQueryInfo recursiveInfo({pathToSelectClass});
    TestParsedInput input(*e1);
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), &input, selectInfo, &recursiveInfo, "");

    bvector<RelatedClassPath> paths = {
        {RelatedClass(*elementClass, SelectClass<ECRelationshipClass>(*elementToAspectRel, "r"), true, SelectClass<ECClass>(*aspectAClass, "a", true))},
        {RelatedClass(*elementClass, SelectClass<ECRelationshipClass>(*elementToAspectRel, "r"), true, SelectClass<ECClass>(*aspectBClass, "b", true))},
        {RelatedClass(*elementClass, SelectClass<ECRelationshipClass>(*elementToAspectRel, "r"), true, SelectClass<ECClass>(*aspectCClass, "c", true))}
        };

    bvector<RelatedClassPath> pathsWithInstances = m_helper->GetRelatedPropertyPathsWithInstances(RelatedClassPath(), SelectClass<ECClass>(*elementClass, "this"), paths, &filteringParams);
    ASSERT_EQ(2, pathsWithInstances.size());
    EXPECT_EQ(pathsWithInstances[0], paths[0]);
    EXPECT_EQ(pathsWithInstances[1], paths[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DEPRECATED_GetRelatedPropertyPathsWithInstances_ReturnsEmptyWhenThereAreNoRecursivelyRelatedFilteredInstances, R"*(
    <ECEntityClass typeName="Element">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Aspect" />
    <ECRelationshipClass typeName="ElementHasAspect" strength="embedding" modifier="None">
        <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true">
            <Class class="Aspect" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(ECSchemaHelperTests, DEPRECATED_GetRelatedPropertyPathsWithInstances_ReturnsEmptyWhenThereAreNoRecursivelyRelatedFilteredInstances)
    {
    ECClassCP elementClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Element");
    ECRelationshipClassCP elementToElementRel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ElementOwnsChildElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementToAspectRel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ElementHasAspect")->GetRelationshipClassCP();
    ECClassCP aspectClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Aspect");

    IECInstancePtr e1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr e2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr aspect = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementToAspectRel, *e2, *aspect);

    SelectClassInfo selectInfo(*elementClass, "this", true);
    RelatedClassPath pathToSelectClass{RelatedClass(*elementClass, SelectClass<ECRelationshipClass>(*elementToElementRel, "e_to_e"), true, SelectClass<ECClass>(*elementClass, "e"))};
    selectInfo.SetPathFromInputToSelectClass(pathToSelectClass);
    InstanceFilteringParams::RecursiveQueryInfo recursiveInfo({pathToSelectClass});
    TestParsedInput input(*e1);
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), &input, selectInfo, &recursiveInfo, "");

    bvector<RelatedClassPath> pathsWithInstances = m_helper->GetRelatedPropertyPathsWithInstances(pathToSelectClass, SelectClass<ECClass>(*elementClass, "this"),
        {
            {RelatedClass(*elementClass, SelectClass<ECRelationshipClass>(*elementToAspectRel, "r"), true, SelectClass<ECClass>(*aspectClass, "a", true))},
        }, &filteringParams);
    EXPECT_EQ(0, pathsWithInstances.size());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaHelperTests, GetECClassesFromClassList_IgnoresInvalidClasses)
    {
    MultiSchemaClass schemaClassDef("test", true, { "does-not-exist" });
    auto result = m_helper->GetECClassesFromClassList({ &schemaClassDef }, false);
    EXPECT_EQ(0, result.size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ECInstancesHelperTests::SetUp()
    {
    ECPresentationTest::SetUp();
    m_project.Create("ECInstancesHelperTests", "RulesEngineTest.01.00.ecschema.xml");
    m_connection = new TestConnection(m_project.GetECDb());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ECInstancesHelperTests::TearDown()
    {
    ECPresentationTest::TearDown();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECInstancesHelperTests, LoadInstance_LoadsInstanceAfterSchemaImport)
    {
    ECInstanceKey key(m_project.GetECDb().Schemas().GetClassId("ECDbMeta", "ECClassDef"), ECInstanceId((uint64_t)1));
    IECInstancePtr instance;
    DbResult result = ECInstancesHelper::LoadInstance(instance, *m_connection, key);
    EXPECT_EQ(BE_SQLITE_ROW, result);
    EXPECT_TRUE(instance.IsValid());

    // import a schema and attempt to load the instance again
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaReadContext = ECSchemaReadContext::CreateContext();
    schemaReadContext->AddSchemaLocater(m_project.GetECDb().GetSchemaLocater());
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_BASIC_1, *schemaReadContext));
    BentleyStatus status = m_project.GetECDb().Schemas().ImportSchemas({ schema.get() });
    ASSERT_EQ(SUCCESS, status);
    m_connection->NotifyConnectionReset();

    result = ECInstancesHelper::LoadInstance(instance, *m_connection, key);
    EXPECT_EQ(BE_SQLITE_ROW, result);
    EXPECT_TRUE(instance.IsValid());

    // repeat 2nd time
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, SCHEMA_BASIC_2, *schemaReadContext));
    status = m_project.GetECDb().Schemas().ImportSchemas({ schema.get() });
    ASSERT_EQ(SUCCESS, status);
    m_connection->NotifyConnectionReset();

    result = ECInstancesHelper::LoadInstance(instance, *m_connection, key);
    EXPECT_EQ(BE_SQLITE_ROW, result);
    EXPECT_TRUE(instance.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECInstancesHelperTests, CachesPreparedStatementsThatRequireLoadingECInstances)
    {
    ConnectionManager manager;
    IConnectionCPtr primaryConnection = manager.CreateConnection(m_project.GetECDb());

    ECClassCP ecClass = m_project.GetECDb().Schemas().GetClass("RulesEngineTest", "Widget");
    IECInstancePtr widget1 = RulesEngineTestHelpers::InsertInstance(m_project.GetECDb(), *ecClass, [](IECInstanceR instance) {instance.SetValue("IntProperty", ECValue(12)); }, true);
    IECInstancePtr widget2 = RulesEngineTestHelpers::InsertInstance(m_project.GetECDb(), *ecClass, [](IECInstanceR instance) {instance.SetValue("IntProperty", ECValue(0)); }, true);

    ECInstanceKey key1(m_project.GetECDb().Schemas().GetClassId("RulesEngineTest", "Widget"), ECInstanceId((uint64_t)1));
    ECInstanceKey key2(m_project.GetECDb().Schemas().GetClassId("RulesEngineTest", "Widget"), ECInstanceId((uint64_t)2));

    IECInstancePtr instance;
    std::thread([&]()
        {
        EXPECT_EQ(BE_SQLITE_ROW, ECInstancesHelper::LoadInstance(instance, *manager.GetConnection(primaryConnection->GetId().c_str()), key1));
        EXPECT_TRUE(instance.IsValid());

        EXPECT_EQ(BE_SQLITE_ROW, ECInstancesHelper::LoadInstance(instance, *manager.GetConnection(primaryConnection->GetId().c_str()), key2));
        EXPECT_TRUE(instance.IsValid());
        }).join();
    }
