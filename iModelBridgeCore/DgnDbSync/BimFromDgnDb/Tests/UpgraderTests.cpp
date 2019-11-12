/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "BimUpgraderTestFixture.h"
#include "DgnDbCreator.h"

#include <ECObjects/ECObjects.h>
#include <ECObjects/ECObjectsAPI.h>
#include <BimFromDgnDb/BimFromDgnDbAPI.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BIM_UPGRADER_TEST_NAMESPACE

struct UpgraderTests : public BimUpgraderTestFixture
    {

    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2019
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UpgraderTests, RemapPropertyNames)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" nameSpacePrefix="test" version="01.00.01" displayLabel="TestSchema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.0">
            <ECSchemaReference name="Generic" version="01.00.00" prefix="generic"/>
            <ECEntityClass typeName="Foo" displayLabel="Foo">
                <BaseClass>generic:PhysicalObject</BaseClass>
                <ECProperty propertyName="Model" typeName="string" displayLabel="Model"/>
                <ECProperty propertyName="ID" typeName="string" displayLabel="ID"/>
                <ECProperty propertyName="Parent" typeName="string" displayLabel="Parent"/>
                <ECProperty propertyName="Category" typeName="string" displayLabel="Category"/>
            </ECEntityClass>
        </ECSchema>)xml";

    BentleyApi::BeFileName dgndbFileName = GetOutputFileName("PropertyNames.dgndb");
    DgnDbCreator creator(dgndbFileName.GetNameUtf8().c_str());
    ASSERT_TRUE(creator.CreateDgnDb());
    ASSERT_TRUE(creator.ImportSchema(schemaXml));

    ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext));

    ECN::ECClassCP testClass = schema->GetClassCP("Foo");
    auto testClassInstance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    // custom-handled properties
    ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("Model", ECN::ECValue("MyModel")));
    ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("ID", ECN::ECValue("MyID")));
    ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("Parent", ECN::ECValue("MyParent")));
    ASSERT_EQ(ECN::ECObjectsStatus::Success, testClassInstance->SetValue("Category", ECN::ECValue("MyCat")));

    BentleyApi::Utf8String ecInstanceXml;
    const InstanceWriteStatus writeStat = testClassInstance->WriteToXmlString(ecInstanceXml, true, false); 
    ASSERT_TRUE(creator.AddElement("TestSchema", ecInstanceXml.c_str()));

    BentleyApi::BeFileName bimFileName = GetOutputFileName("PropertyNames.bim");

    ASSERT_TRUE(BentleyB0200::Dgn::BimFromDgnDb::BimFromDgnDb::Upgrade(dgndbFileName.GetName(), bimFileName.GetName()));

    }
END_BIM_UPGRADER_TEST_NAMESPACE
