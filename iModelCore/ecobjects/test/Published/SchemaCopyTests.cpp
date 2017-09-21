/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/SchemaCopyTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct CopyTestFixture : ECTestFixture 
    {
    ECSchemaPtr m_sourceSchema;
    ECSchemaPtr m_targetSchema;

    public:
        void CreateTestSchema() {EC_ASSERT_SUCCESS(ECSchema::CreateSchema(m_sourceSchema, "TestSchema", "ts", 1, 0, 0));}
    };

struct ClassCopyTest : CopyTestFixture 
    {
    ECClassP m_sourceClass; // This class will live inside of CopyTestFixture::m_sourceSchema 
    ECClassP m_targetClass; // This class will live inside of CopyTestFixture::m_targetSchema 

    protected:
        void SetUp() override;
        void CopyClass(bool copyTypes);
    };

struct SchemaCopyTest : CopyTestFixture
    {
    ECSchemaReadContextPtr   m_schemaContext;
    SearchPathSchemaFileLocaterPtr m_schemaLocater;

    protected:
        void SetUp() override;
        void TearDown() override;

        void CopySchema() { CopySchema(m_targetSchema); }
        void CopySchema(ECSchemaPtr& targetSchema);
    };

//=======================================================================================
//! SchemaCopyTest
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2017
//---------------+---------------+---------------+---------------+---------------+-------
void SchemaCopyTest::SetUp()
    {
    bvector<WString> searchPaths;
    searchPaths.push_back(ECTestFixture::GetTestDataPath(L""));
    m_schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
    m_schemaContext = ECSchemaReadContext::CreateContext();
    m_schemaContext->AddSchemaLocater(*m_schemaLocater);
    CopyTestFixture::SetUp();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2017
//---------------+---------------+---------------+---------------+---------------+-------
void SchemaCopyTest::TearDown()
    {
    m_schemaContext->RemoveSchemaLocater(*m_schemaLocater);
    CopyTestFixture::TearDown();
    }

void SchemaCopyTest::CopySchema(ECSchemaPtr& targetSchema)
    {
    targetSchema = nullptr;
    EC_ASSERT_SUCCESS(m_sourceSchema->CopySchema(targetSchema));
    EXPECT_TRUE(targetSchema.IsValid());
    EXPECT_NE(m_sourceSchema, targetSchema);
    m_sourceSchema = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, Schema_Success)
    {
    CreateTestSchema();

    EC_EXPECT_SUCCESS(m_sourceSchema->SetAlias("alias"));
    EC_EXPECT_SUCCESS(m_sourceSchema->SetDescription("A description of this schema"));
    EC_EXPECT_SUCCESS(m_sourceSchema->SetDisplayLabel("Schema Display Label"));
    EC_EXPECT_SUCCESS(m_sourceSchema->SetVersionRead(3));
    EC_EXPECT_SUCCESS(m_sourceSchema->SetVersionWrite(10));
    EC_EXPECT_SUCCESS(m_sourceSchema->SetVersionMinor(5));

    CopySchema();

    EXPECT_STREQ("alias", m_targetSchema->GetAlias().c_str());
    EXPECT_STREQ("A description of this schema", m_targetSchema->GetDescription().c_str());
    EXPECT_STREQ("Schema Display Label", m_targetSchema->GetDisplayLabel().c_str());
    EXPECT_EQ(3, m_targetSchema->GetVersionRead());
    EXPECT_EQ(10, m_targetSchema->GetVersionWrite());
    EXPECT_EQ(5, m_targetSchema->GetVersionMinor());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaCopyTest, ExpectSuccessWhenCopyingStructs)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(m_sourceSchema, ECTestFixture::GetTestDataPath(L"Widgets.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_TRUE(m_sourceSchema.IsValid());
    EXPECT_EQ(SchemaReadStatus::Success, status);

    CopySchema();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, TestKindOfQuantity)
    {
    CreateTestSchema();

    KindOfQuantityP koq;
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateKindOfQuantity(koq, "TestKoQ"));
    koq->SetDisplayLabel("Test KoQ");
    koq->SetDescription("Test Description");
    koq->SetPersistenceUnit("M");
    koq->AddPresentationUnit("CM");
    koq->AddPresentationUnit("MM");
    koq->SetRelativeError(10e-3);

    CopySchema();

    EXPECT_EQ(1, m_targetSchema->GetKindOfQuantityCount());

    KindOfQuantityCP targetKoq = m_targetSchema->GetKindOfQuantityCP("TestKoQ");
    ASSERT_TRUE(nullptr != targetKoq);
    EXPECT_STREQ("Test KoQ", targetKoq->GetDisplayLabel().c_str());
    EXPECT_STREQ("Test Description", targetKoq->GetDescription().c_str());
    EXPECT_STREQ("CM", targetKoq->GetDefaultPresentationUnit().GetUnitName().c_str());
    EXPECT_STREQ("M", targetKoq->GetPersistenceUnit().GetUnit()->GetName());
    EXPECT_EQ(10e-3, targetKoq->GetRelativeError());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Robert.Schili                      11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaCopyTest, CopySchemaWithEnumeration)
    {
    CreateTestSchema();

    ECEnumerationP enumeration;
    ECEnumeratorP enumeratorA;
    ECEnumeratorP enumeratorB;

    EC_ASSERT_SUCCESS(m_sourceSchema->CreateEnumeration(enumeration, "Enumeration", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_TRUE(enumeration != nullptr);
    enumeration->SetDisplayLabel("My Display Label");
    enumeration->SetDescription("Test Description");
    enumeration->SetIsStrict(false);
    enumeration->CreateEnumerator(enumeratorA, "Value A");
    enumeratorA->SetDisplayLabel("The value for A");
    enumeration->CreateEnumerator(enumeratorB, "Value B");
    enumeratorB->SetDisplayLabel("The value for B");

    CopySchema();

    ECEnumerationP targetEnum = m_targetSchema->GetEnumerationP("Enumeration");
    ASSERT_TRUE(targetEnum != nullptr);
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_String, targetEnum->GetType());
    EXPECT_STREQ("My Display Label", targetEnum->GetDisplayLabel().c_str());
    EXPECT_STREQ("Test Description", targetEnum->GetDescription().c_str());
    EXPECT_FALSE(targetEnum->GetIsStrict());
    EXPECT_EQ(2, targetEnum->GetEnumeratorCount());
    ECEnumeratorCP copiedEnumeratorA = targetEnum->FindEnumerator("Value A");
    ASSERT_TRUE(nullptr != copiedEnumeratorA);
    EXPECT_STREQ("The value for A", copiedEnumeratorA->GetDisplayLabel().c_str());
    EXPECT_STREQ("Value A", copiedEnumeratorA->GetString().c_str());
    ECEnumeratorCP copiedEnumeratorB = targetEnum->FindEnumerator("Value B");
    ASSERT_TRUE(nullptr != copiedEnumeratorB);
    EXPECT_STREQ("The value for B", copiedEnumeratorB->GetDisplayLabel().c_str());
    EXPECT_STREQ("Value B", copiedEnumeratorB->GetString().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopySchemaWithPropertyCategory)
    {
    CreateTestSchema();

    PropertyCategoryP propertyCategory;

    EC_ASSERT_SUCCESS(m_sourceSchema->CreatePropertyCategory(propertyCategory, "PropertyCategory"));
    ASSERT_TRUE(nullptr != propertyCategory);
    
    EC_ASSERT_SUCCESS(propertyCategory->SetDisplayLabel("My Display Label"));
    EC_ASSERT_SUCCESS(propertyCategory->SetDescription("My Description"));
    EC_ASSERT_SUCCESS(propertyCategory->SetPriority(3));

    CopySchema();

    PropertyCategoryCP targetPropertyCategory = m_targetSchema->GetPropertyCategoryCP("PropertyCategory");
    ASSERT_TRUE(nullptr != targetPropertyCategory);
    EXPECT_EQ(3, targetPropertyCategory->GetPriority());
    EXPECT_STREQ("My Display Label", targetPropertyCategory->GetDisplayLabel().c_str());
    EXPECT_STREQ("My Description", targetPropertyCategory->GetDescription().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, TestEntityClass)
    {
    CreateTestSchema();

    ECEntityClassP entity;
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateEntityClass(entity, "Entity"));
    entity->SetClassModifier(ECClassModifier::Sealed);
    entity->SetDescription("Description of my Entity");
    entity->SetDisplayLabel("Entity Class");

    CopySchema();

    ECClassCP targetClass = m_targetSchema->GetClassCP("Entity");
    ASSERT_TRUE(nullptr != targetClass);
    EXPECT_EQ(ECClassType::Entity, targetClass->GetClassType());
    ECEntityClassCP copiedEntity = targetClass->GetEntityClassCP();
    ASSERT_TRUE(nullptr != copiedEntity);
    EXPECT_EQ(ECClassModifier::Sealed, copiedEntity->GetClassModifier());
    EXPECT_STREQ("Description of my Entity", copiedEntity->GetDescription().c_str());
    EXPECT_STREQ("Entity Class", copiedEntity->GetDisplayLabel().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, TestEntityClassWithBaseClasses)
    {
    CreateTestSchema();

    ECEntityClassP baseClass;
    ECEntityClassP ecClass;
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateEntityClass(baseClass, "BaseClass"));
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateEntityClass(ecClass, "Class"));
    EC_ASSERT_SUCCESS(ecClass->AddBaseClass(*baseClass));

    CopySchema();

    EXPECT_EQ(2, m_targetSchema->GetClassCount());
    ECClassCP targetECClass = m_targetSchema->GetClassCP("Class");
    EXPECT_EQ(1, targetECClass->GetBaseClasses().size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, TestEntityClassWithMixin)
    {
    CreateTestSchema();

    ECEntityClassP mixin;
    ECEntityClassP ecClass;
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateEntityClass(ecClass, "Class"));
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateMixinClass(mixin, "Mixin", *ecClass));
    EC_ASSERT_SUCCESS(ecClass->AddBaseClass(*mixin));

    CopySchema();

    EXPECT_EQ(2, m_targetSchema->GetClassCount());
    ECClassCP targetClass = m_targetSchema->GetClassCP("Class");
    EXPECT_EQ(1, targetClass->GetBaseClasses().size());
    ECEntityClassCP copiedMixin = targetClass->GetBaseClasses().front()->GetEntityClassCP();
    EXPECT_TRUE(copiedMixin->IsMixin());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Abeesh.Basheer                  08/2017
//---------------------------------------------------------------------------------------
TEST_F(SchemaCopyTest, TestEntityClassWithBothBaseClassAndMixin)
    {
    CreateTestSchema();

    ECEntityClassP entityBase, entityDerived;
    ECEntityClassP mixin0;

    EC_ASSERT_SUCCESS(m_sourceSchema->CreateEntityClass(entityBase, "Entity0"));
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateEntityClass(entityDerived, "Entity1"));
    
    m_sourceSchema->CreateMixinClass(mixin0, "Mixin0", *entityBase);
    PrimitiveECPropertyP prop;
    mixin0->CreatePrimitiveProperty(prop, "P1");
    entityDerived->AddBaseClass(*entityBase);
    entityDerived->AddBaseClass(*mixin0);

    EXPECT_EQ(2, entityDerived->GetBaseClasses().size());

    CopySchema();

    ECClassCP targetClass = m_targetSchema->GetClassP("Entity1");
    ASSERT_TRUE(nullptr != targetClass);
    EXPECT_EQ(2, targetClass->GetBaseClasses().size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, TestEntityClassWithBaseClassInRefSchema)
    {
    ECEntityClassP baseEntity;
    ECSchemaPtr refSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(refSchema, "RefSchema", "ref", 1, 0, 0));
    EC_ASSERT_SUCCESS(refSchema->CreateEntityClass(baseEntity, "BaseEntity"));

    CreateTestSchema();
    ECEntityClassP entity;
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateEntityClass(entity, "Entity"));
    EC_ASSERT_SUCCESS(m_sourceSchema->AddReferencedSchema(*refSchema));
    EC_EXPECT_SUCCESS(entity->AddBaseClass(*baseEntity));

    CopySchema();

    ECClassCP testClass = m_targetSchema->GetClassCP("Entity");
    ASSERT_TRUE(nullptr != testClass);
    ECClassCP baseClass = testClass->GetBaseClasses().front();
    ASSERT_TRUE(nullptr != baseClass);
    ASSERT_EQ(baseClass, baseEntity);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, TestRelationshipClass)
    {
    CreateTestSchema();

    ECEntityClassP source;
    ECEntityClassP sourceBase;
    ECEntityClassP target;
    ECEntityClassP targetBase;
    ECRelationshipClassP relClass;

    EC_ASSERT_SUCCESS(m_sourceSchema->CreateEntityClass(sourceBase, "SourceBase"));
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateEntityClass(source, "Source"));
    EC_ASSERT_SUCCESS(source->AddBaseClass(*sourceBase));
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateEntityClass(targetBase, "TargetBase"));
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateEntityClass(target, "Target"));
    EC_ASSERT_SUCCESS(target->AddBaseClass(*targetBase));

    EC_ASSERT_SUCCESS(m_sourceSchema->CreateRelationshipClass(relClass, "RelClass"));
    relClass->SetClassModifier(ECClassModifier::Sealed); // default is ECClassModifier::None
    EC_ASSERT_SUCCESS(relClass->SetDescription("Description of RelClass"));
    EC_ASSERT_SUCCESS(relClass->SetDisplayLabel("Relationship Class"));
    EC_ASSERT_SUCCESS(relClass->SetStrength(StrengthType::Embedding)); // default is StrengthType::Referencing
    EC_ASSERT_SUCCESS(relClass->SetStrengthDirection(ECRelatedInstanceDirection::Backward)); // default is ECRelatedInstanceDirection::Forward
    EC_ASSERT_SUCCESS(relClass->GetSource().SetAbstractConstraint(*sourceBase));
    EC_ASSERT_SUCCESS(relClass->GetSource().AddClass(*source));
    EC_ASSERT_SUCCESS(relClass->GetSource().SetRoleLabel("Source Role Label"));
    EC_ASSERT_SUCCESS(relClass->GetSource().SetIsPolymorphic(false)); // default is true
    EC_ASSERT_SUCCESS(relClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroMany())); // default is RelationshipMultiplicity::ZeroOne
    EC_ASSERT_SUCCESS(relClass->GetTarget().SetAbstractConstraint(*targetBase));
    EC_ASSERT_SUCCESS(relClass->GetTarget().AddClass(*target));
    EC_ASSERT_SUCCESS(relClass->GetTarget().SetRoleLabel("Target Role Label"));
    EC_ASSERT_SUCCESS(relClass->GetTarget().SetIsPolymorphic(false)); // default is true
    EC_ASSERT_SUCCESS(relClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneMany())); //default is RelationshipMultiplicity::ZeroOne

    CopySchema();

    ECClassCP targetClass = m_targetSchema->GetClassCP("RelClass");
    ASSERT_TRUE(nullptr != targetClass);
    EXPECT_EQ(ECClassType::Relationship, targetClass->GetClassType());
    EXPECT_EQ(ECClassModifier::Sealed, targetClass->GetClassModifier());
    ECRelationshipClassCP copiedRelClass = targetClass->GetRelationshipClassCP();
    ASSERT_TRUE(nullptr != copiedRelClass);
    EXPECT_EQ(StrengthType::Embedding, copiedRelClass->GetStrength());
    EXPECT_EQ(ECRelatedInstanceDirection::Backward, copiedRelClass->GetStrengthDirection());
    EXPECT_STREQ("Description of RelClass", copiedRelClass->GetDescription().c_str());
    EXPECT_STREQ("Relationship Class", copiedRelClass->GetDisplayLabel().c_str());

    
    ECClassCP copiedSourceBase = m_targetSchema->GetClassCP("SourceBase");
    ECClassCP copiedSource = m_targetSchema->GetClassCP("Source");
    EXPECT_EQ(1, copiedRelClass->GetSource().GetConstraintClasses().size());
    EXPECT_EQ(copiedSourceBase, copiedRelClass->GetSource().GetAbstractConstraint());
    EXPECT_EQ(copiedSource, copiedRelClass->GetSource().GetConstraintClasses().front());
    EXPECT_FALSE(copiedRelClass->GetSource().GetIsPolymorphic());
    EXPECT_STREQ("Source Role Label", copiedRelClass->GetSource().GetRoleLabel().c_str());

    ECClassCP copiedTargetBase = m_targetSchema->GetClassCP("TargetBase");
    ECClassCP copiedTarget = m_targetSchema->GetClassCP("Target");
    EXPECT_EQ(1, copiedRelClass->GetTarget().GetConstraintClasses().size());
    EXPECT_EQ(copiedTargetBase, copiedRelClass->GetTarget().GetAbstractConstraint());
    EXPECT_EQ(copiedTarget, copiedRelClass->GetTarget().GetConstraintClasses().front());
    EXPECT_FALSE(copiedRelClass->GetTarget().GetIsPolymorphic());
    EXPECT_STREQ("Target Role Label", copiedRelClass->GetTarget().GetRoleLabel().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, TestRelationshipClassWithConstraintClassesInRefSchema)
    {
    CreateTestSchema();

    ECSchemaPtr refSchema;
    ECEntityClassP sourceRef;
    ECEntityClassP sourceBaseRef;
    ECEntityClassP targetRef;
    ECEntityClassP targetBaseRef;
    ECRelationshipClassP relClass;

    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(refSchema, "RefSchema", "ref", 1, 0, 0));
    EC_ASSERT_SUCCESS(refSchema->CreateEntityClass(sourceBaseRef, "SourceBase"));
    EC_ASSERT_SUCCESS(refSchema->CreateEntityClass(sourceRef, "Source"));
    EC_ASSERT_SUCCESS(sourceRef->AddBaseClass(*sourceBaseRef));
    EC_ASSERT_SUCCESS(refSchema->CreateEntityClass(targetBaseRef, "TargetBase"));
    EC_ASSERT_SUCCESS(refSchema->CreateEntityClass(targetRef, "Target"));
    EC_ASSERT_SUCCESS(targetRef->AddBaseClass(*targetBaseRef));

    EC_ASSERT_SUCCESS(m_sourceSchema->AddReferencedSchema(*refSchema));

    EC_ASSERT_SUCCESS(m_sourceSchema->CreateRelationshipClass(relClass, "RelClass"));
    EC_ASSERT_SUCCESS(relClass->GetSource().SetAbstractConstraint(*sourceBaseRef));
    EC_ASSERT_SUCCESS(relClass->GetSource().AddClass(*sourceRef));
    EC_ASSERT_SUCCESS(relClass->GetTarget().SetAbstractConstraint(*targetBaseRef));
    EC_ASSERT_SUCCESS(relClass->GetTarget().AddClass(*targetRef));

    CopySchema();

    ECClassCP copiedClass = m_targetSchema->GetClassCP("RelClass");
    ASSERT_TRUE(nullptr != copiedClass);
    ECRelationshipClassCP copiedRelClass = copiedClass->GetRelationshipClassCP();
    ASSERT_TRUE(nullptr != copiedRelClass);

    EXPECT_EQ(1, copiedRelClass->GetSource().GetConstraintClasses().size());
    EXPECT_EQ(sourceBaseRef, copiedRelClass->GetSource().GetAbstractConstraint());
    EXPECT_EQ(sourceRef, copiedRelClass->GetSource().GetConstraintClasses().front());

    EXPECT_EQ(1, copiedRelClass->GetTarget().GetConstraintClasses().size());
    EXPECT_EQ(targetBaseRef, copiedRelClass->GetTarget().GetAbstractConstraint());
    EXPECT_EQ(targetRef, copiedRelClass->GetTarget().GetConstraintClasses().front());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, StructClass_Success)
    {
    CreateTestSchema();

    ECStructClassP structClass;
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateStructClass(structClass, "Struct"));
    structClass->SetClassModifier(ECClassModifier::Sealed); // default is ECClassModifier::None
    EC_ASSERT_SUCCESS(structClass->SetDescription("Description of the struct"));
    EC_ASSERT_SUCCESS(structClass->SetDisplayLabel("Struct Display Label"));

    CopySchema();

    ECClassCP copiedClass = m_targetSchema->GetClassCP("Struct");
    ASSERT_TRUE(nullptr != copiedClass);
    EXPECT_EQ(ECClassType::Struct, copiedClass->GetClassType());
    EXPECT_EQ(ECClassModifier::Sealed, copiedClass->GetClassModifier());
    EXPECT_STREQ("Description of the struct", copiedClass->GetDescription().c_str());
    EXPECT_STREQ("Struct Display Label", copiedClass->GetDisplayLabel().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Caleb.Shafer                           07/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopySimpleSchemaAndCreateInstance)
    {
    SchemaKey key("BaseSchema", 01, 00);
    m_sourceSchema = m_schemaContext->LocateSchema(key, SchemaMatchType::Latest);
    EXPECT_TRUE (m_sourceSchema.IsValid());
    ECClassCP ellipseClass = m_sourceSchema->GetClassCP("ellipse");
    IECInstancePtr ellipseClassInstance = ellipseClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue v;
    v.SetUtf8CP("test");
    ellipseClassInstance->SetValue("Name", v);
    ECValue out;
    EXPECT_EQ(ECObjectsStatus::Success, ellipseClassInstance->GetValue(out, "Name"));
    EXPECT_TRUE (out.Equals (ECValue ("test"))) << "Expect: " << "test" << " Actual: " << out.ToString().c_str();

    ECSchemaPtr copiedSchema = nullptr;
    CopySchema(copiedSchema);

    ellipseClass = copiedSchema->GetClassCP("ellipse");
    ellipseClassInstance = ellipseClass->GetDefaultStandaloneEnabler()->CreateInstance();

    v.SetUtf8CP("test");
    ellipseClassInstance->SetValue("Name", v);
    EXPECT_EQ(ECObjectsStatus::Success, ellipseClassInstance->GetValue(out, "Name"));
    EXPECT_TRUE (out.Equals (ECValue ("test"))) << "Expect: " << "test" << " Actual: " << out.ToString().c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Caleb.Shafer                           07/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopySchemaWithDuplicatePrefixesAndCreateInstance)
    {
    SchemaKey key("DuplicatePrefixes", 01, 00);
    m_sourceSchema = m_schemaContext->LocateSchema(key, SchemaMatchType::Latest);
    EXPECT_TRUE(m_sourceSchema.IsValid());

    ECClassCP ellipseClass = m_sourceSchema->GetClassCP ("Circle");
    EXPECT_TRUE(nullptr != ellipseClass) << "Cannot Load Ellipse Class";
    IECInstancePtr ellipseClassInstance = ellipseClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue v;
    v.SetUtf8CP("test");
    ellipseClassInstance->SetValue("Name", v);
    ECValue out;
    EXPECT_EQ(ECObjectsStatus::Success, ellipseClassInstance->GetValue(out, "Name"));
    EXPECT_TRUE(out.Equals(ECValue("test"))) << "Expect: " << "test" << " Actual: " << out.ToString().c_str();
    
    ECSchemaPtr copiedSchema = nullptr;
    CopySchema(copiedSchema);
    ellipseClass = copiedSchema->GetClassCP("Circle");
    ellipseClassInstance = ellipseClass->GetDefaultStandaloneEnabler()->CreateInstance();

    v.SetUtf8CP("test");
    ellipseClassInstance->SetValue("Name", v);
    EXPECT_EQ(ECObjectsStatus::Success, ellipseClassInstance->GetValue (out, "Name"));
    EXPECT_TRUE(out.Equals(ECValue("test"))) << "Expect: " << "test" << " Actual: " << out.ToString().c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Caleb.Shafer                           07/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopySchemaWithInvalidReferenceAndCreateInstance)
    {
    //create Context with legacy support
    m_schemaContext = nullptr;
    m_schemaContext = ECSchemaReadContext::CreateContext(true);
    m_schemaContext->AddSchemaLocater(*m_schemaLocater);
    SchemaKey key("InvalidReference", 01, 00);
    m_sourceSchema = m_schemaContext->LocateSchema(key, SchemaMatchType::Latest);
    EXPECT_TRUE (m_sourceSchema.IsValid());
    ECClassCP ellipseClass = m_sourceSchema->GetClassCP("circle");
    IECInstancePtr ellipseClassInstance = ellipseClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue v;
    v.SetUtf8CP("test");
    ellipseClassInstance->SetValue("Name", v);
    ECValue out;
    EXPECT_EQ(ECObjectsStatus::Success, ellipseClassInstance->GetValue(out, "Name"));
    EXPECT_TRUE(out.Equals (ECValue ("test"))) << "Expect: " << "test" << " Actual: " << out.ToString().c_str();

    ECSchemaPtr copiedSchema = nullptr;
    CopySchema(copiedSchema);
    
    ellipseClass = copiedSchema->GetClassCP("circle");
    ellipseClassInstance = ellipseClass->GetDefaultStandaloneEnabler ()->CreateInstance ();

    v.SetUtf8CP("test");
    ellipseClassInstance->SetValue("Name", v);
    EXPECT_EQ(ECObjectsStatus::Success, ellipseClassInstance->GetValue(out, "Name"));
    EXPECT_TRUE(out.Equals(ECValue("test"))) << "Expect: " << "test" << " Actual: " << out.ToString().c_str();
    }

