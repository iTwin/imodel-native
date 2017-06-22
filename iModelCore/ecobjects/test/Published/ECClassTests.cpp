/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/ECClassTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct ClassTest : ECTestFixture
    {
    void TestPropertyCount(ECClassCR ecClass, size_t nPropertiesWithoutBaseClasses, size_t nPropertiesWithBaseClasses)
        {
        EXPECT_EQ(ecClass.GetPropertyCount(false), nPropertiesWithoutBaseClasses);
        EXPECT_EQ(ecClass.GetPropertyCount(true), nPropertiesWithBaseClasses);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Paul.Connelly   12/12
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
* @bsimethod                                                    Paul.Connelly   03/13
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
    structClass->CreatePrimitiveProperty(primProp, "StructProp1");
    structClass->CreatePrimitiveProperty(primProp, "StructProp2");

    // 1 base class with 3 primitive properties
    schema->CreateEntityClass(baseClass1, "BaseClass1");
    baseClass1->CreatePrimitiveProperty(primProp, "Base1Prop1");
    baseClass1->CreatePrimitiveProperty(primProp, "Base1Prop2");
    baseClass1->CreatePrimitiveProperty(primProp, "Base1Prop3");

    // 1 base class with 1 primitive and 2 struct properties (each struct has 2 properties
    schema->CreateEntityClass(baseClass2, "BaseClass2");
    baseClass2->CreatePrimitiveProperty(primProp, "Base2Prop1");
    baseClass2->CreateStructProperty(structProp, "Base2Prop2", *structClass);
    baseClass2->CreateStructProperty(structProp, "Base2Prop3", *structClass);

    // Derived class with 1 extra primitive property, 1 extra struct property, derived from 2 base classes
    schema->CreateEntityClass(derivedClass, "DerivedClass");
    derivedClass->CreateStructProperty(structProp, "DerivedProp1", *structClass);
    derivedClass->CreatePrimitiveProperty(primProp, "DerivedProp2");
    derivedClass->AddBaseClass(*baseClass1);
    derivedClass->AddBaseClass(*baseClass2);

    TestPropertyCount(*structClass, 2, 2);
    TestPropertyCount(*baseClass1, 3, 3);
    TestPropertyCount(*baseClass2, 3, 3);
    TestPropertyCount(*derivedClass, 2, 8);
    }

//---------------------------------------------------------------------------------------//
// @bsimethod                                                    Colin.Kerr		06/2017
//+---------------+---------------+---------------+---------------+---------------+------///
TEST_F(ClassTest, GetPropertyCount_WithOverrides)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);

    ECEntityClassP baseClass, derivedClass, derivedClass1;
    schema->CreateEntityClass(baseClass, "Banana");
    schema->CreateEntityClass(derivedClass, "DerivedBanana");
    derivedClass->AddBaseClass(*baseClass);
    schema->CreateEntityClass(derivedClass1, "SuperDerivedBanana");
    derivedClass1->AddBaseClass(*derivedClass);

    PrimitiveECPropertyP prop1, baseProp, derivedPropOverride, derived1PropOverride;
    baseClass->CreatePrimitiveProperty(prop1, "Prop1", PrimitiveType::PRIMITIVETYPE_Integer);
    baseClass->CreatePrimitiveProperty(baseProp, "Property", PrimitiveType::PRIMITIVETYPE_Double);
    derivedClass->CreatePrimitiveProperty(derivedPropOverride, "Property", PrimitiveType::PRIMITIVETYPE_Double);
    derivedClass1->CreatePrimitiveProperty(derived1PropOverride, "Property", PrimitiveType::PRIMITIVETYPE_Double);

    TestPropertyCount(*baseClass, 2, 2);
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
// @bsimethod                                   Carole.MacDonald            01/2016
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
    base->CreatePrimitiveProperty(baseIntProp, "Code", PRIMITIVETYPE_Integer);

    ECObjectsStatus status = parent->AddBaseClass(*base);
    EXPECT_NE(ECObjectsStatus::DataTypeMismatch, status);

    ECPropertyP prop = derived->GetPropertyP("Code", false);
    ASSERT_TRUE(nullptr != prop);
    PrimitiveECPropertyP primProp = prop->GetAsPrimitivePropertyP();
    EXPECT_EQ(PRIMITIVETYPE_String, primProp->GetType());

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2016
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
    class1->CreatePrimitiveProperty(property1, "beta");
    class1->CreatePrimitiveProperty(property2, "gamma");
    class1->CreatePrimitiveProperty(property3, "delta");
    class1->CreatePrimitiveProperty(property4, "alpha");

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

    ab->CreatePrimitiveProperty(a, "a");
    ab->CreatePrimitiveProperty(b, "b");

    cd->CreatePrimitiveProperty(c, "c");
    cd->CreatePrimitiveProperty(d, "d");

    ef->CreatePrimitiveProperty(e, "e");
    ef->CreatePrimitiveProperty(f, "f");

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

    ab->CreatePrimitiveProperty(a, "a");
    ab->CreatePrimitiveProperty(b, "b");

    cd->CreatePrimitiveProperty(c, "c");
    cd->CreatePrimitiveProperty(d, "d");

    ef->CreatePrimitiveProperty(e, "e");
    ef->CreatePrimitiveProperty(f, "f");

    gh->CreatePrimitiveProperty(g, "g");
    gh->CreatePrimitiveProperty(h, "h");

    ij->CreatePrimitiveProperty(i, "i");
    ij->CreatePrimitiveProperty(j, "j");

    kl->CreatePrimitiveProperty(k, "k");
    kl->CreatePrimitiveProperty(l, "l");

    mn->CreatePrimitiveProperty(m, "m");
    mn->CreatePrimitiveProperty(n, "n");

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

    mn->CreatePrimitiveProperty(b2, "b");
    mn->CreatePrimitiveProperty(d2, "d");
    mn->CreatePrimitiveProperty(f2, "f");
    mn->CreatePrimitiveProperty(h2, "h");
    mn->CreatePrimitiveProperty(j2, "j");
    mn->CreatePrimitiveProperty(k2, "k");

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

    kl->CreatePrimitiveProperty(e2, "e");
    kl->CreatePrimitiveProperty(a2, "a");
    kl->CreatePrimitiveProperty(c2, "c");
    kl->CreatePrimitiveProperty(g2, "g");

    ef->CreatePrimitiveProperty(l2, "l");
    gh->CreatePrimitiveProperty(i2, "i");
    ij->CreatePrimitiveProperty(g3, "g");

    gh->CreatePrimitiveProperty(a3, "a");
    gh->CreatePrimitiveProperty(b3, "b");
    ab->CreatePrimitiveProperty(g4, "g");
    ab->CreatePrimitiveProperty(h3, "h");

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

    base->CreatePrimitiveProperty(readOnlyProp, "readOnlyProp");
    readOnlyProp->SetIsReadOnly(true);

    ASSERT_EQ(ECObjectsStatus::Success, child->AddBaseClass(*base));

    ECPropertyP ecProp = GetPropertyByName(*child, "readOnlyProp");
    ASSERT_EQ(true, ecProp->GetIsReadOnly());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                    12/2016
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

    classA->CreatePrimitiveProperty(primProp, "PrimProp");
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

END_BENTLEY_ECN_TEST_NAMESPACE
