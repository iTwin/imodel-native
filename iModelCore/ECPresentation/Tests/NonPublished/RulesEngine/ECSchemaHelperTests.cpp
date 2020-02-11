/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECSchemaHelperTests.h"

ECDbTestProject* ECSchemaHelperTests::s_project = nullptr;
DEFINE_SCHEMA_REGISTRY(ECSchemaHelperTests)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
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
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaHelperTests::TearDownTestCase()
    {
    delete s_project;
    s_project = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                06/2015
//---------------------------------------------------------------------------------------
void ECSchemaHelperTests::SetUp()
    {
    ECPresentationTest::SetUp();
    m_connection = new TestConnection(s_project->GetECDb());
    m_helper = new ECSchemaHelper(*m_connection, nullptr, nullptr, nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                06/2015
//---------------------------------------------------------------------------------------
void ECSchemaHelperTests::TearDown()
    {
    s_project->GetECDb().AbandonChanges();
    DELETE_AND_CLEAR(m_helper);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                06/2015
//---------------------------------------------------------------------------------------
static SupportedClassInfo<ECClass> const* Find(SupportedClassInfos const& classInfos, ECClassCR ecClass)
    {
    auto iter = classInfos.find(SupportedClassInfo<ECClass>(ecClass));
    if (classInfos.end() != iter)
        return &*iter;
    return nullptr;
    }


/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct SupportedClassesParserTests : ECSchemaHelperTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
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
* @bsitest                                      Grigas.Petraitis                06/2015
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
* @bsitest                                      Grigas.Petraitis                06/2015
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
* @bsitest                                      Grigas.Petraitis                06/2015
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
* @bsitest                                      Grigas.Petraitis                06/2015
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
* @bsitest                                      Grigas.Petraitis                06/2015
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
* @bsitest                                      Grigas.Petraitis                06/2015
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
* @bsitest                                      Grigas.Petraitis                06/2015
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
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetSchema)
    {
    ASSERT_TRUE(nullptr == m_helper->GetSchema("does_not_exist"));

    ECSchemaCP schema = m_helper->GetSchema("Basic1");
    ASSERT_TRUE(nullptr != schema);
    ASSERT_STREQ("Basic1", schema->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetECClass)
    {
    ASSERT_TRUE(nullptr == m_helper->GetECClass("Basic1", "does_not_exist"));

    ECClassCP ecClass = m_helper->GetECClass("Basic1", "Class1");
    ASSERT_TRUE(nullptr != ecClass);
    ASSERT_STREQ("Basic1:Class1", ecClass->GetFullName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetECClassesFromSchemaList_SingleSchema)
    {
    ECClassSet classes = m_helper->GetECClassesFromSchemaList("Basic2");
    ASSERT_EQ(1, classes.size());
    ASSERT_STREQ("Basic2:Class2", (*classes.begin()).first->GetFullName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetECClassesFromSchemaList_MultipleSchemas)
    {
    ECClassSet classes = m_helper->GetECClassesFromSchemaList("Basic2,Basic3");
    ASSERT_EQ(2, classes.size());
    ASSERT_TRUE(classes.end() != classes.find(m_helper->GetECClass("Basic2", "Class2")->GetEntityClassCP()));
    ASSERT_TRUE(classes.end() != classes.find(m_helper->GetECClass("Basic3", "Class3")->GetEntityClassCP()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
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
* @bsitest                                      Grigas.Petraitis                06/2015
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
* @bsitest                                      Grigas.Petraitis                06/2015
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
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetOppositeRelationshipEnds_UnsupportedSchema)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass2And3")->GetRelationshipClassCP();

    ECRelationshipConstraintClassList oppositeRelationshipEnds = m_helper->GetRelationshipConstraintClasses(relationship, ECRelatedInstanceDirection::Forward, "Basic1");
    ASSERT_EQ(0, oppositeRelationshipEnds.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
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
    ECSchemaHelper::RelationshipClassPathOptions options(class1, (int)ECRelatedInstanceDirection::Forward, 0, "SchemaComplex", "", "", true, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPaths(options);
    ASSERT_EQ(3, paths.size());
    for (size_t i = 0; i < paths.size(); i++)
        ASSERT_EQ(1, paths[i].size());

    EXPECT_EQ(&class1, paths[0][0].GetSourceClass());
    EXPECT_EQ(&class2, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship1, paths[0][0].GetRelationship());
    EXPECT_TRUE(paths[0][0].IsForwardRelationship());

    EXPECT_EQ(&class1, paths[1][0].GetSourceClass());
    EXPECT_EQ(&class3, &paths[1][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[1][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship1, paths[1][0].GetRelationship());
    EXPECT_TRUE(paths[1][0].IsForwardRelationship());

    EXPECT_EQ(&class1, paths[2][0].GetSourceClass());
    EXPECT_EQ(&class3, &paths[2][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[2][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship2, paths[2][0].GetRelationship());
    EXPECT_TRUE(paths[2][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, Deprecated_GetRelationshipClassPaths_FromRelatedClassNames)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2And3")->GetRelationshipClassCP();
    ECRelationshipClassCR relationship2 = *schema->GetClassCP("Class1HasClass3")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECClassCR class3 = *schema->GetClassCP("Class3");

    ECClassUseCounter relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptions options(class1, (int)ECRelatedInstanceDirection::Forward, 0, "", "", "SchemaComplex:Class3", true, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPaths(options);
    ASSERT_EQ(2, paths.size());
    for (size_t i = 0; i < paths.size(); i++)
        ASSERT_EQ(1, paths[i].size());

    EXPECT_EQ(&class1, paths[0][0].GetSourceClass());
    EXPECT_EQ(&class3, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship1, paths[0][0].GetRelationship());
    EXPECT_TRUE(paths[0][0].IsForwardRelationship());

    EXPECT_EQ(&class1, paths[1][0].GetSourceClass());
    EXPECT_EQ(&class3, &paths[1][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[1][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship2, paths[1][0].GetRelationship());
    EXPECT_TRUE(paths[1][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, Deprecated_GetRelationshipClassPaths_FindsRelatedClassesPolymorphically)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECRelationshipClassCR relationship = *schema->GetClassCP("DerivingRelationship")->GetRelationshipClassCP();
    ECEntityClassCR class4 = *schema->GetClassCP("Class4")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    ECClassUseCounter relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptions options(class3, (int)ECRelatedInstanceDirection::Backward, 0, "", "SchemaComplex:DerivingRelationship", "SchemaComplex:Class4", true, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPaths(options);
    ASSERT_EQ(1, paths.size());
    ASSERT_EQ(1, paths[0].size());
    EXPECT_EQ(&class3, paths[0][0].GetSourceClass());
    EXPECT_EQ(&class4, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship, paths[0][0].GetRelationship());
    EXPECT_FALSE(paths[0][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2019
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
    ECSchemaHelper::RelationshipClassPathOptions options(*elementClass, (int)ECRelatedInstanceDirection::Forward, 0, "",
        rel->GetFullName(), aspectClass->GetFullName(), true, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPaths(options);
    ASSERT_EQ(1, paths.size());
    ASSERT_EQ(1, paths[0].size());
    EXPECT_EQ(elementClass, paths[0][0].GetSourceClass());
    EXPECT_EQ(aspectClass, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(rel, paths[0][0].GetRelationship());
    EXPECT_TRUE(paths[0][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2019
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
    ECSchemaHelper::RelationshipClassPathOptions options(*elementClass, (int)ECRelatedInstanceDirection::Forward, 0, "",
        rel->GetFullName(), aspectClass->GetFullName(), false, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPaths(options);
    ASSERT_EQ(2, paths.size());

    ASSERT_EQ(1, paths[0].size());
    EXPECT_EQ(elementClass, paths[0][0].GetSourceClass());
    EXPECT_EQ(aspectClass, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_FALSE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(rel, paths[0][0].GetRelationship());
    EXPECT_TRUE(paths[0][0].IsForwardRelationship());

    ASSERT_EQ(1, paths[1].size());
    EXPECT_EQ(elementClass, paths[1][0].GetSourceClass());
    EXPECT_EQ(customAspectClass, &paths[1][0].GetTargetClass().GetClass());
    EXPECT_FALSE(paths[1][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(rel, paths[1][0].GetRelationship());
    EXPECT_TRUE(paths[1][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, Deprecated_GetRelationshipClassPaths_FromSupportedRelationships)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass2And3")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECClassCR class2 = *schema->GetClassCP("Class2");
    ECClassCR class3 = *schema->GetClassCP("Class3");

    ECClassUseCounter relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptions options(class1, (int)ECRelatedInstanceDirection::Forward, 0, "", "SchemaComplex:Class1HasClass2And3", "", true, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPaths(options);
    ASSERT_EQ(2, paths.size());
    for (size_t i = 0; i < paths.size(); i++)
        ASSERT_EQ(1, paths[i].size());

    EXPECT_EQ(&class1, paths[0][0].GetSourceClass());
    EXPECT_EQ(&class2, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship, paths[0][0].GetRelationship());
    EXPECT_TRUE(paths[0][0].IsForwardRelationship());

    EXPECT_EQ(&class1, paths[1][0].GetSourceClass());
    EXPECT_EQ(&class3, &paths[1][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[1][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship, paths[1][0].GetRelationship());
    EXPECT_TRUE(paths[1][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, Deprecated_GetRelationshipClassPaths_RespectsPolymorphicRelationshipConstraints)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class4 = *schema->GetClassCP("Class4")->GetEntityClassCP();

    ECClassUseCounter relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptions options1(class1, (int)ECRelatedInstanceDirection::Forward, 0, "", "SchemaComplex:Class1HasClass3", "", true, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPaths(options1);
    ASSERT_EQ(1, paths.size());

    // Class4 derives from Class1, but the relationship is not followed because the source constraint is not polymorphic
    ECSchemaHelper::RelationshipClassPathOptions options2(class4, (int)ECRelatedInstanceDirection::Forward, 0, "", "SchemaComplex:Class1HasClass3", "", true, relationshipUseCounts, nullptr);
    paths = m_helper->GetRelationshipClassPaths(options2);
    ASSERT_EQ(0, paths.size());

    ECSchemaHelper::RelationshipClassPathOptions options3(class1, (int)ECRelatedInstanceDirection::Forward, 0, "", "SchemaComplex:Class1HasClass2And3", "", true, relationshipUseCounts, nullptr);
    paths = m_helper->GetRelationshipClassPaths(options3);
    ASSERT_EQ(2, paths.size());

    // the relationship constraint is polymorphic in this case, so following Class4 is the same as following Class1
    ECSchemaHelper::RelationshipClassPathOptions options4(class4, (int)ECRelatedInstanceDirection::Forward, 0, "", "SchemaComplex:Class1HasClass2And3", "", true, relationshipUseCounts, nullptr);
    paths = m_helper->GetRelationshipClassPaths(options4);
    ASSERT_EQ(2, paths.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
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
    ECSchemaHelper::RelationshipClassPathOptions options(class2, relationshipDirection, 0, "SchemaComplex2", "", "", true, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPaths(options);
    ASSERT_EQ(2, paths.size());
    for (size_t i = 0; i < paths.size(); i++)
        ASSERT_EQ(1, paths[i].size());

    EXPECT_EQ(&class2, paths[0][0].GetSourceClass());
    EXPECT_EQ(&class1, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship1, paths[0][0].GetRelationship());
    EXPECT_FALSE(paths[0][0].IsForwardRelationship());

    EXPECT_EQ(&class2, paths[1][0].GetSourceClass());
    EXPECT_EQ(&class3, &paths[1][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[1][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship2, paths[1][0].GetRelationship());
    EXPECT_FALSE(paths[1][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
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
    ECSchemaHelper::RelationshipClassPathOptions options(class1, relationshipDirection, 1, "", "", "SchemaComplex2:Class3", true, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPaths(options);
    ASSERT_EQ(1, paths.size());
    ASSERT_EQ(2, paths[0].size());

    EXPECT_EQ(&class1, paths[0][0].GetSourceClass());
    EXPECT_EQ(&class2, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship1, paths[0][0].GetRelationship());
    EXPECT_TRUE(paths[0][0].IsForwardRelationship());

    EXPECT_EQ(&class2, paths[0][1].GetSourceClass());
    EXPECT_EQ(&class3, &paths[0][1].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[0][1].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship2, paths[0][1].GetRelationship());
    EXPECT_FALSE(paths[0][1].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, Deprecated_GetRelationshipClassPaths_DoesntDuplicatePathsWithPolymorphicallySimilarClasses)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass2And3")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();

    ECClassUseCounter relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptions options(class2, (int)ECRelatedInstanceDirection::Backward,
        0, "SchemaComplex", "", "", true, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPaths(options);
    ASSERT_EQ(1, paths.size());

    EXPECT_EQ(&class2, paths[0][0].GetSourceClass());
    EXPECT_EQ(&class1, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship, paths[0][0].GetRelationship());
    EXPECT_FALSE(paths[0][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, Deprecated_GetRelationshipClassPaths_DoesntDuplicatePathsWithPolymorphicallySimilarRelationships)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass3")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    ECClassUseCounter relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptions options(class1, (int)ECRelatedInstanceDirection::Forward,
        0, "", "SchemaComplex:Class1HasClass3,DerivingRelationship", "SchemaComplex:Class3", true, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPaths(options);
    ASSERT_EQ(1, paths.size());

    EXPECT_EQ(&class1, paths[0][0].GetSourceClass());
    EXPECT_EQ(&class3, &paths[0][0].GetTargetClass().GetClass());
    EXPECT_TRUE(paths[0][0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&relationship, paths[0][0].GetRelationship());
    EXPECT_TRUE(paths[0][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
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
    ECSchemaHelper::RelationshipClassPathOptions options(widget, (int)ECRelatedInstanceDirection::Forward, -1, "",
        "RulesEngineTest:WidgetHasGadgets,GadgetHasSprockets", "RulesEngineTest:Gadget,Sprocket", true, relationshipUseCounts, nullptr);
    bvector<RelatedClassPath> paths = m_helper->GetRelationshipClassPaths(options);
    ASSERT_EQ(2, paths.size());

    RelatedClassPath const& path1 = paths[0];
    ASSERT_EQ(1, path1.size());
    EXPECT_EQ(&widget, path1[0].GetSourceClass());
    EXPECT_EQ(&gadget, &path1[0].GetTargetClass().GetClass());
    EXPECT_TRUE(path1[0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&wiggetHasGadgets, path1[0].GetRelationship());
    EXPECT_TRUE(path1[0].IsForwardRelationship());

    RelatedClassPath const& path2 = paths[1];
    ASSERT_EQ(2, path2.size());
    EXPECT_EQ(&widget, path2[0].GetSourceClass());
    EXPECT_EQ(&gadget, &path2[0].GetTargetClass().GetClass());
    EXPECT_TRUE(path2[0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&wiggetHasGadgets, path2[0].GetRelationship());
    EXPECT_TRUE(path2[0].IsForwardRelationship());
    EXPECT_EQ(&gadget, path2[1].GetSourceClass());
    EXPECT_EQ(&sprocket, &path2[1].GetTargetClass().GetClass());
    EXPECT_TRUE(path2[0].GetTargetClass().IsSelectPolymorphic());
    EXPECT_EQ(&gadgetHasSprockets, path2[1].GetRelationship());
    EXPECT_TRUE(path2[1].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
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
    EXPECT_EQ(rel, paths[0][0].GetRelationship());
    EXPECT_TRUE(paths[0][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
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
    EXPECT_EQ(rel, paths[0][0].GetRelationship());
    EXPECT_TRUE(paths[0][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(GetRelationshipClassPaths_CreatesPathForHiddenClassIfSpecificallyAskedFor, R"*(
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
        <ECCustomAttributes>
            <HiddenClass xmlns="CoreCustomAttributes.01.00" />
        </ECCustomAttributes>"
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
TEST_F(ECSchemaHelperTests, GetRelationshipClassPaths_CreatesPathForHiddenClassIfSpecificallyAskedFor)
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
    EXPECT_EQ(rel, paths[0][0].GetRelationship());
    EXPECT_TRUE(paths[0][0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetForeignKeyClass_FindsForeignClassByNavigationProperty)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex3");
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    EXPECT_EQ(&class1, &m_helper->GetForeignKeyClass(*class1.GetPropertyP("Parent")).GetTargetClass().GetClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                08/2017
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
* @bsitest                                      Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetECClassesFromSchemaList_DoesNotReturnClassesFromHiddenSchemasWhenAskingForAllSchemas)
    {
    ECClassSet classes = m_helper->GetECClassesFromSchemaList("");
    ECClassCP classFromHiddenSchema = m_helper->GetECClass("HiddenSchema:Class1");
    auto classIter = classes.find(classFromHiddenSchema->GetEntityClassCP());
    EXPECT_TRUE(classes.end() == classIter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesRelatedPropertyPathHaveInstances_ReturnsTrueForPathsWithTargetClassInstances, R"*(
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
TEST_F(ECSchemaHelperTests, DoesRelatedPropertyPathHaveInstances_ReturnsTrueForPathsWithTargetClassInstances)
    {
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ModelContainsElements")->GetRelationshipClassCP();
    ECClassCP modelClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Model");
    ECClassCP elementClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Element");

    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model, *element);

    RelatedClassPath path = {RelatedClass(*modelClass, *rel, true, "r", SelectClass(*elementClass), "e", true)};
    EXPECT_TRUE(m_helper->DoesRelatedPropertyPathHaveInstances(RelatedClassPath(), path, nullptr));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesRelatedPropertyPathHaveInstances_ReturnsFalseForPathsWithoutTargetClassInstances, R"*(
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
TEST_F(ECSchemaHelperTests, DoesRelatedPropertyPathHaveInstances_ReturnsFalseForPathsWithoutTargetClassInstances)
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

    EXPECT_FALSE(m_helper->DoesRelatedPropertyPathHaveInstances(RelatedClassPath(), {RelatedClass(*modelClass, *rel, true, "r", SelectClass(*elementClass), "e", true)}, nullptr));
    EXPECT_FALSE(m_helper->DoesRelatedPropertyPathHaveInstances(RelatedClassPath(), {RelatedClass(*modelClass, *rel, true, "r", SelectClass(*classA), "a", true)}, nullptr));
    EXPECT_TRUE(m_helper->DoesRelatedPropertyPathHaveInstances(RelatedClassPath(), {RelatedClass(*modelClass, *rel, true, "r", SelectClass(*classB), "b", true)}, nullptr));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesRelatedPropertyPathHaveInstances_ReturnsTrueForMultiRelationshipPathsWithTargetClassInstances, R"*(
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
TEST_F(ECSchemaHelperTests, DoesRelatedPropertyPathHaveInstances_ReturnsTrueForMultiRelationshipPathsWithTargetClassInstances)
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
        RelatedClass(*classB, SelectClass(*classA), *rel, false, "a", "ba"),
        RelatedClass(*classA, SelectClass(*classC), *rel, true, "c", "ac"),
        };
    EXPECT_TRUE(m_helper->DoesRelatedPropertyPathHaveInstances(RelatedClassPath(), pathFromSelectClass, nullptr));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesRelatedPropertyPathHaveInstances_ReturnsTrueForPathsWithInstancesMatchingInstanceFilter, R"*(
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
TEST_F(ECSchemaHelperTests, DoesRelatedPropertyPathHaveInstances_ReturnsTrueForPathsWithInstancesMatchingInstanceFilter)
    {
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ModelContainsElements")->GetRelationshipClassCP();
    ECClassCP modelClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Model");
    ECClassCP elementClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Element");

    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance){instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model, *element);

    RelatedClassPath path = {RelatedClass(*modelClass, *rel, true, "r", SelectClass(*elementClass), "e", true)};
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), nullptr, SelectClassInfo(*modelClass), nullptr, "this.Prop = 1");
    EXPECT_TRUE(m_helper->DoesRelatedPropertyPathHaveInstances(RelatedClassPath(), path, &filteringParams));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesRelatedPropertyPathHaveInstances_ReturnsFalseForPathsWithoutInstancesMatchingInstanceFilter, R"*(
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
TEST_F(ECSchemaHelperTests, DoesRelatedPropertyPathHaveInstances_ReturnsFalseForPathsWithoutInstancesMatchingInstanceFilter)
    {
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ModelContainsElements")->GetRelationshipClassCP();
    ECClassCP modelClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Model");
    ECClassCP elementClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Element");

    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass, [](IECInstanceR instance) {instance.SetValue("Prop", ECValue(1));});
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model, *element);

    RelatedClassPath path = {RelatedClass(*modelClass, *rel, true, "r", SelectClass(*elementClass), "e", true)};
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), nullptr, SelectClassInfo(*modelClass), nullptr, "this.Prop = 2");
    EXPECT_FALSE(m_helper->DoesRelatedPropertyPathHaveInstances(RelatedClassPath(), path, &filteringParams));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesRelatedPropertyPathHaveInstances_ReturnsTrueForPathsWithInstancesRelatedToGivenInput, R"*(
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
TEST_F(ECSchemaHelperTests, DoesRelatedPropertyPathHaveInstances_ReturnsTrueForPathsWithInstancesRelatedToGivenInput)
    {
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ModelContainsElements")->GetRelationshipClassCP();
    ECClassCP modelClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Model");
    ECClassCP elementClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Element");

    IECInstancePtr model = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model, *element);

    RelatedClassPath path = {RelatedClass(*modelClass, *rel, true, "r", SelectClass(*elementClass), "e", true)};
    TestParsedInput input(*model);
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), &input, SelectClassInfo(*modelClass), nullptr, "");
    EXPECT_TRUE(m_helper->DoesRelatedPropertyPathHaveInstances(RelatedClassPath(), path, &filteringParams));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesRelatedPropertyPathHaveInstances_ReturnsFalseForPathsWithoutInstancesRelatedToGivenInput, R"*(
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
TEST_F(ECSchemaHelperTests, DoesRelatedPropertyPathHaveInstances_ReturnsFalseForPathsWithoutInstancesRelatedToGivenInput)
    {
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ModelContainsElements")->GetRelationshipClassCP();
    ECClassCP modelClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Model");
    ECClassCP elementClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Element");

    IECInstancePtr model1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr model2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *modelClass);
    IECInstancePtr element = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *model1, *element);

    RelatedClassPath path = {RelatedClass(*modelClass, *rel, true, "r", SelectClass(*elementClass), "e", true)};
    TestParsedInput input(*model2);
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), &input, SelectClassInfo(*modelClass), nullptr, "");
    EXPECT_FALSE(m_helper->DoesRelatedPropertyPathHaveInstances(RelatedClassPath(), path, &filteringParams));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesRelatedPropertyPathHaveInstances_ReturnsTrueForPathsWithInstancesMatchingNestedInstanceFilter, R"*(
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
TEST_F(ECSchemaHelperTests, DoesRelatedPropertyPathHaveInstances_ReturnsTrueForPathsWithInstancesMatchingNestedInstanceFilter)
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
        RelatedClass(*classA, SelectClass(*classB), *rel, true, "b", "ab"),
        RelatedClass(*classB, SelectClass(*classC), *rel, true, "c", "bc"),
        };
    RelatedClassPath pathFromSelectClass = {
        RelatedClass(*classC, SelectClass(*classD), *rel, true, "d", "cd"),
        RelatedClass(*classD, SelectClass(*classE), *rel, true, "e", "de"),
        };
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), nullptr, SelectClassInfo(*classC), nullptr, "this.Prop = 1");
    EXPECT_TRUE(m_helper->DoesRelatedPropertyPathHaveInstances(pathToSelectClass, pathFromSelectClass, &filteringParams));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesRelatedPropertyPathHaveInstances_ReturnsFalsePathsWithoutsInstancesMatchingNestedInstanceFilter, R"*(
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
TEST_F(ECSchemaHelperTests, DoesRelatedPropertyPathHaveInstances_ReturnsFalsePathsWithoutsInstancesMatchingNestedInstanceFilter)
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
        RelatedClass(*classA, SelectClass(*classB), *rel, true, "b", "ab"),
        RelatedClass(*classB, SelectClass(*classC), *rel, true, "c", "bc"),
        };
    RelatedClassPath pathFromSelectClass = {
        RelatedClass(*classC, SelectClass(*classD), *rel, true, "d", "cd"),
        RelatedClass(*classD, SelectClass(*classE), *rel, true, "e", "de"),
        };
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), nullptr, SelectClassInfo(*classC), nullptr, "this.Prop = 1");
    EXPECT_FALSE(m_helper->DoesRelatedPropertyPathHaveInstances(pathToSelectClass, pathFromSelectClass, &filteringParams));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(DoesRelatedPropertyPathHaveInstances_ReturnsCurrectResultForRelatedToRecursivelyFilteredInstances, R"*(
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
)*");
TEST_F(ECSchemaHelperTests, DoesRelatedPropertyPathHaveInstances_ReturnsCurrectResultForRelatedToRecursivelyFilteredInstances)
    {
    ECClassCP elementClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "Element");
    ECRelationshipClassCP elementToElementRel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ElementOwnsChildElements")->GetRelationshipClassCP();
    ECRelationshipClassCP elementToAspectRel = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "ElementHasAspect")->GetRelationshipClassCP();
    ECClassCP aspectAClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "AspectA");
    ECClassCP aspectBClass = s_project->GetECDb().Schemas().GetClass(BeTest::GetNameOfCurrentTest(), "AspectB");

    IECInstancePtr e1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    IECInstancePtr e2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementToElementRel, *e1, *e2);
    IECInstancePtr e3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *elementClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementToElementRel, *e2, *e3);
    IECInstancePtr aspectA = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectAClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementToAspectRel, *e2, *aspectA);
    IECInstancePtr aspectB = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *aspectBClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *elementToAspectRel, *e3, *aspectB);

    SelectClassInfo selectInfo(*elementClass, true);
    RelatedClassPath pathToSelectClass{RelatedClass(*elementClass, *elementClass, *elementToElementRel, true, "e", "e_to_e")};
    InstanceFilteringParams::RecursiveQueryInfo recursiveInfo({pathToSelectClass});
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), nullptr, selectInfo, &recursiveInfo, "");
    EXPECT_TRUE(m_helper->DoesRelatedPropertyPathHaveInstances(pathToSelectClass, {RelatedClass(*elementClass, *elementToAspectRel, true, "r", *aspectAClass, "a", true)}, &filteringParams));
    EXPECT_TRUE(m_helper->DoesRelatedPropertyPathHaveInstances(pathToSelectClass, {RelatedClass(*elementClass, *elementToAspectRel, true, "r", *aspectBClass, "b", true)}, &filteringParams));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                12/2018
//---------------------------------------------------------------------------------------
void ECInstancesHelperTests::SetUp()
    {
    ECPresentationTest::SetUp();
    m_project.Create("ECInstancesHelperTests", "RulesEngineTest.01.00.ecschema.xml");
    m_connection = new TestConnection(m_project.GetECDb());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                12/2018
//---------------------------------------------------------------------------------------
void ECInstancesHelperTests::TearDown()
    {
    ECPresentationTest::TearDown();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2018
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
* @bsitest                                      Haroldas.Vitunskas              02/2019
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
