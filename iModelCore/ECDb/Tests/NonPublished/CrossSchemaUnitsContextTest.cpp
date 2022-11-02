/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct CrossSchemaUnitsContextTest : ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CrossSchemaUnitsContextTest, LoadJsonFormatSameSchema)
    {
    Utf8CP schema1Xml = R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema1" alias="ts1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="TEST_UNITSYSTEM" />
            <Phenomenon typeName="TEST_PHENOM" definition="TEST_PHENOM" />
            <Unit typeName="UNIT_A" definition="UNIT_A" phenomenon="TEST_PHENOM" unitSystem="TEST_UNITSYSTEM" />
        </ECSchema>
    )xml";

    Utf8CP schema2Xml = R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="TestSchema1" version="01.00.00" alias="ts1" />
            <Unit typeName="UNIT_B" definition="ts1:UNIT_B" numerator="2.0" phenomenon="ts1:TEST_PHENOM" unitSystem="ts1:TEST_UNITSYSTEM" />
        </ECSchema>
    )xml";

    ASSERT_EQ(SUCCESS, SetupECDb("CrossSchemaUnitsContextTest_LoadJsonFormat"));
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(schema1Xml)));
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(schema2Xml)));

    Formatting::Format testFormat;
    ECFormatP testFormatNode;
    EXPECT_TRUE(Formatting::Format::FromJson(testFormat, R"json({
            "includeZero": true,
            "composite": {
                "units": [
                    { "name": "TestSchema1.UNIT_A", "label": "aa" }
                ]
            }
    })json", &m_ecdb.Schemas().GetSchema("TestSchema1")->GetUnitsContext()));

    ECSchemaPtr formatHavingSchema;
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(formatHavingSchema, "FormatHavingSchema", "fhs", 1, 0, 0));
    formatHavingSchema->SetOriginalECXmlVersion(3, 2);
    formatHavingSchema->CreateFormat(testFormatNode, "TestFormat", nullptr, nullptr, testFormat.GetNumericSpec(), testFormat.GetCompositeSpec());

    EXPECT_EQ(SUCCESS, GetHelper().ImportSchema(formatHavingSchema));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CrossSchemaUnitsContextTest, LoadJsonFormatSameSchemaManagerContext)
    {
    Utf8CP schema1Xml = R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema1" alias="ts1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="TEST_UNITSYSTEM" />
            <Phenomenon typeName="TEST_PHENOM" definition="TEST_PHENOM" />
            <Unit typeName="UNIT_A" definition="UNIT_A" phenomenon="TEST_PHENOM" unitSystem="TEST_UNITSYSTEM" />
        </ECSchema>
    )xml";

    Utf8CP schema2Xml = R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="TestSchema1" version="01.00.00" alias="ts1" />
            <Unit typeName="UNIT_B" definition="ts1:UNIT_B" numerator="2.0" phenomenon="ts1:TEST_PHENOM" unitSystem="ts1:TEST_UNITSYSTEM" />
        </ECSchema>
    )xml";

    ASSERT_EQ(SUCCESS, SetupECDb("CrossSchemaUnitsContextTest_LoadJsonFormatManagerContext"));
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(schema1Xml)));
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(schema2Xml)));

    Formatting::Format testFormat;
    ECFormatP testFormatNode;
    auto unitsContext = m_ecdb.Schemas().GetUnitsContext();
    EXPECT_TRUE(Formatting::Format::FromJson(testFormat, R"json({
            "includeZero": true,
            "composite": {
                "units": [
                    { "name": "TestSchema1.UNIT_A", "label": "aa" }
                ]
            }
    })json", &unitsContext));

    ECSchemaPtr formatHavingSchema;
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(formatHavingSchema, "FormatHavingSchema", "fhs", 1, 0, 0));
    formatHavingSchema->SetOriginalECXmlVersion(3, 2);
    formatHavingSchema->CreateFormat(testFormatNode, "TestFormat", nullptr, nullptr, testFormat.GetNumericSpec(), testFormat.GetCompositeSpec());

    EXPECT_EQ(SUCCESS, GetHelper().ImportSchema(formatHavingSchema));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CrossSchemaUnitsContextTest, LoadJsonFormatDifferentSchema)
    {
    enum class Cases { UseSchema1UnitsContext, UseSchemaManagerUnitsContext };
    auto inner = [&](Cases case_)
        {
        Utf8CP schema1Xml = R"xml(<?xml version="1.0" encoding="utf-8" ?>
            <ECSchema schemaName="TestSchema1" alias="ts1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <UnitSystem typeName="TEST_UNITSYSTEM" />
                <Phenomenon typeName="TEST_PHENOM" definition="TEST_PHENOM" />
                <Unit typeName="UNIT_A" definition="UNIT_A" phenomenon="TEST_PHENOM" unitSystem="TEST_UNITSYSTEM" />
            </ECSchema>
        )xml";

        Utf8CP schema2Xml = R"xml(<?xml version="1.0" encoding="utf-8" ?>
            <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="TestSchema1" version="01.00.00" alias="ts1" />
                <Unit typeName="UNIT_B" definition="ts1:UNIT_B" numerator="2.0" phenomenon="ts1:TEST_PHENOM" unitSystem="ts1:TEST_UNITSYSTEM" />
            </ECSchema>
        )xml";

        ASSERT_EQ(SUCCESS, SetupECDb("CrossSchemaUnitsContextTest_LoadJsonFormat"));
        ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(schema1Xml)));
        ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(schema2Xml)));

        Formatting::Format testFormat;
        Utf8CP testFormatJson = R"json({
            "includeZero": true,
            "composite": {
                "units": [
                    { "name": "TestSchema1.UNIT_A", "label": "aa" },
                    { "name": "TestSchema2.UNIT_B", "label": "bb" }
                ]
            }
        })json";

        ECSchemaPtr formatHavingSchema;
        ECFormatP testFormatNode;
        auto crossSchemaUnitsContext = m_ecdb.Schemas().GetUnitsContext();
        switch (case_) {
            case Cases::UseSchema1UnitsContext:
                EXPECT_FALSE(Formatting::Format::FromJson(testFormat, testFormatJson, &m_ecdb.Schemas().GetSchema("TestSchema1")->GetUnitsContext()))
                    << "should not be able to reference across schemas with single schema UnitsContext";
                break;

            case Cases::UseSchemaManagerUnitsContext:
                EXPECT_TRUE(Formatting::Format::FromJson(testFormat, testFormatJson, &crossSchemaUnitsContext))
                    << "should be able to reference across schemas with multischema UnitsContext";

                ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(formatHavingSchema, "FormatHavingSchema", "fhs", 1, 0, 0));
                formatHavingSchema->SetOriginalECXmlVersion(3, 2);
                formatHavingSchema->CreateFormat(testFormatNode, "TestFormat", nullptr, nullptr, testFormat.GetNumericSpec(), testFormat.GetCompositeSpec());

                EXPECT_EQ(SUCCESS, GetHelper().ImportSchema(formatHavingSchema));
                break;

            default:
                ASSERT_EQ(true, false) << "unreachable case";
        }
        };

    inner(Cases::UseSchema1UnitsContext);
    inner(Cases::UseSchemaManagerUnitsContext);
    }

