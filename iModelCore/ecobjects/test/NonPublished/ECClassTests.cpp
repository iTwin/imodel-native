/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct ClassTest : ECTestFixture
    {
    void TestPropertyCount(ECClassCR ecClass, size_t nPropertiesWithoutBaseClasses, size_t nPropertiesWithBaseClasses)
        {
        EXPECT_EQ(ecClass.GetPropertyCount(false), nPropertiesWithoutBaseClasses);
        EXPECT_EQ(ecClass.GetPropertyCount(true), nPropertiesWithBaseClasses);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    ECPropertyP GetPropertyByName(ECClassCR ecClass, Utf8CP name, bool expectExists = true)
        {
        ECPropertyP prop = ecClass.GetPropertyP(name);
        EXPECT_EQ(expectExists, NULL != prop);
        Utf8String utf8(name);
        prop = ecClass.GetPropertyP(utf8.c_str());
        EXPECT_EQ(expectExists, NULL != prop);
        return prop;
        }
    };

struct PropertyCopyTest : ECTestFixture 
    {
    ECSchemaPtr m_schema0; // schema containing the original property
    ECEntityClassP m_entity0; // class containing the original property
    ECPropertyP m_prop0; // property to copy

    ECSchemaPtr m_schema1; // schema that will contain the copied property
    ECEntityClassP m_entity1; // class that will contain the copied property
    ECPropertyP m_prop1; // copied property

    virtual void SetUp() override;

    void CopyProperty(bool copyReferences = true);
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassTest, TestGetClassIsCaseInsensitive)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="CoreCustomAttributes" version="1.00.00" alias="CoreCA"/>
            <ECEntityClass typeName="TestClass">
            </ECEntityClass>
        </ECSchema>)xml";

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext));
    ASSERT_TRUE(schema.IsValid());
    ECClassCP ecClass = schema->GetClassCP("TestClass");
    ASSERT_NE(nullptr, ecClass);

    EXPECT_EQ(ecClass, schema->GetClassCP("TESTCLASS"));
    EXPECT_EQ(ecClass, schema->GetClassCP("testclass"));
    EXPECT_EQ(ecClass, schema->GetClassCP("TeStClAsS"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassTest, TestGetPropertyIsCaseInsensitive)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="CoreCustomAttributes" version="1.00.00" alias="CoreCA"/>
            <ECEntityClass typeName="TestClass">
                <ECProperty propertyName="TestProp" typeName="int" displayLabel="TestProp" description="This is a property."/>
            </ECEntityClass>
        </ECSchema>)xml";

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext));
    ASSERT_TRUE(schema.IsValid());
    ECClassCP ecClass = schema->GetClassCP("TestClass");
    ASSERT_NE(nullptr, ecClass);
    ECPropertyCP classProp = ecClass->GetPropertyP("TestProp");
    ASSERT_NE(nullptr, classProp);

    ASSERT_EQ(classProp, ecClass->GetPropertyP("TESTPROP"));
    ASSERT_EQ(classProp, ecClass->GetPropertyP("testprop"));
    ASSERT_EQ(classProp, ecClass->GetPropertyP("TeStPrOp"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, ExpectErrorWithCircularBaseClasses)
    {
    ECSchemaPtr schema;
    ECEntityClassP class1;
    ECEntityClassP baseClass1;
    ECEntityClassP baseClass2;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    schema->CreateEntityClass(class1, "TestClass");
    schema->CreateEntityClass(baseClass1, "BaseClass1");
    schema->CreateEntityClass(baseClass2, "BaseClass2");

    EXPECT_EQ(ECObjectsStatus::Success, class1->AddBaseClass(*baseClass1));
    EXPECT_EQ(ECObjectsStatus::Success, baseClass1->AddBaseClass(*baseClass2));
    EXPECT_EQ(ECObjectsStatus::BaseClassUnacceptable, baseClass2->AddBaseClass(*class1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, GetPropertyCount)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);

    ECEntityClassP baseClass1, baseClass2, derivedClass;
    ECStructClassP structClass;

    PrimitiveECPropertyP primProp;
    StructECPropertyP structProp;

    // Struct class with 2 properties
    schema->CreateStructClass(structClass, "StructClass");
    structClass->CreatePrimitiveProperty(primProp, "StructProp1", PRIMITIVETYPE_String);
    structClass->CreatePrimitiveProperty(primProp, "StructProp2", PRIMITIVETYPE_String);

    // 1 base class with 3 primitive properties
    schema->CreateEntityClass(baseClass1, "BaseClass1");
    baseClass1->CreatePrimitiveProperty(primProp, "Base1Prop1", PRIMITIVETYPE_String);
    baseClass1->CreatePrimitiveProperty(primProp, "Base1Prop2", PRIMITIVETYPE_String);
    baseClass1->CreatePrimitiveProperty(primProp, "Base1Prop3", PRIMITIVETYPE_String);

    // 1 base class with 1 primitive and 2 struct properties (each struct has 2 properties
    schema->CreateEntityClass(baseClass2, "BaseClass2");
    baseClass2->CreatePrimitiveProperty(primProp, "Base2Prop1", PRIMITIVETYPE_String);
    baseClass2->CreateStructProperty(structProp, "Base2Prop2", *structClass);
    baseClass2->CreateStructProperty(structProp, "Base2Prop3", *structClass);

    // Derived class with 1 extra primitive property, 1 extra struct property, derived from 2 base classes
    schema->CreateEntityClass(derivedClass, "DerivedClass");
    derivedClass->CreateStructProperty(structProp, "DerivedProp1", *structClass);
    derivedClass->CreatePrimitiveProperty(primProp, "DerivedProp2", PRIMITIVETYPE_String);
    derivedClass->AddBaseClass(*baseClass1);
    derivedClass->AddBaseClass(*baseClass2);

    TestPropertyCount(*structClass, 2, 2);
    TestPropertyCount(*baseClass1, 3, 3);
    TestPropertyCount(*baseClass2, 3, 3);
    TestPropertyCount(*derivedClass, 2, 8);
    }

//---------------------------------------------------------------------------------------//
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------///
TEST_F(ClassTest, GetPropertyCount_WithOverridesAndDiamonds)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);

    ECEntityClassP baseClass, baseClass1, derivedClass, derivedClass1;
    schema->CreateEntityClass(baseClass, "Banana");
    schema->CreateEntityClass(baseClass1, "Banana1");
    schema->CreateEntityClass(derivedClass, "DerivedBanana");
    derivedClass->AddBaseClass(*baseClass);
    derivedClass->AddBaseClass(*baseClass1);
    schema->CreateEntityClass(derivedClass1, "SuperDerivedBanana");
    derivedClass1->AddBaseClass(*derivedClass);
    derivedClass1->AddBaseClass(*baseClass);

    PrimitiveECPropertyP prop1, prop1Prime, baseProp, derivedPropOverride, derived1PropOverride;
    baseClass->CreatePrimitiveProperty(prop1, "Prop1", PrimitiveType::PRIMITIVETYPE_Integer);
    baseClass->CreatePrimitiveProperty(baseProp, "Property", PrimitiveType::PRIMITIVETYPE_Double);
    baseClass1->CreatePrimitiveProperty(prop1Prime, "Prop1", PrimitiveType::PRIMITIVETYPE_Integer);
    derivedClass->CreatePrimitiveProperty(derivedPropOverride, "Property", PrimitiveType::PRIMITIVETYPE_Double);
    derivedClass1->CreatePrimitiveProperty(derived1PropOverride, "Property", PrimitiveType::PRIMITIVETYPE_Double);

    TestPropertyCount(*baseClass, 2, 2);
    TestPropertyCount(*baseClass1, 1, 1);
    TestPropertyCount(*derivedClass, 1, 2);
    TestPropertyCount(*derivedClass1, 1, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsClassInList(bvector<ECClassP> const& classList, ECClassR searchClass)
    {
    bvector<ECClassP>::const_iterator classIterator;

    for (classIterator = classList.begin(); classIterator != classList.end(); classIterator++)
        {
        if (*classIterator == &searchClass)
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, AddAndRemoveBaseClass)
    {
    ECSchemaPtr schema;
    ECEntityClassP class1;
    ECEntityClassP baseClass1;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    schema->CreateEntityClass(class1, "TestClass");
    schema->CreateEntityClass(baseClass1, "BaseClass");

    EXPECT_EQ(ECObjectsStatus::Success, class1->AddBaseClass(*baseClass1));

    EXPECT_TRUE(IsClassInList(class1->GetBaseClasses(), *baseClass1));
    EXPECT_TRUE(IsClassInList(baseClass1->GetDerivedClasses(), *class1));

    EXPECT_EQ(ECObjectsStatus::Success, class1->RemoveBaseClass(*baseClass1));

    EXPECT_FALSE(IsClassInList(class1->GetBaseClasses(), *baseClass1));
    EXPECT_FALSE(IsClassInList(baseClass1->GetDerivedClasses(), *class1));

    EXPECT_EQ(ECObjectsStatus::ClassNotFound, class1->RemoveBaseClass(*baseClass1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, AddBaseClassWithProperties)
    {
    ECSchemaPtr schema;
    ECEntityClassP class1;
    ECEntityClassP baseClass1;
    ECEntityClassP baseClass2;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    schema->CreateEntityClass(class1, "TestClass");
    schema->CreateEntityClass(baseClass1, "BaseClass");
    schema->CreateEntityClass(baseClass2, "BaseClass2");

    PrimitiveECPropertyP stringProp;
    PrimitiveECPropertyP baseStringProp;
    PrimitiveECPropertyP intProp;
    PrimitiveECPropertyP base2NonIntProp;

    class1->CreatePrimitiveProperty(stringProp, "StringProperty", PRIMITIVETYPE_String);
    baseClass1->CreatePrimitiveProperty(baseStringProp, "StringProperty", PRIMITIVETYPE_String);
    EXPECT_EQ(ECObjectsStatus::Success, class1->AddBaseClass(*baseClass1));

    class1->CreatePrimitiveProperty(intProp, "IntProperty", PRIMITIVETYPE_Integer);
    baseClass2->CreatePrimitiveProperty(base2NonIntProp, "IntProperty", PRIMITIVETYPE_String);
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->AddBaseClass(*baseClass2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, BaseClassOrder)
    {
    ECSchemaPtr schema = nullptr;
    ECEntityClassP class1 = nullptr;
    ECEntityClassP baseClass1 = nullptr;
    ECEntityClassP baseClass2 = nullptr;
    ECEntityClassP baseClass3 = nullptr;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    schema->CreateEntityClass(class1, "TestClass");
    schema->CreateEntityClass(baseClass1, "BaseClass");
    schema->CreateEntityClass(baseClass2, "BaseClass2");
    schema->CreateEntityClass(baseClass3, "BaseClass3");

    PrimitiveECPropertyP prop = nullptr;
    class1->CreatePrimitiveProperty(prop, "StringProperty", PRIMITIVETYPE_String);
    baseClass1->CreatePrimitiveProperty(prop, "StringProperty", PRIMITIVETYPE_String);
    baseClass2->CreatePrimitiveProperty(prop, "SstringProperty", PRIMITIVETYPE_String);
    baseClass3->CreatePrimitiveProperty(prop, "StringProperty", PRIMITIVETYPE_String);

    ASSERT_EQ(ECObjectsStatus::Success, class1->AddBaseClass(*baseClass1));
    ASSERT_EQ(ECObjectsStatus::Success, class1->AddBaseClass(*baseClass2));

    ASSERT_EQ(2, class1->GetBaseClasses().size());
    ASSERT_TRUE(baseClass1 == class1->GetBaseClasses()[0]);
    ASSERT_TRUE(baseClass2 == class1->GetBaseClasses()[1]);

    ASSERT_EQ(ECObjectsStatus::Success, class1->AddBaseClass(*baseClass3, true));
    ASSERT_EQ(3, class1->GetBaseClasses().size());
    ASSERT_TRUE(baseClass3 == class1->GetBaseClasses()[0]);
    ASSERT_TRUE(baseClass1 == class1->GetBaseClasses()[1]);
    ASSERT_TRUE(baseClass2 == class1->GetBaseClasses()[2]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, IsTests)
    {
    ECSchemaPtr schema;
    ECEntityClassP class1;
    ECEntityClassP baseClass1;
    ECEntityClassP baseClass2;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    schema->CreateEntityClass(class1, "TestClass");
    schema->CreateEntityClass(baseClass1, "BaseClass1");
    schema->CreateEntityClass(baseClass2, "BaseClass2");

    EXPECT_FALSE(class1->Is(baseClass1));
    class1->AddBaseClass(*baseClass1);
    EXPECT_TRUE(class1->Is(baseClass1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, CanOverrideBaseProperties)
    {
    ECSchemaPtr schema;
    ECEntityClassP class1;
    ECEntityClassP baseClass1;
    ECStructClassP structClass;
    ECStructClassP structClass2;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    schema->CreateEntityClass(class1, "TestClass");
    schema->CreateEntityClass(baseClass1, "BaseClass1");
    schema->CreateStructClass(structClass, "ClassForStructs");
    schema->CreateStructClass(structClass2, "ClassForStructs2");
    class1->AddBaseClass(*baseClass1);

    PrimitiveECPropertyP baseStringProp;
    PrimitiveECPropertyP baseIntProp;
    PrimitiveECPropertyP baseDoubleProp;
    StructECPropertyP baseStructProp;
    PrimitiveArrayECPropertyP baseStringArrayProperty;
    StructArrayECPropertyP baseStructArrayProp;

    baseClass1->CreatePrimitiveProperty(baseStringProp, "StringProperty", PRIMITIVETYPE_String);
    baseClass1->CreatePrimitiveProperty(baseIntProp, "IntegerProperty", PRIMITIVETYPE_Integer);
    baseClass1->CreatePrimitiveProperty(baseDoubleProp, "DoubleProperty", PRIMITIVETYPE_Double);
    baseClass1->CreateStructProperty(baseStructProp, "StructProperty", *structClass);
    baseClass1->CreatePrimitiveArrayProperty(baseStringArrayProperty, "StringArrayProperty", PRIMITIVETYPE_String);
    baseClass1->CreateStructArrayProperty(baseStructArrayProp, "StructArrayProperty", *structClass);

    PrimitiveECPropertyP longProperty = NULL;
    PrimitiveECPropertyP stringProperty = NULL;

    DISABLE_ASSERTS;
    // Primitives overriding primitives
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->CreatePrimitiveProperty(longProperty, "StringProperty", PRIMITIVETYPE_Long));
    EXPECT_EQ(NULL, longProperty);
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveProperty(stringProperty, "StringProperty", PRIMITIVETYPE_String));
    EXPECT_EQ(baseStringProp, stringProperty->GetBaseProperty());
    class1->RemoveProperty("StringProperty");

    {
    // Primitives overriding structs
    DISABLE_ASSERTS
        EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->CreatePrimitiveProperty(longProperty, "StructProperty", PRIMITIVETYPE_Long));
    }

    // Primitives overriding arrays
    EXPECT_EQ(ECObjectsStatus::InvalidPrimitiveOverrride, class1->CreatePrimitiveProperty(longProperty, "StringArrayProperty", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::InvalidPrimitiveOverrride, class1->CreatePrimitiveProperty(stringProperty, "StringArrayProperty", PRIMITIVETYPE_String));
    class1->RemoveProperty("StringArrayProperty");

    StructECPropertyP structProperty;

    {
    // Structs overriding primitives
    DISABLE_ASSERTS
        EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->CreateStructProperty(structProperty, "IntegerProperty", *structClass2));
    }

    // Structs overriding structs
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->CreateStructProperty(structProperty, "StructProperty", *structClass2));

    // Structs overriding arrays
    EXPECT_EQ(ECObjectsStatus::InvalidPrimitiveOverrride, class1->CreateStructProperty(structProperty, "StringArrayProperty", *structClass));
    EXPECT_EQ(ECObjectsStatus::InvalidPrimitiveOverrride, class1->CreateStructProperty(structProperty, "StructArrayProperty", *structClass));
    EXPECT_EQ(ECObjectsStatus::InvalidPrimitiveOverrride, class1->CreateStructProperty(structProperty, "StructArrayProperty", *structClass2));

    PrimitiveArrayECPropertyP stringArrayProperty;
    PrimitiveArrayECPropertyP stringArrayProperty2;
    StructArrayECPropertyP structArrayProperty;
    // Arrays overriding primitives
    EXPECT_EQ(ECObjectsStatus::InvalidPrimitiveOverrride, class1->CreatePrimitiveArrayProperty(stringArrayProperty, "IntegerProperty", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::InvalidPrimitiveOverrride, class1->CreatePrimitiveArrayProperty(stringArrayProperty, "StringProperty", PRIMITIVETYPE_String));
    EXPECT_EQ(ECObjectsStatus::InvalidPrimitiveOverrride, class1->CreatePrimitiveArrayProperty(stringArrayProperty2, "StringProperty"));

    // Arrays overriding structs
    EXPECT_EQ(ECObjectsStatus::InvalidPrimitiveOverrride, class1->CreateStructArrayProperty(structArrayProperty, "StructProperty", *structClass2));
    EXPECT_EQ(ECObjectsStatus::InvalidPrimitiveOverrride, class1->CreateStructArrayProperty(structArrayProperty, "StructProperty", *structClass));

    PrimitiveArrayECPropertyP intArrayProperty;
    // Arrays overriding arrays
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->CreatePrimitiveArrayProperty(intArrayProperty, "StringArrayProperty", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveArrayProperty(stringArrayProperty, "StringArrayProperty", PRIMITIVETYPE_String));
    class1->RemoveProperty("StringArrayProperty");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ClassTest, CanOverrideBasePropertiesInDerivedClass)
    {
    ECSchemaPtr schema;
    ECEntityClassP parent;
    ECEntityClassP derived;
    ECEntityClassP base;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    schema->CreateEntityClass(base, "BaseClass");
    schema->CreateEntityClass(parent, "Parent");
    schema->CreateEntityClass(derived, "Derived");
    derived->AddBaseClass(*parent);

    PrimitiveECPropertyP derivedStringProp;
    PrimitiveECPropertyP baseIntProp;

    derived->CreatePrimitiveProperty(derivedStringProp, "Code", PRIMITIVETYPE_String);
    base->CreatePrimitiveProperty(baseIntProp, "Code", PRIMITIVETYPE_String);

    ECObjectsStatus status = parent->AddBaseClass(*base);
    EXPECT_EQ(ECObjectsStatus::Success, status);

    ECPropertyP prop = derived->GetPropertyP("Code", false);
    ASSERT_TRUE(nullptr != prop);
    PrimitiveECPropertyP primProp = prop->GetAsPrimitivePropertyP();
    EXPECT_EQ(PRIMITIVETYPE_String, primProp->GetType());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ClassTest, ExpectFailureWhenOverridePropertyConflicts_MultipleLevelsOfDerivedClassesChecked)
    {
    ECSchemaPtr schema;
    ECEntityClassP parent;
    ECEntityClassP derived;
    ECEntityClassP base;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    schema->CreateEntityClass(base, "BaseClass");
    schema->CreateEntityClass(parent, "Parent");
    schema->CreateEntityClass(derived, "Derived");
    derived->AddBaseClass(*parent);

    PrimitiveECPropertyP derivedStringProp;
    PrimitiveECPropertyP baseIntProp;

    derived->CreatePrimitiveProperty(derivedStringProp, "Code", PRIMITIVETYPE_String);
    base->CreatePrimitiveProperty(baseIntProp, "Code", PRIMITIVETYPE_Integer);

    ECObjectsStatus status = parent->AddBaseClass(*base);
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, status);
    EXPECT_FALSE(parent->HasBaseClasses());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ClassTest, ExpectFailureWhenOverridePropertyConflicts)
    {
    ECSchemaPtr schema;
    ECEntityClassP base;
    ECEntityClassP derived;
    ECEntityClassP derived2;
    ECEntityClassP derived3;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    schema->CreateEntityClass(base, "Base");
    schema->CreateEntityClass(derived, "Derived");
    schema->CreateEntityClass(derived2, "Derived2");
    schema->CreateEntityClass(derived3, "Derived3");

    PrimitiveECPropertyP derivedStringProp;
    PrimitiveECPropertyP derivedStringProp2;
    PrimitiveECPropertyP baseIntProp;
    PrimitiveECPropertyP derivedIntPropUpperCase;
    PrimitiveECPropertyP derivedIntPropUpperCase2;

    base->CreatePrimitiveProperty(baseIntProp, "Prop", PRIMITIVETYPE_Integer);

    derived->AddBaseClass(*base);
    ECObjectsStatus status = derived->CreatePrimitiveProperty(derivedStringProp, "Prop", PRIMITIVETYPE_String);
    ASSERT_EQ(ECObjectsStatus::DataTypeMismatch, status);
    ASSERT_EQ(0, derived->GetPropertyCount(false));

    status = derived->CreatePrimitiveProperty(derivedIntPropUpperCase, "PROP", PRIMITIVETYPE_Integer);
    ASSERT_EQ(ECObjectsStatus::CaseCollision, status);
    ASSERT_EQ(0, derived->GetPropertyCount(false));

    derived2->CreatePrimitiveProperty(derivedStringProp2, "Prop", PRIMITIVETYPE_String);
    status = derived2->AddBaseClass(*base);
    ASSERT_EQ(ECObjectsStatus::DataTypeMismatch, status);
    ASSERT_EQ(1, derived2->GetPropertyCount(false));

    derived3->CreatePrimitiveProperty(derivedIntPropUpperCase2, "PROP", PRIMITIVETYPE_Integer);
    status = derived3->AddBaseClass(*base);
    ASSERT_EQ(ECObjectsStatus::CaseCollision, status);
    ASSERT_EQ(1, derived2->GetPropertyCount(false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, ExpectFailureWhenStructTypeIsNotReferenced)
    {
    ECSchemaPtr schema;
    ECSchemaPtr schema2;
    ECEntityClassP class1;
    ECStructClassP structClass;
    ECStructClassP structClass2;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    ECSchema::CreateSchema(schema2, "TestSchema2", "ts", 5, 0, 5);
    schema->CreateEntityClass(class1, "TestClass");
    schema2->CreateStructClass(structClass, "ClassForStructs");
    schema->CreateStructClass(structClass2, "ClassForStructs2");

    StructECPropertyP baseStructProp;
    StructArrayECPropertyP structArrayProperty;
    StructECPropertyP baseStructProp2;
    StructArrayECPropertyP structArrayProperty2;

    EXPECT_EQ(ECObjectsStatus::SchemaNotFound, class1->CreateStructProperty(baseStructProp, "StructProperty", *structClass));
    EXPECT_EQ(ECObjectsStatus::SchemaNotFound, class1->CreateStructArrayProperty(structArrayProperty, "StructArrayProperty", *structClass));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreateStructProperty(baseStructProp2, "StructProperty2", *structClass2));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreateStructArrayProperty(structArrayProperty2, "StructArrayProperty2", *structClass2));
    schema->AddReferencedSchema(*schema2);
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreateStructProperty(baseStructProp, "StructProperty", *structClass));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreateStructArrayProperty(structArrayProperty, "StructArrayProperty", *structClass));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, ExpectPropertiesInOrder)
    {
    std::vector<Utf8CP> propertyNames;
    propertyNames.push_back("beta");
    propertyNames.push_back("gamma");
    propertyNames.push_back("delta");
    propertyNames.push_back("alpha");

    ECSchemaPtr schema;
    ECEntityClassP class1;
    PrimitiveECPropertyP property1;
    PrimitiveECPropertyP property2;
    PrimitiveECPropertyP property3;
    PrimitiveECPropertyP property4;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    schema->CreateEntityClass(class1, "TestClass");
    class1->CreatePrimitiveProperty(property1, "beta", PRIMITIVETYPE_String);
    class1->CreatePrimitiveProperty(property2, "gamma", PRIMITIVETYPE_String);
    class1->CreatePrimitiveProperty(property3, "delta", PRIMITIVETYPE_String);
    class1->CreatePrimitiveProperty(property4, "alpha", PRIMITIVETYPE_String);

    int i = 0;
    ECPropertyIterable  iterable = class1->GetProperties(false);
    for (ECPropertyP prop : iterable)
        {
        EXPECT_EQ(0, prop->GetName().compare(propertyNames[i]));
        i++;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, ExpectProperties)
    {
    ECSchemaPtr schema;
    ECEntityClassP ab;
    ECEntityClassP cd;
    ECEntityClassP ef;

    PrimitiveECPropertyP a;
    PrimitiveECPropertyP b;
    PrimitiveECPropertyP c;
    PrimitiveECPropertyP d;
    PrimitiveECPropertyP e;
    PrimitiveECPropertyP f;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    schema->CreateEntityClass(ab, "ab");
    schema->CreateEntityClass(cd, "cd");
    schema->CreateEntityClass(ef, "ef");

    ab->CreatePrimitiveProperty(a, "a", PRIMITIVETYPE_String);
    ab->CreatePrimitiveProperty(b, "b", PRIMITIVETYPE_String);

    cd->CreatePrimitiveProperty(c, "c", PRIMITIVETYPE_String);
    cd->CreatePrimitiveProperty(d, "d", PRIMITIVETYPE_String);

    ef->CreatePrimitiveProperty(e, "e", PRIMITIVETYPE_String);
    ef->CreatePrimitiveProperty(f, "f", PRIMITIVETYPE_String);

    cd->AddBaseClass(*ab);
    ef->AddBaseClass(*cd);

    EXPECT_TRUE(NULL != GetPropertyByName(*ef, "e"));
    EXPECT_TRUE(NULL != GetPropertyByName(*ef, "c"));
    EXPECT_TRUE(NULL != GetPropertyByName(*ef, "a"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, ExpectPropertiesFromBaseClass)
    {
    ECSchemaPtr schema;
    ECEntityClassP ab;
    ECEntityClassP cd;
    ECEntityClassP ef;
    ECEntityClassP gh;
    ECEntityClassP ij;
    ECEntityClassP kl;
    ECEntityClassP mn;

    PrimitiveECPropertyP a;
    PrimitiveECPropertyP b;
    PrimitiveECPropertyP c;
    PrimitiveECPropertyP d;
    PrimitiveECPropertyP e;
    PrimitiveECPropertyP f;
    PrimitiveECPropertyP g;
    PrimitiveECPropertyP h;
    PrimitiveECPropertyP i;
    PrimitiveECPropertyP j;
    PrimitiveECPropertyP k;
    PrimitiveECPropertyP l;
    PrimitiveECPropertyP m;
    PrimitiveECPropertyP n;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    schema->CreateEntityClass(ab, "ab");
    schema->CreateEntityClass(cd, "cd");
    schema->CreateEntityClass(ef, "ef");
    schema->CreateEntityClass(gh, "gh");
    schema->CreateEntityClass(ij, "ij");
    schema->CreateEntityClass(kl, "kl");
    schema->CreateEntityClass(mn, "mn");

    ab->CreatePrimitiveProperty(a, "a", PRIMITIVETYPE_String);
    ab->CreatePrimitiveProperty(b, "b", PRIMITIVETYPE_String);

    cd->CreatePrimitiveProperty(c, "c", PRIMITIVETYPE_String);
    cd->CreatePrimitiveProperty(d, "d", PRIMITIVETYPE_String);

    ef->CreatePrimitiveProperty(e, "e", PRIMITIVETYPE_String);
    ef->CreatePrimitiveProperty(f, "f", PRIMITIVETYPE_String);

    gh->CreatePrimitiveProperty(g, "g", PRIMITIVETYPE_String);
    gh->CreatePrimitiveProperty(h, "h", PRIMITIVETYPE_String);

    ij->CreatePrimitiveProperty(i, "i", PRIMITIVETYPE_String);
    ij->CreatePrimitiveProperty(j, "j", PRIMITIVETYPE_String);

    kl->CreatePrimitiveProperty(k, "k", PRIMITIVETYPE_String);
    kl->CreatePrimitiveProperty(l, "l", PRIMITIVETYPE_String);

    mn->CreatePrimitiveProperty(m, "m", PRIMITIVETYPE_String);
    mn->CreatePrimitiveProperty(n, "n", PRIMITIVETYPE_String);

    ef->AddBaseClass(*ab);
    ef->AddBaseClass(*cd);

    kl->AddBaseClass(*gh);
    kl->AddBaseClass(*ij);

    mn->AddBaseClass(*ef);
    mn->AddBaseClass(*kl);

    ECPropertyIterable  iterable1 = mn->GetProperties(true);
    std::vector<ECPropertyP> testVector;
    for (ECPropertyP prop : iterable1)
        testVector.push_back(prop);

    EXPECT_EQ(14, testVector.size());
    for (size_t i = 0; i < testVector.size(); i++)
        {
        Utf8Char expectedName[] = {(Utf8Char) ('a' + static_cast<Utf8Char> (i)), 0};
        EXPECT_EQ(0, testVector[i]->GetName().compare(expectedName)) << "Expected: " << expectedName << " Actual: " << testVector[i]->GetName().c_str();
        }

    // now we add some duplicate properties to mn which will "override" those from the base classes
    PrimitiveECPropertyP b2;
    PrimitiveECPropertyP d2;
    PrimitiveECPropertyP f2;
    PrimitiveECPropertyP h2;
    PrimitiveECPropertyP j2;
    PrimitiveECPropertyP k2;

    mn->CreatePrimitiveProperty(b2, "b", PRIMITIVETYPE_String);
    mn->CreatePrimitiveProperty(d2, "d", PRIMITIVETYPE_String);
    mn->CreatePrimitiveProperty(f2, "f", PRIMITIVETYPE_String);
    mn->CreatePrimitiveProperty(h2, "h", PRIMITIVETYPE_String);
    mn->CreatePrimitiveProperty(j2, "j", PRIMITIVETYPE_String);
    mn->CreatePrimitiveProperty(k2, "k", PRIMITIVETYPE_String);

    ECPropertyIterable  iterable2 = mn->GetProperties(true);
    testVector.clear();
    for (ECPropertyP prop : iterable2)
        testVector.push_back(prop);

    EXPECT_EQ(14, testVector.size());
    bvector<Utf8CP> expectedVector {"a", "c", "e", "g", "i", "l", "m", "n", "b", "d", "f", "h", "j", "k"};
    for (size_t i = 0; i < testVector.size(); i++)
        EXPECT_EQ(0, testVector[i]->GetName().compare(expectedVector[i])) << "Expected: " << expectedVector[i] << " Actual: " << testVector[i]->GetName().c_str();

    PrimitiveECPropertyP e2;
    PrimitiveECPropertyP a2;
    PrimitiveECPropertyP c2;
    PrimitiveECPropertyP g2;

    PrimitiveECPropertyP l2;
    PrimitiveECPropertyP i2;
    PrimitiveECPropertyP g3;

    PrimitiveECPropertyP a3;
    PrimitiveECPropertyP b3;
    PrimitiveECPropertyP g4;
    PrimitiveECPropertyP h3;

    kl->CreatePrimitiveProperty(e2, "e", PRIMITIVETYPE_String);
    kl->CreatePrimitiveProperty(a2, "a", PRIMITIVETYPE_String);
    kl->CreatePrimitiveProperty(c2, "c", PRIMITIVETYPE_String);
    kl->CreatePrimitiveProperty(g2, "g", PRIMITIVETYPE_String);

    ef->CreatePrimitiveProperty(l2, "l", PRIMITIVETYPE_String);
    gh->CreatePrimitiveProperty(i2, "i", PRIMITIVETYPE_String);
    ij->CreatePrimitiveProperty(g3, "g", PRIMITIVETYPE_String);

    gh->CreatePrimitiveProperty(a3, "a", PRIMITIVETYPE_String);
    gh->CreatePrimitiveProperty(b3, "b", PRIMITIVETYPE_String);
    ab->CreatePrimitiveProperty(g4, "g", PRIMITIVETYPE_String);
    ab->CreatePrimitiveProperty(h3, "h", PRIMITIVETYPE_String);

    ECPropertyIterable  iterable3 = mn->GetProperties(true);
    testVector.clear();
    for (ECPropertyP prop : iterable3)
        testVector.push_back(prop);

    EXPECT_EQ(14, testVector.size());
    expectedVector = {"a", "g", "c", "e", "l", "i", "m", "n", "b", "d", "f", "h", "j", "k"};
    for (size_t i = 0; i < testVector.size(); i++)
        EXPECT_EQ(0, testVector[i]->GetName().compare(expectedVector[i])) << "Expected: " << expectedVector[i] << " Actual: " << testVector[i]->GetName().c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, AddAndRemoveConstraintClasses)
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    ECSchema::CreateSchema(refSchema, "RefSchema", "ts", 5, 0, 5);

    ECRelationshipClassP relClass;
    ECEntityClassP targetClass;
    ECEntityClassP sourceClass;

    schema->CreateRelationshipClass(relClass, "RElationshipClass");
    schema->CreateEntityClass(targetClass, "Target");
    refSchema->CreateEntityClass(sourceClass, "Source");

    EXPECT_EQ(ECObjectsStatus::Success, relClass->GetTarget().AddClass(*targetClass));
    EXPECT_EQ(ECObjectsStatus::SchemaNotFound, relClass->GetSource().AddClass(*sourceClass));

    schema->AddReferencedSchema(*refSchema);
    EXPECT_EQ(ECObjectsStatus::Success, relClass->GetSource().AddClass(*sourceClass));

    EXPECT_EQ(ECObjectsStatus::Success, relClass->GetTarget().RemoveClass(*targetClass));
    EXPECT_EQ(ECObjectsStatus::ClassNotFound, relClass->GetTarget().RemoveClass(*targetClass));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, ExpectReadOnlyFromBaseClass)
    {
    ECSchemaPtr schema;
    ECEntityClassP child;
    ECEntityClassP base;

    PrimitiveECPropertyP readOnlyProp;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    schema->CreateEntityClass(base, "BaseClass");
    schema->CreateEntityClass(child, "ChildClass");

    base->CreatePrimitiveProperty(readOnlyProp, "readOnlyProp", PRIMITIVETYPE_String);
    readOnlyProp->SetIsReadOnly(true);

    ASSERT_EQ(ECObjectsStatus::Success, child->AddBaseClass(*base));

    ECPropertyP ecProp = GetPropertyByName(*child, "readOnlyProp");
    ASSERT_EQ(true, ecProp->GetIsReadOnly());
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassTest, TestPropertyEnumerationType)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);

    ECEntityClassP classA;
    ECEntityClassP classB;
    ECEnumerationP tsEnum;
    PrimitiveECPropertyP primProp;
    PrimitiveArrayECPropertyP primArrProp;

    schema->CreateEntityClass(classA, "A");
    schema->CreateEntityClass(classB, "B");
    schema->CreateEnumeration(tsEnum, "TestEnum", PRIMITIVETYPE_String);

    classA->CreatePrimitiveProperty(primProp, "PrimProp", PRIMITIVETYPE_String);
    classA->CreatePrimitiveArrayProperty(primArrProp, "PrimArrProp");

    classB->AddBaseClass(*classA);

    EXPECT_EQ(ECObjectsStatus::Success, primProp->SetType(*tsEnum)) << "Failed to set the type as an Enumeration.";
    EXPECT_EQ(ECObjectsStatus::Success, primArrProp->SetType(*tsEnum)) << "Failed to set the type as an Enumeration.";

    // Test that the enumeration was actually set
    ASSERT_NE(nullptr, classA->GetPropertyP("PrimProp")->GetAsPrimitiveProperty()->GetEnumeration()) << "There was not an Enumeration set when there should be";
    EXPECT_EQ(tsEnum->GetName(), classA->GetPropertyP("PrimProp")->GetAsPrimitiveProperty()->GetEnumeration()->GetName()) << "The property did not have the expected Enumeration.";
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_String, classA->GetPropertyP("PrimProp")->GetAsPrimitiveProperty()->GetType()) << "The property did not have the expected primitive type";
    ASSERT_NE(nullptr, classA->GetPropertyP("PrimArrProp")->GetAsPrimitiveArrayProperty()->GetEnumeration()) << "There was not an Enumeration set when there should be";
    EXPECT_EQ(tsEnum->GetName(), classA->GetPropertyP("PrimArrProp")->GetAsPrimitiveArrayProperty()->GetEnumeration()->GetName()) << "The property did not have the expected Enumeration.";
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_String, classA->GetPropertyP("PrimArrProp")->GetAsPrimitiveArrayProperty()->GetPrimitiveElementType()) << "The property did not have the expected primitive type";

    // Test the inheritance of a property with an Enumeration
    ASSERT_NE(nullptr, classB->GetPropertyP("PrimProp")->GetAsPrimitiveProperty()->GetEnumeration()) << "There was not an Enumeration set when there should be";
    EXPECT_EQ(tsEnum->GetName(), classB->GetPropertyP("PrimProp")->GetAsPrimitiveProperty()->GetEnumeration()->GetName()) << "The property did not have the expected Enumeration.";
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_String, classB->GetPropertyP("PrimProp")->GetAsPrimitiveProperty()->GetType()) << "The property did not have the expected primitive type";
    ASSERT_NE(nullptr, classB->GetPropertyP("PrimArrProp")->GetAsPrimitiveArrayProperty()->GetEnumeration()) << "There was not an Enumeration set when there should be";
    EXPECT_EQ(tsEnum->GetName(), classB->GetPropertyP("PrimArrProp")->GetAsPrimitiveArrayProperty()->GetEnumeration()->GetName()) << "The property did not have the expected Enumeration.";
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_String, classB->GetPropertyP("PrimArrProp")->GetAsPrimitiveArrayProperty()->GetPrimitiveElementType()) << "The property did not have the expected primitive type";
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassTest, ClassNotSubClassableInReferencingSchema_API_NoExclusions)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    ECEntityClassP baseClass;
    schema->CreateEntityClass(baseClass, "BaseClass");
    IECInstancePtr notSubClassable = CoreCustomAttributeHelper::GetClass("NotSubclassableInReferencingSchemas")->GetDefaultStandaloneEnabler()->CreateInstance();
    schema->AddReferencedSchema(*CoreCustomAttributeHelper::GetSchema());
    baseClass->SetCustomAttribute(*notSubClassable);

    ECEntityClassP localDerivedClass;
    schema->CreateEntityClass(localDerivedClass, "LocalDerivedClass");
    EXPECT_EQ(ECObjectsStatus::Success, localDerivedClass->AddBaseClass(*baseClass)) << "Failed to add local base class";

    ECSchemaPtr refingSchema;
    ECSchema::CreateSchema(refingSchema, "RefingSchema", "RS", 4, 2, 2);
    refingSchema->AddReferencedSchema(*schema);

    ECEntityClassP remoteDerivedClass;
    refingSchema->CreateEntityClass(remoteDerivedClass, "RemoteDerivedClass");
    EXPECT_EQ(ECObjectsStatus::BaseClassUnacceptable, remoteDerivedClass->AddBaseClass(*baseClass)) << "Should not have been able to add base class because of 'NotSubclassableInReferencingSchemas' CA";

    ECEntityClassP remoteDerivedDerivedClass;
    refingSchema->CreateEntityClass(remoteDerivedDerivedClass, "RemoteDerivedDerivedClass");
    EXPECT_EQ(ECObjectsStatus::Success, remoteDerivedDerivedClass->AddBaseClass(*localDerivedClass)) << 
        "Should  have been able to add base class because 'NotSubclassableInReferencingSchemas' CA applied to base class of applied base class";
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassTest, ClassNotSubClassableInReferencingSchema_API_WithExclusions)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    ECEntityClassP baseClass;
    schema->CreateEntityClass(baseClass, "BaseClass");
    IECInstancePtr notSubClassable = CoreCustomAttributeHelper::GetClass("NotSubclassableInReferencingSchemas")->GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue exclusion0("RefingSchema:RemoteDerivedClass");
    notSubClassable->AddArrayElements("Exceptions", 2);
    ASSERT_EQ(ECObjectsStatus::Success, notSubClassable->SetValue("Exceptions", exclusion0, 0));
    schema->AddReferencedSchema(*CoreCustomAttributeHelper::GetSchema());
    baseClass->SetCustomAttribute(*notSubClassable);

    ECEntityClassP localDerivedClass;
    schema->CreateEntityClass(localDerivedClass, "LocalDerivedClass");
    EXPECT_EQ(ECObjectsStatus::Success, localDerivedClass->AddBaseClass(*baseClass)) << "Failed to add local base class";

    ECSchemaPtr refingSchema;
    ECSchema::CreateSchema(refingSchema, "RefingSchema", "RS", 4, 2, 2);
    refingSchema->AddReferencedSchema(*schema);

    ECEntityClassP remoteDerivedClass;
    refingSchema->CreateEntityClass(remoteDerivedClass, "RemoteDerivedClass");
    EXPECT_EQ(ECObjectsStatus::Success, remoteDerivedClass->AddBaseClass(*baseClass)) << 
        "Should  have been able to add base class because 'NotSubclassableInReferencingSchemas' CA includes exclusion for this class.";

    ECEntityClassP remoteDerivedClass2;
    refingSchema->CreateEntityClass(remoteDerivedClass2, "RemoteDerivedClass2");
    EXPECT_EQ(ECObjectsStatus::BaseClassUnacceptable, remoteDerivedClass2->AddBaseClass(*baseClass)) << 
        "Should not have been able to add base class because 'NotSubclassableInReferencingSchemas' CA does not include an exclusion for this class";

    ECValue exclusion1("RefingSchema:RemoteDerivedClass2");
    ASSERT_EQ(ECObjectsStatus::Success, notSubClassable->SetValue("Exceptions", exclusion1, 1));

    EXPECT_EQ(ECObjectsStatus::Success, remoteDerivedClass2->AddBaseClass(*baseClass)) <<
        "Should  have been able to add base class because 'NotSubclassableInReferencingSchemas' CA includes exclusion for this class.";
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassTest, ClassNotSubClassableInReferencingSchema_XML_NoExclusions)
    {
    Utf8CP baseSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="CoreCustomAttributes" version="1.00.00" alias="CoreCA"/>
                <ECEntityClass typeName="BaseClass">
                    <ECCustomAttributes>
                        <NotSubClassableInReferencingSchemas xmlns="CoreCustomAttributes.01.00"/>
                    </ECCustomAttributes>
                </ECEntityClass>
                <ECEntityClass typeName="LocalDerivedClass">
                    <BaseClass>BaseClass</BaseClass>
                </ECEntityClass>
            </ECSchema>)xml";
    
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, baseSchemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    Utf8CP badRefSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="RefingSchema" alias="RS" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="TestSchema" version="1.00.00" alias="ts"/>
                <ECEntityClass typeName="RemoteDerivedClass">
                    <BaseClass>ts:BaseClass</BaseClass>
                </ECEntityClass>
            </ECSchema>)xml";

    ECSchemaPtr refingSchema;
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(refingSchema, badRefSchemaXml, *context));
    ASSERT_TRUE(!refingSchema.IsValid());

    Utf8CP goodRefSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="RefingSchema" alias="RS" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="TestSchema" version="1.00.00" alias="ts"/>
                <ECEntityClass typeName="RemoteDerivedClass">
                    <BaseClass>ts:LocalDerivedClass</BaseClass>
                </ECEntityClass>
            </ECSchema>)xml";

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refingSchema, goodRefSchemaXml, *context));
    ASSERT_TRUE(refingSchema.IsValid());
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassTest, ClassNotSubClassableInReferencingSchema_XML_WithExclusions)
    {
    Utf8CP baseSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="CoreCustomAttributes" version="1.00.00" alias="CoreCA"/>
                <ECEntityClass typeName="BaseClass">
                    <ECCustomAttributes>
                        <NotSubClassableInReferencingSchemas xmlns="CoreCustomAttributes.01.00">
                            <Exceptions>
                                <string></string>
                                <string>RefingSchema:RemoteDerivedClass</string>
                            </Exceptions>
                        </NotSubClassableInReferencingSchemas>
                    </ECCustomAttributes>
                </ECEntityClass>
                <ECEntityClass typeName="LocalDerivedClass">
                    <BaseClass>BaseClass</BaseClass>
                </ECEntityClass>
            </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, baseSchemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    Utf8CP goodRefSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="RefingSchema" alias="RS" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="TestSchema" version="1.00.00" alias="ts"/>
                <ECEntityClass typeName="RemoteDerivedClass">
                    <BaseClass>ts:BaseClass</BaseClass>
                </ECEntityClass>
            </ECSchema>)xml";

    ECSchemaPtr refingSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refingSchema, goodRefSchemaXml, *context));
    ASSERT_TRUE(refingSchema.IsValid());

    context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, baseSchemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    Utf8CP badRefSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="RefingSchema" alias="RS" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="TestSchema" version="1.00.00" alias="ts"/>
                <ECEntityClass typeName="RemoteDerivedClass2">
                    <BaseClass>ts:BaseClass</BaseClass>
                </ECEntityClass>
            </ECSchema>)xml";

    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(refingSchema, badRefSchemaXml, *context));
    ASSERT_TRUE(!refingSchema.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassTest, SerializeStandaloneEntityClass)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);

    ECEntityClassP baseEntityClass;
    schema->CreateEntityClass(baseEntityClass, "ExampleBaseEntity");

    ECEntityClassP mixinA;
    ECEntityClassP mixinB;

    ECCustomAttributeClassP customAttrClass;
    schema->CreateCustomAttributeClass(customAttrClass, "ExampleCustomAttribute");
    IECInstancePtr customAttr = customAttrClass->GetDefaultStandaloneEnabler()->CreateInstance();

    ECEntityClassP entityClass;
    schema->CreateEntityClass(entityClass, "ExampleEntity");
    entityClass->SetClassModifier(ECClassModifier::Sealed);
    entityClass->SetDisplayLabel("ExampleEntity");
    entityClass->SetDescription("An example entity class.");
    entityClass->AddBaseClass(*baseEntityClass);
    schema->CreateMixinClass(mixinA, "ExampleMixinA", *entityClass);
    schema->CreateMixinClass(mixinB, "ExampleMixinB", *entityClass);
    entityClass->AddBaseClass(*mixinA);
    entityClass->AddBaseClass(*mixinB);
    entityClass->SetCustomAttribute(*customAttr);

    BeJsDocument schemaItemJson;
    EXPECT_TRUE(entityClass->ToJson(schemaItemJson, true));

    BeJsDocument testDataJson;
    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneECEntityClass.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaItemJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaItemJson, testDataJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassTest, SerializeStandaloneStructClass)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);

    ECStructClassP structClass;
    schema->CreateStructClass(structClass, "ExampleStruct");
    structClass->SetClassModifier(ECClassModifier::Sealed);
    PrimitiveArrayECPropertyP primArrProp;
    structClass->CreatePrimitiveArrayProperty(primArrProp, "ExamplePrimitiveArray");
    primArrProp->SetPrimitiveElementType(ECN::PRIMITIVETYPE_Integer);
    primArrProp->SetMinimumValue(ECValue((int32_t)7));
    primArrProp->SetMaximumValue(ECValue((int32_t)20));
    primArrProp->SetMinOccurs(10);
    primArrProp->SetMaxOccurs(25);
    primArrProp->SetExtendedTypeName("FooBar");
    primArrProp->SetDisplayLabel("ExPrimitiveArray");

    BeJsDocument schemaJson;
    EXPECT_TRUE(structClass->ToJson(schemaJson, true));

    BeJsDocument testDataJson;
    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneECStructClass.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassTest, SerializeStandaloneCustomAttributeClass)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);

    ECCustomAttributeClassP customAttrClass;
    schema->CreateCustomAttributeClass(customAttrClass, "ExampleCustomAttribute");
    customAttrClass->SetClassModifier(ECClassModifier::Sealed);
    customAttrClass->SetContainerType(ECN::CustomAttributeContainerType::Schema | ECN::CustomAttributeContainerType::AnyProperty);

    BeJsDocument schemaJson;
    EXPECT_TRUE(customAttrClass->ToJson(schemaJson, true));

    BeJsDocument testDataJson;
    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneECCustomAttributeClass.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassTest, SerializeClassWithProperties)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);
    schema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());

    ECEntityClassP entityClass;
    schema->CreateEntityClass(entityClass, "ExampleEntityClass");

    PropertyCategoryP propertyCategory;
    schema->CreatePropertyCategory(propertyCategory, "ExamplePropertyCategory");

    ECEnumerationP enumeration;
    schema->CreateEnumeration(enumeration, "ExampleEnumeration", PrimitiveType::PRIMITIVETYPE_Integer);

    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKindOfQuantity");
    koq->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M"));

    ECStructClassP structClass;
    schema->CreateStructClass(structClass, "ExampleStructClass");

    ECRelationshipClassP relationshipClass;
    schema->CreateRelationshipClass(relationshipClass, "ExampleRelationshipClass");
    relationshipClass->GetSource().SetRoleLabel("SourceRoleLabel");
    relationshipClass->GetSource().AddClass(*entityClass);
    relationshipClass->GetTarget().SetRoleLabel("TargetRoleLabel");
    relationshipClass->GetTarget().AddClass(*entityClass);

    PrimitiveECPropertyP primProp;
    entityClass->CreatePrimitiveProperty(primProp, "ExamplePrimitiveProperty", PrimitiveType::PRIMITIVETYPE_Integer);
    primProp->SetDescription("An example primitive property.");
    primProp->SetDisplayLabel("PrimitivePropertyDisplayLabel");
    primProp->SetExtendedTypeName("Example Primitive Property");
    primProp->SetIsReadOnly(true);
    primProp->SetCategory(propertyCategory);
    primProp->SetKindOfQuantity(koq);
    primProp->SetMinimumValue(ECValue(42));
    primProp->SetMaximumValue(ECValue(1999));

    PrimitiveECPropertyP enumProp;
    entityClass->CreateEnumerationProperty(enumProp, "ExampleEnumerationProperty", *enumeration);
    enumProp->SetDescription("An example primitive enumeration property.");
    enumProp->SetDisplayLabel("Enumeration Property DisplayLabel");
    enumProp->SetExtendedTypeName("Example_ExtendedType");
    enumProp->SetIsReadOnly(true);
    enumProp->SetCategory(propertyCategory);
    enumProp->SetMinimumValue(ECValue(42));
    enumProp->SetMaximumValue(ECValue(1999));

    StructECPropertyP structProp;
    entityClass->CreateStructProperty(structProp, "ExampleStructProperty", *structClass);

    PrimitiveArrayECPropertyP primArrProp;
    entityClass->CreatePrimitiveArrayProperty(primArrProp, "ExamplePrimitiveArrayProperty", PrimitiveType::PRIMITIVETYPE_String);
    primArrProp->SetMinimumLength(3);
    primArrProp->SetMaximumLength(50);
    primArrProp->SetMinOccurs(7);

    StructArrayECPropertyP structArrProp;
    entityClass->CreateStructArrayProperty(structArrProp, "ExampleStructArrayProperty", *structClass);
    structArrProp->SetMinOccurs(867);
    structArrProp->SetMaxOccurs(5309);

    NavigationECPropertyP navProp;
    entityClass->CreateNavigationProperty(navProp, "ExampleNavigationProperty", *relationshipClass, ECRelatedInstanceDirection::Backward, false);

    BeJsDocument schemaJson;
    EXPECT_TRUE(entityClass->ToJson(schemaJson, true)); 
    BeJsDocument testDataJson;
    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/SchemaWithClassProperties.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassTest, InheritedCustomAttributesAndPropertiesShouldNotBeSerializedByDefault)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);

    PrimitiveECPropertyP primProp;

    PropertyCategoryP propertyCategory;
    schema->CreatePropertyCategory(propertyCategory, "ExamplePropertyCategory");

    KindOfQuantityP kindOfQuantity;
    schema->CreateKindOfQuantity(kindOfQuantity, "ExampleKoQ");

    ECCustomAttributeClassP customAttrClassBP;
    schema->CreateCustomAttributeClass(customAttrClassBP, "CustomAttributeOnBaseProperty");
    IECInstancePtr customAttrBP = customAttrClassBP->GetDefaultStandaloneEnabler()->CreateInstance();

    ECCustomAttributeClassP customAttrClassBC;
    schema->CreateCustomAttributeClass(customAttrClassBC, "CustomAttributeOnBaseClass");
    IECInstancePtr customAttrBC = customAttrClassBC->GetDefaultStandaloneEnabler()->CreateInstance();

    ECCustomAttributeClassP customAttrClassRelCon;
    schema->CreateCustomAttributeClass(customAttrClassRelCon, "CustomAttributeOnRelationshipConstraint");
    IECInstancePtr customAttrRelCon = customAttrClassRelCon->GetDefaultStandaloneEnabler()->CreateInstance();

    ECEntityClassP baseEntityClass;
    schema->CreateEntityClass(baseEntityClass, "BaseEntityClass");
    baseEntityClass->SetCustomAttribute(*customAttrBC);
    baseEntityClass->CreatePrimitiveProperty(primProp, "ExamplePrimitiveProperty", PrimitiveType::PRIMITIVETYPE_Integer);
    primProp->SetExtendedTypeName("FooBar");
    primProp->SetKindOfQuantity(kindOfQuantity);
    primProp->SetPriority(5);
    primProp->SetCategory(propertyCategory);
    primProp->SetCustomAttribute(*customAttrBP);

    ECEntityClassP derivedEntityClass;
    schema->CreateEntityClass(derivedEntityClass, "DerivedEntityClass");
    derivedEntityClass->AddBaseClass(*baseEntityClass);

    BeJsDocument entityClassJson;
    EXPECT_TRUE(derivedEntityClass->ToJson(entityClassJson, true));

    BeJsDocument testDataJson;
    BeFileName entityClassTestDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneClassDefaultSerializeInheritedCustomAttributesAndProperties.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, entityClassTestDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);
    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(entityClassJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(entityClassJson, testDataJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassTest, ExplicitSerializeInheritedProperties)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);

    PrimitiveECPropertyP primPropNonOverrideBase;
    PrimitiveECPropertyP primPropOverrideBase;
    PrimitiveECPropertyP primPropOverrideDerived;

    ECEntityClassP baseEntityClass;
    schema->CreateEntityClass(baseEntityClass, "BaseEntityClass");
    baseEntityClass->CreatePrimitiveProperty(primPropNonOverrideBase, "PrimitivePropertyNonOverride", PrimitiveType::PRIMITIVETYPE_Integer);
    baseEntityClass->CreatePrimitiveProperty(primPropOverrideBase, "PrimitivePropertyOverride", PrimitiveType::PRIMITIVETYPE_String);
    primPropOverrideBase->SetMinimumLength(0);
    primPropOverrideBase->SetMaximumLength(10);

    ECEntityClassP derivedEntityClass;
    schema->CreateEntityClass(derivedEntityClass, "DerivedEntityClass");
    derivedEntityClass->AddBaseClass(*baseEntityClass);
    derivedEntityClass->CreatePrimitiveProperty(primPropOverrideDerived, "PrimitivePropertyOverride", PrimitiveType::PRIMITIVETYPE_String);
    primPropOverrideDerived->SetMinimumLength(3);
    primPropOverrideDerived->SetMaximumLength(12);

    BeJsDocument entityClassJson;
    EXPECT_TRUE(derivedEntityClass->ToJson(entityClassJson, true, true));

    BeJsDocument testDataJson;
    BeFileName entityClassTestDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneClassExplicitlySerializeInheritedProperties.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, entityClassTestDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);
    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(entityClassJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(entityClassJson, testDataJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ClassTest, LookupClassTest)
    {
    Utf8CP baseSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="CoreCustomAttributes" version="1.00.00" alias="CoreCA"/>
                <ECEntityClass typeName="BaseClass">
                    <ECCustomAttributes>
                        <NotSubClassableInReferencingSchemas xmlns="CoreCustomAttributes.01.00"/>
                    </ECCustomAttributes>
                </ECEntityClass>
                <ECEntityClass typeName="LocalDerivedClass">
                    <BaseClass>BaseClass</BaseClass>
                </ECEntityClass>
            </ECSchema>)xml";
    
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, baseSchemaXml, *context));
    ASSERT_TRUE(schema.IsValid());
    ECSchemaPtr refingSchema;
    ASSERT_TRUE(!refingSchema.IsValid());

    Utf8CP goodRefSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="RefingSchema" alias="RS" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="TestSchema" version="1.00.00" alias="ts"/>
                <ECEntityClass typeName="RemoteDerivedClass">
                    <BaseClass>ts:LocalDerivedClass</BaseClass>
                </ECEntityClass>
            </ECSchema>)xml";

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refingSchema, goodRefSchemaXml, *context));
    ASSERT_TRUE(refingSchema.IsValid());

    auto shouldBeNull = refingSchema->LookupClass("");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = refingSchema->LookupClass("banana");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = refingSchema->LookupClass("banana:RemoteDerivedClass");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = refingSchema->LookupClass("banana:BaseClass");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = refingSchema->LookupClass("TestSchema:BaseClass");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = refingSchema->LookupClass("RefingSchema:RemoteDerivedClass");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = refingSchema->LookupClass("rs:RemoteDerivedClass", true);
    EXPECT_EQ(nullptr, shouldBeNull);
    auto shouldNotBeNull = refingSchema->LookupClass("RemoteDerivedClass");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("RemoteDerivedClass", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = refingSchema->LookupClass("ts:BaseClass");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("BaseClass", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = refingSchema->LookupClass("TestSchema:BaseClass", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("BaseClass", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = refingSchema->LookupClass("RefingSchema:RemoteDerivedClass", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("RemoteDerivedClass", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = refingSchema->LookupClass("rs:RemoteDerivedClass");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("RemoteDerivedClass", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = refingSchema->LookupClass("TS:BaseClass");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("BaseClass", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = refingSchema->LookupClass("TESTSCHEMA:BaseClass", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("BaseClass", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = refingSchema->LookupClass("REFINGSCHEMA:RemoteDerivedClass", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("RemoteDerivedClass", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = refingSchema->LookupClass("RS:RemoteDerivedClass");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("RemoteDerivedClass", shouldNotBeNull->GetName().c_str());
    ASSERT_EQ(1, refingSchema->GetClassCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ClassTest, DowncastClassTest) 
    {
    ECClass *ecClass;

    ECSchemaPtr ecSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(ecSchema, "TestSchema", "TestSchema", 3, 1, 0));

    ECEntityClassP ecEntity;
    EC_ASSERT_SUCCESS(ecSchema->CreateEntityClass(ecEntity, "TestEntity"));
    ecClass = ecSchema->GetClassP("TestEntity");
    ASSERT_TRUE(ecClass->IsEntityClass());
    ASSERT_FALSE(ecClass->IsRelationshipClass());
    ASSERT_FALSE(ecClass->IsStructClass());
    ASSERT_FALSE(ecClass->IsCustomAttributeClass());
    ASSERT_FALSE(ecClass->IsMixin());
    ASSERT_NE(ecClass->GetEntityClassCP(), nullptr);
    ASSERT_NE(ecClass->GetEntityClassP(), nullptr);
    ASSERT_EQ(ecClass->GetRelationshipClassP(), nullptr);
    ASSERT_EQ(ecClass->GetRelationshipClassP(), nullptr);
    ASSERT_EQ(ecClass->GetStructClassCP(), nullptr);
    ASSERT_EQ(ecClass->GetStructClassP(), nullptr);
    ASSERT_EQ(ecClass->GetCustomAttributeClassCP(), nullptr);
    ASSERT_EQ(ecClass->GetCustomAttributeClassP(), nullptr);

    ECRelationshipClassP ecRelationship;
    EC_ASSERT_SUCCESS(ecSchema->CreateRelationshipClass(ecRelationship, "TestRelationship", "TestRelationship"));
    ecClass = ecSchema->GetClassP("TestRelationship");
    ASSERT_FALSE(ecClass->IsEntityClass());
    ASSERT_TRUE(ecClass->IsRelationshipClass());
    ASSERT_FALSE(ecClass->IsStructClass());
    ASSERT_FALSE(ecClass->IsCustomAttributeClass());
    ASSERT_FALSE(ecClass->IsMixin());
    ASSERT_EQ(ecClass->GetEntityClassCP(), nullptr);
    ASSERT_EQ(ecClass->GetEntityClassP(), nullptr);
    ASSERT_NE(ecClass->GetRelationshipClassCP(), nullptr);
    ASSERT_NE(ecClass->GetRelationshipClassP(), nullptr);
    ASSERT_EQ(ecClass->GetStructClassCP(), nullptr);
    ASSERT_EQ(ecClass->GetStructClassP(), nullptr);
    ASSERT_EQ(ecClass->GetCustomAttributeClassCP(), nullptr);
    ASSERT_EQ(ecClass->GetCustomAttributeClassP(), nullptr);

    ECStructClassP ecStruct;
    EC_ASSERT_SUCCESS(ecSchema->CreateStructClass(ecStruct, "TestStruct"));
    ecClass = ecSchema->GetClassP("TestStruct");
    ASSERT_FALSE(ecClass->IsEntityClass());
    ASSERT_FALSE(ecClass->IsRelationshipClass());
    ASSERT_TRUE(ecClass->IsStructClass());
    ASSERT_FALSE(ecClass->IsCustomAttributeClass());
    ASSERT_FALSE(ecClass->IsMixin());
    ASSERT_EQ(ecClass->GetEntityClassCP(), nullptr);
    ASSERT_EQ(ecClass->GetEntityClassP(), nullptr);
    ASSERT_EQ(ecClass->GetRelationshipClassP(), nullptr);
    ASSERT_EQ(ecClass->GetRelationshipClassP(), nullptr);
    ASSERT_NE(ecClass->GetStructClassCP(), nullptr);
    ASSERT_NE(ecClass->GetStructClassP(), nullptr);
    ASSERT_EQ(ecClass->GetCustomAttributeClassCP(), nullptr);
    ASSERT_EQ(ecClass->GetCustomAttributeClassP(), nullptr);

    ECCustomAttributeClassP ecCustomAttribute;
    EC_ASSERT_SUCCESS(ecSchema->CreateCustomAttributeClass(ecCustomAttribute, "TestCustomAttribute"));
    ecClass = ecSchema->GetClassP("TestCustomAttribute");
    ASSERT_FALSE(ecClass->IsEntityClass());
    ASSERT_FALSE(ecClass->IsRelationshipClass());
    ASSERT_FALSE(ecClass->IsStructClass());
    ASSERT_TRUE(ecClass->IsCustomAttributeClass());
    ASSERT_FALSE(ecClass->IsMixin());
    ASSERT_EQ(ecClass->GetEntityClassCP(), nullptr);
    ASSERT_EQ(ecClass->GetEntityClassP(), nullptr);
    ASSERT_EQ(ecClass->GetRelationshipClassCP(), nullptr);
    ASSERT_EQ(ecClass->GetRelationshipClassP(), nullptr);
    ASSERT_EQ(ecClass->GetStructClassCP(), nullptr);
    ASSERT_EQ(ecClass->GetStructClassP(), nullptr);
    ASSERT_NE(ecClass->GetCustomAttributeClassCP(), nullptr);
    ASSERT_NE(ecClass->GetCustomAttributeClassP(), nullptr);

    ECEntityClassP ecMixin;
    EC_ASSERT_SUCCESS(ecSchema->CreateMixinClass(ecMixin, "TestMixin", *ecEntity));
    ecClass = ecSchema->GetClassP("TestMixin");
    ASSERT_TRUE(ecClass->IsEntityClass());
    ASSERT_FALSE(ecClass->IsRelationshipClass());
    ASSERT_FALSE(ecClass->IsStructClass());
    ASSERT_FALSE(ecClass->IsCustomAttributeClass());
    ASSERT_TRUE(ecClass->IsMixin());
    ASSERT_NE(ecClass->GetEntityClassCP(), nullptr);
    ASSERT_NE(ecClass->GetEntityClassP(), nullptr);
    ASSERT_EQ(ecClass->GetRelationshipClassCP(), nullptr);
    ASSERT_EQ(ecClass->GetRelationshipClassP(), nullptr);
    ASSERT_EQ(ecClass->GetStructClassCP(), nullptr);
    ASSERT_EQ(ecClass->GetStructClassP(), nullptr);
    ASSERT_EQ(ecClass->GetCustomAttributeClassCP(), nullptr);
    ASSERT_EQ(ecClass->GetCustomAttributeClassP(), nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ClassTest, DeleteMixinAppliesTo)
    {
    ECClass *ecClass;
    ECSchemaPtr ecSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(ecSchema, "TestSchema", "TestSchema", 4, 1, 1));

    ECEntityClassP ecEntity;
    EC_ASSERT_SUCCESS(ecSchema->CreateEntityClass(ecEntity, "TestEntity"));

    ECEntityClassP ecMixin;
    EC_ASSERT_SUCCESS(ecSchema->CreateMixinClass(ecMixin, "TestMixin", *ecEntity));
    ecClass = ecSchema->GetClassP("TestMixin");
    ASSERT_TRUE(ecClass->IsEntityClass());
    ASSERT_FALSE(ecClass->IsRelationshipClass());
    ASSERT_FALSE(ecClass->IsStructClass());
    ASSERT_FALSE(ecClass->IsCustomAttributeClass());
    ASSERT_TRUE(ecClass->IsMixin());
    ASSERT_NE(ecClass->GetEntityClassP(), nullptr);
    ASSERT_EQ(ecClass->GetRelationshipClassP(), nullptr);
    ASSERT_EQ(ecClass->GetStructClassP(), nullptr);
    ASSERT_EQ(ecClass->GetCustomAttributeClassP(), nullptr);
    }


//=======================================================================================
//! PropertyCopyTest
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void PropertyCopyTest::SetUp()
    {
    // m_schema0 and m_entity0 will contain the property to be copied
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(m_schema0, "Schema0", "s0", 1, 0, 0));
    ASSERT_TRUE(m_schema0.IsValid());
    EC_ASSERT_SUCCESS(m_schema0->CreateEntityClass(m_entity0, "Entity"));

    // m_schema1 and m_entity1 will contain the copied property
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(m_schema1, "Schema1", "s1", 1, 0, 0));
    ASSERT_TRUE(m_schema1.IsValid());

    ECTestFixture::SetUp();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void PropertyCopyTest::CopyProperty(bool copyReferences)
    {
    if (copyReferences)
        {
        ECClassP ecClass;
        EC_EXPECT_SUCCESS(m_schema1->CopyClass(ecClass, *m_entity0, true));
        ASSERT_TRUE(nullptr != ecClass);
        m_entity1 = ecClass->GetEntityClassP();
        ASSERT_TRUE(nullptr != m_entity1);
        m_prop1 = m_entity1->GetPropertyP(m_prop0->GetName().c_str());
        }
    else
        {
        EC_ASSERT_SUCCESS(m_schema1->CreateEntityClass(m_entity1, "Entity"));
        ASSERT_TRUE(nullptr != m_entity1);
        EC_EXPECT_SUCCESS(m_entity1->CopyProperty(m_prop1, m_prop0, true));
        }

    ASSERT_TRUE(nullptr != m_prop1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, FailToCopyWithoutSourceProperty)
    {
    PrimitiveECPropertyCP prop = nullptr;
    
    ECPropertyP copiedProp;
    EXPECT_EQ(ECObjectsStatus::NullPointerValue, m_entity0->CopyProperty(copiedProp, prop, true));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, BaseFullyDefinedECProperty)
    {
    PropertyCategoryP propCategory;
    m_schema0->CreatePropertyCategory(propCategory, "TestPropCategory");

    KindOfQuantityP koq;
    m_schema0->CreateKindOfQuantity(koq, "TestKoQ");

    PrimitiveECPropertyP primProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveProperty(primProp, "Prop", PRIMITIVETYPE_String));

    EC_ASSERT_SUCCESS(primProp->SetDescription("Property Description"));
    EC_ASSERT_SUCCESS(primProp->SetDisplayLabel("Property Display Label"));
    EC_ASSERT_SUCCESS(primProp->SetIsReadOnly(true));
    EC_ASSERT_SUCCESS(primProp->SetPriority(5));
    EC_ASSERT_SUCCESS(primProp->SetTypeName("string"));
    EC_ASSERT_SUCCESS(primProp->SetCategory(propCategory));
    EC_ASSERT_SUCCESS(primProp->SetKindOfQuantity(koq));

    m_prop0 = primProp;

    CopyProperty();

    ASSERT_TRUE(nullptr != m_prop1);
    ASSERT_TRUE(m_prop0 != m_prop1);
    EXPECT_STREQ("Prop", m_prop1->GetName().c_str());
    EXPECT_STREQ("Property Description", m_prop1->GetDescription().c_str());
    EXPECT_STREQ("Property Display Label", m_prop1->GetDisplayLabel().c_str());
    EXPECT_TRUE(m_prop1->GetIsReadOnly());
    EXPECT_EQ(5, m_prop1->GetPriority());
    EXPECT_STREQ("string", m_prop1->GetTypeName().c_str());

    PropertyCategoryCP destPropCategory = m_schema1->GetPropertyCategoryCP("TestPropCategory");
    EXPECT_TRUE(nullptr != destPropCategory);
    EXPECT_EQ(destPropCategory, m_prop1->GetCategory());

    KindOfQuantityCP destKoQ = m_schema1->GetKindOfQuantityCP("TestKoQ");
    EXPECT_TRUE(nullptr != destKoQ);
    EXPECT_EQ(destKoQ, m_prop1->GetKindOfQuantity());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, PropertyWithCategoryInSameSchemaWithoutCopyingTypes) 
    {
    PrimitiveECPropertyP primProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveProperty(primProp, "Prop", PRIMITIVETYPE_String));

    PropertyCategoryP propCategory;
    EC_ASSERT_SUCCESS(m_schema0->CreatePropertyCategory(propCategory, "TestPropCategory"));

    EC_ASSERT_SUCCESS(primProp->SetCategory(propCategory));

    m_prop0 = primProp;

    CopyProperty(false);

    ASSERT_TRUE(nullptr != m_prop1);
    ASSERT_TRUE(nullptr != m_prop1->GetCategory());
    EXPECT_STREQ("Schema0", m_prop1->GetCategory()->GetSchema().GetName().c_str());
    EXPECT_EQ(propCategory, m_prop1->GetCategory());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, PropertyWithCategoryInRefSchema) 
    {
    PrimitiveECPropertyP primProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveProperty(primProp, "Prop", PRIMITIVETYPE_String));

    PropertyCategoryP refCategory;
    ECSchemaPtr refSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(refSchema, "RefSchema", "ref", 1, 0, 0));
    EC_ASSERT_SUCCESS(refSchema->CreatePropertyCategory(refCategory, "PropertyCategory"));
    EC_ASSERT_SUCCESS(m_schema0->AddReferencedSchema(*refSchema));

    primProp->SetCategory(refCategory);

    m_prop0 = primProp;

    EC_ASSERT_SUCCESS(m_schema1->AddReferencedSchema(*refSchema));

    CopyProperty();

    ASSERT_TRUE(nullptr != m_prop1);
    ASSERT_TRUE(nullptr != m_prop1->GetCategory());
    EXPECT_STREQ("RefSchema", m_prop1->GetCategory()->GetSchema().GetName().c_str());
    EXPECT_EQ(refCategory, m_prop1->GetCategory());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, PropertyWithKindOfQuantityInSameSchemaWithoutCopyingTypes) 
    {
    PrimitiveECPropertyP primProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveProperty(primProp, "Prop", PRIMITIVETYPE_String));

    KindOfQuantityP koq;
    m_schema0->CreateKindOfQuantity(koq, "TestKoQ");

    EC_ASSERT_SUCCESS(primProp->SetKindOfQuantity(koq));

    m_prop0 = primProp;

    CopyProperty(false);

    ASSERT_TRUE(nullptr != m_prop1);
    ASSERT_TRUE(nullptr != m_prop1->GetKindOfQuantity());
    EXPECT_STREQ("Schema0", m_prop1->GetKindOfQuantity()->GetSchema().GetName().c_str());
    EXPECT_EQ(koq, m_prop1->GetKindOfQuantity());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, PropertyWithKindOfQuantityInRefSchema) 
    {
    PrimitiveECPropertyP primProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveProperty(primProp, "Prop", PRIMITIVETYPE_String));

    KindOfQuantityP refKoQ;
    ECSchemaPtr refSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(refSchema, "RefSchema", "ref", 1, 0, 0));
    EC_ASSERT_SUCCESS(refSchema->CreateKindOfQuantity(refKoQ, "RefKoQ"));
    EC_ASSERT_SUCCESS(m_schema0->AddReferencedSchema(*refSchema));

    primProp->SetKindOfQuantity(refKoQ);

    m_prop0 = primProp;

    EC_ASSERT_SUCCESS(m_schema1->AddReferencedSchema(*refSchema));

    CopyProperty();

    ASSERT_TRUE(nullptr != m_prop1);
    ASSERT_TRUE(nullptr != m_prop1->GetKindOfQuantity());
    EXPECT_STREQ("RefSchema", m_prop1->GetKindOfQuantity()->GetSchema().GetName().c_str());
    EXPECT_EQ(refKoQ, m_prop1->GetKindOfQuantity());
    }

//---------------------------------------------------------------------------------------
// Maximum and Minimum length are only supported for Primitive types string and binary.
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, PrimitivePropertyWithMaxAndMinLength)
    {
    PrimitiveECPropertyP primProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveProperty(primProp, "Prop", PRIMITIVETYPE_String));

    EC_ASSERT_SUCCESS(primProp->SetTypeName("string"));
    EC_ASSERT_SUCCESS(primProp->SetMinimumLength(1));
    EC_ASSERT_SUCCESS(primProp->SetMaximumLength(10));

    m_prop0 = primProp;

    CopyProperty();

    EXPECT_STREQ("string", m_prop1->GetTypeName().c_str());
    EXPECT_EQ(1, m_prop1->GetMinimumLength());
    EXPECT_EQ(10, m_prop1->GetMaximumLength());
    }

//---------------------------------------------------------------------------------------
// Maximum and Minimum value are only supported for Primitive types integer, long and double.
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, PrimitivePropertyWithMaxAndMinValue)
    {
    PrimitiveECPropertyP primProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveProperty(primProp, "Prop", PRIMITIVETYPE_String));

    EC_ASSERT_SUCCESS(primProp->SetTypeName("int"));
    ECValue minValue (5);
    EC_ASSERT_SUCCESS(primProp->SetMinimumValue(minValue));
    ECValue maxValue (25);
    EC_ASSERT_SUCCESS(primProp->SetMaximumValue(maxValue));

    m_prop0 = primProp;

    CopyProperty();

    EXPECT_STREQ("int", m_prop1->GetTypeName().c_str());
    ECValue copiedMinValue;
    ECValue expectMinValue(5);
    EC_EXPECT_SUCCESS(m_prop1->GetMinimumValue(copiedMinValue));
    EXPECT_EQ(expectMinValue, copiedMinValue);

    ECValue copiedMaxValue;
    ECValue expectMaxValue(25);
    EC_EXPECT_SUCCESS(m_prop1->GetMaximumValue(copiedMaxValue));
    EXPECT_EQ(expectMaxValue, copiedMaxValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, PrimitivePropertyWithExtendedType) 
    {
    PrimitiveECPropertyP primProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveProperty(primProp, "Prop", PRIMITIVETYPE_String));

    EC_ASSERT_SUCCESS(primProp->SetTypeName("string"));
    EC_ASSERT_SUCCESS(primProp->SetExtendedTypeName("Json"));

    m_prop0 = primProp;

    CopyProperty();

    EXPECT_STREQ("string", m_prop1->GetTypeName().c_str());
    PrimitiveECPropertyCP primProp1 = m_prop1->GetAsPrimitiveProperty();
    EXPECT_TRUE(primProp1->IsExtendedTypeDefinedLocally());
    EXPECT_STREQ("Json", primProp1->GetExtendedTypeName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, PrimitivePropertyWithEnumerationInSameSchema) 
    {
    PrimitiveECPropertyP primProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveProperty(primProp, "Prop", PRIMITIVETYPE_String));

    ECEnumerationP sourceEnum;
    EC_ASSERT_SUCCESS(m_schema0->CreateEnumeration(sourceEnum, "TestEnum", PrimitiveType::PRIMITIVETYPE_Integer));

    EC_ASSERT_SUCCESS(primProp->SetType(*sourceEnum));

    m_prop0 = primProp;

    CopyProperty();

    EXPECT_STREQ("TestEnum", m_prop1->GetTypeName().c_str());
    PrimitiveECPropertyCP primProp1 = m_prop1->GetAsPrimitiveProperty();
    
    ECEnumerationCP destEnum = m_schema1->GetEnumerationCP("TestEnum");
    EXPECT_TRUE(nullptr != destEnum);
    EXPECT_TRUE(nullptr != primProp1->GetEnumeration());
    EXPECT_EQ(destEnum, primProp1->GetEnumeration());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, PrimitivePropertyWithEnumerationInSameSchemaWithoutCopyingTypes) 
    {
    PrimitiveECPropertyP primProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveProperty(primProp, "Prop", PRIMITIVETYPE_String));

    ECEnumerationP sourceEnum;
    EC_ASSERT_SUCCESS(m_schema0->CreateEnumeration(sourceEnum, "TestEnum", PrimitiveType::PRIMITIVETYPE_Integer));

    EC_ASSERT_SUCCESS(primProp->SetType(*sourceEnum));

    m_prop0 = primProp;

    CopyProperty(false);

    PrimitiveECPropertyCP primProp1 = m_prop1->GetAsPrimitiveProperty();
    
    ECEnumerationCP destEnum = m_schema1->GetEnumerationCP("TestEnum");
    EXPECT_TRUE(nullptr == destEnum);
    EXPECT_TRUE(nullptr != primProp1->GetEnumeration());
    EXPECT_EQ(sourceEnum, primProp1->GetEnumeration());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, PrimitivePropertyWithEnumerationInRefSchema)
    {
    PrimitiveECPropertyP primProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveProperty(primProp, "Prop", PRIMITIVETYPE_String));

    ECEnumerationP refEnum;
    ECSchemaPtr refSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(refSchema, "RefSchema", "ref", 1, 0, 0));
    EC_ASSERT_SUCCESS(refSchema->CreateEnumeration(refEnum, "TestEnum", PrimitiveType::PRIMITIVETYPE_Integer));
    EC_ASSERT_SUCCESS(m_schema0->AddReferencedSchema(*refSchema));

    EC_ASSERT_SUCCESS(primProp->SetType(*refEnum));

    m_prop0 = primProp;

    EC_ASSERT_SUCCESS(m_schema1->AddReferencedSchema(*refSchema));

    CopyProperty();

    PrimitiveECPropertyCP primProp1 = m_prop1->GetAsPrimitiveProperty();

    EXPECT_TRUE(nullptr != primProp1->GetEnumeration());
    EXPECT_STREQ("RefSchema", primProp1->GetEnumeration()->GetSchema().GetName().c_str());
    EXPECT_EQ(refEnum, primProp1->GetEnumeration());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, PrimitiveArrayPropertyWithMaxAndMinOccurs)
    {
    PrimitiveArrayECPropertyP primArrProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveArrayProperty(primArrProp, "Prop"));
    EC_ASSERT_SUCCESS(primArrProp->SetTypeName("string"));
    EC_ASSERT_SUCCESS(primArrProp->SetMinOccurs(5));
    EC_ASSERT_SUCCESS(primArrProp->SetMaxOccurs(15));

    m_prop0 = primArrProp;

    CopyProperty();

    EXPECT_STREQ("string", m_prop1->GetTypeName().c_str());
    PrimitiveArrayECPropertyCP primProp1 = m_prop1->GetAsPrimitiveArrayProperty();
    EXPECT_EQ(5, primProp1->GetMinOccurs());
    EXPECT_EQ(15, primProp1->GetStoredMaxOccurs());
    }

