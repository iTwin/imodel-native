/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include "BeXml/BeXml.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct CopyTestFixture : ECTestFixture 
{
ECSchemaPtr m_sourceSchema;
ECSchemaPtr m_targetSchema;

public:
    void TearDown() override { m_targetSchema = nullptr; m_sourceSchema = nullptr; }
    void CreateTestSchema() {EC_ASSERT_SUCCESS(ECSchema::CreateSchema(m_sourceSchema, "TestSchema", "ts", 1, 0, 0));}

    template<typename T>
    static void ValidateNameDescriptionAndDisplayLabel(T const & sourceItem, T const& targetItem)
        {
        EXPECT_STREQ(sourceItem.GetName().c_str(), targetItem.GetName().c_str()) <<
            "The name '" << sourceItem.GetName().c_str() << "' does not match the name '" << targetItem.GetName().c_str();
        EXPECT_NE(sourceItem.GetName().c_str(), targetItem.GetName().c_str()) <<
            "The description of '" << sourceItem.GetName().c_str() << "' is the same in memory object as the name of the target item '" << targetItem.GetName().c_str() << "'. It should be a copy.";

        EXPECT_STREQ(sourceItem.GetInvariantDescription().c_str(), targetItem.GetInvariantDescription().c_str()) <<
            "The description '" << sourceItem.GetInvariantDescription().c_str() << "' does not match the copied description '" << targetItem.GetInvariantDescription().c_str();
        EXPECT_NE(sourceItem.GetInvariantDescription().c_str(), targetItem.GetInvariantDescription().c_str()) <<
            "The description of '" << sourceItem.GetName().c_str() << "' is the same in memory object as the description of the target item '" << targetItem.GetName().c_str() << "'. It should be a copy.";

        EXPECT_STREQ(sourceItem.GetInvariantDisplayLabel().c_str(), targetItem.GetInvariantDisplayLabel().c_str()) << 
            "The display label '" << sourceItem.GetInvariantDisplayLabel().c_str() << "' does not match the copied display label '" << targetItem.GetInvariantDisplayLabel().c_str();
        EXPECT_NE(sourceItem.GetInvariantDisplayLabel().c_str(), targetItem.GetInvariantDisplayLabel().c_str()) <<
            "The display label of '" << sourceItem.GetName().c_str() << "' is the same in memory object as the display label of the target item '" << targetItem.GetName().c_str() << "'. It should be a copy.";
        }
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

        void ValidateElementOrder(bvector<Utf8String> expectedTypeNames, BeXmlNodeP root);
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2017
//---------------+---------------+---------------+---------------+---------------+-------
void SchemaCopyTest::CopySchema(ECSchemaPtr& targetSchema)
    {
    targetSchema = nullptr;
    EC_ASSERT_SUCCESS(m_sourceSchema->CopySchema(targetSchema));
    EXPECT_TRUE(targetSchema.IsValid());
    EXPECT_NE(m_sourceSchema, targetSchema);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Gintaras.Volkvicius    01/2019
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
    EXPECT_NE(m_sourceSchema->GetAlias().c_str(), m_targetSchema->GetAlias().c_str());

    ValidateNameDescriptionAndDisplayLabel(*m_sourceSchema.get(), *m_targetSchema.get());

    EXPECT_EQ(3, m_targetSchema->GetVersionRead());
    EXPECT_EQ(10, m_targetSchema->GetVersionWrite());
    EXPECT_EQ(5, m_targetSchema->GetVersionMinor());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Kyle.Abramowitz    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopiedSchemaDoesNotPersistOriginalSchemaXMLVersion)
    {
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(m_sourceSchema, "TestSchema", "ts", 1, 0, 0, ECVersion::Latest));
    EXPECT_EQ(3, m_sourceSchema->GetOriginalECXmlVersionMajor());
    EXPECT_EQ(1, m_sourceSchema->GetOriginalECXmlVersionMinor());

    CopySchema();
    EXPECT_EQ(3, m_targetSchema->GetOriginalECXmlVersionMajor());
    EXPECT_EQ(1, m_targetSchema->GetOriginalECXmlVersionMinor());

    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(m_sourceSchema, "TestSchema", "ts", 1, 0, 0, ECVersion::V2_0));
    EXPECT_EQ(3, m_sourceSchema->GetOriginalECXmlVersionMajor());
    EXPECT_EQ(1, m_sourceSchema->GetOriginalECXmlVersionMinor());

    CopySchema();
    EXPECT_EQ(3, m_targetSchema->GetOriginalECXmlVersionMajor());
    EXPECT_EQ(1, m_targetSchema->GetOriginalECXmlVersionMinor());

    m_sourceSchema->SetOriginalECXmlVersion(2, 0);
    EXPECT_EQ(2, m_sourceSchema->GetOriginalECXmlVersionMajor());
    EXPECT_EQ(0, m_sourceSchema->GetOriginalECXmlVersionMinor());

    CopySchema();
    EXPECT_EQ(3, m_targetSchema->GetOriginalECXmlVersionMajor());
    EXPECT_EQ(1, m_targetSchema->GetOriginalECXmlVersionMinor());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2013
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
    EXPECT_TRUE (out.Equals(ECValue ("test"))) << "Expect: " << "test" << " Actual: " << out.ToString().c_str();
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
// @bsimethod                                           Colin.Kerr                  02/2018
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
* @bsimethod                                Robert.Schili                      11/2015
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
// @bsimethod                                                    Caleb.Shafer    06/2017
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
// @bsimethod                                                    Caleb.Shafer    08/2017
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
// @bsimethod                                                    Caleb.Shafer    08/2017
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
// @bsimethod                                                    Caleb.Shafer    08/2017
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
// @bsimethod                          Kyle.Abramowitz                           03/2018
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
// @bsimethod                          Kyle.Abramowitz                           03/2018
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
// @bsimethod                          Kyle.Abramowitz                           03/2018
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
// @bsimethod                                                Gintaras.Volkvicius 11/2018
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
// @bsimethod                          Kyle.Abramowitz                           03/2018
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
// @bsimethod                          Kyle.Abramowitz                           03/2018
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
// @bsimethod                          Kyle.Abramowitz                           03/2018
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
// @bsimethod                                                    Caleb.Shafer    08/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopyKindOfQuantity)
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

    EXPECT_EQ(0, m_targetSchema->GetUnitCount());
    EXPECT_EQ(0, m_targetSchema->GetFormatCount());
    EXPECT_EQ(1, m_targetSchema->GetKindOfQuantityCount());

    KindOfQuantityCP targetKoq = m_targetSchema->GetKindOfQuantityCP("TestKoQ");
    ASSERT_TRUE(nullptr != targetKoq);
    EXPECT_STREQ("Test KoQ", targetKoq->GetDisplayLabel().c_str());
    EXPECT_STREQ("Test Description", targetKoq->GetDescription().c_str());
    EXPECT_EQ(10e-3, targetKoq->GetRelativeError());

    EXPECT_STREQ("M", targetKoq->GetPersistenceUnit()->GetName().c_str());
    
    const auto& formats = targetKoq->GetPresentationFormats();
    EXPECT_EQ(2, formats.size());

    EXPECT_STREQ("AmerFI", targetKoq->GetDefaultPresentationFormat()->GetName().c_str());
    EXPECT_EQ(ECTestFixture::GetFormatsSchema()->GetFormatCP("AmerFI"), targetKoq->GetDefaultPresentationFormat()->GetParentFormat());

    EXPECT_STREQ("DefaultRealU[u:M]", formats.at(1).GetName().c_str());
    EXPECT_EQ(ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultRealU"), formats.at(1).GetParentFormat());
    EXPECT_NE(nullptr, formats.at(1).GetCompositeMajorUnit());
    EXPECT_EQ(ECTestFixture::GetUnitsSchema()->GetUnitCP("M"), formats.at(1).GetCompositeMajorUnit());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopyKindOfQuantity_NoPresentationFormats)
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
    ValidateNameDescriptionAndDisplayLabel(*koq, *targetKoq);
    EXPECT_STREQ("M", targetKoq->GetPersistenceUnit()->GetName().c_str());
    EXPECT_FALSE(targetKoq->HasPresentationFormats());
    EXPECT_EQ(10e-3, targetKoq->GetRelativeError());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopyKindOfQuantity_PersistenceUnitDefinedInSchema)
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
    EXPECT_NE(targetKoq, koq);
    ValidateNameDescriptionAndDisplayLabel(*koq, *targetKoq);
    EXPECT_FALSE(targetKoq->HasPresentationFormats());
    EXPECT_EQ(10e-3, targetKoq->GetRelativeError());

    ECUnitCP targetSmoot = m_targetSchema->GetUnitCP("SMOOT");
    ASSERT_TRUE(nullptr != targetSmoot);
    EXPECT_STREQ(koq->GetPersistenceUnit()->GetName().c_str(), targetKoq->GetPersistenceUnit()->GetName().c_str());
    EXPECT_NE(koq->GetPersistenceUnit(), targetSmoot);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopyKindOfQuantity_PresentationFormatDefinedInSchema)
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

