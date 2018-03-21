/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/InstanceQuantityTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"


USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct InstanceQuantityTests : ECTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Colin.Kerr         10/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceQuantityTests, GetQuantityFromDoubleWithKoqDefined)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Pipe">
                <ECProperty propertyName="Length" typeName="double" kindOfQuantity="PipeLength" />
                <ECArrayProperty propertyName="LengthArray" typeName="double" kindOfQuantity="PipeLength" />
            </ECEntityClass>
            <KindOfQuantity typeName="PipeLength" persistenceUnit="CM" relativeError="10e-3"/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ECClassP pipeClass = schema->GetClassP("Pipe");
    IECInstancePtr instance = pipeClass->GetDefaultStandaloneEnabler()->CreateInstance();

    Units::Quantity q;
    ASSERT_EQ(ECObjectsStatus::PropertyValueNull, instance->GetQuantity(q, "Length"));
    ASSERT_EQ(0.0, q.GetMagnitude()) << "Getting Quantity for null value outputs quantity with 0 magnitude";
    ASSERT_STREQ("CM", q.GetUnit()->GetName().c_str()) << "Getting Quantity for null value outputs quantity with correct unit";
    q = Units::Quantity(42, *q.GetUnit());
    ASSERT_EQ(ECObjectsStatus::Success, instance->SetQuantity("Length", q));
    Units::Quantity oq;
    ASSERT_EQ(ECObjectsStatus::Success, instance->GetQuantity(oq, "Length"));
    ASSERT_EQ(42, oq.GetMagnitude()) << "Length";
    ASSERT_STREQ("CM", oq.GetUnitName()) << "Length";

    ASSERT_EQ(ECObjectsStatus::IndexOutOfRange, instance->GetQuantity(q, "LengthArray", 0));
    ASSERT_EQ(ECObjectsStatus::Success, instance->AddArrayElements("LengthArray", 1));
    ASSERT_EQ(ECObjectsStatus::PropertyValueNull, instance->GetQuantity(q, "LengthArray", 0));
    ASSERT_EQ(0.0, q.GetMagnitude()) << "Getting Quantity for null value outputs quantity with 0 magnitude";
    ASSERT_STREQ("CM", q.GetUnit()->GetName().c_str()) << "Getting Quantity for null value outputs quantity with correct unit";
    q = Units::Quantity(42, *q.GetUnit());
    ASSERT_EQ(ECObjectsStatus::Success, instance->SetQuantity("LengthArray", q, 0));
    ASSERT_EQ(ECObjectsStatus::Success, instance->GetQuantity(oq, "LengthArray", 0));
    ASSERT_EQ(42, oq.GetMagnitude()) << "LengthArray";
    ASSERT_STREQ("CM", oq.GetUnitName()) << "LengthArray";

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Colin.Kerr         10/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceQuantityTests, GetQuantityFromUnsupportedProperties)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Pipe">
                <ECProperty propertyName="DoubleNoKOQ" typeName="double" />
                <ECProperty propertyName="IntHasKOQ" typeName="int" kindOfQuantity="PipeLength" />
                <ECProperty propertyName="StringHasKOQ" typeName="string" kindOfQuantity="PipeLength" />
                <ECArrayProperty propertyName="DoubleArrayNoKOQ" typeName="double" />
                <ECArrayProperty propertyName="IntArrayHasKOQ" typeName="int" kindOfQuantity="PipeLength" />
                <ECArrayProperty propertyName="StringArrayHasKOQ" typeName="string" kindOfQuantity="PipeLength" />
            </ECEntityClass>
            <KindOfQuantity typeName="PipeLength" persistenceUnit="CM" relativeError="10e-3"/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ECClassP pipeClass = schema->GetClassP("Pipe");
    IECInstancePtr instance = pipeClass->GetDefaultStandaloneEnabler()->CreateInstance();

    Units::Quantity q;
    ASSERT_EQ(ECObjectsStatus::PropertyNotFound, instance->GetQuantity(q, "NotAPropertyName"));
    ASSERT_EQ(ECObjectsStatus::PropertyHasNoKindOfQuantity, instance->GetQuantity(q, "DoubleNoKOQ"));

    ASSERT_EQ(ECObjectsStatus::PropertyValueNull, instance->GetQuantity(q, "IntHasKOQ"));
    ECValue v(42);
    ASSERT_EQ(ECObjectsStatus::Success, instance->SetValue("IntHasKOQ", v));
    ASSERT_EQ(ECObjectsStatus::DataTypeNotSupported, instance->GetQuantity(q, "IntHasKOQ"));

    ASSERT_EQ(ECObjectsStatus::PropertyValueNull, instance->GetQuantity(q, "StringHasKOQ"));
    ECValue vs("Banana");
    ASSERT_EQ(ECObjectsStatus::Success, instance->SetValue("StringHasKOQ", vs));
    ASSERT_EQ(ECObjectsStatus::DataTypeNotSupported, instance->GetQuantity(q, "StringHasKOQ"));


    ASSERT_EQ(ECObjectsStatus::PropertyNotFound, instance->GetQuantity(q, "NotAPropertyNameArray", 1));
    ASSERT_EQ(ECObjectsStatus::PropertyHasNoKindOfQuantity, instance->GetQuantity(q, "DoubleArrayNoKOQ", 1));

    ASSERT_EQ(ECObjectsStatus::IndexOutOfRange, instance->GetQuantity(q, "IntArrayHasKOQ", 1));
    ASSERT_EQ(ECObjectsStatus::Success, instance->AddArrayElements("IntArrayHasKOQ", 2));
    ASSERT_EQ(ECObjectsStatus::PropertyValueNull, instance->GetQuantity(q, "IntArrayHasKOQ", 1));
    ECValue va(42);
    ASSERT_EQ(ECObjectsStatus::Success, instance->SetValue("IntArrayHasKOQ", va, 1));
    ASSERT_EQ(ECObjectsStatus::DataTypeNotSupported, instance->GetQuantity(q, "IntArrayHasKOQ", 1));

    ASSERT_EQ(ECObjectsStatus::IndexOutOfRange, instance->GetQuantity(q, "StringArrayHasKOQ", 1));
    ASSERT_EQ(ECObjectsStatus::Success, instance->AddArrayElements("StringArrayHasKOQ", 2));
    ASSERT_EQ(ECObjectsStatus::PropertyValueNull, instance->GetQuantity(q, "StringArrayHasKOQ", 1));
    ECValue vas("Banana");
    ASSERT_EQ(ECObjectsStatus::Success, instance->SetValue("StringArrayHasKOQ", vas, 1));
    ASSERT_EQ(ECObjectsStatus::DataTypeNotSupported, instance->GetQuantity(q, "StringArrayHasKOQ", 1));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Colin.Kerr         10/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceQuantityTests, SetQuantityToDoubleWithKoqDefined)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Pipe">
                <ECProperty propertyName="Length" typeName="double" kindOfQuantity="PipeLength" />
                <ECArrayProperty propertyName="LengthArray" typeName="double" kindOfQuantity="PipeLength" />
            </ECEntityClass>
            <KindOfQuantity typeName="PipeLength" persistenceUnit="CM" relativeError="10e-3"/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ECClassP pipeClass = schema->GetClassP("Pipe");
    IECInstancePtr instance = pipeClass->GetDefaultStandaloneEnabler()->CreateInstance();

    Units::Quantity q;
    ECUnitCP ftUnit = ECTestFixture::GetUnitsSchema()->GetUnitCP("FT");
    ASSERT_NE(nullptr, ftUnit);
    q = Units::Quantity(42, *ftUnit);
    ASSERT_EQ(ECObjectsStatus::Success, instance->SetQuantity("Length", q));
    Units::Quantity oq;
    ASSERT_EQ(ECObjectsStatus::Success, instance->GetQuantity(oq, "Length"));
    ASSERT_EQ(1280.16, oq.GetMagnitude());
    ASSERT_STREQ("CM", oq.GetUnitName());

    ECUnitCP acreUnit = ECTestFixture::GetUnitsSchema()->GetUnitCP("ACRE");
    ASSERT_NE(nullptr, acreUnit);
    ASSERT_EQ(ECObjectsStatus::KindOfQuantityNotCompatible, instance->SetQuantity("Length", Units::Quantity(42, *acreUnit)));
    ASSERT_EQ(ECObjectsStatus::KindOfQuantityNotCompatible, instance->SetQuantity("Length", Units::Quantity()));


    ASSERT_EQ(ECObjectsStatus::Success, instance->AddArrayElements("LengthArray", 3));
    ASSERT_EQ(ECObjectsStatus::Success, instance->SetQuantity("LengthArray", q, 2));
    ASSERT_EQ(ECObjectsStatus::Success, instance->GetQuantity(oq, "LengthArray", 2));
    ASSERT_EQ(1280.16, oq.GetMagnitude());
    ASSERT_STREQ("CM", oq.GetUnitName());

    ASSERT_EQ(ECObjectsStatus::KindOfQuantityNotCompatible, instance->SetQuantity("LengthArray", Units::Quantity(42, *acreUnit), 2));
    ASSERT_EQ(ECObjectsStatus::KindOfQuantityNotCompatible, instance->SetQuantity("LengthArray", Units::Quantity(), 2));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Colin.Kerr         10/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceQuantityTests, SetQuantityToUnsupportedProperties)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Pipe">
                <ECProperty propertyName="DoubleNoKOQ" typeName="double" />
                <ECProperty propertyName="IntHasKOQ" typeName="int" kindOfQuantity="PipeLength" />
                <ECProperty propertyName="StringHasKOQ" typeName="string" kindOfQuantity="PipeLength" />
                <ECArrayProperty propertyName="DoubleArrayNoKOQ" typeName="double" />
                <ECArrayProperty propertyName="IntArrayHasKOQ" typeName="int" kindOfQuantity="PipeLength" />
                <ECArrayProperty propertyName="StringArrayHasKOQ" typeName="string" kindOfQuantity="PipeLength" />
            </ECEntityClass>
            <KindOfQuantity typeName="PipeLength" persistenceUnit="CM" relativeError="10e-3"/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ECClassP pipeClass = schema->GetClassP("Pipe");
    IECInstancePtr instance = pipeClass->GetDefaultStandaloneEnabler()->CreateInstance();

    Units::Quantity q;
    ECUnitCP ftUnit = ECTestFixture::GetUnitsSchema()->GetUnitCP("FT");
    ASSERT_NE(nullptr, ftUnit);
    q = Units::Quantity(42, *ftUnit);
    ASSERT_EQ(ECObjectsStatus::PropertyNotFound, instance->SetQuantity("NotAPropertyName", q));
    ASSERT_EQ(ECObjectsStatus::PropertyHasNoKindOfQuantity, instance->SetQuantity("DoubleNoKOQ", q));

    ASSERT_EQ(ECObjectsStatus::DataTypeMismatch, instance->SetQuantity("IntHasKOQ", q));
    ASSERT_EQ(ECObjectsStatus::DataTypeMismatch, instance->SetQuantity("StringHasKOQ", q));


    ASSERT_EQ(ECObjectsStatus::PropertyNotFound, instance->SetQuantity("NotAPropertyNameArray", q, 0));
    ASSERT_EQ(ECObjectsStatus::PropertyHasNoKindOfQuantity, instance->SetQuantity("DoubleArrayNoKOQ", q, 0));

    ASSERT_EQ(ECObjectsStatus::Success, instance->AddArrayElements("IntArrayHasKOQ", 1));
    ASSERT_EQ(ECObjectsStatus::Success, instance->AddArrayElements("StringArrayHasKOQ", 1));
    ASSERT_EQ(ECObjectsStatus::DataTypeMismatch, instance->SetQuantity("IntArrayHasKOQ", q, 0));
    ASSERT_EQ(ECObjectsStatus::DataTypeMismatch, instance->SetQuantity("StringArrayHasKOQ", q, 0));
    }
END_BENTLEY_ECN_TEST_NAMESPACE

