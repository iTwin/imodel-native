/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/KindOfQuantityTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct KindOfQuantityTest : ECTestFixture {};

struct KindOfQuantityDeserializationTest : ECTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Basanta.Kharel   12/2015
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(KindOfQuantityTest, KindOfQuantityTestTest)
    {
    ECSchemaPtr schema;
    ECEntityClassP entityClass;
    PrimitiveECPropertyP prop;
    KindOfQuantityP koq;

    ECSchema::CreateSchema(schema, "KindOfQuantitySchema", "koq", 5, 0, 6);

    schema->CreateEntityClass(entityClass, "Class");
    entityClass->CreatePrimitiveProperty(prop, "Property");

    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateKindOfQuantity(koq, "MyKindOfQuantity"));
    koq->SetPersistenceUnit("FT(real)");
    prop->SetKindOfQuantity(koq);

    Utf8String schemaXML;
    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(schemaXML, ECVersion::V3_1));

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr refSchema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(refSchema, schemaXML.c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);

    ECClassCP entityClassDup = refSchema->GetClassCP("Class");
    ASSERT_NE(entityClassDup, nullptr);
    PrimitiveECPropertyCP property = entityClassDup->GetPropertyP("Property")->GetAsPrimitiveProperty();
    ASSERT_NE(property, nullptr);

    auto resultKindOfQuantity = property->GetKindOfQuantity();
    ASSERT_NE(resultKindOfQuantity, nullptr);
    EXPECT_STREQ("MyKindOfQuantity", resultKindOfQuantity->GetName().c_str());
    EXPECT_STREQ("FT(DefaultReal)", resultKindOfQuantity->GetPersistenceUnit().ToText(false).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityTest, AddRemovePresentationUnits)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "TestKoQSchema", "koq", 1, 0, 0));
    ASSERT_TRUE(schema.IsValid());

    {
    KindOfQuantityP kindOfQuantity;

    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateKindOfQuantity(kindOfQuantity, "MyKindOfQuantity"));
    EXPECT_EQ(ECObjectsStatus::Success, kindOfQuantity->SetDescription("Kind of a Description here"));
    EXPECT_EQ(ECObjectsStatus::Success, kindOfQuantity->SetDisplayLabel("best quantity of all times"));
    EXPECT_TRUE(kindOfQuantity->SetPersistenceUnit("CM"));
    EXPECT_FALSE(kindOfQuantity->SetPersistenceUnit("PI"));
    kindOfQuantity->SetRelativeError(10e-3);
    EXPECT_TRUE(kindOfQuantity->SetDefaultPresentationUnit("FT"));
    EXPECT_FALSE(kindOfQuantity->AddPresentationUnit("PI"));
    EXPECT_TRUE(kindOfQuantity->AddPresentationUnit("IN"));
    EXPECT_TRUE(kindOfQuantity->AddPresentationUnit("MILLIINCH"));
    }

    {
    KindOfQuantityP koq = schema->GetKindOfQuantityP("MyKindOfQuantity");
    ASSERT_NE(nullptr, koq);
    EXPECT_STREQ("FT", koq->GetDefaultPresentationUnit().GetUnit()->GetName().c_str());
    auto presUnitList = koq->GetPresentationUnitList();
    EXPECT_EQ(3, presUnitList.size());
    EXPECT_STREQ("FT", presUnitList.at(0).GetUnit()->GetName().c_str());
    EXPECT_STREQ("IN", presUnitList.at(1).GetUnit()->GetName().c_str());
    EXPECT_STREQ("MILLIINCH", presUnitList.at(2).GetUnit()->GetName().c_str());

    koq->RemovePresentationUnit("IN");
    }
    {
    KindOfQuantityP koq = schema->GetKindOfQuantityP("MyKindOfQuantity");
    ASSERT_NE(nullptr, koq);
    auto presUnitList = koq->GetPresentationUnitList();
    EXPECT_EQ(2, presUnitList.size());
    EXPECT_STREQ("FT", presUnitList.at(0).GetUnit()->GetName().c_str());
    EXPECT_STREQ("MILLIINCH", presUnitList.at(1).GetUnit()->GetName().c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityTest, RemoveAllPresentationUnits)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "TestKoQSchema", "koq", 1, 0, 0));
    ASSERT_TRUE(schema.IsValid());
    
    KindOfQuantityP kindOfQuantity;

    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateKindOfQuantity(kindOfQuantity, "MyKindOfQuantity"));
    EXPECT_EQ(ECObjectsStatus::Success, kindOfQuantity->SetDescription("Kind of a Description here"));
    EXPECT_EQ(ECObjectsStatus::Success, kindOfQuantity->SetDisplayLabel("best quantity of all times"));
    EXPECT_TRUE(kindOfQuantity->SetPersistenceUnit("CM"));
    kindOfQuantity->SetRelativeError(10e-3);
    
    EXPECT_EQ(0, kindOfQuantity->GetPresentationUnitList().size());

    EXPECT_TRUE(kindOfQuantity->SetDefaultPresentationUnit("FT"));
    EXPECT_EQ(1, kindOfQuantity->GetPresentationUnitList().size());
    kindOfQuantity->RemoveAllPresentationUnits();
    EXPECT_EQ(0, kindOfQuantity->GetPresentationUnitList().size());

    EXPECT_TRUE(kindOfQuantity->AddPresentationUnit("IN"));
    EXPECT_TRUE(kindOfQuantity->AddPresentationUnit("MILLIINCH"));
    EXPECT_EQ(2, kindOfQuantity->GetPresentationUnitList().size());

    kindOfQuantity->RemoveAllPresentationUnits();
    EXPECT_EQ(0, kindOfQuantity->GetPresentationUnitList().size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityTest, TestIncompatiblePersistenceAndPresentationUnits)
    {
    ECSchemaPtr schema;
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "TestKoQSchema", "koq", 1, 0, 0));
    ASSERT_TRUE(schema.IsValid());

    KindOfQuantityP koq;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateKindOfQuantity(koq, "MyKindOfQuantity"));
    EXPECT_TRUE(koq->SetPersistenceUnit("M"));
    EXPECT_FALSE(koq->AddPresentationUnit("MG")) << "The Unit MG is from the MASS Phenomenon which is different than the Persistence Unit.";

    KindOfQuantityP koq2;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateKindOfQuantity(koq2, "MyKindOfQuantity2"));
    EXPECT_TRUE(koq2->AddPresentationUnit("MG"));
    EXPECT_FALSE(koq2->SetPersistenceUnit("M")) << "The Unit MG is from the MASS Phenomenon which is different than the Presentation Unit.";

    KindOfQuantityP koq3;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateKindOfQuantity(koq3, "MyKindOfQuantity3"));
    EXPECT_TRUE(koq3->AddPresentationUnit("M"));
    EXPECT_TRUE(koq3->SetPersistenceUnit("CM"));
    EXPECT_FALSE(koq3->AddPresentationUnit("MG")) << "The Unit MG is from the MASS Phenomenon which is different than the existing Presentation and Persistence Unit.";

    KindOfQuantityP koq4;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateKindOfQuantity(koq4, "MyKindOfQuantity4"));
    EXPECT_TRUE(koq4->AddPresentationUnit("MG"));
    EXPECT_FALSE(koq4->AddPresentationUnit("M")) << "The Unit M is from the LENGTH Phenomenon which is different than the existing Presentation Unit.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(KindOfQuantityTest, SerializeStandaloneChildKindOfQuantity)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);

    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "ExampleKoQ");
    koq->SetPersistenceUnit("MM");
    koq->SetDefaultPresentationUnit("IN");
    koq->AddPresentationUnit("MM");
    koq->AddPresentationUnit("CM");
    koq->SetRelativeError(3);

    Json::Value schemaJson;
    EXPECT_EQ(SchemaWriteStatus::Success, koq->WriteJson(schemaJson, true));

    Json::Value testDataJson;
    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneKindOfQuantity.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, TestEmptyOrMissingName)
    {
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity description="Kind of a Description here"
                displayLabel="best quantity of all times" persistenceUnit="CM" relativeError="10e-3"/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="" description="Kind of a Description here"
                displayLabel="best quantity of all times" persistenceUnit="CM" relativeError="10e-3"/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, TestEmptyOrMissingRelativeError)
    {
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="TestKOQ" description="Kind of a Description here"
                displayLabel="best quantity of all times" persistenceUnit="CM"/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="" description="Kind of a Description here"
                displayLabel="best quantity of all times" persistenceUnit="CM" relativeError=""/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, TestEmptyOrMissingPersistenceUnit)
    {
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                displayLabel="best quantity of all times" relativeError="10e-3"/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                displayLabel="best quantity of all times" persistenceUnit="" relativeError="10e-3"/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Kyle.Abramowitz      02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, TestConstantAsPersistanceUnit)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                displayLabel="best quantity of all times" persistenceUnit="PI" relativeError="10e-3"/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Kyle.Abramowitz      02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, TestConstantAsPresentationUnit)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                displayLabel="best quantity of all times" persistenceUnit="M" presentationUnits="PI" relativeError="10e-3"/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Kyle.Abramowitz      02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, TestUnitInSchemaAsPresentationAndPersistenceUnit)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="1.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                displayLabel="best quantity of all times" persistenceUnit="ts:TestUnit(real4u)" presentationUnits="ts:TestUnit(real4u)" relativeError="10e-3"/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    auto unit = schema->GetUnitCP("TestUnit");
    auto koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");
    auto koqPerUnit = koq->GetPersistenceUnit().GetUnit();
    ASSERT_EQ(unit, koqPerUnit);
    ASSERT_EQ(unit, koq->GetDefaultPresentationUnit().GetUnit());
    ASSERT_STREQ(koq->GetPersistenceUnit().GetNamedFormatSpec()->GetName(), "Real4U");
    ASSERT_STREQ(koq->GetDefaultPresentationUnit().GetNamedFormatSpec()->GetName(), "Real4U");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Kyle.Abramowitz      02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, TestUnitInReferencedSchemaAsPresentationAndPersistenceUnit)
    {
    Utf8CP refXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="refSchema" version="01.00" alias="rs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        </ECSchema>)xml";
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="refSchema" version="01.00" alias="rs"/>
            <Unit typeName="TestUnit" phenomenon="rs:TestPhenomenon" unitSystem="rs:TestUnitSystem" numerator="1.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
            <Unit typeName="Smoot" phenomenon="rs:TestPhenomenon" unitSystem="rs:TestUnitSystem" numerator="1.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
        </ECSchema>)xml";
    Utf8CP koqSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="koqSchema" alias="ks" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="testSchema" version="01.00" alias="ts"/>
            <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
                displayLabel="best quantity of all times" persistenceUnit="ts:TestUnit" presentationUnits="ts:Smoot" relativeError="10e-3"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    Utf8String serializedRefSchemaXml;
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    ECSchemaPtr koqSchema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refXml, *context));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(koqSchema, koqSchemaXml, *context));
    auto koq = koqSchema->GetKindOfQuantityCP("MyKindOfQuantity");
    ASSERT_NE(nullptr, koq);
    auto unit = schema->GetUnitCP("TestUnit");
    auto smoot = schema->GetUnitCP("Smoot");
    auto koqPerUnit = koq->GetPersistenceUnit();
    ASSERT_EQ(unit, koqPerUnit.GetUnit());
    auto koqPresUnit = koq->GetDefaultPresentationUnit();
    ASSERT_EQ(smoot, koqPresUnit.GetUnit());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, TestIncompatiblePersistenceAndPresentationUnits)
    {
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKindOfQuantity" relativeError="10e-3" persistenceUnit="LUX" presentationUnits="M" />
        </ECSchema>)xml";
    ECTestFixture::ExpectSchemaDeserializationFailure(schemaXml);
    }
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKindOfQuantity" relativeError="10e-3" persistenceUnit="M" presentationUnits="LUX"/>
        </ECSchema>)xml";
    ECTestFixture::ExpectSchemaDeserializationFailure(schemaXml);
    }
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <KindOfQuantity typeName="MyKindOfQuantity" relativeError="10e-3" persistenceUnit="M" presentationUnits="CM;LUX"/>
    </ECSchema>)xml";
    ECTestFixture::ExpectSchemaDeserializationFailure(schemaXml);
    }
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <KindOfQuantity typeName="MyKindOfQuantity" relativeError="10e-3" persistenceUnit="LUX" presentationUnits="CM;MM"/>
    </ECSchema>)xml";
    ECTestFixture::ExpectSchemaDeserializationFailure(schemaXml);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, ExpectSuccessWhenKindOfQuantityIsAppliedToStructAndStructArrayPropertes)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'>"
        "       <ECStructProperty propertyName='StructWithKOQ' typeName='S' kindOfQuantity='MyKindOfQuantity' />"
        "       <ECStructArrayProperty propertyName='StructArrayWithKOQ' typeName='S' kindOfQuantity='MyKindOfQuantity' />"
        "   </ECEntityClass>"
        "   <ECStructClass typeName='S'>"
        "       <ECProperty propertyName='S_P' typeName='string' />"
        "   </ECStructClass>"
        "   <KindOfQuantity typeName='MyKindOfQuantity' description='Kind of a Description here' "
        "       displayLabel='best quantity of all times' persistenceUnit='CM' relativeError='10e-3' "
        "       presentationUnits='FT;IN;MILLIINCH'/>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "ECSchema failed to deserialize.";
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->IsECVersion(ECVersion::V3_2));

    ECClassCP aClass = schema->GetClassCP("A");
    ASSERT_NE(nullptr, aClass) << "Could not find 'A' class";
    ECPropertyP structProp = aClass->GetPropertyP("StructWithKOQ");
    ASSERT_NE(nullptr, structProp) << "Can't find 'StructWithKOQ' property";
    KindOfQuantityCP koq = structProp->GetKindOfQuantity();
    ASSERT_NE(nullptr, koq) << "'StructWithKOQ' property does not have a KOQ as expected";
    ECPropertyP structArrayProp = aClass->GetPropertyP("StructArrayWithKOQ");
    ASSERT_NE(nullptr, structArrayProp) << "Can't find 'StructArrayWithKOQ' property";
    koq = structArrayProp->GetKindOfQuantity();
    ASSERT_NE(nullptr, koq) << "'StructArrayWithKOQ' property does not have a KOQ as expected";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, ExpectSuccessWhenKindOfQuantityInherited)
    {
    Utf8CP schemaXml = R"xml(<?xml version='1.0' encoding='UTF-8'?>
        <ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
            <ECEntityClass typeName='A' modifier='abstract'>
                <ECProperty propertyName='KindOfQuantityProperty' typeName='double' kindOfQuantity='MyKindOfQuantity' />
            </ECEntityClass>
            <ECEntityClass typeName='B' modifer='abstract'>
                <BaseClass>A</BaseClass>
                <ECProperty propertyName='KindOfQuantityProperty' typeName='double' />
            </ECEntityClass>
            <ECEntityClass typeName='C' modifer='sealed'>
                <BaseClass>B</BaseClass>
                <ECProperty propertyName='KindOfQuantityProperty' typeName='double' />
            </ECEntityClass>
            <KindOfQuantity typeName='MyKindOfQuantity' description='Kind of a Description here'
                displayLabel='best quantity of all times' persistenceUnit='CM' relativeError='10e-3'
                presentationUnits='FT;IN;MILLIINCH'/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "ECSchema failed to deserialize.";
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->IsECVersion(ECVersion::Latest));

    for (auto const& pClass : schema->GetClasses())
        {
        ECPropertyP p = pClass->GetPropertyP("KindOfQuantityProperty");
        EXPECT_TRUE(p != nullptr);

        PrimitiveECPropertyCP prim = p->GetAsPrimitiveProperty();
        EXPECT_TRUE(prim != nullptr);

        KindOfQuantityCP kindOfQuantity = prim->GetKindOfQuantity();
        EXPECT_TRUE(kindOfQuantity != nullptr);

        if (pClass->GetName() != "A")
            EXPECT_FALSE(prim->IsKindOfQuantityDefinedLocally()) << "The Kind of Quantity is defined in a base class, so it should not be defined locally.";
        else
            EXPECT_TRUE(prim->IsKindOfQuantityDefinedLocally()) << "The Kind of Quantity is defined, so it should be defined locally.";

        EXPECT_STREQ(kindOfQuantity->GetName().c_str(), "MyKindOfQuantity");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, ExpectSuccessWhenDeserializingSchemaWithKindOfQuantityInReferencedFile)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    WString seedPath(ECTestFixture::GetTestDataPath(L"").c_str());
    schemaContext->AddSchemaPath(seedPath.c_str());

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"KindOfQuantityInReferencedSchema.01.00.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    ASSERT_TRUE(schema.IsValid());

    ECClassP pClass = schema->GetClassP("Entity");
    ASSERT_TRUE(nullptr != pClass);

    ECPropertyP p = pClass->GetPropertyP("KindOfQuantityProperty");
    ASSERT_TRUE(p != nullptr);

    PrimitiveECPropertyCP prim = p->GetAsPrimitiveProperty();
    ASSERT_TRUE(prim != nullptr);

    KindOfQuantityCP kindOfQuantity = prim->GetKindOfQuantity();
    ASSERT_TRUE(kindOfQuantity != nullptr);

    EXPECT_STREQ(kindOfQuantity->GetName().c_str(), "MyKindOfQuantity");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, ExpectSuccessWhenRoundtripKindOfQuantityUsingString)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    ASSERT_TRUE(schema.IsValid());

    KindOfQuantityP kindOfQuantity;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateKindOfQuantity(kindOfQuantity, "MyKindOfQuantity"));
    kindOfQuantity->SetDescription("DESC");
    kindOfQuantity->SetDisplayLabel("DL");
    kindOfQuantity->SetPersistenceUnit("CM");
    kindOfQuantity->SetRelativeError(10e-3);
    kindOfQuantity->SetDefaultPresentationUnit("FT");
    kindOfQuantity->AddPresentationUnit("IN");
    kindOfQuantity->AddPresentationUnit("MILLIINCH");

    ECEntityClassP entityClass;
    ASSERT_TRUE(schema->CreateEntityClass(entityClass, "EntityClass") == ECObjectsStatus::Success);
    PrimitiveArrayECPropertyP property;
    auto status = entityClass->CreatePrimitiveArrayProperty(property, "QuantifiedProperty", PrimitiveType::PRIMITIVETYPE_Double);
    ASSERT_TRUE(status == ECObjectsStatus::Success);
    ASSERT_TRUE(property != nullptr);
    property->SetKindOfQuantity(kindOfQuantity);

    Utf8String ecSchemaXmlString;
    SchemaWriteStatus status2 = schema->WriteToXmlString(ecSchemaXmlString, ECVersion::V3_1);
    EXPECT_EQ(SchemaWriteStatus::Success, status2);

    ECSchemaPtr deserializedSchema;
    auto schemaContext = ECSchemaReadContext::CreateContext();
    auto status3 = ECSchema::ReadFromXmlString(deserializedSchema, ecSchemaXmlString.c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status3);

    EXPECT_EQ(1, deserializedSchema->GetKindOfQuantityCount());
    KindOfQuantityCP deserializedKindOfQuantity;
    deserializedKindOfQuantity = deserializedSchema->GetKindOfQuantityCP("MyKindOfQuantity");
    ASSERT_TRUE(deserializedKindOfQuantity != nullptr);
    EXPECT_STREQ("DL", deserializedKindOfQuantity->GetDisplayLabel().c_str());
    EXPECT_STREQ("DESC", deserializedKindOfQuantity->GetDescription().c_str());
    EXPECT_STREQ("CM", deserializedKindOfQuantity->GetPersistenceUnit().GetUnit()->GetName().c_str());
    EXPECT_EQ(10e-3, deserializedKindOfQuantity->GetRelativeError());

    EXPECT_STREQ("FT", deserializedKindOfQuantity->GetDefaultPresentationUnit().GetUnit()->GetName().c_str());
    auto& resultAltUnits = deserializedKindOfQuantity->GetPresentationUnitList();
    EXPECT_EQ(3, resultAltUnits.size()); // Default presentation unit is included in list of presentation units
    EXPECT_STREQ("IN", resultAltUnits[1].GetUnit()->GetName().c_str());
    EXPECT_STREQ("MILLIINCH", resultAltUnits[2].GetUnit()->GetName().c_str());

    ECClassCP deserializedClass = deserializedSchema->GetClassCP("EntityClass");
    ECPropertyP deserializedProperty = deserializedClass->GetPropertyP("QuantifiedProperty");
    KindOfQuantityCP propertyKoq = deserializedProperty->GetAsPrimitiveArrayProperty()->GetKindOfQuantity();
    ASSERT_TRUE(nullptr != propertyKoq);
    EXPECT_EQ(propertyKoq, deserializedKindOfQuantity);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Colin.Kerr     10/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, FailToDeserializeWithUnknownPersistenceUnit)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
           <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
               displayLabel="best quantity of all time" persistenceUnit="SILLYMETER" relativeError="10e-3" />
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Colin.Kerr     10/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, FailToDeserializeWithUnknownPersistenceFormat)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
           <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
               displayLabel="best quantity of all time" persistenceUnit="M(SILLYFORMAT)" relativeError="10e-3" />
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Colin.Kerr     10/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, FailToDeserializeWithUnknownPresentationUnit)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
           <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
               displayLabel="best quantity of all time" persistenceUnit="M" presentationUnits="SILLYMETER;ANOTHERSILLYMETER" relativeError="10e-3" />
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Colin.Kerr     10/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(KindOfQuantityDeserializationTest, FailToDeserializeWithUnknownPresentationFormat)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
           <KindOfQuantity typeName="MyKindOfQuantity" description="Kind of a Description here"
               displayLabel="best quantity of all time" persistenceUnit="M" presentationUnits="MM;CM(SILLYFORMAT)" relativeError="10e-3" />
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityDeserializationTest, SuccessfulWithUnknownUnitInNewerECVersion)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
           <KindOfQuantity typeName="KoQWithPers" description="Kind of a Description here"
               displayLabel="best quantity of all time" persistenceUnit="SILLYMETER" relativeError="10e-3" />

            <KindOfQuantity typeName="KoQWithPres" description="Kind of a Description here"
               displayLabel="best quantity of all time" persistenceUnit="M" presentationUnits="MM;ANOTHERSILLYMETER" relativeError="10e-3" />
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "ECSchema failed to deserialize.";
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->IsECVersion(ECVersion::V3_1));

    // Check Persistence Unit
    KindOfQuantityCP koq = schema->GetKindOfQuantityCP("KoQWithPers");
    ASSERT_NE(nullptr, koq);

    Units::UnitCP unit = koq->GetPersistenceUnit().GetUnit();

    ASSERT_FALSE(unit->IsValid()) << "Expect KOQ to have dummy unit because unit name is unknown";
    ASSERT_FALSE(koq->GetPersistenceUnit().HasProblem()) << "Expect no  problems because unknown units are not exposed as problems";
    ASSERT_STREQ("SILLYMETER(DefaultReal)", koq->GetPersistenceUnit().ToText(false).c_str()) << "Should be able to write FUS with dummy unit to text";

    EXPECT_NE(nullptr, Units::UnitRegistry::Instance().LookupUnit("SILLYMETER"));

    // Check Presentation Units
    KindOfQuantityCP koq2 = schema->GetKindOfQuantityCP("KoQWithPres");
    ASSERT_NE(nullptr, koq2);

    auto& presUnitList = koq2->GetPresentationUnitList();
    ASSERT_EQ(1, presUnitList.size());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityDeserializationTest, UseDefaultFormatInNewerECVersion)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
           <KindOfQuantity typeName="KoQWithPers" description="Kind of a Description here"
               displayLabel="best quantity of all time" persistenceUnit="M(SILLYFORMAT)" relativeError="10e-3" />

            <KindOfQuantity typeName="KoQWithPres" description="Kind of a Description here"
               displayLabel="best quantity of all time" persistenceUnit="M" presentationUnits="MM(DefaultReal);CM(SILLYFORMAT)" relativeError="10e-3" />
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "ECSchema failed to deserialize.";
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->IsECVersion(ECVersion::V3_1));

    // Check Persistence Format
    KindOfQuantityCP koq = schema->GetKindOfQuantityCP("KoQWithPers");
    ASSERT_NE(nullptr, koq);
    
    ASSERT_FALSE(koq->GetPersistenceUnit().HasProblem());

    Formatting::NamedFormatSpecCP nfs = koq->GetPersistenceUnit().GetNamedFormatSpec();
    EXPECT_STREQ("DefaultRealU", nfs->GetName());
    ASSERT_FALSE(nfs->IsProblem());
    ASSERT_FALSE(koq->GetPersistenceUnit().HasProblem()) << "Expect no  problems because unknown units are not exposed as problems";
    ASSERT_STREQ("M(DefaultRealU)", koq->GetPersistenceUnit().ToText(false).c_str()) << "Should be able to write FUS with default format to string";

    // Check Presentation Formats
    KindOfQuantityCP koq2 = schema->GetKindOfQuantityCP("KoQWithPres");
    ASSERT_NE(nullptr, koq2);

    auto& presUnitList = koq2->GetPresentationUnitList();
    ASSERT_EQ(2, presUnitList.size()) << "Neither FUS should be dropped from the list even though one has an invalid Format.";
    
    EXPECT_FALSE(presUnitList[0].HasProblem());
    Formatting::NamedFormatSpecCP nfs1 = presUnitList[0].GetNamedFormatSpec();
    EXPECT_STREQ("DefaultReal", nfs1->GetName());
    ASSERT_FALSE(nfs1->IsProblem());
    ASSERT_STREQ("MM(DefaultReal)", presUnitList[0].ToText(false).c_str()) << "Should be able to write FUS with default format to string";

    EXPECT_FALSE(presUnitList[1].HasProblem());
    Formatting::NamedFormatSpecCP nfs2 = presUnitList[1].GetNamedFormatSpec();
    EXPECT_FALSE(nfs2->IsProblem());
    EXPECT_STREQ("DefaultRealU", nfs2->GetName()) << "The format should have been set to the default.";
    ASSERT_STREQ("CM(DefaultRealU)", presUnitList[1].ToText(false).c_str()) << "Should be able to write FUS with default format to string";
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityDeserializationTest, PropertyCannotBeOverridenWithDummyUnit)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <KindOfQuantity typeName="BridgeMeasurement_Meter" persistenceUnit="M(DefaultReal)" relativeError="10e-3" />

            <KindOfQuantity typeName="BridgeMeasurement_Smoot" persistenceUnit="Smoot(DefaultReal)" relativeError="10e-3" />

            <ECEntityClass typeName="Bridge">
                <ECProperty propertyName="Length" typeName="double" kindOfQuantity="BridgeMeasurement_Meter"/>
            </ECEntityClass>

            <ECEntityClass typeName="HarvardBridge" description="A bridge that is to be measured by a Harvard student and only goes over the Charles River.">
                <BaseClass>Bridge</BaseClass>
                <ECProperty propertyName="Length" typeName="double" kindOfQuantity="BridgeMeasurement_Smoot"/>
            </ECEntityClass>

        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
TEST_F(KindOfQuantityDeserializationTest, SameDummyUnitUsedInBothPersistenceAndPresentationUnit)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <KindOfQuantity typeName="BridgeMeasurement_Meter" persistenceUnit="Smoot(DefaultReal)" presentationUnits="Smoot(DefaultRealU)" relativeError="10e-3" />
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->IsECVersion(ECVersion::V3_1));

    // Check Persistence Format
    KindOfQuantityCP koq = schema->GetKindOfQuantityCP("BridgeMeasurement_Meter");
    ASSERT_NE(nullptr, koq);

    EXPECT_FALSE(koq->GetPersistenceUnit().HasProblem());
    auto& presList = koq->GetPresentationUnitList();
    EXPECT_EQ(0, presList.size()) << "The presentation unit with the dummy unit should have been dropped.";
    }

END_BENTLEY_ECN_TEST_NAMESPACE