//---------------------------------------------------------------------------------------
// @bsimethod                                            Gintaras.Volkvicius    12/2018
//---------------------------------------------------------------------------------------
TEST_F(SchemaCopyTest, CopyKindOfQuantityIncludingReferences)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Unit typeName="Unit" phenomenon="u:LENGTH" unitSystem="u:SI" definition="M"/>
            <KindOfQuantity typeName="KindOfQuantity" persistenceUnit="Unit" relativeError=".5"/>
        </ECSchema>
        )";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ASSERT_NE(nullptr, schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity"));
    ASSERT_NE(nullptr, schemaCopyFrom->GetUnitCP("Unit"));
    ASSERT_EQ(schemaCopyFrom->GetUnitCP("Unit"), schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity")->GetPersistenceUnit());

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 0, 0);
    SchemaKey key("Units", 1, 0, 0);
    schemaCopyTo->AddReferencedSchema(*schemaContext->LocateSchema(key, SchemaMatchType::LatestReadCompatible));

    KindOfQuantityP koq;
    EC_EXPECT_SUCCESS(schemaCopyTo->CopyKindOfQuantity(koq, *schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity"), true));

    EXPECT_NE(nullptr, schemaCopyTo->GetKindOfQuantityCP("KindOfQuantity"));
    EXPECT_NE(schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity"), schemaCopyTo->GetKindOfQuantityCP("KindOfQuantity"));
    EXPECT_NE(nullptr, schemaCopyTo->GetUnitCP("Unit"));
    EXPECT_NE(schemaCopyFrom->GetUnitCP("Unit"), schemaCopyTo->GetUnitCP("Unit"));

    EXPECT_EQ(schemaCopyTo->GetUnitCP("Unit"), schemaCopyTo->GetKindOfQuantityCP("KindOfQuantity")->GetPersistenceUnit());

    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                            Gintaras.Volkvicius    12/2018
//---------------------------------------------------------------------------------------
TEST_F(SchemaCopyTest, CopyKindOfQuantityWithReferencedPresentationUnitSchema)
    {
    Utf8CP referenceSchemaString = R"(
        <ECSchema schemaName="referenceSchema" alias="rs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Unit typeName="Unit" phenomenon="u:LENGTH" unitSystem="u:SI" definition="M"/>
        </ECSchema>
        )";

    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="referenceSchema" version="01.00.00" alias="rs"/>
            <KindOfQuantity typeName="KindOfQuantity" persistenceUnit="rs:Unit" relativeError=".5"/>
        </ECSchema>
        )";

    ECSchemaPtr referenceSchema;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(referenceSchema, referenceSchemaString, *schemaContext));

    ASSERT_NE(nullptr, referenceSchema->GetUnitCP("Unit"));

    ECSchemaPtr schemaCopyFrom;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ASSERT_NE(nullptr, schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity"));
    ASSERT_EQ(referenceSchema->GetUnitCP("Unit"), schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity")->GetPersistenceUnit());

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 0, 0);
    SchemaKey key("Units", 1, 0, 0);
    schemaCopyTo->AddReferencedSchema(*schemaContext->LocateSchema(key, SchemaMatchType::LatestReadCompatible));

    KindOfQuantityP koq;
    EC_EXPECT_SUCCESS(schemaCopyTo->CopyKindOfQuantity(koq, *schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity"), true));

    EXPECT_NE(nullptr, schemaCopyTo->GetKindOfQuantityCP("KindOfQuantity"));
    EXPECT_NE(schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity"), schemaCopyTo->GetKindOfQuantityCP("KindOfQuantity"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetUnitCP("Unit"));

    EXPECT_EQ(referenceSchema->GetUnitCP("Unit"), schemaCopyTo->GetKindOfQuantityCP("KindOfQuantity")->GetPersistenceUnit());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *referenceSchema));
    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                            Gintaras.Volkvicius    12/2018
//---------------------------------------------------------------------------------------
TEST_F(SchemaCopyTest, CopyKindOfQuantityWithReferencedSourceSchema)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Unit typeName="Unit" phenomenon="u:LENGTH" unitSystem="u:SI" definition="M"/>
            <KindOfQuantity typeName="KindOfQuantity" persistenceUnit="Unit" relativeError=".5"/>
        </ECSchema>
        )";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ASSERT_NE(nullptr, schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity"));
    ASSERT_NE(nullptr, schemaCopyFrom->GetUnitCP("Unit"));
    ASSERT_EQ(schemaCopyFrom->GetUnitCP("Unit"), schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity")->GetPersistenceUnit());

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 0, 0);
    SchemaKey key("Units", 1, 0, 0);
    schemaCopyTo->AddReferencedSchema(*schemaContext->LocateSchema(key, SchemaMatchType::LatestReadCompatible));

    KindOfQuantityP koq;
    EC_EXPECT_SUCCESS(schemaCopyTo->CopyKindOfQuantity(koq, *schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity"), false));

    EXPECT_NE(nullptr, schemaCopyTo->GetKindOfQuantityCP("KindOfQuantity"));
    EXPECT_NE(schemaCopyFrom->GetKindOfQuantityCP("KindOfQuantity"), schemaCopyTo->GetKindOfQuantityCP("KindOfQuantity"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetUnitCP("Unit"));

    EXPECT_EQ(schemaCopyFrom->GetUnitCP("Unit"), schemaCopyTo->GetKindOfQuantityCP("KindOfQuantity")->GetPersistenceUnit());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
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

//---------------------------------------------------------------------------------------
// @bsimethod                                              Gintaras.Volkvicius   11/2018
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
    ASSERT_TRUE(originalSchema->GetPrimaryCustomAttribute("testSchema", "aCustomClass").IsValid());
    ASSERT_TRUE(originalSchema->GetPrimaryCustomAttribute("testSchema", "cCustomClass").IsValid());

    ECSchemaPtr copiedSchema;
    EC_ASSERT_SUCCESS(originalSchema->CopySchema(copiedSchema));

    EXPECT_TRUE(copiedSchema->GetClassCP("aEntityClass")->GetCustomAttribute("testSchema", "bCustomClass").IsValid());
    EXPECT_TRUE(copiedSchema->GetClassCP("bEntityClass")->GetCustomAttribute("testSchema", "aCustomClass").IsValid());
    EXPECT_TRUE(copiedSchema->GetClassCP("bEntityClass")->GetPropertyP("bProperty")->GetCustomAttribute("testSchema", "cCustomClass").IsValid());
    EXPECT_TRUE(copiedSchema->GetPrimaryCustomAttribute("testSchema", "aCustomClass").IsValid());
    EXPECT_TRUE(copiedSchema->GetPrimaryCustomAttribute("testSchema", "cCustomClass").IsValid());

    EXPECT_EQ(copiedSchema.get(), &(copiedSchema->GetClassCP("aEntityClass")->GetCustomAttribute("testSchema", "bCustomClass")->GetClass().GetSchema()));
    EXPECT_EQ(copiedSchema.get(), &(copiedSchema->GetClassCP("bEntityClass")->GetCustomAttribute("testSchema", "aCustomClass")->GetClass().GetSchema()));
    EXPECT_EQ(copiedSchema.get(), &(copiedSchema->GetClassCP("bEntityClass")->GetPropertyP("bProperty")->GetCustomAttribute("testSchema", "cCustomClass")->GetClass().GetSchema()));
    EXPECT_EQ(copiedSchema.get(), &(copiedSchema->GetPrimaryCustomAttribute("testSchema", "aCustomClass")->GetClass().GetSchema()));
    EXPECT_EQ(copiedSchema.get(), &(copiedSchema->GetPrimaryCustomAttribute("testSchema", "cCustomClass")->GetClass().GetSchema()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Gintaras.Volkvicius   11/2018
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
// @bsimethod                                              Gintaras.Volkvicius   11/2018
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
// @bsimethod                                              Gintaras.Volkvicius   01/2019
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
// @bsimethod                                              Gintaras.Volkvicius   01/2019
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
// @bsimethod                                              Gintaras.Volkvicius   01/2019
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
// @bsimethod                                             Gintaras.Volkvicius    01/2019
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
// @bsimethod                                             Gintaras.Volkvicius    01/2019
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
    EXPECT_NE(m_sourceClass, m_targetClass);
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

//---------------------------------------------------------------------------------------
// @bsimethod                             Gintaras.Volkvicius                   12/2018
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyCustomAttributesIncludingReferences)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECCustomAttributeClass typeName="CustomClass" appliesTo="Any"/>
            <ECEntityClass typeName="EntityClass">
                <ECCustomAttributes>
                    <CustomClass xmlns="testSchema.01.00.00"/>
                </ECCustomAttributes>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ASSERT_NE(nullptr, schemaCopyFrom->GetClassCP("EntityClass"));
    ASSERT_NE(nullptr, schemaCopyFrom->GetClassCP("CustomClass"));
    ASSERT_TRUE(schemaCopyFrom->GetClassCP("EntityClass")->GetPrimaryCustomAttribute("testSchema", "CustomClass").IsValid());
    ASSERT_EQ(schemaCopyFrom->GetClassCP("CustomClass"), &schemaCopyFrom->GetClassCP("EntityClass")->GetPrimaryCustomAttribute("testSchema", "CustomClass")->GetClass());

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 0, 0);

    ASSERT_EQ(nullptr, schemaCopyTo->GetClassCP("EntityClass"));
    ASSERT_EQ(nullptr, schemaCopyTo->GetClassCP("CustomClass"));

    ECClassP ecClass;
    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("EntityClass"), "EntityClass", true));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("EntityClass"));
    EXPECT_NE(schemaCopyFrom->GetClassCP("EntityClass"), schemaCopyTo->GetClassCP("EntityClass"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("CustomClass"));
    EXPECT_NE(schemaCopyFrom->GetClassCP("CustomClass"), schemaCopyTo->GetClassCP("CustomClass"));

    EXPECT_TRUE(schemaCopyTo->GetClassCP("EntityClass")->GetPrimaryCustomAttribute("testSchema", "CustomClass").IsValid());
    EXPECT_EQ(schemaCopyTo->GetClassCP("CustomClass"), &schemaCopyTo->GetClassCP("EntityClass")->GetPrimaryCustomAttribute("testSchema", "CustomClass")->GetClass());

    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Gintaras.Volkvicius                   12/2018
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyCustomAttributesWithReferencedCAClass)
    {
    Utf8CP referenceSchemaString = R"(
        <ECSchema schemaName="referenceSchema" alias="rs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECCustomAttributeClass typeName="CustomClass" appliesTo="Any"/>
        </ECSchema>
        )";

    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="referenceSchema" version="01.00.00" alias="rs"/>
            <ECEntityClass typeName="EntityClass">
                <ECCustomAttributes>
                    <CustomClass xmlns="referenceSchema.01.00.00"/>
                </ECCustomAttributes>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr referenceSchema;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(referenceSchema, referenceSchemaString, *schemaContext));

    ASSERT_NE(nullptr, referenceSchema->GetClassCP("CustomClass"));

    ECSchemaPtr schemaCopyFrom;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ASSERT_NE(nullptr, schemaCopyFrom->GetClassCP("EntityClass"));
    ASSERT_TRUE(schemaCopyFrom->GetClassCP("EntityClass")->GetPrimaryCustomAttribute("referenceSchema", "CustomClass").IsValid());
    ASSERT_EQ(referenceSchema->GetClassCP("CustomClass"), &schemaCopyFrom->GetClassCP("EntityClass")->GetPrimaryCustomAttribute("referenceSchema", "CustomClass")->GetClass());

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 0, 0);

    ASSERT_EQ(nullptr, schemaCopyTo->GetClassCP("EntityClass"));

    ECClassP ecClass;
    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("EntityClass"), "EntityClass", true));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("EntityClass"));
    EXPECT_NE(schemaCopyFrom->GetClassCP("EntityClass"), schemaCopyTo->GetClassCP("EntityClass"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetClassCP("CustomClass"));

    EXPECT_TRUE(schemaCopyTo->GetClassCP("EntityClass")->GetPrimaryCustomAttribute("referenceSchema", "CustomClass").IsValid());
    EXPECT_EQ(referenceSchema->GetClassCP("CustomClass"), &schemaCopyTo->GetClassCP("EntityClass")->GetPrimaryCustomAttribute("referenceSchema", "CustomClass")->GetClass());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *referenceSchema));
    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Gintaras.Volkvicius                   12/2018
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyCustomAttributesWithReferencedSourceSchema)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECCustomAttributeClass typeName="CustomClass" appliesTo="Any"/>
            <ECEntityClass typeName="EntityClass">
                <ECCustomAttributes>
                    <CustomClass xmlns="testSchema.01.00.00"/>
                </ECCustomAttributes>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ASSERT_NE(nullptr, schemaCopyFrom->GetClassCP("EntityClass"));
    ASSERT_NE(nullptr, schemaCopyFrom->GetClassCP("CustomClass"));
    ASSERT_TRUE(schemaCopyFrom->GetClassCP("EntityClass")->GetPrimaryCustomAttribute("testSchema", "CustomClass").IsValid());
    ASSERT_EQ(schemaCopyFrom->GetClassCP("CustomClass"), &schemaCopyFrom->GetClassCP("EntityClass")->GetPrimaryCustomAttribute("testSchema", "CustomClass")->GetClass());

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 0, 0);

    ASSERT_EQ(nullptr, schemaCopyTo->GetClassCP("EntityClass"));
    ASSERT_EQ(nullptr, schemaCopyTo->GetClassCP("CustomClass"));

    ECClassP ecClass;
    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("EntityClass"), "EntityClass", false));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("EntityClass"));
    EXPECT_NE(schemaCopyFrom->GetClassCP("EntityClass"), schemaCopyTo->GetClassCP("EntityClass"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetClassCP("CustomClass"));

    EXPECT_TRUE(schemaCopyTo->GetClassCP("EntityClass")->GetPrimaryCustomAttribute("testSchema", "CustomClass").IsValid());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Gintaras.Volkvicius                   01/2019
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyNavigationPropertyIncludingReferences)
    {
    Utf8CP schemaString = R"**(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECRelationshipClass typeName="HouseHasRooms" strengthDirection="Forward" modifier="None">
                <Source multiplicity="(0..1)" roleLabel="House" polymorphic="False">
                    <Class class="House"/>
                </Source>
                <Target multiplicity="(0..*)" roleLabel="Room" polymorphic="False">
                    <Class class="Room"/>
                </Target>
            </ECRelationshipClass>
            <ECEntityClass typeName="House"/>
            <ECEntityClass typeName="Room">
                <ECNavigationProperty propertyName="NavigationPropertyToHouse" relationshipName="HouseHasRooms" direction="Backward"/>
            </ECEntityClass>
        </ECSchema>
        )**";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 0, 0);

    ECClassP ecClass;
    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("Room"), "Room", true));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("Room"));
    EXPECT_NE(schemaCopyFrom->GetClassCP("Room"), schemaCopyTo->GetClassCP("Room"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("HouseHasRooms"));
    EXPECT_NE(schemaCopyFrom->GetClassCP("HouseHasRooms"), schemaCopyTo->GetClassCP("HouseHasRooms"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("House"));
    EXPECT_NE(schemaCopyFrom->GetClassCP("House"), schemaCopyTo->GetClassCP("House"));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("Room")->GetPropertyP("NavigationPropertyToHouse"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("Room")->GetPropertyP("NavigationPropertyToHouse")->GetAsNavigationProperty());
    EXPECT_EQ(schemaCopyTo->GetClassCP("HouseHasRooms")->GetRelationshipClassCP(), schemaCopyTo->GetClassCP("Room")->GetPropertyP("NavigationPropertyToHouse")->GetAsNavigationProperty()->GetRelationshipClass());

    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Gintaras.Volkvicius                   01/2019
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyNavigationPropertyOnlyInlcudingRelationshipClass)
    {
    Utf8CP referenceSchemaString = R"(
        <ECSchema schemaName="referenceSchema" alias="rs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="House"/>
        </ECSchema>
        )";

    Utf8CP schemaString = R"**(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="referenceSchema" version="01.00.00" alias="rs"/>
            <ECRelationshipClass typeName="HouseHasRooms" strengthDirection="Forward" modifier="None">
                <Source multiplicity="(0..1)" roleLabel="House" polymorphic="False">
                    <Class class="rs:House"/>
                </Source>
                <Target multiplicity="(0..*)" roleLabel="Room" polymorphic="False">
                    <Class class="Room"/>
                </Target>
            </ECRelationshipClass>
            <ECEntityClass typeName="Room">
                <ECNavigationProperty propertyName="NavigationPropertyToHouse" relationshipName="HouseHasRooms" direction="Backward"/>
            </ECEntityClass>
        </ECSchema>
        )**";

    ECSchemaPtr referenceSchema;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(referenceSchema, referenceSchemaString, *schemaContext));

    ASSERT_NE(nullptr, referenceSchema->GetClassCP("House"));

    ECSchemaPtr schemaCopyFrom;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 0, 0);
    schemaCopyTo->AddReferencedSchema(*referenceSchema);

    ECClassP ecClass;
    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("Room"), "Room", true));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("Room"));
    EXPECT_NE(schemaCopyFrom->GetClassCP("Room"), schemaCopyTo->GetClassCP("Room"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("HouseHasRooms"));
    EXPECT_NE(schemaCopyFrom->GetClassCP("HouseHasRooms"), schemaCopyTo->GetClassCP("HouseHasRooms"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetClassCP("House"));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("Room")->GetPropertyP("NavigationPropertyToHouse"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("Room")->GetPropertyP("NavigationPropertyToHouse")->GetAsNavigationProperty());
    EXPECT_EQ(schemaCopyTo->GetClassCP("HouseHasRooms")->GetRelationshipClassCP(), schemaCopyTo->GetClassCP("Room")->GetPropertyP("NavigationPropertyToHouse")->GetAsNavigationProperty()->GetRelationshipClass());
    ASSERT_EQ(referenceSchema->GetClassCP("House"), schemaCopyTo->GetClassCP("HouseHasRooms")->GetRelationshipClassCP()->GetSource().GetConstraintClasses()[0]);

    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                             Gintaras.Volkvicius                   01/2019
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyNavigationPropertyWithReferencedSourceSchema)
    {
    Utf8CP schemaString = R"**(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECRelationshipClass typeName="HouseHasRooms" strengthDirection="Forward" modifier="None">
                <Source multiplicity="(0..1)" roleLabel="House" polymorphic="False">
                    <Class class="House"/>
                </Source>
                <Target multiplicity="(0..*)" roleLabel="Room" polymorphic="False">
                    <Class class="Room"/>
                </Target>
            </ECRelationshipClass>
            <ECEntityClass typeName="House"/>
            <ECEntityClass typeName="Room">
                <ECNavigationProperty propertyName="NavigationPropertyToHouse" relationshipName="HouseHasRooms" direction="Backward"/>
            </ECEntityClass>
        </ECSchema>
        )**";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 0, 0);

    ECClassP ecClass;
    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("Room"), "Room", false));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("Room"));
    EXPECT_NE(schemaCopyFrom->GetClassCP("Room"), schemaCopyTo->GetClassCP("Room"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetClassCP("HouseHasRooms"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetClassCP("House"));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("Room")->GetPropertyP("NavigationPropertyToHouse"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("Room")->GetPropertyP("NavigationPropertyToHouse")->GetAsNavigationProperty());
    EXPECT_EQ(schemaCopyFrom->GetClassCP("HouseHasRooms")->GetRelationshipClassCP(), schemaCopyTo->GetClassCP("Room")->GetPropertyP("NavigationPropertyToHouse")->GetAsNavigationProperty()->GetRelationshipClass());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Gintaras.Volkvicius   01/2019
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyPropertyCopiesOriginalTypeNameInfo)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="TestClass">
                <ECProperty propertyName="TestProperty" typeName="notDouble"/>
                <ECArrayProperty propertyName="TestArrayProperty" typeName="notInt" minOccurs="0" maxOccurs="unbounded"/>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    Utf8String ecSchemaXml;
    SchemaWriteStatus writeStatus = schemaCopyFrom->WriteToXmlString(ecSchemaXml);
    ASSERT_EQ(SchemaWriteStatus::Success, writeStatus);

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 0, 0);

    ECClassP ecClass;
    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("TestClass"), "TestClass", false));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestProperty"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestArrayProperty"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestArrayProperty")->GetAsArrayProperty());

    writeStatus = schemaCopyTo->WriteToXmlString(ecSchemaXml);
    EXPECT_EQ(SchemaWriteStatus::Success, writeStatus);

    EXPECT_NE(Utf8String::npos, ecSchemaXml.find("typeName=\"notDouble\""));
    EXPECT_NE(Utf8String::npos, ecSchemaXml.find("typeName=\"notInt\""));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Gintaras.Volkvicius   01/2019
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyEnumerationPropertyReferencesOriginalSchema)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.02.03" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEnumeration typeName="TestEnum" backingTypeName="int"/>
            <ECEntityClass typeName="TestClass">
                <ECProperty propertyName="TestProperty" typeName="TestEnum"/>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 3, 4);

    ECClassP ecClass;
    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("TestClass"), "TestClass", false));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestProperty"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetEnumerationP("TestEnum"));
    EXPECT_EQ(schemaCopyFrom->GetEnumerationP("TestEnum"), schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestProperty")->GetAsPrimitivePropertyP()->GetEnumeration());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Gintaras.Volkvicius   01/2019
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyEnumerationPropertyReferencesReferenceSchema)
    {
    Utf8CP referenceSchemaString = R"(
        <ECSchema schemaName="referenceSchema" alias="rs" version="03.02.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEnumeration typeName="TestEnum" backingTypeName="int"/>
        </ECSchema>
        )";

    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.02.03" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="referenceSchema" version="03.02.01" alias="rs"/>
            <ECEntityClass typeName="TestClass">
                <ECProperty propertyName="TestProperty" typeName="rs:TestEnum"/>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr referenceSchema;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(referenceSchema, referenceSchemaString, *schemaContext));

    ECSchemaPtr schemaCopyFrom;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 3, 4);

    ECClassP ecClass;
    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("TestClass"), "TestClass", false));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestProperty"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetEnumerationP("TestEnum"));
    EXPECT_EQ(referenceSchema->GetEnumerationP("TestEnum"), schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestProperty")->GetAsPrimitivePropertyP()->GetEnumeration());

    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *referenceSchema));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Gintaras.Volkvicius   01/2019
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyEnumerationArrayPropertyReferencesOriginalSchema)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.02.03" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEnumeration typeName="TestEnum" backingTypeName="int"/>
            <ECEntityClass typeName="TestClass">
                <ECArrayProperty propertyName="TestArrayProperty" typeName="TestEnum"/>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 3, 4);

    ECClassP ecClass;
    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("TestClass"), "TestClass", false));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestArrayProperty"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetEnumerationP("TestEnum"));
    EXPECT_EQ(schemaCopyFrom->GetEnumerationP("TestEnum"), schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestArrayProperty")->GetAsPrimitiveArrayPropertyP()->GetEnumeration());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Gintaras.Volkvicius   01/2019
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyEnumerationArrayPropertyReferencesReferenceSchema)
    {
    Utf8CP referenceSchemaString = R"(
        <ECSchema schemaName="referenceSchema" alias="rs" version="03.02.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEnumeration typeName="TestEnum" backingTypeName="int"/>
        </ECSchema>
        )";

    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.02.03" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="referenceSchema" version="03.02.01" alias="rs"/>
            <ECEntityClass typeName="TestClass">
                <ECArrayProperty propertyName="TestArrayProperty" typeName="rs:TestEnum"/>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr referenceSchema;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(referenceSchema, referenceSchemaString, *schemaContext));

    ECSchemaPtr schemaCopyFrom;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 3, 4);

    ECClassP ecClass;
    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("TestClass"), "TestClass", false));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestArrayProperty"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetEnumerationP("TestEnum"));
    EXPECT_EQ(referenceSchema->GetEnumerationP("TestEnum"), schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestArrayProperty")->GetAsPrimitiveArrayPropertyP()->GetEnumeration());

    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *referenceSchema));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Gintaras.Volkvicius   01/2019
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyStructPropertyReferencesOriginalSchema)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.02.03" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECStructClass typeName="TestStruct"/>
            <ECEntityClass typeName="TestClass">
                <ECStructProperty propertyName="TestStructProperty" typeName="TestStruct"/>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 3, 4);

    ECClassP ecClass;
    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("TestClass"), "TestClass", false));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestStructProperty"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetClassCP("TestStruct"));
    EXPECT_EQ(schemaCopyFrom->GetClassCP("TestStruct")->GetStructClassCP(), &schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestStructProperty")->GetAsStructPropertyP()->GetType());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Gintaras.Volkvicius   01/2019
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyStructPropertyReferencesReferenceSchema)
    {
    Utf8CP referenceSchemaString = R"(
        <ECSchema schemaName="referenceSchema" alias="rs" version="03.02.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECStructClass typeName="TestStruct"/>
        </ECSchema>
        )";

    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.02.03" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="referenceSchema" version="03.02.01" alias="rs"/>
            <ECEntityClass typeName="TestClass">
                <ECStructProperty propertyName="TestStructProperty" typeName="rs:TestStruct"/>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr referenceSchema;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(referenceSchema, referenceSchemaString, *schemaContext));

    ECSchemaPtr schemaCopyFrom;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 3, 4);

    ECClassP ecClass;
    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("TestClass"), "TestClass", false));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestStructProperty"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetClassCP("TestStruct"));
    EXPECT_EQ(referenceSchema->GetClassCP("TestStruct")->GetStructClassCP(), &schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestStructProperty")->GetAsStructPropertyP()->GetType());

    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *referenceSchema));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Gintaras.Volkvicius   01/2019
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyStructArrayPropertyReferencesOriginalSchema)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.02.03" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECStructClass typeName="TestStruct"/>
            <ECEntityClass typeName="TestClass">
                <ECStructArrayProperty propertyName="TestStructArrayProperty" typeName="TestStruct"/>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 3, 4);

    ECClassP ecClass;
    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("TestClass"), "TestClass", false));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestStructArrayProperty"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetClassCP("TestStruct"));
    EXPECT_EQ(schemaCopyFrom->GetClassCP("TestStruct")->GetStructClassCP(), &schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestStructArrayProperty")->GetAsStructArrayPropertyP()->GetStructElementType());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Gintaras.Volkvicius   01/2019
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyStructArrayPropertyReferencesReferenceSchema)
    {
    Utf8CP referenceSchemaString = R"(
        <ECSchema schemaName="referenceSchema" alias="rs" version="03.02.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECStructClass typeName="TestStruct"/>
        </ECSchema>
        )";

    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.02.03" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="referenceSchema" version="03.02.01" alias="rs"/>
            <ECEntityClass typeName="TestClass">
                <ECStructArrayProperty propertyName="TestStructArrayProperty" typeName="rs:TestStruct"/>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr referenceSchema;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(referenceSchema, referenceSchemaString, *schemaContext));

    ECSchemaPtr schemaCopyFrom;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 3, 4);

    ECClassP ecClass;
    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("TestClass"), "TestClass", false));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestStructArrayProperty"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetClassCP("TestStruct"));
    EXPECT_EQ(referenceSchema->GetClassCP("TestStruct")->GetStructClassCP(), &schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestStructArrayProperty")->GetAsStructArrayPropertyP()->GetStructElementType());

    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *referenceSchema));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Gintaras.Volkvicius   01/2019
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyNavigationPropertyConstraintsAreNotHeld)
    {
    Utf8CP schemaString = R"**(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECRelationshipClass typeName="HouseHasRooms" strengthDirection="Forward" modifier="None">
                <Source multiplicity="(0..1)" roleLabel="House" polymorphic="False">
                    <Class class="House"/>
                </Source>
                <Target multiplicity="(0..*)" roleLabel="Room" polymorphic="False">
                    <Class class="Room"/>
                </Target>
            </ECRelationshipClass>
            <ECEntityClass typeName="House"/>
            <ECEntityClass typeName="Room">
                <ECNavigationProperty propertyName="NavigationPropertyToHouse" relationshipName="HouseHasRooms" direction="Backward"/>
            </ECEntityClass>
        </ECSchema>
        )**";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECClassP ecClass;

    ECSchemaPtr schemaCopyTo1;
    ECSchema::CreateSchema(schemaCopyTo1, "testSchema", "ts", 1, 0, 0); // schema name and version is same

    EXPECT_EQ(ECObjectsStatus::SchemaHasReferenceCycle, schemaCopyTo1->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("Room"), "Room", false));

    ECSchemaPtr schemaCopyTo2;
    ECSchema::CreateSchema(schemaCopyTo2, "testSchema", "ts", 2, 0, 0); // schema name same, but version differs

    EC_EXPECT_SUCCESS(schemaCopyTo2->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("Room"), "Room", false));
    EXPECT_EQ(nullptr, schemaCopyTo2->GetClassCP("HouseHasRooms"));
    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo2, *schemaCopyFrom));
    EXPECT_FALSE(schemaCopyTo2->GetClassP("Room")->GetPropertyP("NavigationPropertyToHouse")->GetAsNavigationPropertyP()->Verify());

    ECSchemaPtr schemaCopyTo3;
    ECSchema::CreateSchema(schemaCopyTo3, "otherSchema", "os", 3, 2, 1); // totally different schema

    EC_EXPECT_SUCCESS(schemaCopyTo3->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("Room"), "Room", false));
    EXPECT_EQ(nullptr, schemaCopyTo3->GetClassCP("HouseHasRooms"));
    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo3, *schemaCopyFrom));
    EXPECT_FALSE(schemaCopyTo3->GetClassP("Room")->GetPropertyP("NavigationPropertyToHouse")->GetAsNavigationPropertyP()->Verify());
    }

END_BENTLEY_ECN_TEST_NAMESPACE