//---------------------------------------------------------------------------------------
// Maximum and Minimum length are only supported for Primitive types string and binary.
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, PrimitiveArrayPropertyWithMaxAndMinLength)
    {
    PrimitiveArrayECPropertyP primArrProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveArrayProperty(primArrProp, "Prop"));

    EC_ASSERT_SUCCESS(primArrProp->SetTypeName("string"));
    EC_ASSERT_SUCCESS(primArrProp->SetMinimumLength(1));
    EC_ASSERT_SUCCESS(primArrProp->SetMaximumLength(10));

    m_prop0 = primArrProp;

    CopyProperty();

    EXPECT_STREQ("string", m_prop1->GetTypeName().c_str());
    EXPECT_EQ(1, m_prop1->GetMinimumLength());
    EXPECT_EQ(10, m_prop1->GetMaximumLength());
    }

//---------------------------------------------------------------------------------------
// Maximum and Minimum value are only supported for Primitive types integer, long and double.
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, PrimitiveArrayPropertyWithMaxAndMinValue)
    {
    PrimitiveArrayECPropertyP primArrProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveArrayProperty(primArrProp, "Prop"));

    EC_ASSERT_SUCCESS(primArrProp->SetTypeName("int"));
    ECValue minValue (5);
    EC_ASSERT_SUCCESS(primArrProp->SetMinimumValue(minValue));
    ECValue maxValue (25);
    EC_ASSERT_SUCCESS(primArrProp->SetMaximumValue(maxValue));

    m_prop0 = primArrProp;

    CopyProperty();

    EXPECT_STREQ("int", m_prop1->GetTypeName().c_str());
    ECValue copiedMinValue;
    ECValue expectMinValue(5);
    EC_EXPECT_SUCCESS(m_prop1->GetMinimumValue(copiedMinValue));
    EXPECT_EQ(expectMinValue, copiedMinValue);

    ECValue copiedMaxValue;
    ECValue expectMaxValue(25);
    EC_EXPECT_SUCCESS(m_prop1->GetMaximumValue(copiedMaxValue));
    EXPECT_EQ(expectMaxValue, copiedMaxValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, PrimitiveArrayPropertyWithExtendedType) 
    {
    PrimitiveArrayECPropertyP primArrProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveArrayProperty(primArrProp, "Prop"));

    EC_ASSERT_SUCCESS(primArrProp->SetTypeName("string"));
    EC_ASSERT_SUCCESS(primArrProp->SetExtendedTypeName("Json"));

    m_prop0 = primArrProp;

    CopyProperty();

    EXPECT_STREQ("string", m_prop1->GetTypeName().c_str());
    PrimitiveArrayECPropertyCP primProp1 = m_prop1->GetAsPrimitiveArrayProperty();
    EXPECT_TRUE(primProp1->IsExtendedTypeDefinedLocally());
    EXPECT_STREQ("Json", primProp1->GetExtendedTypeName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, PrimitiveArrayPropertyWithEnumerationInSameSchema)
    {
    PrimitiveArrayECPropertyP primArrProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveArrayProperty(primArrProp, "Prop"));

    ECEnumerationP sourceEnum;
    EC_ASSERT_SUCCESS(m_schema0->CreateEnumeration(sourceEnum, "TestEnum", PrimitiveType::PRIMITIVETYPE_Integer));

    EC_ASSERT_SUCCESS(primArrProp->SetType(*sourceEnum));

    m_prop0 = primArrProp;

    CopyProperty();

    EXPECT_STREQ("TestEnum", m_prop1->GetTypeName().c_str());
    PrimitiveArrayECPropertyCP primProp1 = m_prop1->GetAsPrimitiveArrayProperty();
    
    ECEnumerationCP destEnum = m_schema1->GetEnumerationCP("TestEnum");
    EXPECT_TRUE(nullptr != destEnum);
    EXPECT_TRUE(nullptr != primProp1->GetEnumeration());
    EXPECT_EQ(destEnum, primProp1->GetEnumeration());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, PrimitiveArrayPropertyWithEnumerationInSameSchemaWithoutCopyingTypes)
    {
    PrimitiveArrayECPropertyP primArrProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveArrayProperty(primArrProp, "Prop"));

    ECEnumerationP sourceEnum;
    EC_ASSERT_SUCCESS(m_schema0->CreateEnumeration(sourceEnum, "TestEnum", PrimitiveType::PRIMITIVETYPE_Integer));

    EC_ASSERT_SUCCESS(primArrProp->SetType(*sourceEnum));

    m_prop0 = primArrProp;

    CopyProperty(false);

    PrimitiveArrayECPropertyCP primProp1 = m_prop1->GetAsPrimitiveArrayProperty();
    
    ECEnumerationCP destEnum = m_schema1->GetEnumerationCP("TestEnum");
    EXPECT_TRUE(nullptr == destEnum);
    EXPECT_TRUE(nullptr != primProp1->GetEnumeration());
    EXPECT_EQ(sourceEnum, primProp1->GetEnumeration());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, PrimitiveArrayPropertyWithEnumerationInRefSchema)
    {
    PrimitiveArrayECPropertyP primArrProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveArrayProperty(primArrProp, "Prop"));

    ECEnumerationP refEnum;
    ECSchemaPtr refSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(refSchema, "RefSchema", "ref", 1, 0, 0));
    EC_ASSERT_SUCCESS(refSchema->CreateEnumeration(refEnum, "TestEnum", PrimitiveType::PRIMITIVETYPE_Integer));
    EC_ASSERT_SUCCESS(m_schema0->AddReferencedSchema(*refSchema));

    EC_ASSERT_SUCCESS(primArrProp->SetType(*refEnum));

    m_prop0 = primArrProp;

    EC_ASSERT_SUCCESS(m_schema1->AddReferencedSchema(*refSchema));

    CopyProperty();

    PrimitiveArrayECPropertyCP primProp1 = m_prop1->GetAsPrimitiveArrayProperty();

    EXPECT_TRUE(nullptr != primProp1->GetEnumeration());
    EXPECT_STREQ("RefSchema", primProp1->GetEnumeration()->GetSchema().GetName().c_str());
    EXPECT_EQ(refEnum, primProp1->GetEnumeration());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, StructPropertyWithStructClassInSameSchema)
    {
    ECStructClassP structClass;
    EC_ASSERT_SUCCESS(m_schema0->CreateStructClass(structClass, "Struct"));

    StructECPropertyP structProp;
    EC_ASSERT_SUCCESS(m_entity0->CreateStructProperty(structProp, "StructProp", *structClass));

    m_prop0 = structProp;

    CopyProperty();

    ECClassCP ecClass = m_schema1->GetClassCP("Struct");
    ASSERT_TRUE(nullptr != ecClass);
    ECStructClassCP destStructClass = ecClass->GetStructClassCP();
    ASSERT_TRUE(nullptr != destStructClass);

    StructECPropertyCP destStructProp = m_prop1->GetAsStructProperty();
    ASSERT_TRUE(nullptr != destStructProp);
    EXPECT_STREQ(m_schema1->GetName().c_str(), destStructProp->GetType().GetSchema().GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, StructPropertyWithStructClassInSameSchemaWithoutCopyingTypes)
    {
    ECStructClassP structClass;
    EC_ASSERT_SUCCESS(m_schema0->CreateStructClass(structClass, "Struct"));

    StructECPropertyP structProp;
    EC_ASSERT_SUCCESS(m_entity0->CreateStructProperty(structProp, "StructProp", *structClass));

    m_prop0 = structProp;

    CopyProperty(false);

    StructECPropertyCP destStructProp = m_prop1->GetAsStructProperty();
    ASSERT_TRUE(nullptr != destStructProp);
    EXPECT_STREQ(m_schema0->GetName().c_str(), destStructProp->GetType().GetSchema().GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, StructPropertyWithStructClassInRefSchema)
    {
    ECStructClassP refStructClass;
    ECSchemaPtr refSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(refSchema, "RefSchema", "ref", 1, 0, 0));
    EC_ASSERT_SUCCESS(refSchema->CreateStructClass(refStructClass, "RefStruct"));
    EC_ASSERT_SUCCESS(m_schema0->AddReferencedSchema(*refSchema));

    StructECPropertyP structProp;
    EC_ASSERT_SUCCESS(m_entity0->CreateStructProperty(structProp, "StructProp", *refStructClass));

    m_prop0 = structProp;

    // Add referenced schema so when the struct property is set an error will not be thrown
    m_schema1->AddReferencedSchema(*refSchema);

    CopyProperty();

    StructECPropertyCP structProp1 = m_prop1->GetAsStructProperty();
    EXPECT_TRUE(nullptr != structProp1);
    EXPECT_STREQ("RefSchema", structProp1->GetType().GetSchema().GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, StructArrayPropertyWithMaxAndMinOccurs)
    {
    ECStructClassP structClass;
    EC_ASSERT_SUCCESS(m_schema0->CreateStructClass(structClass, "Struct"));

    StructArrayECPropertyP structArrProp;
    EC_ASSERT_SUCCESS(m_entity0->CreateStructArrayProperty(structArrProp, "Prop", *structClass));
    EC_ASSERT_SUCCESS(structArrProp->SetMinOccurs(5));
    EC_ASSERT_SUCCESS(structArrProp->SetMaxOccurs(15));

    m_prop0 = structArrProp;

    CopyProperty();

    StructArrayECPropertyCP structArrProp1 = m_prop1->GetAsStructArrayProperty();
    EXPECT_EQ(5, structArrProp1->GetMinOccurs());
    EXPECT_EQ(15, structArrProp1->GetStoredMaxOccurs());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, StructArrayPropertyWithStructClassInSameSchema)
    {
    ECStructClassP structClass;
    EC_ASSERT_SUCCESS(m_schema0->CreateStructClass(structClass, "Struct"));

    StructArrayECPropertyP structArrProp;
    EC_ASSERT_SUCCESS(m_entity0->CreateStructArrayProperty(structArrProp, "StructProp", *structClass));

    m_prop0 = structArrProp;

    CopyProperty();

    ECClassCP ecClass = m_schema1->GetClassCP("Struct");
    ASSERT_TRUE(nullptr != ecClass);
    ECStructClassCP destStructClass = ecClass->GetStructClassCP();
    ASSERT_TRUE(nullptr != destStructClass);

    StructArrayECPropertyCP destStructProp = m_prop1->GetAsStructArrayProperty();
    ASSERT_TRUE(nullptr != destStructProp);
    EXPECT_STREQ(m_schema1->GetName().c_str(), destStructProp->GetStructElementType().GetSchema().GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, StructArrayPropertyWithStructClassInSameSchemaWithoutCopyingTypes)
    {
    ECStructClassP structClass;
    EC_ASSERT_SUCCESS(m_schema0->CreateStructClass(structClass, "Struct"));

    StructArrayECPropertyP structArrProp;
    EC_ASSERT_SUCCESS(m_entity0->CreateStructArrayProperty(structArrProp, "StructProp", *structClass));

    m_prop0 = structArrProp;

    CopyProperty(false);

    StructArrayECPropertyCP destStructProp = m_prop1->GetAsStructArrayProperty();
    ASSERT_TRUE(nullptr != destStructProp);
    EXPECT_STREQ(m_schema0->GetName().c_str(), destStructProp->GetStructElementType().GetSchema().GetName().c_str()) << "The struct type should still be in the source schema.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, StructArrayPropertyWithStructClassInRefSchema)
    {
    ECStructClassP refStructClass;
    ECSchemaPtr refSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(refSchema, "RefSchema", "ref", 1, 0, 0));
    EC_ASSERT_SUCCESS(refSchema->CreateStructClass(refStructClass, "RefStruct"));
    EC_ASSERT_SUCCESS(m_schema0->AddReferencedSchema(*refSchema));

    StructArrayECPropertyP structArrProp;
    EC_ASSERT_SUCCESS(m_entity0->CreateStructArrayProperty(structArrProp, "StructProp", *refStructClass));

    m_prop0 = structArrProp;

    // Add referenced schema so when the struct property is set an error will not be thrown
    m_schema1->AddReferencedSchema(*refSchema);

    CopyProperty();

    StructArrayECPropertyCP structProp1 = m_prop1->GetAsStructArrayProperty();
    EXPECT_TRUE(nullptr != structProp1);
    EXPECT_STREQ("RefSchema", structProp1->GetStructElementType().GetSchema().GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, NavigationPropertyWithRelatedInstanceDirection)
    {
    ECEntityClassP targetClass;
    ECRelationshipClassP relClass;
    EC_ASSERT_SUCCESS(m_schema0->CreateEntityClass(targetClass, "TargetClass"));
    EC_ASSERT_SUCCESS(m_schema0->CreateRelationshipClass(relClass, "RelClass", *m_entity0, "Source", *targetClass, "target"));
    
    NavigationECPropertyP navProp;
    EC_ASSERT_SUCCESS(m_entity0->CreateNavigationProperty(navProp, "NavProp", *relClass, ECRelatedInstanceDirection::Forward));

    m_prop0 = navProp;

    CopyProperty();

    NavigationECPropertyCP destNavProp = m_prop1->GetAsNavigationProperty();
    EXPECT_TRUE(nullptr != destNavProp);
    EXPECT_EQ(ECRelatedInstanceDirection::Forward, destNavProp->GetDirection());
    }

//---------------------------------------------------------------------------------------
// The ECRelationshipClass on a NavigationECProperty can never be in a reference schema 
// because the ECEntityClass the NavigationECProperty is on has to be a constraint class
// in the ECRelationship class. This cannot happen if the ECRelationshipClass is in the 
// referenced schema and the intended ECEntityClass is in the main schema, it would 
// result in a circular reference.
//
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, NavigationPropertyWithRelationshipInSameSchema)
    {
    ECEntityClassP targetClass;
    ECRelationshipClassP relClass;
    EC_ASSERT_SUCCESS(m_schema0->CreateEntityClass(targetClass, "TargetClass"));
    EC_ASSERT_SUCCESS(m_schema0->CreateRelationshipClass(relClass, "RelClass", *m_entity0, "Source", *targetClass, "target"));
    
    NavigationECPropertyP navProp;
    EC_ASSERT_SUCCESS(m_entity0->CreateNavigationProperty(navProp, "NavProp", *relClass, ECRelatedInstanceDirection::Forward));

    m_prop0 = navProp;

    CopyProperty();

    NavigationECPropertyCP destNavProp = m_prop1->GetAsNavigationProperty();
    EXPECT_TRUE(nullptr != destNavProp);
    EXPECT_STREQ(m_schema1->GetName().c_str(), destNavProp->GetRelationshipClass()->GetSchema().GetName().c_str());
    }

//---------------+---------------+---------------+---------------+---------------+-------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyCopyTest, NavigationPropertyWithRelationshipInSameSchemaWithoutCopyingTypes)
    {
    ECEntityClassP targetClass;
    ECRelationshipClassP relClass;
    EC_ASSERT_SUCCESS(m_schema0->CreateEntityClass(targetClass, "TargetClass"));
    EC_ASSERT_SUCCESS(m_schema0->CreateRelationshipClass(relClass, "RelClass", *m_entity0, "Source", *targetClass, "target"));
    
    NavigationECPropertyP navProp;
    EC_ASSERT_SUCCESS(m_entity0->CreateNavigationProperty(navProp, "NavProp", *relClass, ECRelatedInstanceDirection::Forward));

    m_prop0 = navProp;

    CopyProperty(false);

    NavigationECPropertyCP destNavProp = m_prop1->GetAsNavigationProperty();
    EXPECT_TRUE(nullptr != destNavProp);
    EXPECT_STREQ(m_schema0->GetName().c_str(), destNavProp->GetRelationshipClass()->GetSchema().GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(PropertyCopyTest, SimpleCase_CopiedPropertyIsSameProperty)
    {
    PrimitiveECPropertyP primProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveProperty(primProp, "Prop", PRIMITIVETYPE_String));
    
    m_prop0 = primProp;
    CopyProperty(true);
    EXPECT_TRUE(m_prop0->IsSame(*m_prop1));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(PropertyCopyTest, PriorityExplicitlySet_CopiedPropertyIsSameProperty)
    {
    PrimitiveECPropertyP primProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveProperty(primProp, "Prop", PRIMITIVETYPE_String));
    primProp->SetPriority(33);

    m_prop0 = primProp;
    CopyProperty(true);
    EXPECT_TRUE(m_prop0->IsSame(*m_prop1));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(PropertyCopyTest, PropertyHasBasePropertyWithNoPriority_CopiedPropertyIsSameProperty)
    {
    PrimitiveECPropertyP baseProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveProperty(baseProp, "BaseProp", PRIMITIVETYPE_String));

    PrimitiveECPropertyP primProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveProperty(primProp, "Prop", PRIMITIVETYPE_String));
    primProp->SetBaseProperty(baseProp);

    m_prop0 = primProp;
    CopyProperty(true);
    EXPECT_TRUE(m_prop0->IsSame(*m_prop1));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(PropertyCopyTest, PropertyHasBasePropertyWithPriorityExcplicitlySet_CopiedPropertyIsSameProperty)
    {
    PrimitiveECPropertyP baseProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveProperty(baseProp, "BaseProp", PRIMITIVETYPE_String));
    baseProp->SetPriority(31);

    PrimitiveECPropertyP primProp;
    EC_ASSERT_SUCCESS(m_entity0->CreatePrimitiveProperty(primProp, "Prop", PRIMITIVETYPE_String));
    primProp->SetBaseProperty(baseProp);

    m_prop0 = primProp;
    CopyProperty(true);
    EXPECT_TRUE(m_prop0->IsSame(*m_prop1));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassTest, ExplicitSerializeInheritedPropertiesWithLengthGreaterThanInt32Max)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);

    PrimitiveECPropertyP primPropNonOverrideBase;
    PrimitiveECPropertyP primPropOverrideBase;
    PrimitiveECPropertyP primPropOverrideDerived;

    ECEntityClassP baseEntityClass;
    schema->CreateEntityClass(baseEntityClass, "BaseEntityClass");
    baseEntityClass->CreatePrimitiveProperty(primPropNonOverrideBase, "PrimitivePropertyNonOverride", PrimitiveType::PRIMITIVETYPE_Integer);
    baseEntityClass->CreatePrimitiveProperty(primPropOverrideBase, "PrimitivePropertyOverride", PrimitiveType::PRIMITIVETYPE_String);
    primPropOverrideBase->SetMinimumLength(UINT32_MAX);
    primPropOverrideBase->SetMaximumLength(UINT32_MAX);

    ECEntityClassP derivedEntityClass;
    schema->CreateEntityClass(derivedEntityClass, "DerivedEntityClass");
    derivedEntityClass->AddBaseClass(*baseEntityClass);
    derivedEntityClass->CreatePrimitiveProperty(primPropOverrideDerived, "PrimitivePropertyOverride", PrimitiveType::PRIMITIVETYPE_String);
    primPropOverrideDerived->SetMinimumLength(UINT32_MAX);
    primPropOverrideDerived->SetMaximumLength(UINT32_MAX);

    BeJsDocument entityClassJson;
    EXPECT_TRUE(derivedEntityClass->ToJson(entityClassJson, true, true));

    BeJsDocument testDataJson;
    BeFileName entityClassTestDataFile(ECTestFixture::GetTestDataPath(L"ECJson/ExplicitSerializeInheritedPropertiesWithValueGreaterThanInt32Max.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, entityClassTestDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);
    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(entityClassJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(entityClassJson, testDataJson);
    }

END_BENTLEY_ECN_TEST_NAMESPACE
