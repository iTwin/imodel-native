/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/PropertyOverrideTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include "TestFixture.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                   Muhammad Hassan                     07/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PropertyOverrideTests : ECTestFixture
    {
    ECSchemaPtr RoundTripSchema(ECSchemaPtr testSchema);
    void VerifyPropertyInheritance(ECClassCP ab, ECClassCP cd, ECClassCP ef, ECClassCP gh, ECClassCP ij, ECClassCP kl, ECClassCP mn);
    ECSchemaPtr SetUpBaseSchema();

    void ExpectBasePropertyFromClass (Utf8CP propName, ECClassCP derived, ECClassCP base)
        {
        auto baseProp = derived->GetPropertyP (propName)->GetBaseProperty();
        EXPECT_TRUE (baseProp->GetClass().GetName().Equals (base->GetName()))
            << "Base property " << propName
            << " expected from class " << base->GetName().c_str()
            << " actually from class " << baseProp->GetClass().GetName().c_str();
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     07/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSchemaPtr PropertyOverrideTests::RoundTripSchema(ECSchemaPtr testSchema)
    {
    ECSchemaPtr tempSchema;
    WString schemaXml;
    EXPECT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, testSchema->WriteToXmlString(schemaXml));
    ECSchemaReadContextPtr deserializedSchemaContext = ECSchemaReadContext::CreateContext();
    EXPECT_EQ(SchemaReadStatus::SCHEMA_READ_STATUS_Success, ECSchema::ReadFromXmlString(tempSchema, schemaXml.c_str(), *deserializedSchemaContext));
    return tempSchema;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     07/15
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyOverrideTests::VerifyPropertyInheritance(ECClassCP ab, ECClassCP cd, ECClassCP ef, ECClassCP gh, ECClassCP ij, ECClassCP kl, ECClassCP mn)
    {
    Utf8CP propName = "a";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ab->GetName()));
    propName = "b";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ExpectBasePropertyFromClass (propName, mn, ab);
    propName = "c";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(cd->GetName()));
    propName = "d";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ExpectBasePropertyFromClass (propName, mn, cd);
    propName = "e";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ef->GetName()));
    propName = "f";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ExpectBasePropertyFromClass (propName, mn, ef);
    propName = "g";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ab->GetName()));
    propName = "h";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ExpectBasePropertyFromClass (propName, mn, ab);
    propName = "i";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(gh->GetName()));
    propName = "j";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ExpectBasePropertyFromClass (propName, mn, ij);
    propName = "k";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ExpectBasePropertyFromClass (propName, mn, kl);
    propName = "l";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ef->GetName()));
    propName = "m";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    propName = "n";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     07/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSchemaPtr PropertyOverrideTests::SetUpBaseSchema()
    {
    ECSchemaPtr testSchema;
    WString schemaName = L"testSchema";
    EXPECT_EQ(SUCCESS, ECSchema::CreateSchema(testSchema, schemaName, 1, 0));
    testSchema->SetNamespacePrefix(L"ts");

    ECClassP root = NULL;
    EXPECT_EQ(ECOBJECTS_STATUS_Success, testSchema->CreateClass(root, L"Root"));
    ECClassP b1 = NULL;
    EXPECT_EQ(ECOBJECTS_STATUS_Success, testSchema->CreateClass(b1, L"B1"));
    ECClassP b2 = NULL;
    EXPECT_EQ(ECOBJECTS_STATUS_Success, testSchema->CreateClass(b2, L"B2"));
    ECClassP foo = NULL;
    EXPECT_EQ(ECOBJECTS_STATUS_Success, testSchema->CreateClass(foo, L"Foo"));

    return testSchema;
    }
/*---------------------------------------------------------------------------------------
 <summary>Creates a class chain and add properties and then verifies if they
 come in the expected sequence.</summary>
 <Scenario>
 This is the class hierarchy used in this test. The numbers indicate override priority,
 and the letters indicate ECClass name and their inital properties, e.g.
 "4cd" represents the ECClass named "cd" which has ECProperties named "c" and "d"
 and which is 4th overall in override priority... it can override properties from ECClass
 "kl", but not properties from "ab".

 //     3ab 4cd 6gh 7ij
 //       \/      \/
 //       2ef    5kl
 //          \  /
 //          1mn
 </scenario>
 @bsimethod                              Muhammad Hassan                         07/15
 -------------+---------------+---------------+---------------+---------------+---------*/
