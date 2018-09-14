/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/SchemaCopyTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
        void CopyClass(bool copyReferences);
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
    CopyTestFixture::SetUp();
    bvector<WString> searchPaths;
    searchPaths.push_back(ECTestFixture::GetTestDataPath(L""));
    m_schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
    m_schemaContext = ECSchemaReadContext::CreateContext();
    m_schemaContext->AddSchemaLocater(*m_schemaLocater);
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

//---------------------------------------------------------------------------------------
// @bsimethod                                               Kyle.Abramowitz    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopiedSchemaShouldAlwaysHaveOriginalXmlVersionSetToLatest)
    {
    uint32_t latestMajor;
    uint32_t latestMinor;

    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(m_sourceSchema, "TestSchema", "ts", 1, 0, 0, ECVersion::Latest));
    CopySchema();
    ECSchema::ParseECVersion(latestMajor, latestMinor, ECVersion::V3_1);
    EXPECT_EQ(m_targetSchema->GetOriginalECXmlVersionMajor(), latestMajor);
    EXPECT_EQ(m_targetSchema->GetOriginalECXmlVersionMinor(), latestMinor);

    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(m_sourceSchema, "TestSchema", "ts", 1, 0, 0, ECVersion::V2_0));
    CopySchema();
    EXPECT_EQ(m_targetSchema->GetOriginalECXmlVersionMajor(), latestMajor);
    EXPECT_EQ(m_targetSchema->GetOriginalECXmlVersionMinor(), latestMinor);
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
    m_sourceSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());
    m_sourceSchema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema());
    koq->SetDisplayLabel("Test KoQ");
    koq->SetDescription("Test Description");
    koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M"));
    koq->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("AmerFI"));
    koq->AddPresentationFormatSingleUnitOverride(*ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultRealU"), nullptr, ECTestFixture::GetUnitsSchema()->GetUnitCP("M"));
    koq->SetRelativeError(10e-3);

    CopySchema();

    EXPECT_EQ(1, m_targetSchema->GetKindOfQuantityCount());

    KindOfQuantityCP targetKoq = m_targetSchema->GetKindOfQuantityCP("TestKoQ");
    ASSERT_TRUE(nullptr != targetKoq);
    EXPECT_STREQ("Test KoQ", targetKoq->GetDisplayLabel().c_str());
    EXPECT_STREQ("Test Description", targetKoq->GetDescription().c_str());
    EXPECT_STREQ("AmerFI", targetKoq->GetDefaultPresentationFormat()->GetName().c_str());
    EXPECT_EQ(2, targetKoq->GetPresentationFormats().size());
    EXPECT_STREQ("DefaultRealU[u:M]", targetKoq->GetPresentationFormats().at(1).GetName().c_str());
    EXPECT_STREQ("M", targetKoq->GetPersistenceUnit()->GetName().c_str());
    EXPECT_EQ(10e-3, targetKoq->GetRelativeError());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, TestKindOfQuantity_NoPresentationUnit)
    {
    CreateTestSchema();

    KindOfQuantityP koq;
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateKindOfQuantity(koq, "TestKoQ"));
    m_sourceSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());
    koq->SetDisplayLabel("Test KoQ");
    koq->SetDescription("Test Description");
    koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M"));
    koq->SetRelativeError(10e-3);

    CopySchema();

    EXPECT_EQ(1, m_targetSchema->GetKindOfQuantityCount());

    KindOfQuantityCP targetKoq = m_targetSchema->GetKindOfQuantityCP("TestKoQ");
    ASSERT_TRUE(nullptr != targetKoq);
    EXPECT_STREQ("Test KoQ", targetKoq->GetDisplayLabel().c_str());
    EXPECT_STREQ("Test Description", targetKoq->GetDescription().c_str());
    EXPECT_STREQ("M", targetKoq->GetPersistenceUnit()->GetName().c_str());
    EXPECT_FALSE(targetKoq->HasPresentationFormats());
    EXPECT_EQ(10e-3, targetKoq->GetRelativeError());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, TestKindOfQuantity_PersistanceUnitDefinedInSchema)
    {
    CreateTestSchema();

    KindOfQuantityP koq;
    ECUnitP unit;
    UnitSystemP system;
    PhenomenonP phenom;
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateUnitSystem(system, "SMOOT_SYSTEM", "SMOOT_SYSTEM_LABEL", "SMOOT_SYSTEM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(m_sourceSchema->CreatePhenomenon(phenom, "SMOOT_PHENOM", "SMOOT", "SMOOT_PHENOM_LABEL", "SMOOT_PHENOM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateUnit(unit, "SMOOT", "SMOOT", *phenom, *system, "SMOOT", "SMOOT"));
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateKindOfQuantity(koq, "TestKoQ"));
    koq->SetDisplayLabel("Test KoQ");
    koq->SetDescription("Test Description");
    koq->SetPersistenceUnit(*m_sourceSchema->GetUnitCP("SMOOT"));
    koq->SetRelativeError(10e-3);

    CopySchema();

    EXPECT_EQ(1, m_targetSchema->GetKindOfQuantityCount());

    KindOfQuantityCP targetKoq = m_targetSchema->GetKindOfQuantityCP("TestKoQ");
    ASSERT_TRUE(nullptr != targetKoq);
    EXPECT_STREQ("Test KoQ", targetKoq->GetDisplayLabel().c_str());
    EXPECT_STREQ("Test Description", targetKoq->GetDescription().c_str());
    EXPECT_STREQ("SMOOT", targetKoq->GetPersistenceUnit()->GetName().c_str());
    EXPECT_FALSE(targetKoq->HasPresentationFormats());
    EXPECT_EQ(10e-3, targetKoq->GetRelativeError());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, TestKindOfQuantity_PresentationUnitDefinedInSchema)
    {
    CreateTestSchema();

    KindOfQuantityP koq;
    ECUnitP unit;
    UnitSystemP system;
    PhenomenonP phenom;
    ECFormatP format;
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateUnitSystem(system, "SMOOT_SYSTEM", "SMOOT_SYSTEM_LABEL", "SMOOT_SYSTEM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(m_sourceSchema->CreatePhenomenon(phenom, "SMOOT_PHENOM", "SMOOT", "SMOOT_PHENOM_LABEL", "SMOOT_PHENOM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateUnit(unit, "SMOOT", "SMOOT", *phenom, *system, "SMOOT", "SMOOT"));
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateUnit(unit, "SMOOT_SQUARED", "SMOOT", *phenom, *system, "SMOOT", "SMOOT"));
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateFormat(format, "SMOOT_FORMAT"));
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateKindOfQuantity(koq, "TestKoQ"));
    koq->SetDisplayLabel("Test KoQ");
    koq->SetDescription("Test Description");
    koq->SetPersistenceUnit(*m_sourceSchema->GetUnitCP("SMOOT"));
    koq->SetDefaultPresentationFormat(*format, nullptr, unit);
    koq->SetRelativeError(10e-3);

    CopySchema();

    EXPECT_EQ(1, m_targetSchema->GetKindOfQuantityCount());

    KindOfQuantityCP targetKoq = m_targetSchema->GetKindOfQuantityCP("TestKoQ");
    ECFormatCP targetFormat = m_targetSchema->GetFormatCP("SMOOT_FORMAT");
    ASSERT_TRUE(nullptr != targetKoq);
    ASSERT_TRUE(nullptr != targetFormat);
    EXPECT_STREQ("Test KoQ", targetKoq->GetDisplayLabel().c_str());
    EXPECT_STREQ("Test Description", targetKoq->GetDescription().c_str());
    EXPECT_STREQ("SMOOT", targetKoq->GetPersistenceUnit()->GetName().c_str());
    EXPECT_STREQ("SMOOT_FORMAT[SMOOT_SQUARED]", targetKoq->GetDefaultPresentationFormat()->GetName().c_str());
    EXPECT_TRUE(targetKoq->HasPresentationFormats());
    EXPECT_EQ(10e-3, targetKoq->GetRelativeError());
    }

