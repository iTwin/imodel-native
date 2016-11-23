/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/ECInterfaceTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include "BeXml/BeXml.h"

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct ECInterfaceTests : ECTestFixture {};

void CreateInterfaceClass(ECSchemaP ecSchema, Utf8StringCR className, ECInterfaceClassP& ecClass)
    {
    ECObjectsStatus status = ecSchema->CreateInterfaceClass(ecClass, className);
    ASSERT_EQ(ECObjectsStatus::Success, status) << "Failed to create ECInterfaceClass " << className;
    ASSERT_NE(nullptr, ecClass) << "ECInterfaceClass " << className << " null even though success returned";
    ASSERT_EQ(className, ecClass->GetName()) << "ECInterfaceClass " << className << " does not have expected name";
    }

void CreatePrimitiveInterfaceProperty(ECInterfaceClassP ecClass, Utf8StringCR name, PrimitiveECInterfacePropertyP& prop, PrimitiveType primitiveType = PrimitiveType::PRIMITIVETYPE_String)
    {
    ECObjectsStatus status = ecClass->CreatePrimitiveInterfaceProperty(prop, name, primitiveType);
    ASSERT_EQ(ECObjectsStatus::Success, status) << "Failed to create PrimitiveECInterfaceProperty " << name;
    ASSERT_NE(nullptr, prop) << "PrimitiveECInterfaceProperty " << name << " null even though success returned";
    ASSERT_EQ(name, prop->GetName()) << "PrimitiveECInterfaceProperty " << name << " does not have expected name";
    ASSERT_EQ(primitiveType, prop->GetType()) << "PrimitiveECInterfaceProperty  " << name << " does not have expected type";
    }

void CreateNavigationInterfaceProperty(ECInterfaceClassP ecClass, Utf8StringCR propName, ECRelationshipClassCR relClass, ECRelatedInstanceDirection direction, NavigationECInterfacePropertyP& navProp, PrimitiveType type = PRIMITIVETYPE_String)
    {
    ECObjectsStatus status = ecClass->CreateNavigationInterfaceProperty(navProp, propName, relClass, direction, type);
    ASSERT_EQ(ECObjectsStatus::Success, status) << "Failed to create navigation property " << propName << " on the ECInterfaceClass " << ecClass->GetFullName();
    ASSERT_NE(nullptr, navProp) << "Navigation property " << propName << " null though success returned";
    ASSERT_EQ(propName, navProp->GetName()) << "Navigation property " << propName << " does not have expected name";
    ASSERT_EQ(relClass.GetName(), navProp->GetRelationshipClass()->GetName()) << "Navigation property " << propName << " does not have expected relationship";
    ASSERT_EQ(direction, navProp->GetDirection()) << "Navigation property " << propName << " does not have expected direction";
    }

void ValidateDeserializedPrimitiveInterfaceProperty(ECInterfaceClassCP deserializedInterface, PrimitiveECInterfacePropertyCP expectedProperty, Utf8StringCR propName)
    {
    ECPropertyCP deserializedProp = deserializedInterface->GetPropertyP(propName.c_str());
    ASSERT_TRUE(nullptr != deserializedProp) << "Property '" << propName << "' not found in deserialized schema";
    PrimitiveECInterfacePropertyCP primProp = deserializedProp->GetAsPrimitiveInterfaceProperty();
    ASSERT_TRUE(nullptr != primProp) << "Property '" << propName << "' is not an PrimitiveECInterfaceProperty in deserialized schema.";

    ASSERT_STREQ(expectedProperty->GetName().c_str(), primProp->GetName().c_str()) << "Interface Primitive property '" << propName << "' does not have correct name in deserialized schema";
    ASSERT_STREQ(expectedProperty->GetTypeName().c_str(), primProp->GetTypeName().c_str()) << "Interface Primitive property '" << propName << "' does not have correct PrimitiveType in deserialized schema";
    }

void ValidateDeserializedNavInterfaceProperty(ECInterfaceClassCP deserializedInterface, NavigationECInterfacePropertyCP expectedProperty, Utf8StringCR propName)
    {
    ECPropertyCP deserializedProp = deserializedInterface->GetPropertyP(propName.c_str());
    ASSERT_TRUE(nullptr != deserializedProp) << "Property '" << propName << "' not found in deserialized schema";
    NavigationECInterfacePropertyCP navProp = deserializedProp->GetAsNavigationInterfaceProperty();
    ASSERT_TRUE(nullptr != navProp) << "Property '" << propName << "' is not an NavigationECInterfaceProperty in deserialized schema.";
    
    ASSERT_STREQ(expectedProperty->GetName().c_str(), navProp->GetName().c_str()) << "Interface Navigation property '" << propName << "' does not have correct name in deserialized schema";
    ASSERT_STREQ(expectedProperty->GetRelationshipClass()->GetFullName(), navProp->GetRelationshipClass()->GetFullName()) << "Interface Navigation property '" << propName << "' does not have correct relationship class in deserialized schema";
    ASSERT_EQ(expectedProperty->GetDirection(), navProp->GetDirection()) << "Interface Navigation property '" << propName << "' does not have correct direction in deserialized schema";
    }