TEST_F(CrossSchemaUnitsContextTest, LoadJsonFormatUnknownUnit)
    {
    Utf8CP schema1Xml = R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema1" alias="ts1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="TEST_UNITSYSTEM" />
            <Phenomenon typeName="TEST_PHENOM" definition="TEST_PHENOM" />
            <Unit typeName="UNIT_A" definition="UNIT_A" phenomenon="TEST_PHENOM" unitSystem="TEST_UNITSYSTEM" />
        </ECSchema>
    )xml";

    Utf8CP schema2Xml = R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="TestSchema1" version="01.00.00" alias="ts1" />
            <Unit typeName="UNIT_B" definition="ts1:UNIT_B" numerator="2.0" phenomenon="ts1:TEST_PHENOM" unitSystem="ts1:TEST_UNITSYSTEM" />
        </ECSchema>
    )xml";

    ASSERT_EQ(SUCCESS, SetupECDb("CrossSchemaUnitsContextTest_LoadJsonFormat"));
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(schema1Xml)));
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(schema2Xml)));

    Formatting::Format testFormat;
    EXPECT_FALSE(Formatting::Format::FromJson(testFormat, R"json({
            "includeZero": true,
            "composite": {
                "units": [
                    { "name": "TestSchema1.UNIT_C", "label": "cc" }
                ]
            }
    })json", &m_ecdb.Schemas().GetSchema("TestSchema1")->GetUnitsContext()));
    }