TEST_F(PropertyOverrideTests, PropertyOverrideInMultiInheritance)
    {
    //create Schema
    ECSchemaPtr testSchema;
    WString schemaName = L"testSchema";
    ASSERT_EQ(SUCCESS, ECSchema::CreateSchema(testSchema, schemaName, 1, 0));
    testSchema->SetNamespacePrefix(L"ts");

    ECClassP ab = NULL;
    ASSERT_EQ(ECOBJECTS_STATUS_Success, testSchema->CreateClass(ab, L"ab"));
    ECClassP cd = NULL;
    ASSERT_EQ(ECOBJECTS_STATUS_Success, testSchema->CreateClass(cd, L"cd"));
    ECClassP ef = NULL;
    ASSERT_EQ(ECOBJECTS_STATUS_Success, testSchema->CreateClass(ef, L"ef"));
    ECClassP gh = NULL;
    ASSERT_EQ(ECOBJECTS_STATUS_Success, testSchema->CreateClass(gh, L"gh"));
    ECClassP ij = NULL;
    ASSERT_EQ(ECOBJECTS_STATUS_Success, testSchema->CreateClass(ij, L"ij"));
    ECClassP kl = NULL;
    ASSERT_EQ(ECOBJECTS_STATUS_Success, testSchema->CreateClass(kl, L"kl"));
    ECClassP mn = NULL;
    ASSERT_EQ(ECOBJECTS_STATUS_Success, testSchema->CreateClass(mn, L"mn"));

    //add base classes of ef
    ef->AddBaseClass(*ab);
    ef->AddBaseClass(*cd);

    //add base classes of kl
    kl->AddBaseClass(*gh);
    kl->AddBaseClass(*ij);

    //add base classes of mn
    mn->AddBaseClass(*ef);
    mn->AddBaseClass(*kl);

    PrimitiveECPropertyP primitiveProperty;
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, ab->CreatePrimitiveProperty(primitiveProperty, L"a", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, ab->CreatePrimitiveProperty(primitiveProperty, L"b", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, cd->CreatePrimitiveProperty(primitiveProperty, L"c", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, cd->CreatePrimitiveProperty(primitiveProperty, L"d", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, ef->CreatePrimitiveProperty(primitiveProperty, L"e", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, ef->CreatePrimitiveProperty(primitiveProperty, L"f", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, gh->CreatePrimitiveProperty(primitiveProperty, L"g", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, gh->CreatePrimitiveProperty(primitiveProperty, L"h", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, ij->CreatePrimitiveProperty(primitiveProperty, L"i", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, ij->CreatePrimitiveProperty(primitiveProperty, L"j", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, kl->CreatePrimitiveProperty(primitiveProperty, L"k", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, kl->CreatePrimitiveProperty(primitiveProperty, L"l", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, mn->CreatePrimitiveProperty(primitiveProperty, L"m", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, mn->CreatePrimitiveProperty(primitiveProperty, L"n", PrimitiveType::PRIMITIVETYPE_String));

    //verify that properties successfully added for classes
    Utf8CP propName = "a";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ab->GetName()));
    propName = "b";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ab->GetName()));
    propName = "c";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(cd->GetName()));
    propName = "d";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(cd->GetName()));
    propName = "e";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ef->GetName()));
    propName = "f";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ef->GetName()));
    propName = "g";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(gh->GetName()));
    propName = "h";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(gh->GetName()));
    propName = "i";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ij->GetName()));
    propName = "j";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ij->GetName()));
    propName = "k";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(kl->GetName()));
    propName = "l";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(kl->GetName()));
    propName = "m";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    propName = "n";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));

    //now we add some duplicate properties to mn, which will "override" those from the base classes 
    mn->CreatePrimitiveProperty(primitiveProperty, L"b", PrimitiveType::PRIMITIVETYPE_String);
    mn->CreatePrimitiveProperty(primitiveProperty, L"d", PrimitiveType::PRIMITIVETYPE_String);
    mn->CreatePrimitiveProperty(primitiveProperty, L"f", PrimitiveType::PRIMITIVETYPE_String);
    mn->CreatePrimitiveProperty(primitiveProperty, L"h", PrimitiveType::PRIMITIVETYPE_String);
    mn->CreatePrimitiveProperty(primitiveProperty, L"j", PrimitiveType::PRIMITIVETYPE_String);
    mn->CreatePrimitiveProperty(primitiveProperty, L"k", PrimitiveType::PRIMITIVETYPE_String);

    propName = "a";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ab->GetName()));
    propName = "b";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetBaseProperty()->GetClass()).GetName().Equals(ab->GetName()));
    propName = "c";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(cd->GetName()));
    propName = "d";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetBaseProperty()->GetClass()).GetName().Equals(cd->GetName()));
    propName = "e";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ef->GetName()));
    propName = "f";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetBaseProperty()->GetClass()).GetName().Equals(ef->GetName()));
    propName = "g";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(gh->GetName()));
    propName = "h";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetBaseProperty()->GetClass()).GetName().Equals(gh->GetName()));
    propName = "i";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ij->GetName()));
    propName = "j";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetBaseProperty()->GetClass()).GetName().Equals(ij->GetName()));
    propName = "k";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetBaseProperty()->GetClass()).GetName().Equals(kl->GetName()));
    propName = "l";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(kl->GetName()));
    propName = "m";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    propName = "n";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));

    //Override more properties of base classes (Add eacg to kl, iab to gh, l to ef, g to ij and gh to ab) and verify property inheritance
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, kl->CreatePrimitiveProperty(primitiveProperty, L"e", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, kl->CreatePrimitiveProperty(primitiveProperty, L"a", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, kl->CreatePrimitiveProperty(primitiveProperty, L"c", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, kl->CreatePrimitiveProperty(primitiveProperty, L"g", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, gh->CreatePrimitiveProperty(primitiveProperty, L"a", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, gh->CreatePrimitiveProperty(primitiveProperty, L"b", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, gh->CreatePrimitiveProperty(primitiveProperty, L"i", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, ef->CreatePrimitiveProperty(primitiveProperty, L"l", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, ij->CreatePrimitiveProperty(primitiveProperty, L"g", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, ab->CreatePrimitiveProperty(primitiveProperty, L"g", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, ab->CreatePrimitiveProperty(primitiveProperty, L"h", PrimitiveType::PRIMITIVETYPE_String));

    VerifyPropertyInheritance(ab, cd, ef, gh, ij, kl, mn);

    //Roundtrip Schema and test that order is still the same
    ECSchemaPtr testSchemaCopy1 = RoundTripSchema(testSchema);
    ECSchemaPtr testSchemaCopy2 = RoundTripSchema(testSchemaCopy1);

    ECClassCP ab1 = testSchemaCopy1->GetClassCP(L"ab");
    ASSERT_TRUE(ab1 != nullptr);
    ECClassCP cd1 = testSchemaCopy1->GetClassCP(L"cd");
    ASSERT_TRUE(cd1 != nullptr);
    ECClassCP ef1 = testSchemaCopy1->GetClassCP(L"ef");
    ASSERT_TRUE(ef1 != nullptr);
    ECClassCP gh1 = testSchemaCopy1->GetClassCP(L"gh");
    ASSERT_TRUE(gh1 != nullptr);
    ECClassCP ij1 = testSchemaCopy1->GetClassCP(L"ij");
    ASSERT_TRUE(ij1 != nullptr);
    ECClassCP kl1 = testSchemaCopy1->GetClassCP(L"kl");
    ASSERT_TRUE(kl1 != nullptr);
    ECClassCP mn1 = testSchemaCopy1->GetClassCP(L"mn");
    ASSERT_TRUE(mn1 != nullptr);
    //Verify Property Inheritance for the 1st copy of Schema (after xml Deserialization)
    VerifyPropertyInheritance(ab1, cd1, ef1, gh1, ij1, kl1, mn1);

    ECClassCP ab2 = testSchemaCopy2->GetClassCP(L"ab");
    ASSERT_TRUE(ab2 != nullptr);
    ECClassCP cd2 = testSchemaCopy2->GetClassCP(L"cd");
    ASSERT_TRUE(cd2 != nullptr);
    ECClassCP ef2 = testSchemaCopy2->GetClassCP(L"ef");
    ASSERT_TRUE(ef2 != nullptr);
    ECClassCP gh2 = testSchemaCopy2->GetClassCP(L"gh");
    ASSERT_TRUE(gh2 != nullptr);
    ECClassCP ij2 = testSchemaCopy2->GetClassCP(L"ij");
    ASSERT_TRUE(ij2 != nullptr);
    ECClassCP kl2 = testSchemaCopy2->GetClassCP(L"kl");
    ASSERT_TRUE(kl2 != nullptr);
    ECClassCP mn2 = testSchemaCopy2->GetClassCP(L"mn");
    ASSERT_TRUE(mn2 != nullptr);
    //Verify Property Inheritance for the 2st copy of Schema (after xml Deserialization)
    VerifyPropertyInheritance(ab2, cd2, ef2, gh2, ij2, kl2, mn2);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyOverrideTests, AddingBasePropertyOverrideChangesPropertyInDerivedClass)
    {
    ECSchemaPtr testSchema;
    WString schemaName = L"testSchema";
    ASSERT_EQ(SUCCESS, ECSchema::CreateSchema(testSchema, schemaName, 1, 0));
    testSchema->SetNamespacePrefix(L"ts");
    ECClassP baseClass = NULL;
    ASSERT_EQ(ECOBJECTS_STATUS_Success, testSchema->CreateClass(baseClass, L"BaseClass"));
    PrimitiveECPropertyP primitiveProperty;
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, baseClass->CreatePrimitiveProperty(primitiveProperty, L"testProp", PrimitiveType::PRIMITIVETYPE_String));
    ECClassP derivedClass = NULL;
    ASSERT_EQ(ECOBJECTS_STATUS_Success, testSchema->CreateClass(derivedClass, L"DerivedClass"));
    derivedClass->AddBaseClass(*baseClass);
    ECClassP doubleDerivedClass = NULL;
    ASSERT_EQ(ECOBJECTS_STATUS_Success, testSchema->CreateClass(doubleDerivedClass, L"DoubleDerivedClass"));
    doubleDerivedClass->AddBaseClass(*derivedClass);

    ASSERT_EQ(doubleDerivedClass->GetPropertyP("testProp"), baseClass->GetPropertyP("testProp")) << "Expected base and double derived classes to return the base property";

    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, derivedClass->CreatePrimitiveProperty(primitiveProperty, L"testProp", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_NE(doubleDerivedClass->GetPropertyP("testProp"), baseClass->GetPropertyP("testProp")) << "Expected base and double derived classes to return the base property and Derived Property respectively";
    ASSERT_EQ(doubleDerivedClass->GetPropertyP("testProp"), (derivedClass->GetPropertyP("testProp"))) << "Expected derived and double derived classes to return the derived Propertyy";

    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, doubleDerivedClass->CreatePrimitiveProperty(primitiveProperty, L"testProp", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, derivedClass->RemoveProperty(L"testProp"));

    ASSERT_EQ(doubleDerivedClass->GetPropertyP("testProp")->GetBaseProperty(), baseClass->GetPropertyP("testProp")) << "doubleDerivedClass.testProp.GetBaseProperty to have BaseClass.testProp because derivedClass Property has been removed";
    }

/*---------------------------------------------------------------------------------------
 <summary>Creates a class chain and add properties and then verifies if they
 come in the expected sequence depending on Traversal Order of ECClasses.</summary>
 <Scenario>
 <ECClass typeName = "Root" / > //Defines a property “A”
 <ECClass typeName = "B1">
 <BaseClass>Root< / BaseClass >
 <ECClass typeName = "B2" / >  // Define’s a property “A”
 <ECClass typeName = "Foo">
 <BaseClass>B1< / BaseClass >
 <BaseClass>B2< / BaseClass >
 //     3Root
 //    /                   //digits show overall override priority
 //   2B1    4B2
 //     \  /
 //      1Foo
 The traversal order will be: Foo, B1, Root, B2, and Root’s definition of property “A” will “win”,
 </scenario>
 @bsimethod                              Muhammad Hassan                         07/15
 -------------+---------------+---------------+---------------+---------------+---------*/
TEST_F(PropertyOverrideTests, VerifyClassTraversalOrderAfterPropertyOverride)
    {
    ECSchemaPtr testSchema = SetUpBaseSchema();
    ECClassP root = testSchema->GetClassP(L"Root");
    ECClassP b1 = testSchema->GetClassP(L"B1");
    ECClassP b2 = testSchema->GetClassP(L"B2");
    ECClassP foo = testSchema->GetClassP(L"Foo");

    b1->AddBaseClass(*root);

    foo->AddBaseClass(*b1);
    foo->AddBaseClass(*b2);

    PrimitiveECPropertyP primitiveProp;
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, root->CreatePrimitiveProperty(primitiveProp, L"A", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_EQ(foo->GetPropertyP("A"), root->GetPropertyP("A")) << "Expected Foo and Root to return the Root Property";

    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, foo->CreatePrimitiveProperty(primitiveProp, L"A", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_NE(foo->GetPropertyP("A"), root->GetPropertyP("A")) << "Expected Foo and Root to return the Foo Property and Root property Respectively";
    ASSERT_EQ(foo->GetPropertyP("A")->GetBaseProperty(), root->GetPropertyP("A")) << "Expected Foo and Root both to return Root  property";

    //now add same property A to class B2, as B2 has Higher Override Priority so traversal Order should not get changed.
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, b2->CreatePrimitiveProperty(primitiveProp, L"A", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_NE(foo->GetPropertyP("A"), root->GetPropertyP("A")) << "Expected Foo and Root to return the Foo Property and Root property Respectively";
    ASSERT_EQ(foo->GetPropertyP("A")->GetBaseProperty(), root->GetPropertyP("A")) << "Expected Foo and Root both to return Root property";
    }

/*---------------------------------------------------------------------------------------
 <summary>Creates a class chain and add properties and then verifies if they
 come in the expected sequence depending on Traversal Order of ECClasses.</summary>
 <Scenario>
 <ECClass typeName = "Root" / > //Defines a property “A”
 <ECClass typeName = "B1">
 <BaseClass>Root< / BaseClass >
 <ECClass typeName = "B2" / >  // Define’s a property “A”
 <ECClass typeName = "Foo">
 <BaseClass>B2< / BaseClass >
 <BaseClass>B1< / BaseClass >
 //     4Root
 //    /                   //digits show overall override priority
 //   3B1    2B2
 //     \  /
 //      1Foo
 The traversal order will be: Foo, B2, B1, Root and B2’s definition of property “A” will “win”.
 </scenario>
 @bsimethod                              Muhammad Hassan                         07/15
 -------------+---------------+---------------+---------------+---------------+---------*/
TEST_F(PropertyOverrideTests, VerifyClassTraversalOrderAfterPropertyOverride1)
    {
    ECSchemaPtr testSchema = SetUpBaseSchema();
    ECClassP root = testSchema->GetClassP(L"Root");
    ECClassP b1 = testSchema->GetClassP(L"B1");
    ECClassP b2 = testSchema->GetClassP(L"B2");
    ECClassP foo = testSchema->GetClassP(L"Foo");

    b1->AddBaseClass(*root);

    foo->AddBaseClass(*b2);
    foo->AddBaseClass(*b1);

    PrimitiveECPropertyP primitiveProp;
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, b2->CreatePrimitiveProperty(primitiveProp, L"A", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_EQ(foo->GetPropertyP("A"), b2->GetPropertyP("A")) << "Expected Foo and B2 to return the B2 Property";

    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, foo->CreatePrimitiveProperty(primitiveProp, L"A", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_NE(foo->GetPropertyP("A"), b2->GetPropertyP("A")) << "Expected Foo and B2 to return the Foo Property and B2 property Respectively";
    ASSERT_EQ(foo->GetPropertyP("A")->GetBaseProperty(), b2->GetPropertyP("A")) << "Expected Foo and B2 both to return B2  property";

    //now add same property A to class B1, as B1 has Lower Override Priority so traversal Order should not get changed.
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, b1->CreatePrimitiveProperty(primitiveProp, L"A", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_NE(foo->GetPropertyP("A"), b2->GetPropertyP("A")) << "Expected Foo and B2 to return the Foo Property and B2 property Respectively";
    ASSERT_EQ(foo->GetPropertyP("A")->GetBaseProperty(), b2->GetPropertyP("A")) << "Expected Foo and B2 both to return B2 property";
    }

/*---------------------------------------------------------------------------------------
 <summary>Creates a class chain and add properties and then verifies if they
 come in the expected sequence depending on Traversal Order of ECClasses.</summary>
 <Scenario>
 <ECClass typeName = "Root" / > //Defines a property “A”
 <ECClass typeName = "B1">
 <BaseClass>Root< / BaseClass >
 <ECClass typeName = "B2" / >  // Define’s a property “A”
 <ECClass typeName = "Foo">
 <BaseClass>B1< / BaseClass >
 <BaseClass>B2< / BaseClass >
 //     3Root
 //    /                   //digits show overall override priority
 //   2B1    4B2
 //     \  /
 //      1Foo
 The traversal order will be: Foo, B1, Root, B2, and Root’s definition of property “A” will “win”,
 </scenario>
 @bsimethod                              Muhammad Hassan                         07/15
 -------------+---------------+---------------+---------------+---------------+---------*/
TEST_F(PropertyOverrideTests, VerifyTraversalOrderAfterSerializingDeserializingSchemaToXmlString)
    {
    ECSchemaPtr testSchema = SetUpBaseSchema();
    ECClassP root = testSchema->GetClassP(L"Root");
    ECClassP b1 = testSchema->GetClassP(L"B1");
    ECClassP b2 = testSchema->GetClassP(L"B2");
    ECClassP foo = testSchema->GetClassP(L"Foo");

    b1->AddBaseClass(*root);

    foo->AddBaseClass(*b1);
    foo->AddBaseClass(*b2);

    PrimitiveECPropertyP primitiveProp;
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, root->CreatePrimitiveProperty(primitiveProp, L"A", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, foo->CreatePrimitiveProperty(primitiveProp, L"A", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::ECOBJECTS_STATUS_Success, b2->CreatePrimitiveProperty(primitiveProp, L"A", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_NE(foo->GetPropertyP("A"), root->GetPropertyP("A")) << "Expected Foo and Root to return the Foo Property and Root property Respectively";
    ASSERT_EQ(foo->GetPropertyP("A")->GetBaseProperty(), root->GetPropertyP("A")) << "Expected Foo and Root both to return Root property even after adding property A to class B2";

    ECSchemaPtr testSchemaCopy1 = RoundTripSchema(testSchema);
    ECClassCP root1 = testSchemaCopy1->GetClassCP(L"Root");
    ASSERT_TRUE(root1 != nullptr);
    ECClassCP b11 = testSchemaCopy1->GetClassCP(L"B1");
    ASSERT_TRUE(b11 != nullptr);
    ECClassCP b21 = testSchemaCopy1->GetClassCP(L"B2");
    ASSERT_TRUE(b21 != nullptr);
    ECClassCP foo1 = testSchemaCopy1->GetClassCP(L"Foo");
    ASSERT_TRUE(foo1 != nullptr);

    ASSERT_NE(foo1->GetPropertyP("A"), root1->GetPropertyP("A")) << "Expected Foo and Root to return the Foo Property and Root property Respectively";
    ASSERT_EQ(foo1->GetPropertyP("A")->GetBaseProperty(), root1->GetPropertyP("A")) << "Expected Foo and Root both to return Root property even after adding property A to class B2";
    }

END_BENTLEY_ECOBJECT_NAMESPACE