void ValidateDeserializedInterface(ECSchemaPtr deserializedSchema, ECInterfaceClassCP expectedInterface, Utf8StringCR className)
    {
    ECClassCP deserialized = deserializedSchema->GetClassCP(className.c_str());
    ASSERT_TRUE(nullptr != deserialized) << "Class '" << className << "' not found in deserialized schema";
    ECInterfaceClassCP deserializedInterface = deserialized->GetInterfaceClassCP();
    ASSERT_TRUE(nullptr != deserializedInterface) << "Class '" << className << "' is not an ECInterfaceClass in deserialized schema";

    ASSERT_EQ(expectedInterface->AppliesToAny(), deserializedInterface->AppliesToAny()) << "The interface " << className << " does not have the same applies to any flag.";
    if (!expectedInterface->AppliesToAny())
        {
        ECEntityClassCP expectedAppliesTo = expectedInterface->GetAppliesTo();
        ECEntityClassCP derserializedAppliesTo = deserializedInterface->GetAppliesTo();
        ASSERT_TRUE(nullptr != expectedAppliesTo);
        ASSERT_TRUE(nullptr != derserializedAppliesTo);
        ASSERT_STREQ(expectedInterface->GetAppliesTo()->GetFullName(), deserializedInterface->GetAppliesTo()->GetFullName()) << "Interface Class " << className << " does not have the correct appliesTo attribute in deserialized schema";
        }

    for (auto const& expectedProperty : expectedInterface->GetProperties(false))
        {
        if (expectedProperty->GetIsNavigationInterfaceProperty())
            ValidateDeserializedNavInterfaceProperty(deserializedInterface, expectedProperty->GetAsNavigationInterfaceProperty(), expectedProperty->GetName());
        else
            ValidateDeserializedPrimitiveInterfaceProperty(deserializedInterface, expectedProperty->GetAsPrimitiveInterfaceProperty(), expectedProperty->GetName());
        }
    }