//---------------------------------------------------------------------------------------//
// @bsimethod                                           Colin.Kerr                  02/2018
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(SchemaCopyTest, CopySchemaWithReferencesCopiedThroughBaseClassOrRelationshipConstraints)
    {
    CreateTestSchema();

    ECEnumerationP enumeration;
    ECEnumeratorP enumeratorA;
    ECEntityClassP entityClass;
    PrimitiveECPropertyP enumProp;
    ECRelationshipClassP relClass;

    EC_ASSERT_SUCCESS(m_sourceSchema->CreateEnumeration(enumeration, "Enumeration", PrimitiveType::PRIMITIVETYPE_Integer));
    ASSERT_TRUE(enumeration != nullptr);
    enumeration->SetIsStrict(false);
    enumeration->CreateEnumerator(enumeratorA, "enumeratorA", 42);

    EC_ASSERT_SUCCESS(m_sourceSchema->CreateEntityClass(entityClass, "Banana"));
    EC_ASSERT_SUCCESS(entityClass->CreatePrimitiveProperty(enumProp, "Silly", PrimitiveType::PRIMITIVETYPE_Integer));
    EC_ASSERT_SUCCESS(enumProp->SetType(*enumeration));

    // Important that this class name sorts before 'Banana' class so it is copied first
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateRelationshipClass(relClass, "ARelClass"));
    relClass->GetSource().AddClass(*entityClass);
    relClass->GetSource().SetRoleLabel("From Banana");
    relClass->GetTarget().AddClass(*entityClass);
    relClass->GetTarget().SetRoleLabel("To Banana");

    ECStructClassP structClass;
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateStructClass(structClass, "Struct"));

    ECEntityClassP entity2Class;
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateEntityClass(entity2Class, "Dill"));
    StructECPropertyP structProp;
    EC_ASSERT_SUCCESS(entity2Class->CreateStructProperty(structProp, "StructProp", *structClass));

    // Important that this class name sorts before both 'Struct' and 'Dill'
    ECEntityClassP entity3Class;
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateEntityClass(entity3Class, "BestPickleClass"));
    entity3Class->AddBaseClass(*entity2Class);
    
    CopySchema();

    ECEnumerationP targetEnum = m_targetSchema->GetEnumerationP("Enumeration");
    ASSERT_TRUE(targetEnum != nullptr);
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_Integer, targetEnum->GetType());
    EXPECT_FALSE(targetEnum->GetIsStrict());
    EXPECT_EQ(1, targetEnum->GetEnumeratorCount());
    ECEnumeratorCP copiedEnumeratorA = targetEnum->FindEnumerator(42);
    ASSERT_TRUE(nullptr != copiedEnumeratorA);
    EXPECT_EQ(42, copiedEnumeratorA->GetInteger());

    ECClassP targetClass = m_targetSchema->GetClassP("Banana");
    ASSERT_NE(nullptr, targetClass);
    ECPropertyP targetProp = targetClass->GetPropertyP("Silly");
    ASSERT_NE(nullptr, targetProp);
    ASSERT_TRUE(targetProp->GetIsPrimitive());
    ECEnumerationCP targetEnumFromProp = targetProp->GetAsPrimitivePropertyP()->GetEnumeration();
    EXPECT_NE(nullptr, targetEnumFromProp);
    if (nullptr != targetEnumFromProp)
        {
        EXPECT_STREQ("Enumeration", targetEnumFromProp->GetName().c_str());
        EXPECT_EQ(targetEnum, targetEnumFromProp) << "Should be same memory reference";
        }

    ECRelationshipClassP targetRelClass = m_targetSchema->GetClassP("ARelClass")->GetRelationshipClassP();
    ASSERT_NE(nullptr, targetRelClass);
    EXPECT_TRUE(targetRelClass->GetSource().SupportsClass(*targetClass));
    EXPECT_TRUE(targetRelClass->GetTarget().SupportsClass(*targetClass));

    ECClassP target2Class = m_targetSchema->GetClassP("Dill");
    ASSERT_NE(nullptr, target2Class);
    ECPropertyP target2Prop = target2Class->GetPropertyP("StructProp");
    ASSERT_TRUE(target2Prop->GetIsStruct());
    ECStructClassCR targetStructType = target2Prop->GetAsStructProperty()->GetType();
    ASSERT_EQ(m_targetSchema->GetClassP("Struct"), &targetStructType) << "Should be same memory reference";
    }

