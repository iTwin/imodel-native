/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/PropertyOverrideTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <ECObjects/ECObjectsAPI.h>
#include <Bentley/BeTest.h>

using namespace BentleyApi::ECN;

//---------------------------------------------------------------------------------------
// @bsiclass                                   Muhammad Hassan                     07/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PropertyOverrideTests : Public::testing::Test
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
    Utf8String schemaXml;
    EXPECT_EQ(SchemaWriteStatus::Success, testSchema->WriteToXmlString(schemaXml));
    ECSchemaReadContextPtr deserializedSchemaContext = ECSchemaReadContext::CreateContext();
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(tempSchema, schemaXml.c_str(), *deserializedSchemaContext));
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
    Utf8String schemaName = "testSchema";
    EXPECT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(testSchema, schemaName, 1, 0));
    testSchema->SetAlias ("ts");

    ECEntityClassP root = NULL;
    EXPECT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(root, "Root"));
    ECEntityClassP b1 = NULL;
    EXPECT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(b1, "B1"));
    ECEntityClassP b2 = NULL;
    EXPECT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(b2, "B2"));
    ECEntityClassP foo = NULL;
    EXPECT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(foo, "Foo"));

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
    Utf8String schemaName = "testSchema";
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(testSchema, schemaName, 1, 0));
    testSchema->SetAlias("ts");

    ECEntityClassP ab = NULL;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(ab, "ab"));
    ECEntityClassP cd = NULL;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(cd, "cd"));
    ECEntityClassP ef = NULL;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(ef, "ef"));
    ECEntityClassP gh = NULL;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(gh, "gh"));
    ECEntityClassP ij = NULL;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(ij, "ij"));
    ECEntityClassP kl = NULL;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(kl, "kl"));
    ECEntityClassP mn = NULL;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(mn, "mn"));

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
    ASSERT_EQ(ECObjectsStatus::Success, ab->CreatePrimitiveProperty(primitiveProperty, "a", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, ab->CreatePrimitiveProperty(primitiveProperty, "b", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, cd->CreatePrimitiveProperty(primitiveProperty, "c", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, cd->CreatePrimitiveProperty(primitiveProperty, "d", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, ef->CreatePrimitiveProperty(primitiveProperty, "e", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, ef->CreatePrimitiveProperty(primitiveProperty, "f", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, gh->CreatePrimitiveProperty(primitiveProperty, "g", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, gh->CreatePrimitiveProperty(primitiveProperty, "h", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, ij->CreatePrimitiveProperty(primitiveProperty, "i", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, ij->CreatePrimitiveProperty(primitiveProperty, "j", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, kl->CreatePrimitiveProperty(primitiveProperty, "k", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, kl->CreatePrimitiveProperty(primitiveProperty, "l", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, mn->CreatePrimitiveProperty(primitiveProperty, "m", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, mn->CreatePrimitiveProperty(primitiveProperty, "n", PrimitiveType::PRIMITIVETYPE_String));

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
    mn->CreatePrimitiveProperty(primitiveProperty, "b", PrimitiveType::PRIMITIVETYPE_String);
    mn->CreatePrimitiveProperty(primitiveProperty, "d", PrimitiveType::PRIMITIVETYPE_String);
    mn->CreatePrimitiveProperty(primitiveProperty, "f", PrimitiveType::PRIMITIVETYPE_String);
    mn->CreatePrimitiveProperty(primitiveProperty, "h", PrimitiveType::PRIMITIVETYPE_String);
    mn->CreatePrimitiveProperty(primitiveProperty, "j", PrimitiveType::PRIMITIVETYPE_String);
    mn->CreatePrimitiveProperty(primitiveProperty, "k", PrimitiveType::PRIMITIVETYPE_String);

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
    ASSERT_EQ(ECObjectsStatus::Success, kl->CreatePrimitiveProperty(primitiveProperty, "e", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, kl->CreatePrimitiveProperty(primitiveProperty, "a", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, kl->CreatePrimitiveProperty(primitiveProperty, "c", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, kl->CreatePrimitiveProperty(primitiveProperty, "g", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_EQ(ECObjectsStatus::Success, gh->CreatePrimitiveProperty(primitiveProperty, "a", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, gh->CreatePrimitiveProperty(primitiveProperty, "b", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, gh->CreatePrimitiveProperty(primitiveProperty, "i", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_EQ(ECObjectsStatus::Success, ef->CreatePrimitiveProperty(primitiveProperty, "l", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_EQ(ECObjectsStatus::Success, ij->CreatePrimitiveProperty(primitiveProperty, "g", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_EQ(ECObjectsStatus::Success, ab->CreatePrimitiveProperty(primitiveProperty, "g", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, ab->CreatePrimitiveProperty(primitiveProperty, "h", PrimitiveType::PRIMITIVETYPE_String));

    VerifyPropertyInheritance(ab, cd, ef, gh, ij, kl, mn);

    //Roundtrip Schema and test that order is still the same
    ECSchemaPtr testSchemaCopy1 = RoundTripSchema(testSchema);
    ECSchemaPtr testSchemaCopy2 = RoundTripSchema(testSchemaCopy1);

    ECClassCP ab1 = testSchemaCopy1->GetClassCP("ab");
    ASSERT_TRUE(ab1 != nullptr);
    ECClassCP cd1 = testSchemaCopy1->GetClassCP("cd");
    ASSERT_TRUE(cd1 != nullptr);
    ECClassCP ef1 = testSchemaCopy1->GetClassCP("ef");
    ASSERT_TRUE(ef1 != nullptr);
    ECClassCP gh1 = testSchemaCopy1->GetClassCP("gh");
    ASSERT_TRUE(gh1 != nullptr);
    ECClassCP ij1 = testSchemaCopy1->GetClassCP("ij");
    ASSERT_TRUE(ij1 != nullptr);
    ECClassCP kl1 = testSchemaCopy1->GetClassCP("kl");
    ASSERT_TRUE(kl1 != nullptr);
    ECClassCP mn1 = testSchemaCopy1->GetClassCP("mn");
    ASSERT_TRUE(mn1 != nullptr);
    //Verify Property Inheritance for the 1st copy of Schema (after xml Deserialization)
    VerifyPropertyInheritance(ab1, cd1, ef1, gh1, ij1, kl1, mn1);

    ECClassCP ab2 = testSchemaCopy2->GetClassCP("ab");
    ASSERT_TRUE(ab2 != nullptr);
    ECClassCP cd2 = testSchemaCopy2->GetClassCP("cd");
    ASSERT_TRUE(cd2 != nullptr);
    ECClassCP ef2 = testSchemaCopy2->GetClassCP("ef");
    ASSERT_TRUE(ef2 != nullptr);
    ECClassCP gh2 = testSchemaCopy2->GetClassCP("gh");
    ASSERT_TRUE(gh2 != nullptr);
    ECClassCP ij2 = testSchemaCopy2->GetClassCP("ij");
    ASSERT_TRUE(ij2 != nullptr);
    ECClassCP kl2 = testSchemaCopy2->GetClassCP("kl");
    ASSERT_TRUE(kl2 != nullptr);
    ECClassCP mn2 = testSchemaCopy2->GetClassCP("mn");
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
    Utf8String schemaName = "testSchema";
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(testSchema, schemaName, 1, 0));
    testSchema->SetAlias("ts");
    ECEntityClassP baseClass = NULL;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(baseClass, "BaseClass"));
    PrimitiveECPropertyP primitiveProperty;
    ASSERT_EQ(ECObjectsStatus::Success, baseClass->CreatePrimitiveProperty(primitiveProperty, "testProp", PrimitiveType::PRIMITIVETYPE_String));
    ECEntityClassP derivedClass = NULL;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(derivedClass, "DerivedClass"));
    derivedClass->AddBaseClass(*baseClass);
    ECEntityClassP doubleDerivedClass = NULL;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(doubleDerivedClass, "DoubleDerivedClass"));
    doubleDerivedClass->AddBaseClass(*derivedClass);

    ASSERT_EQ(doubleDerivedClass->GetPropertyP("testProp"), baseClass->GetPropertyP("testProp")) << "Expected base and double derived classes to return the base property";

    ASSERT_EQ(ECObjectsStatus::Success, derivedClass->CreatePrimitiveProperty(primitiveProperty, "testProp", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_NE(doubleDerivedClass->GetPropertyP("testProp"), baseClass->GetPropertyP("testProp")) << "Expected base and double derived classes to return the base property and Derived Property respectively";
    ASSERT_EQ(doubleDerivedClass->GetPropertyP("testProp"), (derivedClass->GetPropertyP("testProp"))) << "Expected derived and double derived classes to return the derived Propertyy";

    ASSERT_EQ(ECObjectsStatus::Success, doubleDerivedClass->CreatePrimitiveProperty(primitiveProperty, "testProp", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_EQ(ECObjectsStatus::Success, derivedClass->RemoveProperty("testProp"));

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
    ECClassP root = testSchema->GetClassP("Root");
    ECClassP b1 = testSchema->GetClassP("B1");
    ECClassP b2 = testSchema->GetClassP("B2");
    ECClassP foo = testSchema->GetClassP("Foo");

    b1->AddBaseClass(*root);

    foo->AddBaseClass(*b1);
    foo->AddBaseClass(*b2);

    PrimitiveECPropertyP primitiveProp;
    ASSERT_EQ(ECObjectsStatus::Success, root->CreatePrimitiveProperty(primitiveProp, "A", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_EQ(foo->GetPropertyP("A"), root->GetPropertyP("A")) << "Expected Foo and Root to return the Root Property";

    ASSERT_EQ(ECObjectsStatus::Success, foo->CreatePrimitiveProperty(primitiveProp, "A", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_NE(foo->GetPropertyP("A"), root->GetPropertyP("A")) << "Expected Foo and Root to return the Foo Property and Root property Respectively";
    ASSERT_EQ(foo->GetPropertyP("A")->GetBaseProperty(), root->GetPropertyP("A")) << "Expected Foo and Root both to return Root  property";

    //now add same property A to class B2, as B2 has Higher Override Priority so traversal Order should not get changed.
    ASSERT_EQ(ECObjectsStatus::Success, b2->CreatePrimitiveProperty(primitiveProp, "A", PrimitiveType::PRIMITIVETYPE_String));

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
    ECClassP root = testSchema->GetClassP("Root");
    ECClassP b1 = testSchema->GetClassP("B1");
    ECClassP b2 = testSchema->GetClassP("B2");
    ECClassP foo = testSchema->GetClassP("Foo");

    b1->AddBaseClass(*root);

    foo->AddBaseClass(*b2);
    foo->AddBaseClass(*b1);

    PrimitiveECPropertyP primitiveProp;
    ASSERT_EQ(ECObjectsStatus::Success, b2->CreatePrimitiveProperty(primitiveProp, "A", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_EQ(foo->GetPropertyP("A"), b2->GetPropertyP("A")) << "Expected Foo and B2 to return the B2 Property";

    ASSERT_EQ(ECObjectsStatus::Success, foo->CreatePrimitiveProperty(primitiveProp, "A", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_NE(foo->GetPropertyP("A"), b2->GetPropertyP("A")) << "Expected Foo and B2 to return the Foo Property and B2 property Respectively";
    ASSERT_EQ(foo->GetPropertyP("A")->GetBaseProperty(), b2->GetPropertyP("A")) << "Expected Foo and B2 both to return B2  property";

    //now add same property A to class B1, as B1 has Lower Override Priority so traversal Order should not get changed.
    ASSERT_EQ(ECObjectsStatus::Success, b1->CreatePrimitiveProperty(primitiveProp, "A", PrimitiveType::PRIMITIVETYPE_String));

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
    ECClassP root = testSchema->GetClassP("Root");
    ECClassP b1 = testSchema->GetClassP("B1");
    ECClassP b2 = testSchema->GetClassP("B2");
    ECClassP foo = testSchema->GetClassP("Foo");

    b1->AddBaseClass(*root);

    foo->AddBaseClass(*b1);
    foo->AddBaseClass(*b2);

    PrimitiveECPropertyP primitiveProp;
    ASSERT_EQ(ECObjectsStatus::Success, root->CreatePrimitiveProperty(primitiveProp, "A", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, foo->CreatePrimitiveProperty(primitiveProp, "A", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, b2->CreatePrimitiveProperty(primitiveProp, "A", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_NE(foo->GetPropertyP("A"), root->GetPropertyP("A")) << "Expected Foo and Root to return the Foo Property and Root property Respectively";
    ASSERT_EQ(foo->GetPropertyP("A")->GetBaseProperty(), root->GetPropertyP("A")) << "Expected Foo and Root both to return Root property even after adding property A to class B2";

    ECSchemaPtr testSchemaCopy1 = RoundTripSchema(testSchema);
    ECClassCP root1 = testSchemaCopy1->GetClassCP("Root");
    ASSERT_TRUE(root1 != nullptr);
    ECClassCP b11 = testSchemaCopy1->GetClassCP("B1");
    ASSERT_TRUE(b11 != nullptr);
    ECClassCP b21 = testSchemaCopy1->GetClassCP("B2");
    ASSERT_TRUE(b21 != nullptr);
    ECClassCP foo1 = testSchemaCopy1->GetClassCP("Foo");
    ASSERT_TRUE(foo1 != nullptr);

    ASSERT_NE(foo1->GetPropertyP("A"), root1->GetPropertyP("A")) << "Expected Foo and Root to return the Foo Property and Root property Respectively";
    ASSERT_EQ(foo1->GetPropertyP("A")->GetBaseProperty(), root1->GetPropertyP("A")) << "Expected Foo and Root both to return Root property even after adding property A to class B2";
    }