void ValidateRoundTripSerialization(ECSchemaPtr schema, bvector<ECInterfaceClassCP> expectedInterfaces)
    {
    Utf8String schemaString;
    SchemaWriteStatus writeStatus = schema->WriteToXmlString(schemaString);
    ASSERT_EQ(SchemaWriteStatus::Success, writeStatus) << "Failed to serialize schema with Interfaces";

    ECSchemaPtr deserializedSchema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus readStatus = ECSchema::ReadFromXmlString(deserializedSchema, schemaString.c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, readStatus) << "Failed to deserialize schema with Interfaces";

    for (auto const& interfaceClass : expectedInterfaces)
        ValidateDeserializedInterface(deserializedSchema, interfaceClass, interfaceClass->GetName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    11/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECInterfaceTests, TestInterfaceContracts)
    {
    ECSchemaPtr schemaPtr;
    ECSchema::CreateSchema(schemaPtr, "TestSchema", "ts", 1, 0, 0);

    ECSchemaP ecSchema = schemaPtr.get();
    ECInterfaceClassP interfaceClass;
    ECEntityClassP entityClass;
    ECRelationshipClassP relClass;
    ECStructClassP structClass;
    ECCustomAttributeClassP custAttrClass;

    PrimitiveECInterfacePropertyP iPrimProp;
    PrimitiveECPropertyP entityProp;
    PrimitiveECPropertyP relProp;
    PrimitiveECPropertyP custAttrProp;
    PrimitiveECPropertyP structProp;

    CreateInterfaceClass(ecSchema, "Interface", interfaceClass);
    CreatePrimitiveInterfaceProperty(interfaceClass, "TestProp", iPrimProp);
    
    ecSchema->CreateEntityClass(entityClass, "Entity");
    EXPECT_NE(ECObjectsStatus::Success, entityClass->AddInterface(*interfaceClass)) << "Should have failed because entity class does not have all the required properties";
    EXPECT_EQ(ECObjectsStatus::Success, entityClass->CreatePrimitiveProperty(entityProp, "TestProp", PrimitiveType::PRIMITIVETYPE_Integer)) << "Succeeded even though this is an invalid implementation";
    EXPECT_NE(ECObjectsStatus::Success, entityClass->AddInterface(*interfaceClass)) << "Should have failed to add the interface because entity class has the needed required property but it doesn't have the correct type";
    entityProp->SetType(PrimitiveType::PRIMITIVETYPE_String);
    EXPECT_EQ(ECObjectsStatus::Success, entityClass->AddInterface(*interfaceClass)) << "Should have succeeded now that the type of the property matches the interface property type";

    ecSchema->CreateStructClass(structClass, "Struct");
    EXPECT_NE(ECObjectsStatus::Success, structClass->AddInterface(*interfaceClass)) << "Should have failed because struct class does not have all the required properties";
    EXPECT_EQ(ECObjectsStatus::Success, structClass->CreatePrimitiveProperty(structProp, "TestProp", PrimitiveType::PRIMITIVETYPE_Integer)) << "Succeeded even though this is an invalid implementation";
    EXPECT_NE(ECObjectsStatus::Success, structClass->AddInterface(*interfaceClass)) << "Should have failed to add the interface because struct class has the needed required property but it doesn't have the correct type";
    structProp->SetType(PrimitiveType::PRIMITIVETYPE_String);
    EXPECT_EQ(ECObjectsStatus::Success, structClass->AddInterface(*interfaceClass)) << "Should have succeeded now that the type of the property matches the interface property type";
    
    ecSchema->CreateCustomAttributeClass(custAttrClass, "CA");
    EXPECT_NE(ECObjectsStatus::Success, custAttrClass->AddInterface(*interfaceClass)) << "Should have failed because custom attribute class does not have all the required properties";
    EXPECT_EQ(ECObjectsStatus::Success, custAttrClass->CreatePrimitiveProperty(custAttrProp, "TestProp", PrimitiveType::PRIMITIVETYPE_Integer)) << "Succeeded even though this is an invalid implementation";
    EXPECT_NE(ECObjectsStatus::Success, custAttrClass->AddInterface(*interfaceClass)) << "Should have failed to add the interface because custom attribute class has the needed required property but it doesn't have the correct type";
    custAttrProp->SetType(PrimitiveType::PRIMITIVETYPE_String);
    EXPECT_EQ(ECObjectsStatus::Success, custAttrClass->AddInterface(*interfaceClass)) << "Should have succeeded now that the type of the property matches the interface property type";
    
    ecSchema->CreateRelationshipClass(relClass, "Rel");
    relClass->GetSource().SetRoleLabel("Source");
    relClass->GetSource().AddClass(*interfaceClass);
    relClass->GetTarget().SetRoleLabel("Target");
    relClass->GetTarget().AddClass(*interfaceClass);

    EXPECT_NE(ECObjectsStatus::Success, relClass->AddInterface(*interfaceClass)) << "Should have failed because relationship class does not have all the required properties";
    EXPECT_EQ(ECObjectsStatus::Success, relClass->CreatePrimitiveProperty(relProp, "TestProp", PrimitiveType::PRIMITIVETYPE_Integer)) << "Succeeded even though this is an invalid implementation";
    EXPECT_NE(ECObjectsStatus::Success, relClass->AddInterface(*interfaceClass)) << "Should have failed to add the interface because relationship class has the needed required property but it doesn't have the correct type";
    relProp->SetType(PrimitiveType::PRIMITIVETYPE_String);
    EXPECT_EQ(ECObjectsStatus::Success, relClass->AddInterface(*interfaceClass)) << "Should have succeeded now that the type of the property matches the interface property type";

    bvector<ECInterfaceClassCP> interfaces = {interfaceClass};
    ValidateRoundTripSerialization(schemaPtr, interfaces);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    11/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECInterfaceTests, TestNavigationInterface)
    {
    ECSchemaPtr schemaPtr;
    ECSchema::CreateSchema(schemaPtr, "TestSchema", "ts", 1, 0, 0);

    ECSchemaP ecSchema = schemaPtr.get();
    ECInterfaceClassP source;
    ECInterfaceClassP target;
    ECRelationshipClassP relClass;
    PrimitiveECInterfacePropertyP iprimProperty;
    NavigationECInterfacePropertyP inavProperty;

    CreateInterfaceClass(ecSchema, "Source", source);
    CreateInterfaceClass(ecSchema, "Target", target);

    EXPECT_EQ(ECObjectsStatus::Success, source->AddInterface(*target)) << "Failed to add the interface when it should have succeeded.";

    CreatePrimitiveInterfaceProperty(source, "PrimProperty", iprimProperty);

    // Test Navigation Properties
    ecSchema->CreateRelationshipClass(relClass, "relClass");
    relClass->GetSource().AddClass(*source);
    relClass->GetSource().SetRoleLabel("Source");
    relClass->GetTarget().AddClass(*target);
    relClass->GetTarget().SetRoleLabel("Target");

    CreateNavigationInterfaceProperty(source, "MyTarget", *relClass, ECRelatedInstanceDirection::Forward, inavProperty);

    ECEntityClassP entityClass;
    PrimitiveECPropertyP primProprerty;
    NavigationECPropertyP navProperty;
    ecSchema->CreateEntityClass(entityClass, "Entity");
    EXPECT_NE(ECObjectsStatus::Success, entityClass->AddInterface(*source));
    EXPECT_EQ(ECObjectsStatus::Success, entityClass->CreatePrimitiveProperty(primProprerty, "PrimProperty"));
    EXPECT_EQ(ECObjectsStatus::Success, entityClass->CreateNavigationProperty(navProperty, "MyTarget", *relClass, ECRelatedInstanceDirection::Forward, PrimitiveType::PRIMITIVETYPE_String, false));
    EXPECT_EQ(ECObjectsStatus::Success, entityClass->AddInterface(*source));
    navProperty->Verify();

    bvector<ECInterfaceClassCP> interfaces = {source, target};
    ValidateRoundTripSerialization(schemaPtr, interfaces);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    11/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECInterfaceTests, TestInterfacePrimitivePropertyInheritance)
    {
    ECSchemaPtr schemaPtr;
    ECSchema::CreateSchema(schemaPtr, "TestSchema", "ts", 1, 0, 1);

    ECSchemaP ecSchema = schemaPtr.get();
    ECInterfaceClassP baseInterface;
    ECInterfaceClassP interface1;
    ECInterfaceClassP interface2;
    PrimitiveECInterfacePropertyP basePrimitive;
    PrimitiveECInterfacePropertyP primitive1;
    PrimitiveECInterfacePropertyP primitive2;

    CreateInterfaceClass(ecSchema, "BaseInterfaceClass", baseInterface);
    CreatePrimitiveInterfaceProperty(baseInterface, "BasePrimitive", basePrimitive);
    
    ECPropertyIterable iterable1 = baseInterface->GetProperties();
    bvector<ECPropertyP> testVector;
    for (ECPropertyP prop : iterable1)
        testVector.push_back(prop);
    EXPECT_EQ(1, testVector.size());
    EXPECT_STREQ("BasePrimitive", testVector[0]->GetName().c_str());

    CreateInterfaceClass(ecSchema, "InterfaceClass", interface1);
    interface1->AddInterface(*baseInterface);
    CreatePrimitiveInterfaceProperty(interface1, "Primitive1", primitive1);
    CreatePrimitiveInterfaceProperty(interface1, "Primitive2", primitive2);
    
    EXPECT_EQ(3, interface1->GetPropertyCount()) << "Interface " << interface1->GetName() << " should get the properties from all interfaces above it." ;
    
    CreateInterfaceClass(ecSchema, "DerivedInterface", interface2);
    interface2->AddInterface(*interface1);
    EXPECT_EQ(3, interface2->GetPropertyCount()) << "Interface " << interface2->GetName() << " should get the properties from all interfaces above it.";
    EXPECT_EQ(0, interface2->GetPropertyCount(false));

    PrimitiveECInterfacePropertyP overrideProp;
    ECObjectsStatus status = interface2->CreatePrimitiveInterfaceProperty(overrideProp, "BasePrimitive", PrimitiveType::PRIMITIVETYPE_Integer);
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, status) << "Attempting to override a primitive interface property should fail if the types differ";
    EXPECT_EQ(3, interface2->GetPropertyCount()) << "Changed even though return status was an error";
    EXPECT_EQ(0, interface2->GetPropertyCount(false));

    CreatePrimitiveInterfaceProperty(interface2, "BasePrimitive", overrideProp);

    testVector.clear();
    ECPropertyIterable iterable2 = interface2->GetProperties();
    for (ECPropertyP prop : iterable2)
        testVector.push_back(prop);
    EXPECT_EQ(3, testVector.size());
    EXPECT_EQ(4, interface2->GetPropertyCount());
    EXPECT_EQ(1, interface2->GetPropertyCount(false));

    bvector<ECInterfaceClassCP> interfaces = { baseInterface, interface1, interface2 };
    ValidateRoundTripSerialization(schemaPtr, interfaces);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    11/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECInterfaceTests, TestInterfaceNavigationPropertyInheritance)
    {
    ECSchemaPtr schemaPtr;
    ECSchema::CreateSchema(schemaPtr, "TestSchema", "ts", 1, 0, 1);

    ECSchemaP ecSchema = schemaPtr.get();
    ECInterfaceClassP baseInterface;
    ECInterfaceClassP interface1;
    ECInterfaceClassP interface2;
    ECEntityClassP entity;
    ECRelationshipClassP relClass;
    NavigationECInterfacePropertyP baseNavProp;
    NavigationECInterfacePropertyP navProp;

    ecSchema->CreateEntityClass(entity, "Target");
    CreateInterfaceClass(ecSchema, "BaseInterface", baseInterface);
    CreateInterfaceClass(ecSchema, "Interface1", interface1);
    CreateInterfaceClass(ecSchema, "Interface2", interface2);

    ecSchema->CreateRelationshipClass(relClass, "relClass");
    relClass->GetSource().AddClass(*baseInterface);
    relClass->GetSource().SetRoleLabel("Source");
    relClass->GetTarget().AddClass(*entity);
    relClass->GetTarget().SetRoleLabel("Target");

    CreateNavigationInterfaceProperty(baseInterface, "MyTarget", *relClass, ECRelatedInstanceDirection::Forward, baseNavProp);

    interface1->AddInterface(*baseInterface);
    CreateNavigationInterfaceProperty(interface1, "MyTarget", *relClass, ECRelatedInstanceDirection::Forward, navProp, PrimitiveType::PRIMITIVETYPE_Long);
    EXPECT_EQ(1, interface1->GetPropertyCount(false)) << "The count didn't increment even though a new property was successfully added.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    11/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECInterfaceTests, TestMultiInheritanceOnInterfaceClass)
    {
    ECSchemaPtr schemaPtr;
    ECSchema::CreateSchema(schemaPtr, "TestSchema", "ts", 1, 0, 2);
    ECSchemaP ecSchema = schemaPtr.get();

    ECInterfaceClassP a;
    ECInterfaceClassP b;
    ECInterfaceClassP ab;
    ECInterfaceClassP c;
    ECInterfaceClassP abc;
    PrimitiveECInterfacePropertyP aProp;
    PrimitiveECInterfacePropertyP bProp;
    PrimitiveECInterfacePropertyP abProp;
    PrimitiveECInterfacePropertyP cProp;
    PrimitiveECInterfacePropertyP aPropOverride;
    //PrimitiveECInterfacePropertyP bPropOverride;
    PrimitiveECInterfacePropertyP abPropOverride;

    ecSchema->CreateInterfaceClass(a, "a");
    ecSchema->CreateInterfaceClass(b, "b");
    ecSchema->CreateInterfaceClass(ab, "ab");
    ecSchema->CreateInterfaceClass(c, "c");
    ecSchema->CreateInterfaceClass(abc, "abc");

    CreatePrimitiveInterfaceProperty(a, "aProp", aProp);
    CreatePrimitiveInterfaceProperty(b, "bProp", bProp);

    ab->AddInterface(*a);
    ab->AddInterface(*b);

    ECPropertyIterable iterable1 = ab->GetProperties();
    bvector<ECPropertyP> testVector;
    for (auto prop : iterable1)
        testVector.push_back(prop);

    bvector<Utf8CP> expectedProps = {"aProp", "bProp"};
    for (size_t i = 0; i < testVector.size(); i++)
        {
        EXPECT_STREQ(expectedProps[i], testVector[i]->GetName().c_str());
        }

    EXPECT_EQ(2, testVector.size());
    EXPECT_EQ(2, ab->GetPropertyCount(true));
    EXPECT_EQ(0, ab->GetPropertyCount(false));

    c->CreatePrimitiveInterfaceProperty(cProp, "cProp", PRIMITIVETYPE_String);

    abc->AddInterface(*ab);
    abc->AddInterface(*c);

    ECPropertyIterable iterable2 = abc->GetProperties();
    testVector.clear();
    for (auto prop : iterable2)
        testVector.push_back(prop);

    expectedProps = {"aProp", "bProp", "cProp"};
    for (size_t i = 0; i < testVector.size(); i++)
        {
        EXPECT_STREQ(expectedProps[i], testVector[i]->GetName().c_str());
        }

    EXPECT_EQ(3, testVector.size());
    EXPECT_EQ(3, abc->GetPropertyCount(true));
    EXPECT_EQ(0, abc->GetPropertyCount(false));

    CreatePrimitiveInterfaceProperty(ab, "abProp", abProp);
    EXPECT_EQ(3, ab->GetPropertyCount(true));
    EXPECT_EQ(1, ab->GetPropertyCount(false));
    EXPECT_EQ(4, abc->GetPropertyCount(true));
    EXPECT_EQ(0, abc->GetPropertyCount(false));

    // Attempt to override with invalid an invalid prop
    EXPECT_NE(ECObjectsStatus::Success, ab->CreatePrimitiveInterfaceProperty(aPropOverride, "aProp", PrimitiveType::PRIMITIVETYPE_Long)) << "Success was returned even though this is an invalid property override.";
    EXPECT_EQ(1, ab->GetPropertyCount(false)) << "The number of properties changed even though Success was not returned.";
    aPropOverride = nullptr;
    EXPECT_EQ(ECObjectsStatus::Success, ab->CreatePrimitiveInterfaceProperty(aPropOverride, "aProp", PrimitiveType::PRIMITIVETYPE_String)) << "Failed to override with valid primitive type";
    aPropOverride->SetDisplayLabel("Overriden");
    EXPECT_EQ(2, ab->GetPropertyCount(false)) << "The number of properties did not increase even though a new property was added.";
    EXPECT_EQ(4, ab->GetPropertyCount(true));
    EXPECT_EQ(5, abc->GetPropertyCount(true));

    EXPECT_STREQ("Overriden", abc->GetPropertyP("aProp")->GetDisplayLabel().c_str()) << "This is not the overriden property.";

    EXPECT_NE(ECObjectsStatus::Success, abc->CreatePrimitiveInterfaceProperty(abPropOverride, "abProp", PrimitiveType::PRIMITIVETYPE_Boolean));
    EXPECT_EQ(0, abc->GetPropertyCount(false));
    abPropOverride = nullptr;
    EXPECT_EQ(ECObjectsStatus::Success, abc->CreatePrimitiveInterfaceProperty(abPropOverride, "abProp", PrimitiveType::PRIMITIVETYPE_String));
    abPropOverride->SetDisplayLabel("Overriden");
    EXPECT_EQ(1, abc->GetPropertyCount(false));

    EXPECT_STREQ("Overriden", abc->GetPropertyP("abProp")->GetDisplayLabel().c_str()) << "The overriden property " << abPropOverride->GetName().c_str() << " was not returned ";

    bvector<ECInterfaceClassCP> interfaces = {a, b, ab, c, abc};
    ValidateRoundTripSerialization(schemaPtr, interfaces);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    11/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECInterfaceTests, TestMultiInheritance)
    {
    ECSchemaPtr schemaPtr;
    ECSchema::CreateSchema(schemaPtr, "TestSchema", "ts", 1, 0, 0);
    ECSchemaP ecSchema = schemaPtr.get();

    ECInterfaceClassP iA;
    ECInterfaceClassP iB;
    ECInterfaceClassP iAB;
    ECEntityClassP a;
    ECEntityClassP b;
    ECEntityClassP ab;
    PrimitiveECInterfacePropertyP iAProp;
    PrimitiveECInterfacePropertyP iBProp;
    //PrimitiveECInterfacePropertyP iABProp;
    PrimitiveECPropertyP aProp;
    PrimitiveECPropertyP bProp;
    PrimitiveECPropertyP aProp2;
    PrimitiveECPropertyP bProp2;
    //PrimitiveECPropertyP abProp;

    ecSchema->CreateInterfaceClass(iA, "InterfaceA");
    ecSchema->CreateInterfaceClass(iB, "InterfaceB");
    ecSchema->CreateInterfaceClass(iAB, "InterfaceAB");
    iAB->AddInterface(*iA);
    iAB->AddInterface(*iB);
    
    ecSchema->CreateEntityClass(a, "A");
    a->AddInterface(*iA);

    ecSchema->CreateEntityClass(b, "B");
    b->AddInterface(*iB);

    ecSchema->CreateEntityClass(ab, "AB");
    ab->AddInterface(*iAB);

    CreatePrimitiveInterfaceProperty(iA, "AProp", iAProp);
    CreatePrimitiveInterfaceProperty(iB, "BProp", iBProp);
    //CreateIPrimitiveProperty(iAB, "ABProp", iABProp);
    
    EXPECT_FALSE(ecSchema->Validate()) << "The schema passed validation even though there are EntityClasses that don't implement all ";

    EXPECT_EQ(0, a->GetPropertyCount(true));
    EXPECT_NE(ECObjectsStatus::Success, a->CreatePrimitiveProperty(aProp, "AProp", PrimitiveType::PRIMITIVETYPE_Double));

    a->CreatePrimitiveProperty(aProp, "AProp");
    b->CreatePrimitiveProperty(bProp, "BProp");
    ab->CreatePrimitiveProperty(aProp2, "AProp");
    ab->CreatePrimitiveProperty(bProp2, "BProp");

    bvector<ECInterfaceClassCP> interfaces = {iA, iB, iAB};
    ValidateRoundTripSerialization(schemaPtr, interfaces);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    11/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECInterfaceTests, TestNarrowingConstraints)
    {
    ECSchemaPtr schemaPtr;
    ECSchema::CreateSchema(schemaPtr, "TestSchema", "ts", 1, 0, 0);

    ECSchemaP ecSchema = schemaPtr.get();

    ECInterfaceClassP ia;
    ECInterfaceClassP ib;
    ECInterfaceClassP ic;
    ECEntityClassP a;
    ECEntityClassP b;

    ecSchema->CreateInterfaceClass(ia, "ia");
    ecSchema->CreateInterfaceClass(ib, "ib");
    ecSchema->CreateInterfaceClass(ic, "ic");
    ecSchema->CreateEntityClass(a, "A");
    ecSchema->CreateEntityClass(b, "B");

    EXPECT_EQ(ECObjectsStatus::Success, ia->SetAppliesTo(*a));
    EXPECT_NE(ECObjectsStatus::Success, ib->AddInterface(*ia)) << "The interface was added when the applies to constraint is wider than the added interface.";
    EXPECT_EQ(ECObjectsStatus::Success, ib->SetAppliesTo(*b)) << "Unable to set the appliesTo attribute to " << b->GetName().c_str() << ".";
    EXPECT_NE(ECObjectsStatus::Success, ib->AddInterface(*ia)) << "The interface was added when the applies to constraints of the two interfaces are unrelated classes.";

    b->AddBaseClass(*a);

    EXPECT_EQ(ECObjectsStatus::Success, ib->AddInterface(*ia)) << "Unable to add " << ia->GetName() << " as an interface to " << ib->GetName().c_str() << " even though they should be valid.";

    bvector<ECInterfaceClassCP> interfaces = {ia, ib, ic};
    ValidateRoundTripSerialization(schemaPtr, interfaces);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    11/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECInterfaceTests, TestInvalidPropertyOperations)
    {
    ECSchemaPtr schemaPtr;
    ECSchema::CreateSchema(schemaPtr, "TestSchema", "ts", 1, 0, 0);

    ECSchemaP ecSchema = schemaPtr.get();

    ECInterfaceClassP interfaceClass;
    ECStructClassP structClass;
    PrimitiveECPropertyP primProp;
    PrimitiveECInterfacePropertyP iPrimProp;
    StructECPropertyP structProp;
    PrimitiveArrayECPropertyP primArrayProp;
    StructArrayECPropertyP structArrayProp;

    ecSchema->CreateStructClass(structClass, "Struct");

    ECObjectsStatus status = ecSchema->CreateInterfaceClass(interfaceClass, "InterfaceClass");
    EXPECT_EQ(ECObjectsStatus::Success, status);

    status = interfaceClass->CreatePrimitiveProperty(primProp, "InvalidPrimProp");
    EXPECT_EQ(ECObjectsStatus::PropertyNotSupported, status);

    status = interfaceClass->CreateStructProperty(structProp, "InvalidStructProp", *structClass);
    EXPECT_EQ(ECObjectsStatus::PropertyNotSupported, status);

    status = interfaceClass->CreatePrimitiveArrayProperty(primArrayProp, "InvalidPrimArrayProp");
    EXPECT_EQ(ECObjectsStatus::PropertyNotSupported, status);

    status = interfaceClass->CreateStructArrayProperty(structArrayProp, "InvalidStructArrayProp", *structClass);
    EXPECT_EQ(ECObjectsStatus::PropertyNotSupported, status);

    status = interfaceClass->CreatePrimitiveInterfaceProperty(iPrimProp, "InterfacePrimitive", PrimitiveType::PRIMITIVETYPE_Integer);
    ASSERT_EQ(ECObjectsStatus::Success, status);
    ASSERT_NE(nullptr, iPrimProp);

    ECValue value;
    value.SetInteger(355);
    EXPECT_EQ(ECObjectsStatus::OperationNotSupported, iPrimProp->SetMaximumValue(value)); 
    EXPECT_FALSE(iPrimProp->IsMaximumValueDefined());
    
    value.SetInteger(5);
    EXPECT_EQ(ECObjectsStatus::OperationNotSupported, iPrimProp->SetMinimumValue(value));
    EXPECT_FALSE(iPrimProp->IsMinimumValueDefined());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    11/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECInterfaceTests, TestInvalidCustomAttributesOnInterfaceProperty)
    {
    ECSchemaPtr schemaPtr;
    ECSchemaP ecSchema;
    ECInterfaceClassP interfaceClass;
    ECCustomAttributeClassP customAttributeClass;
    PrimitiveECInterfacePropertyP primProperty;
    ECRelationshipClassP relClass;
    NavigationECInterfacePropertyP navProperty;
    
    ECSchema::CreateSchema(schemaPtr, "TestSchema", "ts", 1, 0, 0);
    ecSchema = schemaPtr.get();

    ecSchema->CreateInterfaceClass(interfaceClass, "InterfaceClass");
    ecSchema->CreateCustomAttributeClass(customAttributeClass, "CustomAttribClass");
    ecSchema->CreateRelationshipClass(relClass, "relClass");
    relClass->GetSource().AddClass(*interfaceClass);
    relClass->GetTarget().AddClass(*interfaceClass);
    
    ECClassP ecClass = ecSchema->GetClassP("CustomAttribClass");
    StandaloneECEnablerPtr enabler = ecClass->GetDefaultStandaloneEnabler();
    IECInstancePtr instance = enabler->CreateInstance().get();

    ECObjectsStatus status = interfaceClass->CreatePrimitiveInterfaceProperty(primProperty, "primProperty", PRIMITIVETYPE_String);
    ASSERT_EQ(ECObjectsStatus::Success, status);
        {
        DISABLE_ASSERTS
        EXPECT_NE(ECObjectsStatus::Success, primProperty->SetCustomAttribute(*instance));
        }

    status = interfaceClass->CreateNavigationInterfaceProperty(navProperty, "navProperty", *relClass, ECRelatedInstanceDirection::Forward);
    ASSERT_EQ(ECObjectsStatus::Success, status);

        {
        DISABLE_ASSERTS
        EXPECT_NE(ECObjectsStatus::Success, navProperty->SetCustomAttribute(*instance));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    11/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECInterfaceTests, TestInterfaceDeserialization)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECInterfaceClass typeName='IA' appliesTo='A'>"
        "       <ECInterfaceProperty propertyName='property' typeName='int'/>"
        "   </ECInterfaceClass>"
        "   <ECEntityClass typeName='A' modifier='abstract'>"
        "       <Interface>IA</Interface>"
        "       <ECProperty propertyName='property' typeName='int'/>"
        "   </ECEntityClass>"
        "</ECSchema>";


    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);

    ECEntityClassCP entityClassA = schema->GetClassCP("A")->GetEntityClassCP();
    ASSERT_NE(nullptr, entityClassA->GetPropertyP("property")->GetAsPrimitiveProperty());
    ASSERT_EQ(nullptr, entityClassA->GetPropertyP("property")->GetAsPrimitiveInterfaceProperty());

    ECInterfaceClassCP interfaceClassA = schema->GetClassCP("IA")->GetInterfaceClassCP();
    ASSERT_EQ(nullptr, interfaceClassA->GetPropertyP("property")->GetAsPrimitiveProperty());
    ASSERT_NE(nullptr, interfaceClassA->GetPropertyP("property")->GetAsPrimitiveInterfaceProperty());
    ASSERT_NE(nullptr, interfaceClassA->GetPropertyP("property")->GetAsInterfaceProperty());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    11/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECInterfaceTests, TestFailureWhenInterfacePropertyConflicts)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='01.00.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECInterfaceClass typeName='InterfaceA' appliesTo='Any'>"
        "       <IECProperty propertyName='AProp' typeName='string' />"
        "   </ECInterfaceClass>"
        "   <ECInterfaceClass typeName = 'InterfaceB' appliesTo = 'Any'>"
        "       <IECProperty propertyName='BProp' typeName='string' />"
        "   </ECInterfaceClass>"
        "   <ECInterfaceClass typeName='ConflictB' appliesTo='AB'>"
        "       <IECProperty propertyName='BProp' typeName='int' />"
        "   </ECInterfaceClass>"
        "   <ECEntityClass typeName='A'>"
        "       <ECProperty propertyName='AProp' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='B'>"
        "       <Interface>InterfaceB</Interface>"
        "       <ECProperty propertyName='BProp' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='AB'>"
        "       <BaseClass>B</BaseClass>"
        "       <Interface>ConflictB</Interface>"
        "       <ECProperty propertyName='BProp' typeName='int' />"
        "   </ECEntityClass>"
        "</ECSchema>";


    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_NE(SchemaReadStatus::Success, status);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    11/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECInterfaceTests, TestDeserializationFailureWithInvalidProperties)
    {
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECInterfaceClass typeName='IA' modifier='abstract' appliesTo='A'>"
        "       <ECProperty propertyName='property' typeName='int'/>"
        "   </ECInterfaceClass>"
        "   <ECEntityClass typeName='A' modifier='abstract'>"
        "       <Interface>IA</Interface>"
        "       <ECProperty propertyName='Property' typeName='string'/>"
        "   </ECEntityClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_NE(SchemaReadStatus::Success, status) << "Schema with a PrimitiveECProperty on an ECInterfaceClass should fail to deserialize.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECInterfaceClass typeName='IA' modifier='abstract' appliesTo='A'>"
        "       <ECStructProperty propertyName='property' typeName='myStruct'/>"
        "   </ECInterfaceClass>"
        "   <ECEntityClass typeName='A' modifier='abstract'>"
        "       <Interface>IA</Interface>"
        "       <ECStructProperty propertyName='Property' typeName='myStruct'/>"
        "   </ECEntityClass>"
        "   <ECStructClass typeName='myStruct'>"
        "       <ECProperty propertyName='prop' typeName='string'/>"
        "   </ECStructClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_NE(SchemaReadStatus::Success, status) << "Schema with a StructECProperty on an ECInterfaceClass should fail to deserialize.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECInterfaceClass typeName='IA' modifier='abstract' appliesTo='A'>"
        "       <ECStructArrayProperty propertyName='property' typeName='myStruct'/>"
        "   </ECInterfaceClass>"
        "   <ECEntityClass typeName='A' modifier='abstract'>"
        "       <Interface>IA</Interface>"
        "       <ECStructProperty propertyName='Property' typeName='myStruct'/>"
        "   </ECEntityClass>"
        "   <ECStructClass typeName='myStruct'>"
        "       <ECProperty propertyName='prop' typeName='string'/>"
        "   </ECStructClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_NE(SchemaReadStatus::Success, status) << "Schema with a StructArrayECProperty on an ECInterfaceClass should fail to deserialize.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECRelationshipClass typeName='testRel' >"
        "       <Source multiplicity='(0..1)' polymorphic='true' roleLabel='Source'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='true' roleLabel='Source'>"
        "           <Class class='A' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "   <ECInterfaceClass typeName='IA' modifier='abstract' appliesTo='A'>"
        "       <ECNavigationProperty propertyName='property' typeName='long' relationshipName='testRel' direction='forward' />"
        "   </ECInterfaceClass>"
        "   <ECEntityClass typeName='A' modifier='abstract'>"
        "       <Interface>IA</Interface>"
        "       <ECNavigationProperty propertyName='Property' typeName='long' relationshipName='testRel' direction='forward' />"
        "   </ECEntityClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_NE(SchemaReadStatus::Success, status) << "Schema with a NavigationECProperty on an ECInterfaceClass should fail to deserialize.";
    }
    }

END_BENTLEY_ECN_TEST_NAMESPACE