//---------------------------------------------------------------------------------------//
// @bsimethod                                           Colin.Kerr                  02/2018
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(SchemaCopyTest, CopySchemaWithIntEnumeration)
    {
    CreateTestSchema();

    ECEnumerationP enumeration;
    ECEnumeratorP enumeratorA;
    ECEnumeratorP enumeratorB;

    EC_ASSERT_SUCCESS(m_sourceSchema->CreateEnumeration(enumeration, "Enumeration", PrimitiveType::PRIMITIVETYPE_Integer));
    ASSERT_TRUE(enumeration != nullptr);
    enumeration->SetDisplayLabel("My Display Label");
    enumeration->SetDescription("Test Description");
    enumeration->SetIsStrict(true);
    enumeration->CreateEnumerator(enumeratorA, "enumeratorA", 42);
    enumeratorA->SetDisplayLabel("The value for 42");
    enumeration->CreateEnumerator(enumeratorB, "enumeratorB", 56);
    enumeratorB->SetDisplayLabel("The value for 56");

    CopySchema();

    ECEnumerationP targetEnum = m_targetSchema->GetEnumerationP("Enumeration");
    ASSERT_TRUE(targetEnum != nullptr);
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_Integer, targetEnum->GetType());
    EXPECT_STREQ("My Display Label", targetEnum->GetDisplayLabel().c_str());
    EXPECT_STREQ("Test Description", targetEnum->GetDescription().c_str());
    EXPECT_TRUE(targetEnum->GetIsStrict());
    EXPECT_EQ(2, targetEnum->GetEnumeratorCount());
    ECEnumeratorCP copiedEnumeratorA = targetEnum->FindEnumerator(42);
    ASSERT_TRUE(nullptr != copiedEnumeratorA);
    EXPECT_STREQ("The value for 42", copiedEnumeratorA->GetDisplayLabel().c_str());
    EXPECT_EQ(42, copiedEnumeratorA->GetInteger());
    ECEnumeratorCP copiedEnumeratorB = targetEnum->FindEnumerator(56);
    ASSERT_TRUE(nullptr != copiedEnumeratorB);
    EXPECT_STREQ("The value for 56", copiedEnumeratorB->GetDisplayLabel().c_str());
    EXPECT_EQ(56, copiedEnumeratorB->GetInteger());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Robert.Schili                      11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaCopyTest, CopySchemaWithStringEnumeration)
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
    enumeration->CreateEnumerator(enumeratorA, "EnumeratorA", "Value A");
    enumeratorA->SetDisplayLabel("The value for A");
    enumeration->CreateEnumerator(enumeratorB, "EnumeratorB", "Value B");
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

