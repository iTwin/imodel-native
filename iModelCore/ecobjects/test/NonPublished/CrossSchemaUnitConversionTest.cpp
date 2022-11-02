/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct CrossSchemaUnitConversionTests : ECTestFixture {};

//! linear reference: (converting ts1:UNIT -> ts2:UNIT)
//! ts1 <--- ts2
//! nonlinear reference: (converting ts2:UNIT -> ts3:UNIT)
//! ts3 <--- ts1
//!    ^---- ts2
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CrossSchemaUnitConversionTests, ConvertUnitCrossSchemaNonLinearReference)
    {
    Utf8CP schema1Xml = R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema1" alias="ts1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="TEST_UNITSYSTEM" />
            <Phenomenon typeName="TEST_PHENOM" definition="TestPhenom" />
            <Unit typeName="BASE_UNIT" definition="BASE_UNIT" phenomenon="TEST_PHENOM" unitSystem="TEST_UNITSYSTEM" />
        </ECSchema>
    )xml";

    Utf8CP schema2Xml = R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="TestSchema1" version="01.00.00" alias="ts1" />
            <Unit typeName="UNIT_A" definition="ts1:BASE_UNIT" numerator="2.0" phenomenon="ts1:TEST_PHENOM" unitSystem="ts1:TEST_UNITSYSTEM" />
        </ECSchema>
    )xml";

    Utf8CP schema3Xml = R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema3" alias="ts3" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="TestSchema1" version="01.00.00" alias="ts1" />
            <Unit typeName="UNIT_B" definition="ts1:BASE_UNIT" numerator="4.0" phenomenon="ts1:TEST_PHENOM" unitSystem="ts1:TEST_UNITSYSTEM" />
        </ECSchema>
    )xml";

    ECSchemaPtr schema1, schema2, schema3;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema1, schema1Xml, *context));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema2, schema2Xml, *context));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema3, schema3Xml, *context));

    ECUnitCP unitA = schema2->GetUnitCP("UNIT_A");
    ASSERT_NE(nullptr, unitA) << "Unit A badly defined";

    ECUnitCP unitB = schema3->GetUnitCP("UNIT_B");
    ASSERT_NE(nullptr, unitB) << "Unit B badly defined";

    double actualValue = 0;
    EXPECT_EQ(Units::UnitsProblemCode::NoProblem, unitA->Convert(actualValue, 2.0, unitB)) << "";
    EXPECT_EQ(actualValue, 1.0) << "unitA to unitB conversion should be simple halving";

    EXPECT_EQ(Units::UnitsProblemCode::NoProblem, unitB->Convert(actualValue, 2.0, unitB)) << "";
    EXPECT_EQ(actualValue, 2.0) << "unitB to unitA conversion should be simple doubling";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CrossSchemaUnitConversionTests, ConvertUnitCrossSchemaLinearReference)
    {
    Utf8CP schema1Xml = R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema1" alias="ts1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="TEST_UNITSYSTEM" />
            <Phenomenon typeName="TEST_PHENOM" definition="TestPhenom" />
            <Unit typeName="UNIT_A" definition="UNIT_A" phenomenon="TEST_PHENOM" unitSystem="TEST_UNITSYSTEM" />
        </ECSchema>
    )xml";

    Utf8CP schema2Xml = R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="TestSchema1" version="01.00.00" alias="ts1" />
            <Unit typeName="UNIT_B" definition="ts1:UNIT_A" numerator="2.0" phenomenon="ts1:TEST_PHENOM" unitSystem="ts1:TEST_UNITSYSTEM" />
        </ECSchema>
    )xml";

    ECSchemaPtr schema1, schema2;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema1, schema1Xml, *context));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema2, schema2Xml, *context));

    ECUnitCP unitA = schema1->GetUnitCP("UNIT_A");
    ASSERT_NE(nullptr, unitA) << "Unit A badly defined";

    ECUnitCP unitB = schema2->GetUnitCP("UNIT_B");
    ASSERT_NE(nullptr, unitB) << "Unit B badly defined";

    double actualValue = 0;
    EXPECT_EQ(Units::UnitsProblemCode::NoProblem, unitA->Convert(actualValue, 2.0, unitB)) << "";
    EXPECT_EQ(actualValue, 1.0) << "unitA to unitB conversion should be simple halving";

    EXPECT_EQ(Units::UnitsProblemCode::NoProblem, unitB->Convert(actualValue, 2.0, unitB)) << "";
    EXPECT_EQ(actualValue, 2.0) << "unitB to unitA conversion should be simple doubling";
    }

END_BENTLEY_ECN_TEST_NAMESPACE
