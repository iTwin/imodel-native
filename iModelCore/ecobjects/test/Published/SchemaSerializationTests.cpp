/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include "BeXml/BeXml.h"
#include <regex>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaXmlSerializationTest : ECTestFixture
    {
    };

struct SchemaJsonSerializationTest : ECTestFixture
    {
    protected:
        static ECSchemaPtr CreateSchemaWithNoItems()
            {
            ECSchemaPtr schema;
            Utf8String schemaName = "ExampleSchema";
            Utf8String schemaAlias = "ex";
            uint32_t versionRead = 3;
            uint32_t versionWrite = 1;
            uint32_t versionMinor = 0;
            ECVersion ecVersion = ECVersion::Latest;
            Utf8String label = "Example Schema";
            Utf8String description = "The quick brown fox jumps over the lazy dog.";

            ECSchemaPtr refSchemaA;
            Utf8String refSchemaAName = "refSchemaA";
            Utf8String refSchemaAAlias = "refA";
            ECSchema::CreateSchema(refSchemaA, refSchemaAName, refSchemaAAlias, versionRead, versionWrite, versionMinor, ecVersion);
            ECCustomAttributeClassP customAttributeClassA;
            refSchemaA->CreateCustomAttributeClass(customAttributeClassA, "SomeCustomAttributeClass");
            customAttributeClassA->SetContainerType(CustomAttributeContainerType::Any);
            customAttributeClassA->SetDescription("SomeCustomAttributeClass description! How exciting!");

            ECSchemaPtr refSchemaB;
            Utf8String refSchemaBAlias = "refB";
            Utf8String refSchemaBName = "refSchemaB";
            ECSchema::CreateSchema(refSchemaB, refSchemaBName, refSchemaBAlias, versionRead, versionWrite, versionMinor, ecVersion);
            ECCustomAttributeClassP customAttributeClassB;
            refSchemaB->CreateCustomAttributeClass(customAttributeClassB, "AnotherCustomAttributeClass");
            customAttributeClassB->SetContainerType(CustomAttributeContainerType::Any);
            customAttributeClassB->SetDescription("AnotherCustomAttributeClass description! Wowzers!");

            ECSchema::CreateSchema(schema, schemaName, schemaAlias, versionRead, versionWrite, versionMinor, ecVersion);
            schema->SetDisplayLabel(label);
            schema->SetDescription(description);
            schema->AddReferencedSchema(*refSchemaA);
            schema->AddReferencedSchema(*refSchemaB);

            IECInstancePtr customAttributeA = customAttributeClassA->GetDefaultStandaloneEnabler()->CreateInstance();
            ECValue valA;
            valA.SetUtf8CP("some string");
            customAttributeA->SetValue("Primitive", valA);
            schema->SetCustomAttribute(*customAttributeA);

            IECInstancePtr customAttributeB = customAttributeClassB->GetDefaultStandaloneEnabler()->CreateInstance();
            ECValue valB;
            valB.SetUtf8CP("another string");
            customAttributeB->SetValue("Primitive", valB);
            schema->SetCustomAttribute(*customAttributeB);

            return schema;
            }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                    Carole.MacDonald                01/2010
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaXmlSerializationTest, ExpectSuccessWithSerializingBaseClasses)
    {
    ECSchemaPtr schema;
    ECSchemaPtr schema2;
    ECSchemaPtr schema3;

    ECSchema::CreateSchema(schema, "Widget", "ecw", 5, 5, 5);
    ECSchema::CreateSchema(schema2, "BaseSchema", "base", 5, 5, 5);
    ECSchema::CreateSchema(schema3, "BaseSchema2", "base", 5, 5, 5);

    ECEntityClassP class1;
    ECEntityClassP baseClass;
    ECEntityClassP anotherBase;
    ECEntityClassP gadget;
    ECEntityClassP bolt;
    ECEnumerationP enumeration;
    schema->CreateEntityClass(class1, "TestClass");
    schema->CreateEntityClass(gadget, "Gadget");
    schema->CreateEntityClass(bolt, "Bolt");
    schema->CreateEnumeration(enumeration, "Enumeration", PrimitiveType::PRIMITIVETYPE_Integer);
    enumeration->SetDisplayLabel("This is a display label.");
    enumeration->SetDescription("This is a description.");
    ECEnumeratorP enumerator;
    EXPECT_EQ(ECObjectsStatus::Success, enumeration->CreateEnumerator(enumerator, "Enumerator1", 1));
    enumerator->SetDisplayLabel("First");
    EXPECT_EQ(ECObjectsStatus::Success, enumeration->CreateEnumerator(enumerator, "Enumerator2", 2));
    enumerator->SetDisplayLabel("Second");
    EXPECT_EQ(ECObjectsStatus::Success, enumeration->CreateEnumerator(enumerator, "Enumerator3", 3));
    enumerator->SetDisplayLabel("Third");

    PrimitiveECPropertyP prop;
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreateEnumerationProperty(prop, "EnumeratedProperty", *enumeration));

    schema2->CreateEntityClass(baseClass, "BaseClass");
    schema3->CreateEntityClass(anotherBase, "AnotherBase");

    EXPECT_EQ(ECObjectsStatus::SchemaNotFound, class1->AddBaseClass(*baseClass));
    schema->AddReferencedSchema(*schema2);
    schema->AddReferencedSchema(*schema3);
    EXPECT_EQ(ECObjectsStatus::Success, class1->AddBaseClass(*baseClass));
    EXPECT_EQ(ECObjectsStatus::Success, class1->AddBaseClass(*anotherBase));
    EXPECT_EQ(ECObjectsStatus::Success, gadget->AddBaseClass(*class1));

    SchemaWriteStatus status = schema->WriteToXmlFile(ECTestFixture::GetTempDataPath(L"base.xml").c_str());
    EXPECT_EQ(SchemaWriteStatus::Success, status);

    status = schema->WriteToXmlFile(ECTestFixture::GetTempDataPath(L"base_ec3.xml").c_str(), ECVersion::V3_2);
    EXPECT_EQ(SchemaWriteStatus::Success, status);

    WString ecSchemaXmlString;
    status = schema->WriteToXmlString(ecSchemaXmlString);
    EXPECT_EQ(SchemaWriteStatus::Success, status);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Robert.Schili                       12/2015
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaXmlSerializationTest, SerializeComprehensiveSchema)
    {
    //Load Bentley_Standard_CustomAttributes
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SearchPathSchemaFileLocaterPtr schemaLocater;
    bvector<WString> searchPaths;
    searchPaths.push_back(ECTestFixture::GetTestDataPath(L""));
    schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
    schemaContext->AddSchemaLocater(*schemaLocater);

    SchemaKey schemaKey("Bentley_Standard_CustomAttributes", 1, 12);
    ECSchemaPtr standardCASchema = schemaContext->LocateSchema(schemaKey, SchemaMatchType::Latest);
    EXPECT_TRUE(standardCASchema.IsValid());

    //Compose our new schema
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ComprehensiveSchema", "cmpr", 1, 5, 2);
    schema->SetDescription("Comprehensive Schema to demonstrate use of all ECSchema concepts.");
    schema->SetDisplayLabel("Comprehensive Schema");
    schema->AddReferencedSchema(*standardCASchema);
    schema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());
    schema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema());

    ECEntityClassP baseEntityClass;
    ECEntityClassP entityClass;
    ECStructClassP structClass;
    ECCustomAttributeClassP classCustomAttributeClass;
    ECCustomAttributeClassP generalCustomAttributeClass;
    ECEnumerationP enumeration;

    schema->CreateEntityClass(baseEntityClass, "BaseEntity");
    PrimitiveECPropertyP inheritedPrimitiveProperty;
    baseEntityClass->CreatePrimitiveProperty(inheritedPrimitiveProperty, "InheritedProperty", PrimitiveType::PRIMITIVETYPE_String);
    baseEntityClass->SetClassModifier(ECClassModifier::Abstract);
    baseEntityClass->SetDisplayLabel("Base Entity");
    baseEntityClass->SetDescription("Base Entity Description");

    schema->CreateEntityClass(entityClass, "Entity");
    entityClass->SetClassModifier(ECClassModifier::Sealed);
    entityClass->AddBaseClass(*baseEntityClass);
    PrimitiveECPropertyP primitiveProperty1;
    entityClass->CreatePrimitiveProperty(primitiveProperty1, "Primitive1", PrimitiveType::PRIMITIVETYPE_Binary);
    primitiveProperty1->SetDisplayLabel("Property Display Label");
    PrimitiveECPropertyP primitiveProperty2;
    entityClass->CreatePrimitiveProperty(primitiveProperty2, "Primitive2", PrimitiveType::PRIMITIVETYPE_Boolean);
    primitiveProperty2->SetDescription("Property Description");
    PrimitiveECPropertyP primitiveProperty3;
    entityClass->CreatePrimitiveProperty(primitiveProperty3, "Primitive3", PrimitiveType::PRIMITIVETYPE_DateTime);
    primitiveProperty3->SetIsReadOnly(true);
    PrimitiveECPropertyP primitiveProperty4;
    entityClass->CreatePrimitiveProperty(primitiveProperty4, "Primitive4", PrimitiveType::PRIMITIVETYPE_Double);
    PrimitiveECPropertyP primitiveProperty5;
    entityClass->CreatePrimitiveProperty(primitiveProperty5, "Primitive5", PrimitiveType::PRIMITIVETYPE_IGeometry);
    PrimitiveECPropertyP primitiveProperty6;
    entityClass->CreatePrimitiveProperty(primitiveProperty6, "Primitive6", PrimitiveType::PRIMITIVETYPE_Integer);
    PrimitiveECPropertyP primitiveProperty7;
    entityClass->CreatePrimitiveProperty(primitiveProperty7, "Primitive7", PrimitiveType::PRIMITIVETYPE_Long);
    PrimitiveECPropertyP primitiveProperty8;
    entityClass->CreatePrimitiveProperty(primitiveProperty8, "Primitive8", PrimitiveType::PRIMITIVETYPE_Point2d);
    PrimitiveECPropertyP primitiveProperty9;
    entityClass->CreatePrimitiveProperty(primitiveProperty9, "Primitive9", PrimitiveType::PRIMITIVETYPE_Point3d);
    PrimitiveECPropertyP primitiveProperty10;
    entityClass->CreatePrimitiveProperty(primitiveProperty10, "Primitive10", PrimitiveType::PRIMITIVETYPE_String);
    PrimitiveECPropertyP calculatedProperty;
    entityClass->CreatePrimitiveProperty(calculatedProperty, "Calculated", PrimitiveType::PRIMITIVETYPE_String);
    PrimitiveArrayECPropertyP arrayProperty;
    entityClass->CreatePrimitiveArrayProperty(arrayProperty, "Array", PrimitiveType::PRIMITIVETYPE_Long);

    ECClassCP calcSpecClass = standardCASchema->GetClassCP("CalculatedECPropertySpecification");
    IECInstancePtr calcSpecAttr = calcSpecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue v;
    v.SetUtf8CP("\"Primitve 10=\" & this.Primitive10");
    calcSpecAttr->SetValue("ECExpression", v);
    calculatedProperty->SetCalculatedPropertySpecification(calcSpecAttr.get());

    schema->CreateStructClass(structClass, "Struct");
    structClass->SetDisplayLabel("Struct Class");
    PrimitiveECPropertyP structPrimitive1;
    structClass->CreatePrimitiveProperty(structPrimitive1, "Primitive1", PrimitiveType::PRIMITIVETYPE_Integer);
    StructECPropertyP structProperty;
    entityClass->CreateStructProperty(structProperty, "Struct1", *structClass);

    StructArrayECPropertyP structArrayProperty;
    entityClass->CreateStructArrayProperty(structArrayProperty, "StructArray", *structClass);

    schema->CreateCustomAttributeClass(classCustomAttributeClass, "ClassCustomAttribute");
    classCustomAttributeClass->SetDescription("Custom Attribute that can only be applied to classes.");
    classCustomAttributeClass->SetContainerType(CustomAttributeContainerType::AnyClass);
    PrimitiveECPropertyP classCustomAttributeProperty;
    classCustomAttributeClass->CreatePrimitiveProperty(classCustomAttributeProperty, "Primitive", PrimitiveType::PRIMITIVETYPE_String);
    IECInstancePtr classCA = classCustomAttributeClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue cV;
    cV.SetUtf8CP("General Value on Class");
    classCA->SetValue("Primitive", cV);
    entityClass->SetCustomAttribute(*classCA);

    schema->CreateCustomAttributeClass(generalCustomAttributeClass, "GeneralCustomAttribute");
    generalCustomAttributeClass->SetDescription("Custom Attribute that can be applied to anything.");
    PrimitiveECPropertyP generalCustomAttributeProperty;
    generalCustomAttributeClass->CreatePrimitiveProperty(generalCustomAttributeProperty, "Primitive", PrimitiveType::PRIMITIVETYPE_String);
    IECInstancePtr generalCA = generalCustomAttributeClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue gV;
    gV.SetUtf8CP("General Value");
    generalCA->SetValue("Primitive", gV);
    schema->SetCustomAttribute(*generalCA);

    schema->CreateEnumeration(enumeration, "Enumeration", PrimitiveType::PRIMITIVETYPE_Integer);
    enumeration->SetDisplayLabel("This is a display label.");
    enumeration->SetDescription("This is a description.");
    ECEnumeratorP enumerator;
    EXPECT_EQ(ECObjectsStatus::Success, enumeration->CreateEnumerator(enumerator, "Enumerator1", 1));
    enumerator->SetDisplayLabel("First");
    EXPECT_EQ(ECObjectsStatus::Success, enumeration->CreateEnumerator(enumerator, "Enumerator2", 2));
    enumerator->SetDisplayLabel("Second");
    EXPECT_EQ(ECObjectsStatus::Success, enumeration->CreateEnumerator(enumerator, "Enumerator3", 3));
    enumerator->SetDisplayLabel("Third");

    PrimitiveECPropertyP prop;
    EXPECT_EQ(ECObjectsStatus::Success, entityClass->CreateEnumerationProperty(prop, "Enumerated", *enumeration));

    ECRelationshipClassP relationshipClass;
    schema->CreateRelationshipClass(relationshipClass, "RelationshipClass");
    PrimitiveECPropertyP relationshipProperty;
    relationshipClass->CreatePrimitiveProperty(relationshipProperty, "RelationshipProperty");
    relationshipClass->SetStrength(StrengthType::Referencing);
    relationshipClass->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    relationshipClass->GetSource().AddClass(*entityClass);
    relationshipClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relationshipClass->GetTarget().AddClass(*entityClass);
    relationshipClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::ZeroOne());

    NavigationECPropertyP navProp;
    entityClass->CreateNavigationProperty(navProp, "NavigationProperty", *relationshipClass, ECRelatedInstanceDirection::Forward);

    KindOfQuantityP kindOfQuantity;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateKindOfQuantity(kindOfQuantity, "MyKindOfQuantity"));
    kindOfQuantity->SetDescription("Kind of a Description here");
    kindOfQuantity->SetDisplayLabel("best quantity of all times");
    kindOfQuantity->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("CM"));
    kindOfQuantity->SetRelativeError(10e-3);
    kindOfQuantity->SetDefaultPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultRealU"));
    kindOfQuantity->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultReal"));
    kindOfQuantity->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("AmerFI"));

    WString fullSchemaName;
    fullSchemaName.AssignUtf8(schema->GetFullSchemaName().c_str());
    fullSchemaName.append(L".ecschema.xml");

    WString legacyFullSchemaName;
    legacyFullSchemaName.AssignUtf8(schema->GetLegacyFullSchemaName().c_str());
    legacyFullSchemaName.append(L".ecschema.xml");

    SchemaWriteStatus status2 = schema->WriteToXmlFile(ECTestFixture::GetTempDataPath(fullSchemaName.c_str()).c_str(), ECVersion::V3_2);
    EXPECT_EQ(SchemaWriteStatus::Success, status2);

    SchemaWriteStatus status3 = schema->WriteToXmlFile(ECTestFixture::GetTempDataPath(legacyFullSchemaName.c_str()).c_str(), ECVersion::V2_0);
    EXPECT_EQ(SchemaWriteStatus::Success, status3);

    WString ecSchemaXmlString;
    SchemaWriteStatus status4 = schema->WriteToXmlString(ecSchemaXmlString);
    EXPECT_EQ(SchemaWriteStatus::Success, status4);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                            Caleb.Shafer                           09/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaXmlSerializationTest, ExpectSuccessWithInheritedKindOfQuantities)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "testSchema", "ts", 1, 0, 0);
    schema->SetDescription("Schema to test Kind of Quantity Inheritance serialization.");
    schema->SetDisplayLabel("KOQ Inheritance Test Schema");
    schema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());
    schema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema());

    ECEntityClassP parentEntityClass;
    ECEntityClassP derivedEntityClass1;
    ECEntityClassP derivedEntityClass2;
    ECEntityClassP derivedEntityClass3;
    KindOfQuantityP kindOfQuantity;
    KindOfQuantityP kindOfQuantity2;

    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateKindOfQuantity(kindOfQuantity, "MyKindOfQuantity"));
    kindOfQuantity->SetDescription("Kind of a Description here");
    kindOfQuantity->SetDisplayLabel("best quantity of all times");
    kindOfQuantity->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("CM"));
    kindOfQuantity->SetRelativeError(10e-3);
    kindOfQuantity->SetDefaultPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultRealU"));
    kindOfQuantity->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultReal"));
    kindOfQuantity->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("AmerFI"));

    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateKindOfQuantity(kindOfQuantity2, "OverrideKindOfQuantity"));
    kindOfQuantity2->SetDescription("Kind of a Description here");
    kindOfQuantity2->SetDisplayLabel("best quantity of all times");
    kindOfQuantity2->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("CM"));
    kindOfQuantity2->SetRelativeError(10e-4);
    kindOfQuantity2->SetDefaultPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultRealU"));
    kindOfQuantity2->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultReal"));
    kindOfQuantity2->AddPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("AmerFI"));

    schema->CreateEntityClass(parentEntityClass, "ParentEntity");
    parentEntityClass->SetClassModifier(ECClassModifier::Abstract);
    parentEntityClass->SetDisplayLabel("Parent Entity");
    parentEntityClass->SetDescription("Parent Entity Description");
    PrimitiveECPropertyP parentPrimitiveProperty;
    parentEntityClass->CreatePrimitiveProperty(parentPrimitiveProperty, "InheritedProperty", PrimitiveType::PRIMITIVETYPE_String);
    parentPrimitiveProperty->SetKindOfQuantity(kindOfQuantity);

    schema->CreateEntityClass(derivedEntityClass1, "DerivedEntity1");
    derivedEntityClass1->AddBaseClass(*parentEntityClass);
    derivedEntityClass1->SetClassModifier(ECClassModifier::Abstract);
    derivedEntityClass1->SetDisplayLabel("Derived Entity 1");
    derivedEntityClass1->SetDescription("Derived Entity Description");
    PrimitiveECPropertyP derivedPrimitiveProperty1;
    derivedEntityClass1->CreatePrimitiveProperty(derivedPrimitiveProperty1, "InheritedProperty", PrimitiveType::PRIMITIVETYPE_String);
    derivedPrimitiveProperty1->SetBaseProperty(parentPrimitiveProperty);

    schema->CreateEntityClass(derivedEntityClass2, "DerivedEntity2");
    derivedEntityClass2->AddBaseClass(*derivedEntityClass1);
    derivedEntityClass2->SetClassModifier(ECClassModifier::Sealed);
    derivedEntityClass2->SetDisplayLabel("Derived Entity 2");
    derivedEntityClass2->SetDescription("Derived Entity Description");
    PrimitiveECPropertyP derivedPrimitiveProperty2;
    derivedEntityClass2->CreatePrimitiveProperty(derivedPrimitiveProperty2, "InheritedProperty", PrimitiveType::PRIMITIVETYPE_String);
    derivedPrimitiveProperty2->SetBaseProperty(derivedPrimitiveProperty1);

    schema->CreateEntityClass(derivedEntityClass3, "DerivedEntity3");
    derivedEntityClass3->AddBaseClass(*derivedEntityClass1);
    derivedEntityClass3->SetClassModifier(ECClassModifier::Sealed);
    derivedEntityClass3->SetDisplayLabel("Derived Entity 3");
    derivedEntityClass3->SetDescription("Derived Entity Description");
    PrimitiveECPropertyP derivedPrimitiveProperty3;
    derivedEntityClass3->CreatePrimitiveProperty(derivedPrimitiveProperty3, "InheritedProperty", PrimitiveType::PRIMITIVETYPE_String);
    derivedPrimitiveProperty3->SetBaseProperty(derivedPrimitiveProperty1);
    derivedPrimitiveProperty3->SetKindOfQuantity(kindOfQuantity2);

    SchemaWriteStatus writeStatus = schema->WriteToXmlFile(ECTestFixture::GetTempDataPath(L"InheritedKOQ.01.00.00.ecschema.xml").c_str());
    ASSERT_EQ(SchemaWriteStatus::Success, writeStatus);

    ECSchemaPtr readSchema;
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus readStatus = ECSchema::ReadFromXmlFile(readSchema, ECTestFixture::GetTempDataPath(L"InheritedKOQ.01.00.00.ecschema.xml").c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, readStatus);
    ASSERT_TRUE(readSchema.IsValid());

    PrimitiveECPropertyCP parentProp = readSchema->GetClassCP("ParentEntity")->GetPropertyP("InheritedProperty", false)->GetAsPrimitiveProperty();
    ASSERT_TRUE(parentProp != nullptr);
    ASSERT_TRUE(parentProp->IsKindOfQuantityDefinedLocally());
    ASSERT_STREQ("MyKindOfQuantity", parentProp->GetKindOfQuantity()->GetName().c_str());

    PrimitiveECPropertyCP derivedProp1 = readSchema->GetClassCP("DerivedEntity1")->GetPropertyP("InheritedProperty", false)->GetAsPrimitiveProperty();
    ASSERT_TRUE(derivedProp1 != nullptr);
    ASSERT_FALSE(derivedProp1->IsKindOfQuantityDefinedLocally());
    ASSERT_STREQ("MyKindOfQuantity", derivedProp1->GetKindOfQuantity()->GetName().c_str());

    PrimitiveECPropertyCP derivedProp2 = readSchema->GetClassCP("DerivedEntity2")->GetPropertyP("InheritedProperty", false)->GetAsPrimitiveProperty();
    ASSERT_TRUE(derivedProp2 != nullptr);
    ASSERT_FALSE(derivedProp2->IsKindOfQuantityDefinedLocally());
    ASSERT_STREQ("MyKindOfQuantity", derivedProp2->GetKindOfQuantity()->GetName().c_str());

    PrimitiveECPropertyCP derivedProp3 = readSchema->GetClassCP("DerivedEntity3")->GetPropertyP("InheritedProperty", false)->GetAsPrimitiveProperty();
    ASSERT_TRUE(derivedProp3 != nullptr);
    ASSERT_TRUE(derivedProp3->IsKindOfQuantityDefinedLocally());
    ASSERT_STREQ("OverrideKindOfQuantity", derivedProp3->GetKindOfQuantity()->GetName().c_str());
    }