//---------------------------------------------------------------------------------------
// @bsimethod                          Kyle.Abramowitz                           03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopySchemaWithSystemAndPhenomenonInStandardUnitSchema)
    {
    CreateTestSchema();
    m_sourceSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema(true));
    ECUnitP unit;
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateUnit(unit, "SMOOT", "SMOOT", *ECTestFixture::GetUnitsSchema()->GetPhenomenonCP("LENGTH"), *ECTestFixture::GetUnitsSchema()->GetUnitSystemCP("SI"), 10.0, 10.0, 10.0, "SMOOT", "SMOOT"));

    CopySchema();

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*m_targetSchema, *ECTestFixture::GetUnitsSchema()));
    EXPECT_EQ(1, m_targetSchema->GetUnitCount());

    ECUnitCP targetUnit = m_targetSchema->GetUnitCP("SMOOT");
    ASSERT_TRUE(nullptr != targetUnit);
    EXPECT_STREQ("SMOOT", targetUnit->GetDisplayLabel().c_str());
    EXPECT_STREQ("SMOOT", targetUnit->GetDescription().c_str());
    EXPECT_STREQ("SMOOT", targetUnit->GetDefinition().c_str());
    EXPECT_DOUBLE_EQ(10.0, targetUnit->GetNumerator());
    EXPECT_DOUBLE_EQ(10.0, targetUnit->GetDenominator());
    EXPECT_DOUBLE_EQ(10.0, targetUnit->GetOffset());
    EXPECT_STRCASEEQ(ECTestFixture::GetUnitsSchema()->GetPhenomenonCP("LENGTH")->GetName().c_str(), targetUnit->GetPhenomenon()->GetName().c_str());
    EXPECT_STRCASEEQ(ECTestFixture::GetUnitsSchema()->GetUnitSystemCP("SI")->GetName().c_str(), targetUnit->GetUnitSystem()->GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                          Kyle.Abramowitz                           03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopySchemaWithInvertedUnitWithSystemPhenomenonAndUnitInStandardSchema)
    {
    CreateTestSchema();
    m_sourceSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema(true));
    ECUnitP unit;
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateInvertedUnit(unit, *ECTestFixture::GetUnitsSchema()->GetUnitCP("M"), "SMOOT", *ECTestFixture::GetUnitsSchema()->GetUnitSystemCP("SI"), "SMOOT", "SMOOT"));

    CopySchema();

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*m_targetSchema, *ECTestFixture::GetUnitsSchema()));
    EXPECT_EQ(1, m_targetSchema->GetUnitCount());

    ECUnitCP targetUnit = m_targetSchema->GetInvertedUnitCP("SMOOT");
    ASSERT_EQ(targetUnit->GetInvertingUnit(), ECTestFixture::GetUnitsSchema()->GetUnitCP("M"));
    ASSERT_TRUE(nullptr != targetUnit);
    EXPECT_STREQ("SMOOT", targetUnit->GetDisplayLabel().c_str());
    EXPECT_STREQ("SMOOT", targetUnit->GetDescription().c_str());
    EXPECT_STRCASEEQ(ECTestFixture::GetUnitsSchema()->GetUnitCP("M")->GetName().c_str(), targetUnit->GetInvertingUnit()->GetName().c_str());
    EXPECT_STRCASEEQ(ECTestFixture::GetUnitsSchema()->GetPhenomenonCP("LENGTH")->GetName().c_str(), targetUnit->GetPhenomenon()->GetName().c_str());
    ASSERT_TRUE(targetUnit->HasUnitSystem());
    EXPECT_STRCASEEQ(ECTestFixture::GetUnitsSchema()->GetUnitSystemCP("SI")->GetName().c_str(), targetUnit->GetUnitSystem()->GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                          Kyle.Abramowitz                           03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopySchemaWithConstantWithPhenomenonInStandardSchema)
    {
    CreateTestSchema();
    m_sourceSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema(true));
    ECUnitP unit;
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateConstant(unit, "SMOOT", "SMOOT", *ECTestFixture::GetUnitsSchema()->GetPhenomenonCP("LENGTH"), 10.0, 10.0, "SMOOT", "SMOOT"));

    CopySchema();

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*m_targetSchema, *ECTestFixture::GetUnitsSchema()));
    EXPECT_EQ(1, m_targetSchema->GetUnitCount());

    ECUnitCP targetUnit = m_targetSchema->GetConstantCP("SMOOT");
    ASSERT_TRUE(nullptr != targetUnit);
    EXPECT_STREQ("SMOOT", targetUnit->GetDisplayLabel().c_str());
    EXPECT_STREQ("SMOOT", targetUnit->GetDescription().c_str());
    EXPECT_STREQ("SMOOT", targetUnit->GetDefinition().c_str());
    EXPECT_DOUBLE_EQ(10.0, targetUnit->GetNumerator());
    EXPECT_DOUBLE_EQ(10.0, targetUnit->GetDenominator());
    EXPECT_STRCASEEQ(ECTestFixture::GetUnitsSchema()->GetPhenomenonCP("LENGTH")->GetName().c_str(), targetUnit->GetPhenomenon()->GetName().c_str());
    ASSERT_FALSE(targetUnit->HasUnitSystem());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                          Kyle.Abramowitz                           03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopySchemaWithUnitAllDefinedInSchema)
    {
    CreateTestSchema();
    ECUnitP unit;
    UnitSystemP system;
    PhenomenonP phenom;
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateUnitSystem(system, "SMOOT_SYSTEM", "SMOOT_SYSTEM_LABEL", "SMOOT_SYSTEM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(m_sourceSchema->CreatePhenomenon(phenom, "SMOOT_PHENOM", "SMOOT", "SMOOT_PHENOM_LABEL", "SMOOT_PHENOM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateUnit(unit, "SMOOT", "SMOOT", *phenom, *system, 10.0, 9.0, 8.0, "SMOOT", "SMOOT"));

    CopySchema();

    EXPECT_EQ(1, m_targetSchema->GetUnitCount());

    ECUnitCP targetUnit = m_targetSchema->GetUnitCP("SMOOT");
    ASSERT_TRUE(nullptr != targetUnit);
    EXPECT_STREQ("SMOOT", targetUnit->GetDisplayLabel().c_str());
    EXPECT_STREQ("SMOOT", targetUnit->GetDescription().c_str());
    EXPECT_STREQ("SMOOT", targetUnit->GetDefinition().c_str());
    EXPECT_DOUBLE_EQ(10.0, targetUnit->GetNumerator());
    EXPECT_DOUBLE_EQ(9.0, targetUnit->GetDenominator());
    EXPECT_DOUBLE_EQ(8.0, targetUnit->GetOffset());
    EXPECT_STREQ("SMOOT_PHENOM", targetUnit->GetPhenomenon()->GetName().c_str());
    EXPECT_STREQ("SMOOT_SYSTEM", targetUnit->GetUnitSystem()->GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                          Kyle.Abramowitz                           03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopySchemaWithInvertedUnitAllDefinedInSchema)
    {
    CreateTestSchema();
    ECUnitP unit;
    ECUnitP invUnit;
    UnitSystemP system;
    PhenomenonP phenom;
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateUnitSystem(system, "SMOOT_SYSTEM", "SMOOT_SYSTEM_LABEL", "SMOOT_SYSTEM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(m_sourceSchema->CreatePhenomenon(phenom, "SMOOT_PHENOM", "SMOOT", "SMOOT_PHENOM_LABEL", "SMOOT_PHENOM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateUnit(unit, "SMOOT", "SMOOT", *phenom, *system, "SMOOT", "SMOOT"));
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateInvertedUnit(invUnit, *unit, "INVERSE_SMOOT", *system, "SMOOT", "SMOOT"));

    CopySchema();

    EXPECT_EQ(2, m_targetSchema->GetUnitCount());

    ECUnitCP targetUnit = m_targetSchema->GetInvertedUnitCP("INVERSE_SMOOT");
    ASSERT_TRUE(nullptr != targetUnit);
    ASSERT_TRUE(targetUnit->IsInvertedUnit());
    ASSERT_TRUE(targetUnit->HasUnitSystem());
    EXPECT_STRCASEEQ("SMOOT", targetUnit->GetInvertingUnit()->GetName().c_str());
    EXPECT_STREQ("SMOOT", targetUnit->GetDisplayLabel().c_str());
    EXPECT_STREQ("SMOOT", targetUnit->GetDescription().c_str());
    EXPECT_STREQ("SMOOT_PHENOM", targetUnit->GetPhenomenon()->GetName().c_str());
    EXPECT_STREQ("SMOOT_SYSTEM", targetUnit->GetUnitSystem()->GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                          Kyle.Abramowitz                           03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopySchemaWithConstantAllDefinedInSchema)
    {
    CreateTestSchema();
    ECUnitP unit;
    UnitSystemP system;
    PhenomenonP phenom;
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateUnitSystem(system, "SMOOT_SYSTEM", "SMOOT_SYSTEM_LABEL", "SMOOT_SYSTEM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(m_sourceSchema->CreatePhenomenon(phenom, "SMOOT_PHENOM", "SMOOT", "SMOOT_PHENOM_LABEL", "SMOOT_PHENOM_DESCRIPTION"));
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateConstant(unit, "SMOOT", "SMOOT", *phenom, 10.0, 9.0, "SMOOT", "SMOOT"));

    CopySchema();

    EXPECT_EQ(1, m_targetSchema->GetUnitCount());

    ECUnitCP targetUnit = m_targetSchema->GetUnitCP("SMOOT");
    ASSERT_TRUE(nullptr != targetUnit);
    ASSERT_TRUE(targetUnit->IsConstant());
    ASSERT_FALSE(targetUnit->HasUnitSystem());
    EXPECT_STREQ("SMOOT", targetUnit->GetDisplayLabel().c_str());
    EXPECT_STREQ("SMOOT", targetUnit->GetDescription().c_str());
    EXPECT_STREQ("SMOOT", targetUnit->GetDefinition().c_str());
    EXPECT_DOUBLE_EQ(10.0, targetUnit->GetNumerator());
    EXPECT_DOUBLE_EQ(9.0, targetUnit->GetDenominator());
    EXPECT_STREQ("SMOOT_PHENOM", targetUnit->GetPhenomenon()->GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                          Kyle.Abramowitz                           03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopyStandardUnitsSchema)
    {
    ECTestFixture::GetUnitsSchema()->CopySchema(m_targetSchema);
    PhenomenonCP length = m_targetSchema->GetPhenomenonCP("LENGTH");
    ECUnitCP m = m_targetSchema->GetUnitCP("M");
    ECUnitCP pi = m_targetSchema->GetConstantCP("PI");

    EXPECT_TRUE(Units::Phenomenon::AreEqual(length, ECTestFixture::GetUnitsSchema()->GetPhenomenonCP("LENGTH")));
    EXPECT_TRUE(Units::Unit::AreEqual(m, ECTestFixture::GetUnitsSchema()->GetUnitCP("M")));
    EXPECT_TRUE(Units::Unit::AreEqual(pi, ECTestFixture::GetUnitsSchema()->GetConstantCP("PI")));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                          Kyle.Abramowitz                           03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopyStandardFormatsSchema)
    {
    ECTestFixture::GetFormatsSchema()->CopySchema(m_targetSchema);
    ECFormatCP def = m_targetSchema->GetFormatCP("DefaultRealU");
    ECFormatCP amer = m_targetSchema->GetFormatCP("AmerFI");

    ASSERT_NE(nullptr, def);
    ASSERT_NE(nullptr, amer);
    auto referenceDefault = ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultRealU")->GetNumericSpec();
    auto defNum = def->GetNumericSpec();

    EXPECT_EQ(referenceDefault->GetDecimalPrecision(), defNum->GetDecimalPrecision());
    EXPECT_EQ(referenceDefault->GetDecimalSeparator(), defNum->GetDecimalSeparator());
    EXPECT_EQ(referenceDefault->GetFormatTraits(), defNum->GetFormatTraits());
    EXPECT_EQ(referenceDefault->GetMinWidth(), defNum->GetMinWidth());
    EXPECT_EQ(referenceDefault->GetPresentationType(), defNum->GetPresentationType());
    EXPECT_DOUBLE_EQ(referenceDefault->GetRoundingFactor(), defNum->GetRoundingFactor());
    EXPECT_EQ(referenceDefault->GetSignOption(), defNum->GetSignOption());
    EXPECT_EQ(referenceDefault->GetStationOffsetSize(), defNum->GetStationOffsetSize());
    EXPECT_EQ(referenceDefault->GetStationSeparator(), defNum->GetStationSeparator());
    EXPECT_EQ(referenceDefault->GetThousandSeparator(), defNum->GetThousandSeparator());
    EXPECT_STREQ(referenceDefault->GetUomSeparator(), defNum->GetUomSeparator());
    EXPECT_FALSE(def->HasComposite());

    auto refAmerFiNum = ECTestFixture::GetFormatsSchema()->GetFormatCP("AmerFI")->GetNumericSpec();
    auto refAmerFIComp = ECTestFixture::GetFormatsSchema()->GetFormatCP("AmerFI")->GetCompositeSpec();

    auto amerFiNum = amer->GetNumericSpec();
    auto amerFiComp = amer->GetCompositeSpec();

    ASSERT_TRUE(amer->HasComposite());
    EXPECT_EQ(refAmerFiNum->GetDecimalPrecision(), amerFiNum->GetDecimalPrecision());
    EXPECT_EQ(refAmerFiNum->GetDecimalSeparator(), amerFiNum->GetDecimalSeparator());
    EXPECT_EQ(refAmerFiNum->GetFormatTraits(), amerFiNum->GetFormatTraits());
    EXPECT_EQ(refAmerFiNum->GetMinWidth(), amerFiNum->GetMinWidth());
    EXPECT_EQ(refAmerFiNum->GetPresentationType(), amerFiNum->GetPresentationType());
    EXPECT_DOUBLE_EQ(refAmerFiNum->GetRoundingFactor(), amerFiNum->GetRoundingFactor());
    EXPECT_EQ(refAmerFiNum->GetSignOption(), amerFiNum->GetSignOption());
    EXPECT_EQ(refAmerFiNum->GetStationOffsetSize(), amerFiNum->GetStationOffsetSize());
    EXPECT_EQ(refAmerFiNum->GetStationSeparator(), amerFiNum->GetStationSeparator());
    EXPECT_EQ(refAmerFiNum->GetThousandSeparator(), amerFiNum->GetThousandSeparator());
    EXPECT_STREQ(refAmerFiNum->GetUomSeparator(), amerFiNum->GetUomSeparator());

    EXPECT_STRCASEEQ(refAmerFIComp->GetSpacer().c_str(), amerFiComp->GetSpacer().c_str());
    EXPECT_STRCASEEQ(refAmerFIComp->GetMajorLabel().c_str(), amerFiComp->GetMajorLabel().c_str());
    EXPECT_EQ(refAmerFIComp->GetMajorUnit(), amerFiComp->GetMajorUnit());
    EXPECT_STRCASEEQ(refAmerFIComp->GetMiddleLabel().c_str(), amerFiComp->GetMiddleLabel().c_str());
    EXPECT_EQ(refAmerFIComp->GetMiddleUnit(), amerFiComp->GetMiddleUnit());
    EXPECT_STRCASEEQ(refAmerFIComp->GetMinorLabel().c_str(), amerFiComp->GetMinorLabel().c_str());
    EXPECT_EQ(refAmerFIComp->GetMinorUnit(), amerFiComp->GetMinorUnit());
    EXPECT_STRCASEEQ(refAmerFIComp->GetSubLabel().c_str(), amerFiComp->GetSubLabel().c_str());
    EXPECT_EQ(refAmerFIComp->GetSubUnit(), amerFiComp->GetSubUnit());
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
void ClassCopyTest::CopyClass(bool copyReferences)
    {
    EC_EXPECT_SUCCESS(m_targetSchema->CopyClass(m_targetClass, *m_sourceClass, m_sourceClass->GetName().c_str(), copyReferences));
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