TEST_F(CrossSchemaUnitsContextTest, LoadJsonFormatUnknownUnitManagerContext)
    {
    Utf8CP schema1Xml = R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema1" alias="ts1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="TEST_UNITSYSTEM" />
            <Phenomenon typeName="TEST_PHENOM" definition="TEST_PHENOM" />
            <Unit typeName="UNIT_A" definition="UNIT_A" phenomenon="TEST_PHENOM" unitSystem="TEST_UNITSYSTEM" />
        </ECSchema>
    )xml";

    Utf8CP schema2Xml = R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="TestSchema1" version="01.00.00" alias="ts1" />
            <Unit typeName="UNIT_B" definition="ts1:UNIT_B" numerator="2.0" phenomenon="ts1:TEST_PHENOM" unitSystem="ts1:TEST_UNITSYSTEM" />
        </ECSchema>
    )xml";

    ASSERT_EQ(SUCCESS, SetupECDb("CrossSchemaUnitsContextTest_LoadJsonFormat"));
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(schema1Xml)));
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(schema2Xml)));

    Formatting::Format testFormat;
    auto unitsContext = m_ecdb.Schemas().GetUnitsContext();
    EXPECT_FALSE(Formatting::Format::FromJson(testFormat, R"json({
            "includeZero": true,
            "composite": {
                "units": [
                    { "name": "TestSchema1.UNIT_C", "label": "cc" }
                ]
            }
    })json", &unitsContext));
    }

TEST_F(CrossSchemaUnitsContextTest, CrossSchemaUnitsContextLookupsMustBeQualified)
    {
    Utf8CP schema1Xml = R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema1" alias="ts1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="TEST_UNITSYSTEM" />
            <Phenomenon typeName="TEST_PHENOM" definition="TEST_PHENOM" />
            <Unit typeName="UNIT_A" definition="UNIT_A" phenomenon="TEST_PHENOM" unitSystem="TEST_UNITSYSTEM" />
        </ECSchema>
    )xml";

    Utf8CP schema2Xml = R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="TestSchema1" version="01.00.00" alias="ts1" />
            <Unit typeName="UNIT_B" definition="ts1:UNIT_B" numerator="2.0" phenomenon="ts1:TEST_PHENOM" unitSystem="ts1:TEST_UNITSYSTEM" />
        </ECSchema>
    )xml";

    ECSchemaPtr schema1, schema2;

    ASSERT_EQ(SUCCESS, SetupECDb("CrossSchemaUnitsContextTest_LoadJsonFormat"));
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(schema1Xml)));
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(schema2Xml)));

    auto unitsContext = m_ecdb.Schemas().GetUnitsContext();
    EXPECT_EQ(nullptr, unitsContext.LookupUnit("UNIT_A"));
    EXPECT_EQ(nullptr, unitsContext.LookupUnit("UNIT_B"));

    EXPECT_NE(nullptr, m_ecdb.Schemas().GetSchema("TestSchema1")->LookupUnit("UNIT_A"));
    EXPECT_NE(nullptr, m_ecdb.Schemas().GetSchema("TestSchema2")->LookupUnit("UNIT_B"));

    EXPECT_NE(nullptr, unitsContext.LookupUnit("TestSchema1:UNIT_A"));
    EXPECT_NE(nullptr, unitsContext.LookupUnit("TestSchema2:UNIT_B"));

    EXPECT_NE(nullptr, unitsContext.LookupUnit("ts1:UNIT_A"));
    EXPECT_NE(nullptr, unitsContext.LookupUnit("ts2:UNIT_B"));

    EXPECT_EQ(nullptr, unitsContext.LookupUnit("ts2:UNIT_A"));
    EXPECT_EQ(nullptr, unitsContext.LookupUnit("ts1:UNIT_B"));
    }

END_ECDBUNITTESTS_NAMESPACE