void extendTargetXmlRegex(Utf8StringR regexStrOut, Utf8String str)
    {
    regexStrOut.append(".*" + str + ".*\n");
    }

std::regex generateTargetXmlRegex(Utf8StringR regexStrOut, bvector<Utf8String> orderedNames, Utf8String prefix = "")
    {
    for (Utf8String name : orderedNames)
        extendTargetXmlRegex(regexStrOut, prefix + name);

    std::regex r(regexStrOut.c_str(), std::regex::extended);
    return r;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                            Joseph.Urbano                          05/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaXmlSerializationTest, ExpectReferencedSchemasSerializedInOrder)
    {
    ECSchemaPtr schema1, schema2;
    bvector<ECSchemaPtr> refSchemas(4);

    bvector<Utf8String> refSchemaNames;
    refSchemaNames.push_back("aRefSchema");
    refSchemaNames.push_back("bRefSchema");
    refSchemaNames.push_back("cRefSchema");
    refSchemaNames.push_back("dRefSchema");

    Utf8String targetXmlRegexStr;
    std::regex targetXmlRegex = generateTargetXmlRegex(targetXmlRegexStr, refSchemaNames, "<ECSchemaReference name=\"");

    int i = 0;
    for(Utf8String name : refSchemaNames)
        ECSchema::CreateSchema(refSchemas[i++], name, "rs", 1, 0, 0);

    // add references in order, serialize, and make sure they're still in order
    ECSchema::CreateSchema(schema1, "testSchema1", "ts", 1, 0, 0);
    
    i = 0;
    for (ECSchemaPtr refSchema : refSchemas)
        schema1->AddReferencedSchema(*refSchema);

    Utf8String schemaXml;
    schema1->WriteToXmlString(schemaXml);

    EXPECT_TRUE(std::regex_search(schemaXml.c_str(), targetXmlRegex));

    // add references in reverse order, serialize, and make sure they're still in order
    bvector<ECSchemaPtr> refSchemasReversed;
    refSchemasReversed.resize(refSchemas.size());
    std::reverse_copy(refSchemas.begin(), refSchemas.end(), refSchemasReversed.begin());

    ECSchema::CreateSchema(schema2, "testSchema2", "ts", 1, 0, 0);

    for (ECSchemaPtr refSchema : refSchemasReversed)
        schema2->AddReferencedSchema(*refSchema);

    schema2->WriteToXmlString(schemaXml);

    EXPECT_TRUE(std::regex_search(schemaXml.c_str(), targetXmlRegex));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                            Joseph.Urbano                          05/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaXmlSerializationTest, ExpectCustomAttributesSerializedInOrder)
    {
    bvector<Utf8String> caClassNames;
    caClassNames.push_back("aCustomClass");
    caClassNames.push_back("bCustomClass");
    caClassNames.push_back("cCustomClass");
    caClassNames.push_back("dCustomClass");

    // create custom attribute classes and instances in order, verify they serialize in order
    {
    ECSchemaPtr schema;
    bvector<ECCustomAttributeClassP> caClasses(4);
    bvector<IECInstancePtr> caInstances(4);

    Utf8String targetXmlRegexStr;
    std::regex targetXmlRegex = generateTargetXmlRegex(targetXmlRegexStr, caClassNames, "<ECCustomAttributeClass typeName=\"");

    ECSchema::CreateSchema(schema, "testSchema", "ts", 1, 0, 0);

    int i = 0;
    for (Utf8String name : caClassNames)
        schema->CreateCustomAttributeClass(caClasses[i++], name);

    Utf8String schemaXml;
    schema->WriteToXmlString(schemaXml);

    EXPECT_TRUE(std::regex_search(schemaXml.c_str(), targetXmlRegex));

    extendTargetXmlRegex(targetXmlRegexStr, "<ECCustomAttributes>");
    targetXmlRegex = generateTargetXmlRegex(targetXmlRegexStr, caClassNames, "<");
    extendTargetXmlRegex(targetXmlRegexStr, "</ECCustomAttributes>");

    i = 0;
    for (ECCustomAttributeClassP caClass : caClasses)
        {
        StandaloneECEnablerPtr enabler = caClass->GetDefaultStandaloneEnabler();
        caInstances[i] = enabler->CreateInstance().get();
        schema->SetCustomAttribute(*caInstances[i++]);
        }

    schema->WriteToXmlString(schemaXml);

    EXPECT_TRUE(std::regex_search(schemaXml.c_str(), targetXmlRegex));
    }

    // create custom attribute classes and instances in reverse order, verify they serialize in order
    {
    ECSchemaPtr schema;
    bvector<ECCustomAttributeClassP> caClasses(4);
    bvector<IECInstancePtr> caInstances(4);

    Utf8String targetXmlRegexStr;
    std::regex targetXmlRegex = generateTargetXmlRegex(targetXmlRegexStr, caClassNames, "<ECCustomAttributeClass typeName=\"");

    ECSchema::CreateSchema(schema, "testSchema", "ts", 1, 0, 0);

    int i = (int)caClassNames.size();
    for (Utf8String name : caClassNames)
        schema->CreateCustomAttributeClass(caClasses[--i], name);

    Utf8String schemaXml;
    schema->WriteToXmlString(schemaXml);

    EXPECT_TRUE(std::regex_search(schemaXml.c_str(), targetXmlRegex));

    extendTargetXmlRegex(targetXmlRegexStr, "<ECCustomAttributes>");
    targetXmlRegex = generateTargetXmlRegex(targetXmlRegexStr, caClassNames, "<");
    extendTargetXmlRegex(targetXmlRegexStr, "</ECCustomAttributes>");

    i = 0;
    for (ECCustomAttributeClassP caClass : caClasses)
        {
        StandaloneECEnablerPtr enabler = caClass->GetDefaultStandaloneEnabler();
        caInstances[i++] = enabler->CreateInstance().get();
        }

    for (i = (int)caClassNames.size()-1; i >= 0; i--)
        schema->SetCustomAttribute(*caInstances[i]);

    schema->WriteToXmlString(schemaXml);

    EXPECT_TRUE(std::regex_search(schemaXml.c_str(), targetXmlRegex));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Caleb.Shafer                            08/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaXmlSerializationTest, ExpectCustomAttributeVersionAsTwoPartWhenEC2)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "testSchema", "ts", 5, 1, 0);

    ECCustomAttributeClassP caClass;
    schema->CreateCustomAttributeClass(caClass, "TestCA");

    StandaloneECEnablerPtr enabler = caClass->GetDefaultStandaloneEnabler();

    // Schema level
    auto schemaCAInstance = enabler->CreateInstance();
    EC_EXPECT_SUCCESS(schema->SetCustomAttribute(*schemaCAInstance));

    // Class level
    auto relCAInstance = enabler->CreateInstance();
    ECRelationshipClassP relClass;
    schema->CreateRelationshipClass(relClass, "TestClass");

    EC_EXPECT_SUCCESS(relClass->SetCustomAttribute(*relCAInstance));

    // Property level
    auto propCAInstance = enabler->CreateInstance();
    PrimitiveECPropertyP prop;
    relClass->CreatePrimitiveProperty(prop, "TestProp");

    EC_EXPECT_SUCCESS(prop->SetCustomAttribute(*propCAInstance));

    // Relationship constraint level
    auto constraintCAInstance = enabler->CreateInstance();
    EC_EXPECT_SUCCESS(relClass->GetSource().SetCustomAttribute(*constraintCAInstance));

    Utf8String schemaXml;
    schema->WriteToEC2XmlString(schemaXml, schema.get());

    int numOfMatches = 0;
    size_t offset = 0;
    while((offset = schemaXml.find("xmlns=\"testSchema.05.00\"", offset+1)) != Utf8String::npos)
        numOfMatches += 1;

    EXPECT_EQ(numOfMatches, 4);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Eimantas.Morkunas     05/2019
