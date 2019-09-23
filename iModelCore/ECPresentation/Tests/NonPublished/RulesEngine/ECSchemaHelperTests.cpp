/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECSchemaHelperTests.h"

ECDbTestProject* ECSchemaHelperTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaHelperTests::SetUpTestCase()
    {
    s_project = new ECDbTestProject();
    s_project->Create("ECSchemaHelperTests", "RulesEngineTest.01.00.ecschema.xml");

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

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SupportedClassNamesParserTests, Parse_IncludedclassInfosFromSingleSchema)
    {
    SupportedClassNamesParser parser(*m_helper, "Basic2:Class2", true);
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
TEST_F (SupportedClassNamesParserTests, Parse_IncludedclassInfosFromMultipleSchemas)
    {
    SupportedClassNamesParser parser(*m_helper, "Basic2:Class2;Basic3:Class3", true);
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
TEST_F (SupportedClassNamesParserTests, Parse_ExcludedclassInfosFromSingleSchema)
    {
    SupportedClassNamesParser parser(*m_helper, "E:Basic2:Class2", true);
    SupportedClassInfos const& classInfos = parser.GetClassInfos();
    ASSERT_EQ(1, classInfos.size());

    SupportedClassInfo<ECClass> const* info = Find(classInfos, *m_helper->GetECClass("Basic2", "Class2")->GetEntityClassCP());
    ASSERT_TRUE(nullptr != info);
    ASSERT_TRUE(info->IsExclude());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SupportedClassNamesParserTests, Parse_ExcludedclassInfosFromMultipleSchemas)
    {
    SupportedClassNamesParser parser(*m_helper, "E:Basic2:Class2;Basic3:Class3", true);
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
TEST_F (SupportedClassNamesParserTests, Parse_PolymorphicallyExcludedclassInfosFromSingleSchema)
    {
    SupportedClassNamesParser parser(*m_helper, "PE:Basic2:Class2", true);
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
TEST_F (SupportedClassNamesParserTests, Parse_PolymorphicallyExcludedclassInfosFromMultipleSchemas)
    {
    SupportedClassNamesParser parser(*m_helper, "PE:Basic2:Class2;Basic3:Class3", true);
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
TEST_F (SupportedClassNamesParserTests, Parse_IncludedAndExcluded)
    {
    SupportedClassNamesParser parser(*m_helper, "Basic1:Class1;E:Basic1:Class1B;PE:Basic2:Class2", true);
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
TEST_F (SupportedClassNamesParserTests, GetECclassInfosFromClassList_IncludedAndExcluded)
    {
    SupportedClassNamesParser parser(*m_helper, "Basic1:Class1;E:Basic1:Class1", true);
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
TEST_F (ECSchemaHelperTests, GetRelationshipClassPaths_FromSupportedSchemas)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2And3")->GetRelationshipClassCP();
    ECRelationshipClassCR relationship2 = *schema->GetClassCP("Class1HasClass3")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECClassCR class2 = *schema->GetClassCP("Class2");
    ECClassCR class3 = *schema->GetClassCP("Class3");

    bmap<ECRelationshipClassCP, int> relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptions options(class1, (int)ECRelatedInstanceDirection::Forward, 0, "SchemaComplex", "", "", relationshipUseCounts, nullptr);
    bvector<bpair<RelatedClassPath, bool>> paths = m_helper->GetRelationshipClassPaths(options);
    ASSERT_EQ(3, paths.size());
    for (size_t i = 0; i < paths.size(); i++)
        {
        ASSERT_EQ(1, paths[i].first.size());
        ASSERT_TRUE(paths[i].second);
        }

    ASSERT_EQ(&class1, paths[0].first[0].GetSourceClass());
    ASSERT_EQ(&class2, paths[0].first[0].GetTargetClass());
    ASSERT_EQ(&relationship1, paths[0].first[0].GetRelationship());
    ASSERT_FALSE(paths[0].first[0].IsForwardRelationship());

    ASSERT_EQ(&class1, paths[1].first[0].GetSourceClass());
    ASSERT_EQ(&class3, paths[1].first[0].GetTargetClass());
    ASSERT_EQ(&relationship1, paths[1].first[0].GetRelationship());
    ASSERT_FALSE(paths[1].first[0].IsForwardRelationship());

    ASSERT_EQ(&class1, paths[2].first[0].GetSourceClass());
    ASSERT_EQ(&class3, paths[2].first[0].GetTargetClass());
    ASSERT_EQ(&relationship2, paths[2].first[0].GetRelationship());
    ASSERT_FALSE(paths[2].first[0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetRelationshipClassPaths_FromRelatedClassNames)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2And3")->GetRelationshipClassCP();
    ECRelationshipClassCR relationship2 = *schema->GetClassCP("Class1HasClass3")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECClassCR class3 = *schema->GetClassCP("Class3");

    bmap<ECRelationshipClassCP, int> relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptions options(class1, (int)ECRelatedInstanceDirection::Forward, 0, "", "", "SchemaComplex:Class3", relationshipUseCounts, nullptr);
    bvector<bpair<RelatedClassPath, bool>> paths = m_helper->GetRelationshipClassPaths(options);
    ASSERT_EQ(2, paths.size());
    for (size_t i = 0; i < paths.size(); i++)
        {
        ASSERT_EQ(1, paths[i].first.size());
        ASSERT_TRUE(paths[i].second);
        }

    ASSERT_EQ(&class1, paths[0].first[0].GetSourceClass());
    ASSERT_EQ(&class3, paths[0].first[0].GetTargetClass());
    ASSERT_EQ(&relationship1, paths[0].first[0].GetRelationship());
    ASSERT_FALSE(paths[0].first[0].IsForwardRelationship());

    ASSERT_EQ(&class1, paths[1].first[0].GetSourceClass());
    ASSERT_EQ(&class3, paths[1].first[0].GetTargetClass());
    ASSERT_EQ(&relationship2, paths[1].first[0].GetRelationship());
    ASSERT_FALSE(paths[1].first[0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetRelationshipClassPaths_FindsRelatedClassesPolymorphically)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECRelationshipClassCR relationship = *schema->GetClassCP("DerivingRelationship")->GetRelationshipClassCP();
    ECEntityClassCR class4 = *schema->GetClassCP("Class4")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    bmap<ECRelationshipClassCP, int> relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptions options(class3, (int)ECRelatedInstanceDirection::Backward, 0, "", "SchemaComplex:DerivingRelationship", "SchemaComplex:Class4", relationshipUseCounts, nullptr);
    bvector<bpair<RelatedClassPath, bool>> paths = m_helper->GetRelationshipClassPaths(options);
    ASSERT_EQ(1, paths.size());
    ASSERT_EQ(1, paths[0].first.size());
    ASSERT_TRUE(paths[0].second);

    ASSERT_EQ(&class3, paths[0].first[0].GetSourceClass());
    ASSERT_EQ(&class4, paths[0].first[0].GetTargetClass());
    ASSERT_EQ(&relationship, paths[0].first[0].GetRelationship());
    ASSERT_TRUE(paths[0].first[0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetRelationshipClassPaths_FromSupportedRelationships)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass2And3")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECClassCR class2 = *schema->GetClassCP("Class2");
    ECClassCR class3 = *schema->GetClassCP("Class3");

    bmap<ECRelationshipClassCP, int> relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptions options(class1, (int)ECRelatedInstanceDirection::Forward, 0, "", "SchemaComplex:Class1HasClass2And3", "", relationshipUseCounts, nullptr);
    bvector<bpair<RelatedClassPath, bool>> paths = m_helper->GetRelationshipClassPaths(options);
    ASSERT_EQ(2, paths.size());
    for (size_t i = 0; i < paths.size(); i++)
        {
        ASSERT_EQ(1, paths[i].first.size());
        ASSERT_TRUE(paths[i].second);
        }

    ASSERT_EQ(&class1, paths[0].first[0].GetSourceClass());
    ASSERT_EQ(&class2, paths[0].first[0].GetTargetClass());
    ASSERT_EQ(&relationship, paths[0].first[0].GetRelationship());
    ASSERT_FALSE(paths[0].first[0].IsForwardRelationship());

    ASSERT_EQ(&class1, paths[1].first[0].GetSourceClass());
    ASSERT_EQ(&class3, paths[1].first[0].GetTargetClass());
    ASSERT_EQ(&relationship, paths[1].first[0].GetRelationship());
    ASSERT_FALSE(paths[1].first[0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetRelationshipClassPaths_RespectsPolymorphicRelationshipConstraints)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class4 = *schema->GetClassCP("Class4")->GetEntityClassCP();

    bmap<ECRelationshipClassCP, int> relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptions options1(class1, (int)ECRelatedInstanceDirection::Forward, 0, "", "SchemaComplex:Class1HasClass3", "", relationshipUseCounts, nullptr);
    bvector<bpair<RelatedClassPath, bool>> paths = m_helper->GetRelationshipClassPaths(options1);
    ASSERT_EQ(1, paths.size());

    // Class4 derives from Class1, but the relationship is not followed because the source constraint is not polymorphic
    ECSchemaHelper::RelationshipClassPathOptions options2(class4, (int)ECRelatedInstanceDirection::Forward, 0, "", "SchemaComplex:Class1HasClass3", "", relationshipUseCounts, nullptr);
    paths = m_helper->GetRelationshipClassPaths(options2);
    ASSERT_EQ(0, paths.size());

    ECSchemaHelper::RelationshipClassPathOptions options3(class1, (int)ECRelatedInstanceDirection::Forward, 0, "", "SchemaComplex:Class1HasClass2And3", "", relationshipUseCounts, nullptr);
    paths = m_helper->GetRelationshipClassPaths(options3);
    ASSERT_EQ(2, paths.size());

    // the relationship constraint is polymorphic in this case, so following Class4 is the same as following Class1
    ECSchemaHelper::RelationshipClassPathOptions options4(class4, (int)ECRelatedInstanceDirection::Forward, 0, "", "SchemaComplex:Class1HasClass2And3", "", relationshipUseCounts, nullptr);
    paths = m_helper->GetRelationshipClassPaths(options4);
    ASSERT_EQ(2, paths.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetRelationshipClassPaths_FollowsBothRelationshipsDirections)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex2");
    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECRelationshipClassCR relationship2 = *schema->GetClassCP("Class3HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    static int relationshipDirection = (int)ECRelatedInstanceDirection::Forward | (int)ECRelatedInstanceDirection::Backward;

    bmap<ECRelationshipClassCP, int> relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptions options(class2, relationshipDirection, 0, "SchemaComplex2", "", "", relationshipUseCounts, nullptr);
    bvector<bpair<RelatedClassPath, bool>> paths = m_helper->GetRelationshipClassPaths(options);
    ASSERT_EQ(2, paths.size());
    for (size_t i = 0; i < paths.size(); i++)
        {
        ASSERT_EQ(1, paths[i].first.size());
        ASSERT_TRUE(paths[i].second);
        }

    ASSERT_EQ(&class2, paths[0].first[0].GetSourceClass());
    ASSERT_EQ(&class1, paths[0].first[0].GetTargetClass());
    ASSERT_EQ(&relationship1, paths[0].first[0].GetRelationship());
    ASSERT_TRUE(paths[0].first[0].IsForwardRelationship());

    ASSERT_EQ(&class2, paths[1].first[0].GetSourceClass());
    ASSERT_EQ(&class3, paths[1].first[0].GetTargetClass());
    ASSERT_EQ(&relationship2, paths[1].first[0].GetRelationship());
    ASSERT_TRUE(paths[1].first[0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetRelationshipClassPaths_FollowsBothRelationshipsDirectionsWithDepthMoreThan0)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex2");
    ECRelationshipClassCR relationship1 = *schema->GetClassCP("Class1HasClass2")->GetRelationshipClassCP();
    ECRelationshipClassCR relationship2 = *schema->GetClassCP("Class3HasClass2")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    static int relationshipDirection = (int)ECRelatedInstanceDirection::Forward | (int)ECRelatedInstanceDirection::Backward;

    bmap<ECRelationshipClassCP, int> relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptions options(class1, relationshipDirection, 1, "", "", "SchemaComplex2:Class3", relationshipUseCounts, nullptr);
    bvector<bpair<RelatedClassPath, bool>> paths = m_helper->GetRelationshipClassPaths(options);
    ASSERT_EQ(1, paths.size());
    ASSERT_EQ(2, paths[0].first.size());
    ASSERT_TRUE(paths[0].second);

    ASSERT_EQ(&class1, paths[0].first[0].GetSourceClass());
    ASSERT_EQ(&class2, paths[0].first[0].GetTargetClass());
    ASSERT_EQ(&relationship1, paths[0].first[0].GetRelationship());
    ASSERT_FALSE(paths[0].first[0].IsForwardRelationship());

    ASSERT_EQ(&class2, paths[0].first[1].GetSourceClass());
    ASSERT_EQ(&class3, paths[0].first[1].GetTargetClass());
    ASSERT_EQ(&relationship2, paths[0].first[1].GetRelationship());
    ASSERT_TRUE(paths[0].first[1].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetRelationshipClassPaths_DoesntDuplicatePathsWithPolymorphicallySimilarClasses)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass2And3")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class2 = *schema->GetClassCP("Class2")->GetEntityClassCP();

    bmap<ECRelationshipClassCP, int> relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptions options(class2, (int)ECRelatedInstanceDirection::Backward,
        0, "SchemaComplex", "", "", relationshipUseCounts, nullptr);
    bvector<bpair<RelatedClassPath, bool>> paths = m_helper->GetRelationshipClassPaths(options);
    ASSERT_EQ(1, paths.size());

    ASSERT_EQ(&class2, paths[0].first[0].GetSourceClass());
    ASSERT_EQ(&class1, paths[0].first[0].GetTargetClass());
    ASSERT_EQ(&relationship, paths[0].first[0].GetRelationship());
    ASSERT_TRUE(paths[0].first[0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetRelationshipClassPaths_DoesntDuplicatePathsWithPolymorphicallySimilarRelationships)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex");
    ECRelationshipClassCR relationship = *schema->GetClassCP("Class1HasClass3")->GetRelationshipClassCP();
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    ECEntityClassCR class3 = *schema->GetClassCP("Class3")->GetEntityClassCP();

    bmap<ECRelationshipClassCP, int> relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptions options(class1, (int)ECRelatedInstanceDirection::Forward,
        0, "", "SchemaComplex:Class1HasClass3,DerivingRelationship", "SchemaComplex:Class3", relationshipUseCounts, nullptr);
    bvector<bpair<RelatedClassPath, bool>> paths = m_helper->GetRelationshipClassPaths(options);
    ASSERT_EQ(1, paths.size());

    ASSERT_EQ(&class1, paths[0].first[0].GetSourceClass());
    ASSERT_EQ(&class3, paths[0].first[0].GetTargetClass());
    ASSERT_EQ(&relationship, paths[0].first[0].GetRelationship());
    ASSERT_FALSE(paths[0].first[0].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetRelationshipClassPaths_RecursivelyUsingSuppliedRelationshipsAndClasses)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("RulesEngineTest");
    ECEntityClassCR widget = *schema->GetClassCP("Widget")->GetEntityClassCP();
    ECEntityClassCR gadget = *schema->GetClassCP("Gadget")->GetEntityClassCP();
    ECEntityClassCR sprocket = *schema->GetClassCP("Sprocket")->GetEntityClassCP();
    ECRelationshipClassCR wiggetHasGadgets = *schema->GetClassCP("WidgetHasGadgets")->GetRelationshipClassCP();
    ECRelationshipClassCR gadgetHasSprockets = *schema->GetClassCP("GadgetHasSprockets")->GetRelationshipClassCP();

    bmap<ECRelationshipClassCP, int> relationshipUseCounts;
    ECSchemaHelper::RelationshipClassPathOptions options(widget, (int)ECRelatedInstanceDirection::Forward, -1, "",
        "RulesEngineTest:WidgetHasGadgets,GadgetHasSprockets", "RulesEngineTest:Gadget,Sprocket", relationshipUseCounts, nullptr);
    bvector<bpair<RelatedClassPath, bool>> paths = m_helper->GetRelationshipClassPaths(options);
    ASSERT_EQ(2, paths.size());
    ASSERT_TRUE(paths[0].second);

    RelatedClassPath const& path1 = paths[0].first;
    ASSERT_EQ(1, path1.size());
    EXPECT_EQ(&widget, path1[0].GetSourceClass());
    EXPECT_EQ(&gadget, path1[0].GetTargetClass());
    EXPECT_EQ(&wiggetHasGadgets, path1[0].GetRelationship());
    EXPECT_FALSE(path1[0].IsForwardRelationship());

    RelatedClassPath const& path2 = paths[1].first;
    ASSERT_EQ(2, path2.size());
    EXPECT_EQ(&widget, path2[0].GetSourceClass());
    EXPECT_EQ(&gadget, path2[0].GetTargetClass());
    EXPECT_EQ(&wiggetHasGadgets, path2[0].GetRelationship());
    EXPECT_FALSE(path2[0].IsForwardRelationship());
    EXPECT_EQ(&gadget, path2[1].GetSourceClass());
    EXPECT_EQ(&sprocket, path2[1].GetTargetClass());
    EXPECT_EQ(&gadgetHasSprockets, path2[1].GetRelationship());
    EXPECT_FALSE(path2[1].IsForwardRelationship());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetForeignKeyClass_FindsForeignClassByNavigationProperty)
    {
    ECSchemaCP schema = s_project->GetECDb().Schemas().GetSchema("SchemaComplex3");
    ECEntityClassCR class1 = *schema->GetClassCP("Class1")->GetEntityClassCP();
    EXPECT_EQ(&class1, m_helper->GetForeignKeyClass(*class1.GetPropertyP("Parent")).GetTargetClass());
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
* @bsitest                                      Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetPolymorphicallyRelatedClassesWithInstances_ReturnsOnlyRelatedInstanceClasses)
    {
    ECEntityClassCP class1 = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class1")->GetEntityClassCP();
    ECEntityClassCP baseof2and3 = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "BaseOf2and3")->GetEntityClassCP();
    ECEntityClassCP class2 = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class2")->GetEntityClassCP();
    ECEntityClassCP class3 = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class3")->GetEntityClassCP();
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class1HasClass2And3")->GetRelationshipClassCP();

    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class1);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class2);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *instance1, *instance2);

    bvector<RelatedClassPath> result = m_helper->GetPolymorphicallyRelatedClassesWithInstances(*class1,
        rel->GetFullName(), ECRelatedInstanceDirection::Forward, baseof2and3->GetFullName(), RelatedClassPath(), nullptr);
    ASSERT_EQ(1, result.size());
    ASSERT_EQ(1, result[0].size());
    EXPECT_EQ(class2, result[0][0].GetTargetClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetPolymorphicallyRelatedClassesWithInstances_ReturnsRelatedInstanceClasses_WhenRelatedClassIsNotSpecified)
    {
    ECEntityClassCP class1 = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class1")->GetEntityClassCP();
    // ECEntityClassCP baseof2and3 = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "BaseOf2and3")->GetEntityClassCP();
    ECEntityClassCP class2 = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class2")->GetEntityClassCP();
    ECEntityClassCP class3 = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class3")->GetEntityClassCP();
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class1HasClass2And3")->GetRelationshipClassCP();

    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class1);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class2);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *instance1, *instance2);

    bvector<RelatedClassPath> result = m_helper->GetPolymorphicallyRelatedClassesWithInstances(*class1,
        rel->GetFullName(), ECRelatedInstanceDirection::Forward, "", RelatedClassPath(), nullptr);
    ASSERT_EQ(1, result.size());
    ASSERT_EQ(1, result[0].size());
    EXPECT_EQ(class2, result[0][0].GetTargetClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetPolymorphicallyRelatedClassesWithInstances_ReturnsRelatedInstanceClasses_WhenRelationshipIsNotSpecified)
    {
    ECEntityClassCP class1 = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class1")->GetEntityClassCP();
    ECEntityClassCP baseof2and3 = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "BaseOf2and3")->GetEntityClassCP();
    ECEntityClassCP class2 = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class2")->GetEntityClassCP();
    ECEntityClassCP class3 = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class3")->GetEntityClassCP();
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class1HasClass2And3")->GetRelationshipClassCP();

    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class1);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class2);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *instance1, *instance2);

    bvector<RelatedClassPath> result = m_helper->GetPolymorphicallyRelatedClassesWithInstances(*class1,
        "", ECRelatedInstanceDirection::Forward, baseof2and3->GetFullName(), RelatedClassPath(), nullptr);
    ASSERT_EQ(1, result.size());
    ASSERT_EQ(1, result[0].size());
    EXPECT_EQ(class2, result[0][0].GetTargetClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetPolymorphicallyRelatedClassesWithInstances_ReturnsOnlySubclassesOfProvidedBaseClass)
    {
    ECEntityClassCP class1 = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class1")->GetEntityClassCP();
    ECEntityClassCP class2 = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class2")->GetEntityClassCP();
    ECEntityClassCP class3 = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class3")->GetEntityClassCP();
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class1HasClass2And3")->GetRelationshipClassCP();

    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class1);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class2);
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *instance1, *instance2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *instance1, *instance3);

    bvector<RelatedClassPath> result = m_helper->GetPolymorphicallyRelatedClassesWithInstances(*class1,
        rel->GetFullName(), ECRelatedInstanceDirection::Forward, class2->GetFullName(), RelatedClassPath(), nullptr);
    ASSERT_EQ(1, result.size());
    ASSERT_EQ(1, result[0].size());
    EXPECT_EQ(class2, result[0][0].GetTargetClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetPolymorphicallyRelatedClassesWithInstances_ReturnsClassesOfInstancesWhichMatchInstanceFilter)
    {
    ECEntityClassCP class1 = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class1")->GetEntityClassCP();
    ECEntityClassCP baseof2and3 = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "BaseOf2and3")->GetEntityClassCP();
    ECEntityClassCP class2 = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class2")->GetEntityClassCP();
    ECEntityClassCP class3 = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class3")->GetEntityClassCP();
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class1HasClass2And3")->GetRelationshipClassCP();

    IECInstancePtr instance11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class1);
    IECInstancePtr instance12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class1);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class2);
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *instance11, *instance2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *instance12, *instance3);

    SelectClassInfo selectInfo(*class1, true);
    Utf8PrintfString instanceFilter("this.ECInstanceId = %s", instance11->GetInstanceId().c_str());
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), nullptr, selectInfo, nullptr, instanceFilter.c_str());

    bvector<RelatedClassPath> result = m_helper->GetPolymorphicallyRelatedClassesWithInstances(*class1,
        rel->GetFullName(), ECRelatedInstanceDirection::Forward, baseof2and3->GetFullName(), RelatedClassPath(), &filteringParams);
    ASSERT_EQ(1, result.size());
    ASSERT_EQ(1, result[0].size());
    EXPECT_EQ(class2, result[0][0].GetTargetClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetPolymorphicallyRelatedClassesWithInstances_ReturnsClassesOfInstancesRelatedToFilteredInstances)
    {
    ECEntityClassCP class1 = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class1")->GetEntityClassCP();
    ECEntityClassCP baseof2and3 = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "BaseOf2and3")->GetEntityClassCP();
    ECEntityClassCP class2 = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class2")->GetEntityClassCP();
    ECEntityClassCP class3 = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class3")->GetEntityClassCP();
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass("SchemaComplex", "Class1HasClass2And3")->GetRelationshipClassCP();

    IECInstancePtr instance11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class1);
    IECInstancePtr instance12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class1);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class2);
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *instance11, *instance2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel, *instance12, *instance3);

    SelectClassInfo selectInfo(*class1, true);
    TestParsedInput input(*instance11);
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), &input, selectInfo, nullptr, "");

    bvector<RelatedClassPath> result = m_helper->GetPolymorphicallyRelatedClassesWithInstances(*class1,
        rel->GetFullName(), ECRelatedInstanceDirection::Forward, baseof2and3->GetFullName(), RelatedClassPath(), &filteringParams);
    ASSERT_EQ(1, result.size());
    ASSERT_EQ(1, result[0].size());
    EXPECT_EQ(class2, result[0][0].GetTargetClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetPolymorphicallyRelatedClassesWithInstances_ReturnsRecursivelyRelatedInstanceClasses)
    {
    ECEntityClassCP class1 = s_project->GetECDb().Schemas().GetClass("SchemaComplex3", "Class1")->GetEntityClassCP();
    ECEntityClassCP class2 = s_project->GetECDb().Schemas().GetClass("SchemaComplex3", "Class2")->GetEntityClassCP();
    ECRelationshipClassCP rel1 = s_project->GetECDb().Schemas().GetClass("SchemaComplex3", "Class1HasClass1")->GetRelationshipClassCP();
    ECRelationshipClassCP rel2 = s_project->GetECDb().Schemas().GetClass("SchemaComplex3", "Class1HasClass2")->GetRelationshipClassCP();

    IECInstancePtr instance11 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class1);
    IECInstancePtr instance12 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class1);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel1, *instance11, *instance12);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel2, *instance12, *instance2);

    SelectClassInfo selectInfo(*class1, true);
    InstanceFilteringParams::RecursiveQueryInfo recursiveInfo({
        {RelatedClass(*class1, *class1, *rel1, true)},
        {RelatedClass(*class1, *class2, *rel2, true)}
        });
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), nullptr, selectInfo, &recursiveInfo, "");

    bvector<RelatedClassPath> result = m_helper->GetPolymorphicallyRelatedClassesWithInstances(*class1,
        Utf8PrintfString("%s:%s,%s", rel1->GetSchema().GetName().c_str(), rel1->GetName().c_str(), rel2->GetName().c_str()),
        ECRelatedInstanceDirection::Forward,
        Utf8PrintfString("%s:%s,%s", class1->GetSchema().GetName().c_str(), class1->GetName().c_str(), class2->GetName().c_str()),
        RelatedClassPath(),
        &filteringParams);
    ASSERT_EQ(2, result.size());
    ASSERT_EQ(1, result[0].size());
    EXPECT_EQ(class1, result[0][0].GetTargetClass());
    ASSERT_EQ(1, result[1].size());
    EXPECT_EQ(class2, result[1][0].GetTargetClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECSchemaHelperTests, GetPolymorphicallyRelatedClassesWithInstances_ReturnsNestedRalationshipInstanceClassesWhichMatchInstanceFilter)
    {
    ECEntityClassCP class1 = s_project->GetECDb().Schemas().GetClass("SchemaComplex2", "Class1")->GetEntityClassCP();
    ECEntityClassCP class2 = s_project->GetECDb().Schemas().GetClass("SchemaComplex2", "Class2")->GetEntityClassCP();
    ECEntityClassCP class3 = s_project->GetECDb().Schemas().GetClass("SchemaComplex2", "Class3")->GetEntityClassCP();
    ECRelationshipClassCP rel1 = s_project->GetECDb().Schemas().GetClass("SchemaComplex2", "Class1HasClass2")->GetRelationshipClassCP();
    ECRelationshipClassCP rel2 = s_project->GetECDb().Schemas().GetClass("SchemaComplex2", "Class3HasClass2")->GetRelationshipClassCP();

    IECInstancePtr instance1 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class1);
    IECInstancePtr instance2 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class2);
    IECInstancePtr instance3 = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *class3);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel1, *instance1, *instance2);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *rel2, *instance3, *instance2);

    SelectClassInfo selectInfo(*class3, true);
    Utf8PrintfString instanceFilter("this.ECInstanceId = %s", instance3->GetInstanceId().c_str());
    InstanceFilteringParams filteringParams(*m_connection, m_helper->GetECExpressionsCache(), nullptr, selectInfo, nullptr, instanceFilter.c_str());
    RelatedClassPath nestedRelationship({RelatedClass(*class2, *class3, *rel2, false)});

    bvector<RelatedClassPath> result = m_helper->GetPolymorphicallyRelatedClassesWithInstances(*class2,
        rel1->GetFullName(), ECRelatedInstanceDirection::Backward, class1->GetFullName(), nestedRelationship, &filteringParams);

    ASSERT_EQ(1, result.size());
    ASSERT_EQ(1, result[0].size());
    EXPECT_EQ(class1, result[0][0].GetTargetClass());
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
