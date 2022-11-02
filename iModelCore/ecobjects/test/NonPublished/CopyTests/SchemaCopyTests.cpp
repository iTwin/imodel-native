/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../../ECObjectsTestPCH.h"
#include "../../TestFixture/TestFixture.h"
#include "BeXml/BeXml.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

//=======================================================================================
//! SchemaCopyTest
//=======================================================================================

struct SchemaCopyTest : CopyTestFixture
    {
    ECSchemaReadContextPtr   m_schemaContext;
    SearchPathSchemaFileLocaterPtr m_schemaLocater;

    protected:
        void SetUp() override;
        void TearDown() override;

        void CopySchema() { CopySchema(m_targetSchema); }
        void CopySchema(ECSchemaPtr& targetSchema);

        void ValidateElementOrder(bvector<Utf8String> expectedTypeNames, BeXmlNodeP root);
    };

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void SchemaCopyTest::TearDown()
    {
    m_schemaContext->RemoveSchemaLocater(*m_schemaLocater);
    CopyTestFixture::TearDown();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void SchemaCopyTest::CopySchema(ECSchemaPtr& targetSchema)
    {
    targetSchema = nullptr;
    EC_ASSERT_SUCCESS(m_sourceSchema->CopySchema(targetSchema));
    EXPECT_TRUE(targetSchema.IsValid());
    EXPECT_NE(m_sourceSchema, targetSchema);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void SchemaCopyTest::ValidateElementOrder(bvector<Utf8String> expectedTypeNames, BeXmlNodeP root)
    {
    BeXmlNodeP currentNode = root->GetFirstChild();
    for(auto expectedTypeName : expectedTypeNames)
        {
        if(currentNode == nullptr)
            {
            FAIL() << "Expected end of document, Node '" << expectedTypeName << "' expected.";
            }

        Utf8String nodeTypeName;
        EXPECT_EQ(BeXmlStatus::BEXML_Success, currentNode->GetAttributeStringValue(nodeTypeName, "typeName"));
        EXPECT_EQ(expectedTypeName, nodeTypeName);

        currentNode = currentNode->GetNextSibling();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
    EXPECT_NE(m_sourceSchema->GetAlias().c_str(), m_targetSchema->GetAlias().c_str());

    ValidateNameDescriptionAndDisplayLabel(*m_sourceSchema.get(), *m_targetSchema.get());

    EXPECT_EQ(3, m_targetSchema->GetVersionRead());
    EXPECT_EQ(10, m_targetSchema->GetVersionWrite());
    EXPECT_EQ(5, m_targetSchema->GetVersionMinor());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopiedSchemaDoesNotPersistOriginalSchemaXMLVersion)
    {
    uint32_t currentMajor, currentMinor;
    ECSchema::ParseECVersion(currentMajor, currentMinor, ECVersion::Latest);

    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(m_sourceSchema, "TestSchema", "ts", 1, 0, 0));
    m_sourceSchema->SetOriginalECXmlVersion(3, 1);
    CopySchema();
    EXPECT_EQ(currentMajor, m_targetSchema->GetOriginalECXmlVersionMajor());
    EXPECT_EQ(currentMinor, m_targetSchema->GetOriginalECXmlVersionMinor());

    m_sourceSchema->SetOriginalECXmlVersion(2, 0);
    CopySchema();
    EXPECT_EQ(currentMajor, m_targetSchema->GetOriginalECXmlVersionMajor());
    EXPECT_EQ(currentMinor, m_targetSchema->GetOriginalECXmlVersionMinor());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// TODO: Caleb - Investigate what this is testing...
TEST_F(SchemaCopyTest, ExpectSuccessWhenCopyingStructs)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(m_sourceSchema, ECTestFixture::GetTestDataPath(L"Widgets.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_TRUE(m_sourceSchema.IsValid());
    EXPECT_EQ(SchemaReadStatus::Success, status);

    CopySchema();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
    EXPECT_TRUE (out.Equals(ECValue ("test"))) << "Expect: " << "test" << " Actual: " << out.ToString().c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
    EXPECT_TRUE(out.Equals(ECValue ("test"))) << "Expect: " << "test" << " Actual: " << out.ToString().c_str();

    ECSchemaPtr copiedSchema = nullptr;
    CopySchema(copiedSchema);
    
    ellipseClass = copiedSchema->GetClassCP("circle");
    ellipseClassInstance = ellipseClass->GetDefaultStandaloneEnabler ()->CreateInstance ();

    v.SetUtf8CP("test");
    ellipseClassInstance->SetValue("Name", v);
    EXPECT_EQ(ECObjectsStatus::Success, ellipseClassInstance->GetValue(out, "Name"));
    EXPECT_TRUE(out.Equals(ECValue("test"))) << "Expect: " << "test" << " Actual: " << out.ToString().c_str();
    }

//---------------------------------------------------------------------------------------//
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------//
// TODO: Caleb - Investigate if needed now that all other tests are in place.
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
    EXPECT_NE(enumeration, targetEnum);
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_Integer, targetEnum->GetType());
    EXPECT_FALSE(targetEnum->GetIsStrict());
    EXPECT_EQ(1, targetEnum->GetEnumeratorCount());
    ECEnumeratorCP copiedEnumeratorA = targetEnum->FindEnumerator(42);
    ASSERT_TRUE(nullptr != copiedEnumeratorA);
    EXPECT_NE(enumeratorA, copiedEnumeratorA);
    EXPECT_EQ(42, copiedEnumeratorA->GetInteger());

    ECClassP targetClass = m_targetSchema->GetClassP("Banana");
    ASSERT_NE(nullptr, targetClass);
    EXPECT_NE(entityClass, targetClass);
    ECPropertyP targetProp = targetClass->GetPropertyP("Silly");
    ASSERT_NE(nullptr, targetProp);
    EXPECT_NE(nullptr, enumProp);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaCopyTest, CopyEnumeration)
    {
    CreateTestSchema();

    ECEnumerationP sourceStringEnum;
    ECEnumeratorP sourceStringEnumeratorA;
    ECEnumeratorP sourceStringEnumeratorB;
    ECEnumerationP sourceIntEnum;
    ECEnumeratorP sourceIntEnumeratorA;
    ECEnumeratorP sourceIntEnumeratorB;

    EC_ASSERT_SUCCESS(m_sourceSchema->CreateEnumeration(sourceStringEnum, "StringEnumeration", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_TRUE(sourceStringEnum != nullptr);
    sourceStringEnum->SetDisplayLabel("My Display Label");
    sourceStringEnum->SetDescription("Test Description");
    sourceStringEnum->SetIsStrict(false);
    sourceStringEnum->CreateEnumerator(sourceStringEnumeratorA, "EnumeratorA", "Value A");
    sourceStringEnumeratorA->SetDisplayLabel("The value for A");
    sourceStringEnumeratorA->SetDescription("Test Description A");
    sourceStringEnum->CreateEnumerator(sourceStringEnumeratorB, "EnumeratorB", "Value B");
    sourceStringEnumeratorB->SetDisplayLabel("The value for B");
    sourceStringEnumeratorB->SetDescription("Test Description B");

    EC_ASSERT_SUCCESS(m_sourceSchema->CreateEnumeration(sourceIntEnum, "IntEnumeration", PrimitiveType::PRIMITIVETYPE_Integer));
    ASSERT_TRUE(sourceIntEnum != nullptr);
    sourceIntEnum->SetDisplayLabel("My Display Label");
    sourceIntEnum->SetDescription("Test Description");
    sourceIntEnum->SetIsStrict(true);
    sourceIntEnum->CreateEnumerator(sourceIntEnumeratorA, "enumeratorA", 42);
    sourceIntEnumeratorA->SetDisplayLabel("The value for 42");
    sourceIntEnumeratorA->SetDescription("Test Description A");
    sourceIntEnum->CreateEnumerator(sourceIntEnumeratorB, "enumeratorB", 56);
    sourceIntEnumeratorB->SetDisplayLabel("The value for 56");
    sourceIntEnumeratorB->SetDescription("Test Description A");

    CopySchema();

    // String backed
    ECEnumerationP targetStringEnum = m_targetSchema->GetEnumerationP("StringEnumeration");
    ASSERT_TRUE(targetStringEnum != nullptr);
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_String, targetStringEnum->GetType());
    ValidateNameDescriptionAndDisplayLabel(*sourceStringEnum, *targetStringEnum);
    EXPECT_FALSE(targetStringEnum->GetIsStrict());
    EXPECT_EQ(2, targetStringEnum->GetEnumeratorCount());
    
    ECEnumeratorCP targetEnumeratorA = targetStringEnum->FindEnumerator("Value A");
    ASSERT_TRUE(nullptr != targetEnumeratorA);
    EXPECT_NE(sourceStringEnumeratorA, targetEnumeratorA);
    ValidateNameDescriptionAndDisplayLabel(*sourceStringEnumeratorA, *targetEnumeratorA);
    EXPECT_STREQ(sourceStringEnumeratorA->GetString().c_str(), targetEnumeratorA->GetString().c_str());
    EXPECT_NE(sourceStringEnumeratorA->GetString().c_str(), targetEnumeratorA->GetString().c_str());

    ECEnumeratorCP targetEnumeratorB = targetStringEnum->FindEnumerator("Value B");
    ASSERT_TRUE(nullptr != targetEnumeratorB);
    EXPECT_NE(sourceStringEnumeratorB, targetEnumeratorB);
    ValidateNameDescriptionAndDisplayLabel(*sourceStringEnumeratorB, *targetEnumeratorB);
    EXPECT_STREQ(sourceStringEnumeratorB->GetString().c_str(), targetEnumeratorB->GetString().c_str());
    EXPECT_NE(sourceStringEnumeratorB->GetString().c_str(), targetEnumeratorB->GetString().c_str());

    // Int backed

    ECEnumerationP targetIntEnum = m_targetSchema->GetEnumerationP("IntEnumeration");
    ASSERT_TRUE(targetIntEnum != nullptr);
    EXPECT_NE(sourceIntEnum, targetIntEnum);
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_Integer, targetIntEnum->GetType());
    ValidateNameDescriptionAndDisplayLabel(*sourceIntEnum, *targetIntEnum);
    EXPECT_TRUE(targetIntEnum->GetIsStrict());
    EXPECT_EQ(2, targetIntEnum->GetEnumeratorCount());

    ECEnumeratorCP targetIntEnumeratorA = targetIntEnum->FindEnumerator(42);
    ASSERT_TRUE(nullptr != targetEnumeratorA);
    EXPECT_NE(sourceIntEnumeratorA, targetIntEnumeratorA);
    ValidateNameDescriptionAndDisplayLabel(*sourceIntEnumeratorA, *targetIntEnumeratorA);
    EXPECT_EQ(sourceIntEnumeratorA->GetInteger(), targetIntEnumeratorA->GetInteger());

    ECEnumeratorCP targetIntEnumeratorB = targetIntEnum->FindEnumerator(56);
    ASSERT_TRUE(nullptr != targetIntEnumeratorB);
    EXPECT_NE(sourceIntEnumeratorB, targetIntEnumeratorB);
    EXPECT_EQ(sourceIntEnumeratorB->GetInteger(), targetIntEnumeratorB->GetInteger());
    ValidateNameDescriptionAndDisplayLabel(*sourceIntEnumeratorB, *targetIntEnumeratorB);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopyPropertyCategory)
    {
    CreateTestSchema();

    PropertyCategoryP sourcePropertyCategory;

    EC_ASSERT_SUCCESS(m_sourceSchema->CreatePropertyCategory(sourcePropertyCategory, "PropertyCategory"));
    ASSERT_TRUE(nullptr != sourcePropertyCategory);
    
    EC_ASSERT_SUCCESS(sourcePropertyCategory->SetDisplayLabel("My Display Label"));
    EC_ASSERT_SUCCESS(sourcePropertyCategory->SetDescription("My Description"));
    EC_ASSERT_SUCCESS(sourcePropertyCategory->SetPriority(3));

    CopySchema();

    PropertyCategoryCP targetPropertyCategory = m_targetSchema->GetPropertyCategoryCP("PropertyCategory");
    ASSERT_TRUE(nullptr != targetPropertyCategory);
    EXPECT_NE(sourcePropertyCategory, targetPropertyCategory);
    ValidateNameDescriptionAndDisplayLabel(*sourcePropertyCategory, *targetPropertyCategory);
    EXPECT_EQ(3, targetPropertyCategory->GetPriority());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopyEntityClass)
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
    EXPECT_NE(entity, targetClass);
    ValidateNameDescriptionAndDisplayLabel(*(ECClassCP) entity, *targetClass);
    EXPECT_EQ(ECClassType::Entity, targetClass->GetClassType());
    EXPECT_EQ(ECClassModifier::Sealed, targetClass->GetClassModifier());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopyEntityClassWithBaseClasses)
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
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopyEntityClassWithMixin)
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
// @bsimethod
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
    mixin0->CreatePrimitiveProperty(prop, "P1", PRIMITIVETYPE_String);
    entityDerived->AddBaseClass(*entityBase);
    entityDerived->AddBaseClass(*mixin0);

    EXPECT_EQ(2, entityDerived->GetBaseClasses().size());

    CopySchema();

    ECClassCP targetClass = m_targetSchema->GetClassP("Entity1");
    ASSERT_TRUE(nullptr != targetClass);
    EXPECT_EQ(2, targetClass->GetBaseClasses().size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
    EXPECT_NE(relClass, targetClass);
    EXPECT_EQ(ECClassType::Relationship, targetClass->GetClassType());
    EXPECT_EQ(ECClassModifier::Sealed, targetClass->GetClassModifier());
    ECRelationshipClassCP targetRelClass = targetClass->GetRelationshipClassCP();
    ASSERT_TRUE(nullptr != targetRelClass);
    EXPECT_EQ(relClass->GetStrength(), targetRelClass->GetStrength());
    EXPECT_EQ(relClass->GetStrengthDirection(), targetRelClass->GetStrengthDirection());
    ValidateNameDescriptionAndDisplayLabel(*relClass, *targetRelClass);

    ECClassCP targetSourceBase = m_targetSchema->GetClassCP("SourceBase");
    ECClassCP targetSource = m_targetSchema->GetClassCP("Source");
    EXPECT_EQ(1, targetRelClass->GetSource().GetConstraintClasses().size());
    EXPECT_EQ(targetSourceBase, targetRelClass->GetSource().GetAbstractConstraint());
    EXPECT_EQ(targetSource, targetRelClass->GetSource().GetConstraintClasses().front());
    EXPECT_FALSE(targetRelClass->GetSource().GetIsPolymorphic());
    EXPECT_STREQ("Source Role Label", targetRelClass->GetSource().GetRoleLabel().c_str());

    // Poor naming pattern...
    ECClassCP targetTargetBase = m_targetSchema->GetClassCP("TargetBase");
    ECClassCP targetTarget = m_targetSchema->GetClassCP("Target");
    EXPECT_EQ(1, targetRelClass->GetTarget().GetConstraintClasses().size());
    EXPECT_EQ(targetTargetBase, targetRelClass->GetTarget().GetAbstractConstraint());
    EXPECT_EQ(targetTarget, targetRelClass->GetTarget().GetConstraintClasses().front());
    EXPECT_FALSE(targetRelClass->GetTarget().GetIsPolymorphic());
    EXPECT_STREQ("Target Role Label", targetRelClass->GetTarget().GetRoleLabel().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopyStructClass)
    {
    CreateTestSchema();

    ECStructClassP structClass;
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateStructClass(structClass, "Struct"));
    structClass->SetClassModifier(ECClassModifier::Sealed); // default is ECClassModifier::None
    EC_ASSERT_SUCCESS(structClass->SetDescription("Description of the struct"));
    EC_ASSERT_SUCCESS(structClass->SetDisplayLabel("Struct Display Label"));

    CopySchema();

    ECClassCP targetClass = m_targetSchema->GetClassCP("Struct");
    ASSERT_TRUE(nullptr != targetClass);
    EXPECT_NE(structClass, targetClass);
    EXPECT_EQ(ECClassType::Struct, targetClass->GetClassType());
    EXPECT_EQ(ECClassModifier::Sealed, targetClass->GetClassModifier());
    ValidateNameDescriptionAndDisplayLabel(*(ECClassCP) structClass, *targetClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopyUnit_AllReferencesInSchema)
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
    EXPECT_NE(unit, targetUnit);
    ValidateNameDescriptionAndDisplayLabel(*unit, *targetUnit);
    EXPECT_STREQ("SMOOT", targetUnit->GetDefinition().c_str());
    EXPECT_DOUBLE_EQ(10.0, targetUnit->GetNumerator());
    EXPECT_DOUBLE_EQ(9.0, targetUnit->GetDenominator());
    EXPECT_DOUBLE_EQ(8.0, targetUnit->GetOffset());
    EXPECT_STREQ("SMOOT_PHENOM", targetUnit->GetPhenomenon()->GetName().c_str());
    EXPECT_STREQ("SMOOT_SYSTEM", targetUnit->GetUnitSystem()->GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopyUnit_AllReferencesInRefSchema)
    {
    PhenomenonCP standardLengthPhenom = ECTestFixture::GetUnitsSchema()->GetPhenomenonCP("LENGTH");
    UnitSystemCP standardSISystem = ECTestFixture::GetUnitsSchema()->GetUnitSystemCP("SI");

    CreateTestSchema();
    m_sourceSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema(false));
    ECUnitP unit;
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateUnit(unit, "SMOOT", "SMOOT", *standardLengthPhenom, *standardSISystem, 10.0, 10.0, 10.0, "SMOOT", "SMOOT"));

    CopySchema();

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*m_targetSchema, *ECTestFixture::GetUnitsSchema()));
    EXPECT_EQ(1, m_targetSchema->GetUnitCount());

    ECUnitCP targetUnit = m_targetSchema->GetUnitCP("SMOOT");
    ASSERT_TRUE(nullptr != targetUnit);
    EXPECT_NE(unit, targetUnit);
    ValidateNameDescriptionAndDisplayLabel(*unit, *targetUnit);
    EXPECT_STREQ(unit->GetDefinition().c_str(), targetUnit->GetDefinition().c_str());
    EXPECT_NE(unit->GetDefinition().c_str(), targetUnit->GetDefinition().c_str());
    EXPECT_DOUBLE_EQ(10.0, targetUnit->GetNumerator());
    EXPECT_DOUBLE_EQ(10.0, targetUnit->GetDenominator());
    EXPECT_DOUBLE_EQ(10.0, targetUnit->GetOffset());
    EXPECT_EQ(unit->GetPhenomenon(), targetUnit->GetPhenomenon());
    EXPECT_EQ(unit->GetUnitSystem(), targetUnit->GetUnitSystem());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopyInvertedUnit_AllReferencesInSchema)
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
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaCopyTest, CopyInvertedUnit_CopySchemaSucceedsWhenInvertedUnitIsCopiedAfterItsParentUnit)
    {
    CreateTestSchema();
    ECUnitP unit;
    ECUnitP invUnit;
    UnitSystemP system;
    PhenomenonP phenom;
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateUnitSystem(system, "A_SYSTEM"));
    EC_ASSERT_SUCCESS(m_sourceSchema->CreatePhenomenon(phenom, "A_PHENOM", "A"));
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateUnit(unit, "A", "A", *phenom, *system));
    // This will be added after parent unit because A < INVERSE_A
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateInvertedUnit(invUnit, *unit, "INVERSE_A", *system));

    CopySchema();

    EXPECT_EQ(2, m_targetSchema->GetUnitCount());

    ECUnitCP targetUnit = m_targetSchema->GetInvertedUnitCP("INVERSE_A");
    ASSERT_TRUE(nullptr != targetUnit);
    ASSERT_TRUE(targetUnit->IsInvertedUnit());
    ASSERT_TRUE(targetUnit->HasUnitSystem());
    EXPECT_STRCASEEQ("A", targetUnit->GetInvertingUnit()->GetName().c_str());
    EXPECT_STREQ("INVERSE_A", targetUnit->GetDisplayLabel().c_str());
    EXPECT_STREQ("", targetUnit->GetDescription().c_str());
    EXPECT_STREQ("A_PHENOM", targetUnit->GetPhenomenon()->GetName().c_str());
    EXPECT_STREQ("A_SYSTEM", targetUnit->GetUnitSystem()->GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopyInvertedUnit_AllReferencesInRefSchema)
    {
    CreateTestSchema();
    m_sourceSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());
    ECUnitP unit;
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateInvertedUnit(unit, *ECTestFixture::GetUnitsSchema()->GetUnitCP("M"), "SMOOT", *ECTestFixture::GetUnitsSchema()->GetUnitSystemCP("SI"), "SMOOT", "SMOOT"));

    CopySchema();

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*m_targetSchema, *ECTestFixture::GetUnitsSchema()));
    EXPECT_EQ(1, m_targetSchema->GetUnitCount());

    ECUnitCP targetUnit = m_targetSchema->GetInvertedUnitCP("SMOOT");
    ASSERT_TRUE(nullptr != targetUnit);
    EXPECT_NE(unit, targetUnit);
    ValidateNameDescriptionAndDisplayLabel(*unit, *targetUnit);
    EXPECT_EQ(unit->GetInvertingUnit(), targetUnit->GetInvertingUnit());
    EXPECT_EQ(unit->GetPhenomenon(), targetUnit->GetPhenomenon());
    EXPECT_EQ(unit->HasUnitSystem(), targetUnit->HasUnitSystem());
    EXPECT_EQ(unit->GetUnitSystem(), targetUnit->GetUnitSystem());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopyConstant_AllReferencesInSchema)
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
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopyConstant_AllReferencesInRefSchema)
    {
    CreateTestSchema();
    m_sourceSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());
    ECUnitP unit;
    EC_ASSERT_SUCCESS(m_sourceSchema->CreateConstant(unit, "SMOOT", "SMOOT", *ECTestFixture::GetUnitsSchema()->GetPhenomenonCP("LENGTH"), 10.0, 10.0, "SMOOT", "SMOOT"));

    CopySchema();

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*m_targetSchema, *ECTestFixture::GetUnitsSchema()));
    EXPECT_EQ(1, m_targetSchema->GetUnitCount());

    ECUnitCP targetUnit = m_targetSchema->GetConstantCP("SMOOT");
    ASSERT_TRUE(nullptr != targetUnit);
    EXPECT_NE(unit, targetUnit);
    ValidateNameDescriptionAndDisplayLabel(*unit, *targetUnit);
    EXPECT_STREQ(unit->GetDefinition().c_str(), targetUnit->GetDefinition().c_str());
    EXPECT_NE(unit->GetDefinition().c_str(), targetUnit->GetDefinition().c_str());
    EXPECT_DOUBLE_EQ(unit->GetNumerator(), targetUnit->GetNumerator());
    EXPECT_DOUBLE_EQ(unit->GetDenominator(), targetUnit->GetDenominator());
    EXPECT_STRCASEEQ(ECTestFixture::GetUnitsSchema()->GetPhenomenonCP("LENGTH")->GetName().c_str(), targetUnit->GetPhenomenon()->GetName().c_str());
    EXPECT_EQ(unit->HasUnitSystem(), targetUnit->HasUnitSystem());
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopyStandardUnitsSchema)
    {
    ECTestFixture::GetUnitsSchema()->CopySchema(m_targetSchema);

    for (const auto sourcePhen : ECTestFixture::GetUnitsSchema()->GetPhenomena())
        {
        PhenomenonCP targetPhen = m_targetSchema->GetPhenomenonCP(sourcePhen->GetName().c_str());
        EXPECT_NE(nullptr, targetPhen) << sourcePhen->GetName().c_str();
        EXPECT_TRUE(Units::Phenomenon::AreEqual(targetPhen, sourcePhen)) << sourcePhen->GetName().c_str();
        EXPECT_EQ(sourcePhen->GetUnits().size(), targetPhen->GetUnits().size()) << sourcePhen->GetName().c_str();
        }
    
    for (const auto sourceSystem : ECTestFixture::GetUnitsSchema()->GetUnitSystems())
        {
        UnitSystemCP targetSystem = m_targetSchema->GetUnitSystemCP(sourceSystem->GetName().c_str());
        EXPECT_NE(nullptr, targetSystem) << sourceSystem->GetName();
        EXPECT_STREQ(sourceSystem->GetDisplayLabel().c_str(), targetSystem->GetDisplayLabel().c_str());
        }

    for (const auto sourceUnit : ECTestFixture::GetUnitsSchema()->GetUnits())
        {
        ECUnitCP targetUnit = m_targetSchema->GetUnitCP(sourceUnit->GetName().c_str());
        EXPECT_NE(nullptr, targetUnit) << sourceUnit->GetName().c_str();
        EXPECT_TRUE(Units::Unit::AreEqual(targetUnit, sourceUnit)) << sourceUnit->GetName();
        EXPECT_TRUE(Units::Phenomenon::AreEqual(sourceUnit->GetPhenomenon(), targetUnit->GetPhenomenon())) << sourceUnit->GetName();
        }
    PhenomenonCP length = m_targetSchema->GetPhenomenonCP("LENGTH");
    ECUnitCP m = m_targetSchema->GetUnitCP("M");
    ECUnitCP pi = m_targetSchema->GetConstantCP("PI");

    EXPECT_TRUE(Units::Phenomenon::AreEqual(length, ECTestFixture::GetUnitsSchema()->GetPhenomenonCP("LENGTH")));
    EXPECT_TRUE(Units::Unit::AreEqual(m, ECTestFixture::GetUnitsSchema()->GetUnitCP("M")));
    EXPECT_TRUE(Units::Unit::AreEqual(pi, ECTestFixture::GetUnitsSchema()->GetConstantCP("PI")));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaCopyTest, SuccessfullyCopiesSchemaWithCustomAttributesInSchemaAndClassesAndProperties)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECCustomAttributeClass typeName="aCustomClass" appliesTo="Any"/>
            <ECEntityClass typeName="aEntityClass">
                <ECCustomAttributes>
                    <bCustomClass xmlns="testSchema.01.00.00"/>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="bEntityClass">
                <ECCustomAttributes>
                    <aCustomClass xmlns="testSchema.01.00.00"/>
                </ECCustomAttributes>
                <ECProperty propertyName="bProperty" typeName="aEntityClass">
                    <ECCustomAttributes>
                        <cCustomClass xmlns="testSchema.01.00.00"/>
                    </ECCustomAttributes>
                </ECProperty>
            </ECEntityClass>
            <ECCustomAttributeClass typeName="bCustomClass" appliesTo="Any"/>
            <ECCustomAttributeClass typeName="cCustomClass" appliesTo="Any"/>
            <ECCustomAttributes>
                <aCustomClass xmlns="testSchema.01.00.00"/>
                <cCustomClass xmlns="testSchema.01.00.00"/>
            </ECCustomAttributes>
        </ECSchema>
        )";

    ECSchemaPtr originalSchema;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(originalSchema, schemaString, *schemaContext));

    ASSERT_TRUE(originalSchema->GetClassCP("aEntityClass")->GetCustomAttribute("testSchema", "bCustomClass").IsValid());
    ASSERT_TRUE(originalSchema->GetClassCP("bEntityClass")->GetCustomAttribute("testSchema", "aCustomClass").IsValid());
    ASSERT_TRUE(originalSchema->GetClassCP("bEntityClass")->GetPropertyP("bProperty")->GetCustomAttribute("testSchema", "cCustomClass").IsValid());
    ASSERT_TRUE(originalSchema->GetCustomAttribute("testSchema", "aCustomClass").IsValid());
    ASSERT_TRUE(originalSchema->GetCustomAttribute("testSchema", "cCustomClass").IsValid());

    ECSchemaPtr copiedSchema;
    EC_ASSERT_SUCCESS(originalSchema->CopySchema(copiedSchema));

    EXPECT_TRUE(copiedSchema->GetClassCP("aEntityClass")->GetCustomAttribute("testSchema", "bCustomClass").IsValid());
    EXPECT_TRUE(copiedSchema->GetClassCP("bEntityClass")->GetCustomAttribute("testSchema", "aCustomClass").IsValid());
    EXPECT_TRUE(copiedSchema->GetClassCP("bEntityClass")->GetPropertyP("bProperty")->GetCustomAttribute("testSchema", "cCustomClass").IsValid());
    EXPECT_TRUE(copiedSchema->GetCustomAttribute("testSchema", "aCustomClass").IsValid());
    EXPECT_TRUE(copiedSchema->GetCustomAttribute("testSchema", "cCustomClass").IsValid());

    EXPECT_EQ(copiedSchema.get(), &(copiedSchema->GetClassCP("aEntityClass")->GetCustomAttribute("testSchema", "bCustomClass")->GetClass().GetSchema()));
    EXPECT_EQ(copiedSchema.get(), &(copiedSchema->GetClassCP("bEntityClass")->GetCustomAttribute("testSchema", "aCustomClass")->GetClass().GetSchema()));
    EXPECT_EQ(copiedSchema.get(), &(copiedSchema->GetClassCP("bEntityClass")->GetPropertyP("bProperty")->GetCustomAttribute("testSchema", "cCustomClass")->GetClass().GetSchema()));
    EXPECT_EQ(copiedSchema.get(), &(copiedSchema->GetCustomAttribute("testSchema", "aCustomClass")->GetClass().GetSchema()));
    EXPECT_EQ(copiedSchema.get(), &(copiedSchema->GetCustomAttribute("testSchema", "cCustomClass")->GetClass().GetSchema()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaCopyTest, OriginalAndCopiedSchemasSerializedXMLValuesMatches)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECCustomAttributeClass typeName="aCustomClass" appliesTo="Any"/>
            <ECCustomAttributeClass typeName="bCustomClass" appliesTo="Any"/>
            <ECCustomAttributeClass typeName="cCustomClass" appliesTo="Any"/>
            <ECCustomAttributeClass typeName="dCustomClass" appliesTo="Any"/>
            <ECCustomAttributes>
                <aCustomClass xmlns="testSchema.01.00.00"/>
                <bCustomClass xmlns="testSchema.01.00.00"/>
                <cCustomClass xmlns="testSchema.01.00.00"/>
                <dCustomClass xmlns="testSchema.01.00.00"/>
            </ECCustomAttributes>
        </ECSchema>
        )";

    ECSchemaPtr originalSchema;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(originalSchema, schemaString, *schemaContext));

    ECSchemaPtr copiedSchema, copiedSchema2;
    EC_ASSERT_SUCCESS(originalSchema->CopySchema(copiedSchema));
    EC_ASSERT_SUCCESS(copiedSchema->CopySchema(copiedSchema2));

    Utf8String originalSchemaXml;
    ASSERT_EQ(SchemaWriteStatus::Success, originalSchema->WriteToXmlString(originalSchemaXml));

    Utf8String copiedSchemaXml;
    ASSERT_EQ(SchemaWriteStatus::Success, copiedSchema->WriteToXmlString(copiedSchemaXml));

    Utf8String copiedSchema2Xml;
    ASSERT_EQ(SchemaWriteStatus::Success, copiedSchema2->WriteToXmlString(copiedSchema2Xml));

    EXPECT_STREQ(originalSchemaXml.c_str(), copiedSchemaXml.c_str());
    EXPECT_STREQ(copiedSchemaXml.c_str(), copiedSchema2Xml.c_str());
    EXPECT_STREQ(originalSchemaXml.c_str(), copiedSchema2Xml.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaCopyTest, SuccessfullCopiesKindOfQuantityPropertyWithPersistenceUnit)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="1.0.0" alias="u"/>
            <ECSchemaReference name="Formats" version="1.0.0" alias="f"/>
            <Unit typeName="TestUnit" phenomenon="u:LENGTH" unitSystem="u:SI" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
            <KindOfQuantity typeName="TestKindOfQuantity" persistenceUnit="TestUnit" relativeError=".5"/>
            <ECEntityClass typeName="TestClass">
                <ECProperty propertyName="TestProperty" typeName="double" kindOfQuantity="TestKindOfQuantity"/>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr originalSchema;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(originalSchema, schemaString, *schemaContext));
    
    ASSERT_NE(nullptr, originalSchema->GetUnitCP("TestUnit"));
    ASSERT_NE(nullptr, originalSchema->GetKindOfQuantityCP("TestKindOfQuantity"));

    ASSERT_EQ(originalSchema.get(), &(originalSchema->GetUnitCP("TestUnit")->GetSchema()));
    ASSERT_EQ(originalSchema.get(), &(originalSchema->GetKindOfQuantityCP("TestKindOfQuantity")->GetSchema()));
    ASSERT_EQ(originalSchema->GetUnitCP("TestUnit"), originalSchema->GetKindOfQuantityCP("TestKindOfQuantity")->GetPersistenceUnit());

    ECSchemaPtr copiedSchema;
    EC_ASSERT_SUCCESS(originalSchema->CopySchema(copiedSchema));

    EXPECT_NE(nullptr, copiedSchema->GetUnitCP("TestUnit"));
    EXPECT_NE(nullptr, copiedSchema->GetKindOfQuantityCP("TestKindOfQuantity"));

    EXPECT_EQ(copiedSchema.get(), &(copiedSchema->GetUnitCP("TestUnit")->GetSchema()));
    EXPECT_EQ(copiedSchema.get(), &(copiedSchema->GetKindOfQuantityCP("TestKindOfQuantity")->GetSchema()));
    EXPECT_EQ(copiedSchema->GetUnitCP("TestUnit"), copiedSchema->GetKindOfQuantityCP("TestKindOfQuantity")->GetPersistenceUnit());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaCopyTest, CopyCustomAttributeEC2)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" nameSpacePrefix="ts" version="01.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="CustomClass" isCustomAttributeClass="True"/>
            <ECCustomAttributes>
                <CustomClass xmlns="testSchema.01.02"/>
            </ECCustomAttributes>
        </ECSchema>
        )";

    ECSchemaPtr originalSchema;
    auto&& schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(originalSchema, schemaString, *schemaContext));
    
    ECSchemaPtr copySchema;
    EC_EXPECT_SUCCESS(originalSchema->CopySchema(copySchema));

    EXPECT_TRUE(copySchema->GetCustomAttribute("testSchema", "CustomClass").IsValid());
    EXPECT_STREQ("testSchema.01.02", copySchema->GetCustomAttribute("testSchema", "CustomClass")->GetClass().GetSchema().GetLegacyFullSchemaName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaCopyTest, CopyCustomAttributeEC3)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.02.03" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECCustomAttributeClass typeName="CustomClass" appliesTo="Any"/>
            <ECCustomAttributes>
                <CustomClass xmlns="testSchema.01.02.03"/>
            </ECCustomAttributes>
        </ECSchema>
        )";

    ECSchemaPtr originalSchema;
    auto&& schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(originalSchema, schemaString, *schemaContext));

    ECSchemaPtr copySchema;
    EC_EXPECT_SUCCESS(originalSchema->CopySchema(copySchema));

    EXPECT_TRUE(copySchema->GetCustomAttribute("testSchema", "CustomClass").IsValid());
    EXPECT_STREQ("testSchema.01.02.03", copySchema->GetCustomAttribute("testSchema", "CustomClass")->GetClass().GetSchema().GetFullSchemaName().c_str());
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaCopyTest, RoundtripCopiedEC2SchemaDropsMinMaxValue)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" nameSpacePrefix="ts" version="01.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" isDomainClass="true">
                <ECProperty propertyName="TestProperty" typeName="double" MinimumValue="3.0" MaximumValue="42"/>
            </ECClass>
        </ECSchema>
        )";

    ECSchemaPtr originalSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(originalSchema, schemaString, *ECSchemaReadContext::CreateContext()));

    ECSchemaPtr copiedSchema;
    EC_ASSERT_SUCCESS(originalSchema->CopySchema(copiedSchema));

    ASSERT_NE(nullptr, copiedSchema->GetClassCP("TestClass"));
    ASSERT_NE(nullptr, copiedSchema->GetClassCP("TestClass")->GetPropertyP("TestProperty"));

    ECValue minValue;
    ECValue maxValue;
    EC_ASSERT_SUCCESS(copiedSchema->GetClassCP("TestClass")->GetPropertyP("TestProperty")->GetMinimumValue(minValue));
    EC_ASSERT_SUCCESS(copiedSchema->GetClassCP("TestClass")->GetPropertyP("TestProperty")->GetMaximumValue(maxValue));
    ASSERT_FALSE(minValue.IsNull());
    ASSERT_FALSE(maxValue.IsNull());
    ASSERT_EQ( 3, minValue.GetDouble());
    ASSERT_EQ(42, maxValue.GetDouble());

    Utf8String serializedSchema;
    ASSERT_EQ(SchemaWriteStatus::Success, copiedSchema->WriteToXmlString(serializedSchema, ECVersion::V2_0));

    ECSchemaPtr roundTrippedSchema;
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(roundTrippedSchema, serializedSchema.c_str(), *ECSchemaReadContext::CreateContext()));

    EXPECT_NE(nullptr, roundTrippedSchema->GetClassCP("TestClass"));
    EXPECT_NE(nullptr, roundTrippedSchema->GetClassCP("TestClass")->GetPropertyP("TestProperty"));

    EXPECT_FALSE(roundTrippedSchema->GetClassCP("TestClass")->GetPropertyP("TestProperty")->IsMinimumValueDefined());
    EXPECT_FALSE(roundTrippedSchema->GetClassCP("TestClass")->GetPropertyP("TestProperty")->IsMaximumValueDefined());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaCopyTest, TestCopySchemaNotPreserveElementOrder)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->SetPreserveElementOrder(true);

    Utf8CP schemaXML = R"(
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.0">
            <ECEntityClass typeName="GHI" description="Project ECClass" displayLabel="Class GHI" />
            <ECEntityClass typeName="ABC" description="Project ECClass" displayLabel="Class ABC" />
            <ECEnumeration typeName="DEF" displayLabel="Enumeration DEF" backingTypeName="int" />
        </ECSchema>
        )";
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext));
   
    ECSchemaPtr copySchema;
    EC_EXPECT_SUCCESS(schema->CopySchema(copySchema));

    WString ecSchemaXmlString;
    ASSERT_EQ(SchemaWriteStatus::Success, copySchema->WriteToXmlString(ecSchemaXmlString, ECVersion::V3_0));

    size_t stringByteCount = ecSchemaXmlString.length() * sizeof(Utf8Char);
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, ecSchemaXmlString.c_str(), stringByteCount);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    // Enumerations(DEF) are serialized first, then classes(ABC, GHI)
    bvector<Utf8String> typeNames = {"DEF", "ABC", "GHI"};
    ValidateElementOrder(typeNames, xmlDom.get()->GetRootElement());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaCopyTest, TestCopySchemaNotPreserveOrderWithBaseClassAndRelationships)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->SetPreserveElementOrder(true);
    
    Utf8CP schemaXML = R"*(
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.0">
            <ECEntityClass typeName="GHI" description="Project ECClass" displayLabel="Class GHI" />
            <ECEntityClass typeName="ABC" description="Project ECClass" displayLabel="Class ABC">
                <BaseClass>MNO</BaseClass>
            </ECEntityClass>
            <ECRelationshipClass typeName="DEF" isDomainClass="True" strength="referencing" strengthDirection="forward">
              <Source cardinality="(0, 1)" polymorphic="True">
                  <Class class="MNO" />
              </Source>
              <Target cardinality="(0, 1)" polymorphic="True">
                  <Class class="JKL">
                      <Key>
                          <Property name="Property1" />
                          <Property name="Property2" />
                      </Key>
                  </Class>
              </Target>
            </ECRelationshipClass>
            <ECEntityClass typeName="MNO" description="Project ECClass" displayLabel="Class MNO" />
            <ECEntityClass typeName="JKL" description="Project ECClass" displayLabel="Class JKL">
                <ECProperty propertyName="Property1" typeName="string" />
                <ECProperty propertyName="Property2" typeName="string" />
            </ECEntityClass>
            <ECEnumeration typeName="PQR" displayLabel="Enumeration PQR" backingTypeName="int" />
        </ECSchema>
        )*";
    ECSchemaPtr schema;
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext));

    ECSchemaPtr copySchema;
    EC_EXPECT_SUCCESS(schema->CopySchema(copySchema));

    WString ecSchemaXmlString;
    EXPECT_EQ(SchemaWriteStatus::Success, copySchema->WriteToXmlString(ecSchemaXmlString, ECVersion::V3_0));

    size_t stringByteCount = ecSchemaXmlString.length() * sizeof(Utf8Char);
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, ecSchemaXmlString.c_str(), stringByteCount);
    EXPECT_EQ(BEXML_Success, xmlStatus);

    // First Enumeration(PQR), then classes alphabetically(ABC, DEF, GHI). As MNO is the base class of ABC and
    // JKL has a constraint in DEF, those two classes are written before the class they depend in.
    bvector<Utf8String> typeNames = {"PQR", "MNO", "ABC", "JKL", "DEF", "GHI"};
    ValidateElementOrder(typeNames, xmlDom.get()->GetRootElement());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaCopyTest, RoundtripCopiedSchemaMinMaxLength)
    {
    Utf8CP schemaXml = R"(
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="TestClass" isDomainClass="true">
               <ECProperty propertyName="TestProperty" typeName="string" minimumLength="4294967295" maximumLength="4294967295"/>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr originalSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(originalSchema, schemaXml, *ECSchemaReadContext::CreateContext()));

    ECSchemaPtr copiedSchema;
    EC_ASSERT_SUCCESS(originalSchema->CopySchema(copiedSchema));

    ASSERT_NE(nullptr, copiedSchema->GetClassCP("TestClass"));
    ASSERT_NE(nullptr, copiedSchema->GetClassCP("TestClass")->GetPropertyP("TestProperty"));


    ASSERT_EQ(UINT32_MAX, copiedSchema->GetClassCP("TestClass")->GetPropertyP("TestProperty")->GetMinimumLength());
    ASSERT_EQ(UINT32_MAX, copiedSchema->GetClassCP("TestClass")->GetPropertyP("TestProperty")->GetMaximumLength());

    Utf8String serializedSchema;
    ASSERT_EQ(SchemaWriteStatus::Success, copiedSchema->WriteToXmlString(serializedSchema, ECVersion::V3_0));

    ECSchemaPtr roundTrippedSchema;
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(roundTrippedSchema, serializedSchema.c_str(), *ECSchemaReadContext::CreateContext()));

    EXPECT_NE(nullptr, roundTrippedSchema->GetClassCP("TestClass"));
    EXPECT_NE(nullptr, roundTrippedSchema->GetClassCP("TestClass")->GetPropertyP("TestProperty"));

    EXPECT_TRUE(roundTrippedSchema->GetClassCP("TestClass")->GetPropertyP("TestProperty")->IsMinimumLengthDefined());
    EXPECT_TRUE(roundTrippedSchema->GetClassCP("TestClass")->GetPropertyP("TestProperty")->IsMaximumLengthDefined());
    }


END_BENTLEY_ECN_TEST_NAMESPACE