//---------------------------------------------------------------------------------------
TEST_F(SchemaXmlSerializationTest, ArrayPropertyMinAndMaxOccursSerializesCorrectly)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0));

    ECEntityClassP ecClass;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(ecClass, "TestClass"));

    PrimitiveArrayECPropertyP arrayProperty;
    ASSERT_EQ(ECObjectsStatus::Success, ecClass->CreatePrimitiveArrayProperty(arrayProperty, "TestArrayProperty"));

    arrayProperty->SetMinOccurs(2);
    arrayProperty->SetMaxOccurs(4);

    Utf8String serializedSchema;
    ASSERT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedSchema));

    ECSchemaPtr deserializedSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(deserializedSchema, serializedSchema.c_str(), *ECSchemaReadContext::CreateContext()));

    ECClassCP deserializedClass = deserializedSchema->GetClassCP("TestClass");
    ASSERT_TRUE(nullptr != deserializedClass);

    ECPropertyCP deserializedProperty = deserializedClass->GetPropertyP("TestArrayProperty");
    ASSERT_TRUE(nullptr != deserializedProperty);

    ArrayECPropertyCP deserializedArrayProperty = deserializedProperty->GetAsArrayProperty();
    ASSERT_TRUE(nullptr != deserializedArrayProperty);

    EXPECT_EQ(2, deserializedArrayProperty->GetMinOccurs());
    EXPECT_EQ(4, deserializedArrayProperty->GetStoredMaxOccurs());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaJsonSerializationTest, SchemaWithNoItems)
    {
    ECSchemaPtr schema = SchemaJsonSerializationTest::CreateSchemaWithNoItems();

    Json::Value schemaJson;
    EXPECT_TRUE(schema->WriteToJsonValue(schemaJson));

    Json::Value testDataJson;
    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/SchemaWithNoItems.ecschema.json").c_str());
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaJsonSerializationTest, SchemaWithItems)
    {
    ECSchemaPtr schema = SchemaJsonSerializationTest::CreateSchemaWithNoItems();
    schema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());
    schema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema());

    // Entity Class / Mixin Classes
    ECEntityClassP baseEntityClass;
    schema->CreateEntityClass(baseEntityClass, "ExampleBaseEntity");
    ECEntityClassP entityClass;
    schema->CreateEntityClass(entityClass, "ExampleEntity");
    entityClass->SetClassModifier(ECClassModifier::Sealed);
    entityClass->SetDisplayLabel("ExampleEntity");
    entityClass->SetDescription("An example entity class.");
    entityClass->AddBaseClass(*baseEntityClass);
    ECEntityClassP mixinA;
    schema->CreateMixinClass(mixinA, "ExampleMixinA", *entityClass);
    entityClass->AddBaseClass(*mixinA);
    ECEntityClassP mixinB;
    schema->CreateMixinClass(mixinB, "ExampleMixinB", *entityClass);
    entityClass->AddBaseClass(*mixinB);

    // Struct Class
    ECStructClassP structClass;
    schema->CreateStructClass(structClass, "ExampleStruct");
    structClass->SetClassModifier(ECClassModifier::Sealed);
    PrimitiveArrayECPropertyP primArrProp;
    structClass->CreatePrimitiveArrayProperty(primArrProp, "ExamplePrimitiveArray");
    primArrProp->SetPrimitiveElementType(ECN::PRIMITIVETYPE_Integer);
    primArrProp->SetMinimumValue(ECValue((int32_t) 7));
    primArrProp->SetMaximumValue(ECValue((int32_t) 20));
    primArrProp->SetMinOccurs(10);
    primArrProp->SetMaxOccurs(25);
    primArrProp->SetExtendedTypeName("FooBar");
    primArrProp->SetDisplayLabel("ExPrimitiveArray");

    // Relationship Class
    ECEntityClassP sourceClass;
    schema->CreateEntityClass(sourceClass, "ExampleSource");

    ECEntityClassP targetClass;
    schema->CreateEntityClass(targetClass, "ExampleTarget");

    ECRelationshipClassP relationshipClass;
    schema->CreateRelationshipClass(relationshipClass, "ExampleRelationship");
    relationshipClass->SetClassModifier(ECClassModifier::Sealed);
    relationshipClass->SetStrength(StrengthType::Embedding);
    relationshipClass->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    relationshipClass->GetSource().AddClass(*sourceClass);
    relationshipClass->GetSource().SetRoleLabel("source roleLabel");
    relationshipClass->GetSource().SetIsPolymorphic(true);
    relationshipClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relationshipClass->GetTarget().AddClass(*targetClass);
    relationshipClass->GetTarget().SetRoleLabel("target roleLabel");
    relationshipClass->GetSource().SetIsPolymorphic(true);
    relationshipClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::ZeroOne());

    // Custom Attribute Class
    ECCustomAttributeClassP customAttrClass;
    schema->CreateCustomAttributeClass(customAttrClass, "ExampleCustomAttribute");
    customAttrClass->SetClassModifier(ECClassModifier::Sealed);
    customAttrClass->SetContainerType(ECN::CustomAttributeContainerType::Schema | ECN::CustomAttributeContainerType::AnyProperty);

    // UnitSystem
    UnitSystemP unitSystem;
    schema->CreateUnitSystem(unitSystem, "ExampleUnitSystem", "ExampleUnitSystemLabel", "ExampleUnitSystemDescription");
    
    // Phenomenon
    PhenomenonP phenom;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH", "ExamplePhenomenonLabel", "ExamplePhenomenonDescription");

    // Unit
    ECUnitP unit;
    schema->CreateUnit(unit, "ExampleUnit", "[MILLI]M", *phenom, *unitSystem, 10.0, 1.0, 1.0, "ExampleUnitLabel", "ExampleUnitDescription");

    // Constant
    ECUnitP constant;
    schema->CreateConstant(constant, "ExampleConstant", "ONE", *phenom, 10.0, "ExampleConstantLabel", "ExampleConstantDescription");

    // InvertedUnit
    ECUnitP invertedUnit;
    schema->CreateInvertedUnit(invertedUnit, *unit, "ExampleInvertedUnit", *unitSystem, "ExampleInvertedUnitLabel", "ExampleInvertedUnitDescription");

    // Format
    ECFormatP format;
    schema->CreateFormat(format, "ExampleFormat", "ExampleFormatLabel", "ExampleFormatDescription");
    Formatting::NumericFormatSpec numspec;
    format->SetNumericSpec(numspec);
    EXPECT_FALSE(format->IsProblem());

    // Kind of Quantity
    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "ExampleKoQ");
    koq->SetPersistenceUnit(*unit);
    koq->SetDefaultPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultRealU"), nullptr, unit, "example");
    koq->SetRelativeError(3);

    // Enumeration
    ECEnumerationP enumeration;
    schema->CreateEnumeration(enumeration, "ExampleEnumeration", PrimitiveType::PRIMITIVETYPE_Integer);
    enumeration->SetIsStrict(true);
    ECEnumeratorP enumeratorA;
    enumeration->CreateEnumerator(enumeratorA, "EnumeratorA", 1);
    enumeratorA->SetDisplayLabel("None");
    ECEnumeratorP enumeratorB;
    enumeration->CreateEnumerator(enumeratorB, "EnumeratorB", 2);
    enumeratorB->SetDisplayLabel("SomeVal");
    ECEnumeratorP enumeratorC;
    enumeration->CreateEnumerator(enumeratorC, "EnumeratorC", 3);
    enumeratorC->SetDisplayLabel("AnotherVal");

    // Property Category
    PropertyCategoryP prop;
    schema->CreatePropertyCategory(prop, "ExamplePropertyCategory");
    prop->SetPriority(5);

    // Test the schema
    Json::Value schemaJson;
    EXPECT_TRUE(schema->WriteToJsonValue(schemaJson));

    Json::Value testDataJson;
    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/SchemaWithItems.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

END_BENTLEY_ECN_TEST_NAMESPACE