//=======================================================================================
//! ClassCopyTest
//
// These tests live inside of the SchemaCopyTests file because the CopyClass method is on
// ECSchema and not on ECClass.
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod                             Caleb.Shafer                           09/2017
//---------------+---------------+---------------+---------------+---------------+-------
void ClassCopyTest::SetUp()
    {
    CreateTestSchema();

    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(m_targetSchema, "TargetSchema", "ts", 1, 1, 1));
    ASSERT_TRUE(m_targetSchema.IsValid());

    CopyTestFixture::SetUp();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Caleb.Shafer                           09/2017
//---------------+---------------+---------------+---------------+---------------+-------
void ClassCopyTest::CopyClass(bool copyTypes)
    {
    EC_EXPECT_SUCCESS(m_targetSchema->CopyClass(m_targetClass, *m_sourceClass, m_sourceClass->GetName().c_str(), copyTypes));
    EXPECT_TRUE(nullptr != m_targetClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Caleb.Shafer                           09/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ClassCopyTest, EntityClassWithBaseClassWithoutCopyingType)
    {
    ECEntityClassP sourceEntity;
    ECEntityClassP baseEntity;
    EC_EXPECT_SUCCESS(m_sourceSchema->CreateEntityClass(sourceEntity, "EntityClass"));
    EC_EXPECT_SUCCESS(m_sourceSchema->CreateEntityClass(baseEntity, "BaseEntity"));
    sourceEntity->AddBaseClass(*baseEntity);

    m_sourceClass = sourceEntity;

    CopyClass(false);

    EXPECT_TRUE(m_targetClass->HasBaseClasses());
    ECClassP baseClass = m_targetClass->GetBaseClasses().front();
    EXPECT_STREQ(m_sourceSchema->GetName().c_str(), baseClass->GetSchema().GetName().c_str());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*m_targetSchema, *m_sourceSchema));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Caleb.Shafer                           09/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ClassCopyTest, RelationshipClassWithContraintClassesWithoutCopyingType)
    {
    ECRelationshipClassP relClass;
    ECEntityClassP entity1;
    ECEntityClassP entity2;
    EC_EXPECT_SUCCESS(m_sourceSchema->CreateEntityClass(entity1, "Source"));
    EC_EXPECT_SUCCESS(m_sourceSchema->CreateEntityClass(entity2, "Target"));
    EC_EXPECT_SUCCESS(m_sourceSchema->CreateRelationshipClass(relClass, "RelClass", *entity1, "Source", *entity2, "Target"));

    m_sourceClass = relClass;

    CopyClass(false);

    ECRelationshipClassCP targetRelClass = m_targetClass->GetRelationshipClassCP();
    ASSERT_TRUE(nullptr != targetRelClass);
    EXPECT_EQ(1, targetRelClass->GetSource().GetConstraintClasses().size());
    ECClassCP destSourceClass = targetRelClass->GetSource().GetConstraintClasses().front();
    EXPECT_STREQ(m_sourceSchema->GetName().c_str(), destSourceClass->GetSchema().GetName().c_str());

    EXPECT_EQ(1, targetRelClass->GetTarget().GetConstraintClasses().size());
    ECClassCP destTargetClass = targetRelClass->GetTarget().GetConstraintClasses().front();
    EXPECT_STREQ(m_sourceSchema->GetName().c_str(), destTargetClass->GetSchema().GetName().c_str());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*m_targetSchema, *m_sourceSchema));

    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Caleb.Shafer                           09/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ClassCopyTest, RelationshipClassWithAbstractContraintWithoutCopyingType)
    {
    ECRelationshipClassP relClass;
    ECEntityClassP entity1;
    ECEntityClassP entity2;
    EC_EXPECT_SUCCESS(m_sourceSchema->CreateEntityClass(entity1, "Source"));
    EC_EXPECT_SUCCESS(m_sourceSchema->CreateEntityClass(entity2, "Target"));
    EC_EXPECT_SUCCESS(m_sourceSchema->CreateRelationshipClass(relClass, "RelClass"));
    EC_EXPECT_SUCCESS(relClass->GetSource().SetAbstractConstraint(*entity1));
    EC_EXPECT_SUCCESS(relClass->GetTarget().SetAbstractConstraint(*entity2));

    m_sourceClass = relClass;

    CopyClass(false);

    ECRelationshipClassCP targetRelClass = m_targetClass->GetRelationshipClassCP();
    ASSERT_TRUE(nullptr != targetRelClass);

    EXPECT_TRUE(targetRelClass->GetSource().IsAbstractConstraintDefined());
    ECClassCP destSourceClass = targetRelClass->GetSource().GetAbstractConstraint();
    EXPECT_STREQ(m_sourceSchema->GetName().c_str(), destSourceClass->GetSchema().GetName().c_str());

    EXPECT_TRUE(targetRelClass->GetTarget().IsAbstractConstraintDefined());
    ECClassCP destTargetClass = targetRelClass->GetTarget().GetAbstractConstraint();
    EXPECT_STREQ(m_sourceSchema->GetName().c_str(), destTargetClass->GetSchema().GetName().c_str());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*m_targetSchema, *m_sourceSchema));
    }

END_BENTLEY_ECN_TEST_NAMESPACE
