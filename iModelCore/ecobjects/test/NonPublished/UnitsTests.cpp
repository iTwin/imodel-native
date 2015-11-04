/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/UnitsTests.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

#include <ECUnits/Units.h>

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

static Utf8CP s_refSchemaXml =
    "<ECSchema schemaName=\"RefSchema\" nameSpacePrefix=\"ref\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
    "  <ECSchemaReference name=\"Unit_Attributes\" version=\"01.00\" prefix=\"units_attribs\" />"
    "  <ECCustomAttributes>"
    "      <IsUnitSystemSchema xmlns=\"Unit_Attributes.01.00\" />"
    "      <UnitSpecifications xmlns=\"Unit_Attributes.01.00\">"
    "          <UnitSpecificationList>"
    "              <UnitSpecification>"
    "                  <KindOfQuantityName>LENGTH</KindOfQuantityName>"
    "                  <!-- Note no UnitName specified -->"
    "              </UnitSpecification>"
    "              <UnitSpecification>"
    "                  <KindOfQuantityName>DIAMETER</KindOfQuantityName>"
    "                  <UnitName>CENTIMETRE</UnitName>"
    "              </UnitSpecification>"
    "              <UnitSpecification>"
    "                  <DimensionName>L</DimensionName>"
    "                  <UnitName>KILOMETRE</UnitName>"
    "              </UnitSpecification>"
    "              <UnitSpecification>"
    "                  <KindOfQuantityName>NONEXISTENT_KOQ_WITH_DIMENSION</KindOfQuantityName>"
    "                  <DimensionName>L3</DimensionName>"
    "              </UnitSpecification>"
    "              <UnitSpecification>"
    "                  <KindOfQuantityName>NONEXISTENT_KOQ_WITH_UNIT</KindOfQuantityName>"
    "                  <UnitName>NONEXISTENT_UNIT</UnitName>"
    "              </UnitSpecification>"
    "          </UnitSpecificationList>"
    "      </UnitSpecifications>"
    "  </ECCustomAttributes>"
    "</ECSchema>";

static Utf8CP s_schemaXml =
    "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
    "  <ECSchemaReference name=\"Unit_Attributes\" version=\"01.00\" prefix=\"units_attribs\" />"
    "  <ECSchemaReference name=\"RefSchema\" version=\"01.00\" prefix=\"refSchema\" />"
    "  <ECCustomAttributes>"
    "      <UnitSpecifications xmlns=\"Unit_Attributes.01.00\">"
    "          <UnitSpecificationList>"
    "              <!-- Dimension L3 referenced in UnitSpecification in referenced schema, with Unit defined in domain schema -->"
    "              <UnitSpecification>"
    "                  <DimensionName>L3</DimensionName>"
    "                  <UnitName>DECIMETRE</UnitName>"
    "              </UnitSpecification>"
    "          </UnitSpecificationList>"
    "      </UnitSpecifications>"
    "  </ECCustomAttributes>"
    "    <!-- Class for testing units defined on properties, for unit conversions -->"
    "    <ECClass typeName=\"TestClass\" isDomainClass=\"True\">"
    "        <ECProperty propertyName=\"FromProperty\" typeName=\"double\">"
    "          <ECCustomAttributes>"
    "              <UnitSpecification xmlns=\"Unit_Attributes.01.00\">"
    "                  <UnitName>KILOMETRE</UnitName>"
    "              </UnitSpecification>"
    "          </ECCustomAttributes>"
    "        </ECProperty>"
    "        <ECProperty propertyName=\"ToProperty\" typeName=\"double\">"
    "          <ECCustomAttributes>"
    "              <UnitSpecification xmlns=\"Unit_Attributes.01.00\">"
    "                  <UnitName>KILOMETRE</UnitName>"
    "              </UnitSpecification>"
    "          </ECCustomAttributes>"
    "        </ECProperty>"
    "    </ECClass>"
    "  <!-- Class for testing UnitSpecifications defined at schema level (including referenced schema) -->"
    "  <ECClass typeName=\"UnitSpecClass\" isDomainClass=\"True\">"
    "        <ECProperty propertyName=\"FromKOQ\" typeName=\"double\">"
    "          <ECCustomAttributes>"
    "              <UnitSpecification xmlns=\"Unit_Attributes.01.00\">"
    "                  <KindOfQuantityName>DIAMETER</KindOfQuantityName>"
    "              </UnitSpecification>"
    "          </ECCustomAttributes>"
    "        </ECProperty>"
    "        <ECProperty propertyName=\"FromParentKOQ\" typeName=\"double\">"
    "          <ECCustomAttributes>"
    "              <UnitSpecification xmlns=\"Unit_Attributes.01.00\">"
    "                  <KindOfQuantityName>DIAMETER_LARGE</KindOfQuantityName>"
    "              </UnitSpecification>"
    "              <DisplayUnitSpecification xmlns=\"Unit_Attributes.01.00\">"
    "                  <DisplayUnitName>DECIMETRE</DisplayUnitName>"
    "              </DisplayUnitSpecification>"
    "          </ECCustomAttributes>"
    "        </ECProperty>"
    "        <ECProperty propertyName=\"FromKOQDimension\" typeName=\"double\">"
    "          <ECCustomAttributes>"
    "              <UnitSpecification xmlns=\"Unit_Attributes.01.00\">"
    "                  <KindOfQuantityName>LENGTH</KindOfQuantityName>"
    "              </UnitSpecification>"
    "          </ECCustomAttributes>"
    "        </ECProperty>"
    "        <ECProperty propertyName=\"FromDimension\" typeName=\"double\">"
    "          <ECCustomAttributes>"
    "              <UnitSpecification xmlns=\"Unit_Attributes.01.00\">"
    "                  <DimensionName>L</DimensionName>"
    "              </UnitSpecification>"
    "              <DisplayUnitSpecification xmlns=\"Unit_Attributes.01.00\">"
    "                  <DisplayUnitName>MILE</DisplayUnitName>"
    "                  <DisplayFormatString>0000.###### \"ignored\"</DisplayFormatString>"
    "              </DisplayUnitSpecification>"
    "          </ECCustomAttributes>"
    "        </ECProperty>"
    "        <ECProperty propertyName=\"FromNonExistentKOQWithDimension\" typeName=\"double\">"
    "          <ECCustomAttributes>"
    "              <UnitSpecification xmlns=\"Unit_Attributes.01.00\">"
    "                  <KindOfQuantityName>NONEXISTENT_KOQ_WITH_DIMENSION</KindOfQuantityName>"
    "              </UnitSpecification>"
    "          </ECCustomAttributes>"
    "        </ECProperty>"
    "        <ECProperty propertyName=\"FromNonExistentKOQWithUnit\" typeName=\"double\">"
    "          <ECCustomAttributes>"
    "              <UnitSpecification xmlns=\"Unit_Attributes.01.00\">"
    "                  <KindOfQuantityName>NONEXISTENT_KOQ_WITH_UNIT</KindOfQuantityName>"
    "              </UnitSpecification>"
    "          </ECCustomAttributes>"
    "        </ECProperty>"
    "  </ECClass>"
    "</ECSchema>";

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct UnitsTest : ECTestFixture
    {
    ECSchemaPtr     m_refSchema;
    ECSchemaPtr     m_schema;
    ECPropertyCP    m_fromProperty, m_toProperty;

    virtual void    SetUp() override;

    void            SetUnits (Utf8CP fromUnitName, Utf8CP toUnitName)
        {
        m_fromProperty->GetCustomAttribute ("UnitSpecification")->SetValue ("UnitName", ECValue (fromUnitName));
        m_toProperty->GetCustomAttribute ("UnitSpecification")->SetValue ("UnitName", ECValue (toUnitName));
        }

    void            GetUnits (UnitR from, UnitR to)
        {
        EXPECT_TRUE (Unit::GetUnitForECProperty (from, *m_fromProperty));
        EXPECT_TRUE (Unit::GetUnitForECProperty (to, *m_toProperty));
        }

    void            TestUnitConversion (double fromVal, Utf8CP fromUnitName, double expectedVal, Utf8CP targetUnitName, double tolerance);
    void            TestUnitSpecification (Utf8CP propName, double expectedValueOfOneMeter);
    void            TestUnitFormatting (Utf8CP propName, double storedValue, Utf8CP formattedValue);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitsTest::SetUp()
    {
    ECTestFixture::SetUp();

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    EXPECT_EQ (SchemaReadStatus::Success, ECSchema::ReadFromXmlString (m_refSchema, s_refSchemaXml, *context));
    EXPECT_EQ (SchemaReadStatus::Success, ECSchema::ReadFromXmlString (m_schema, s_schemaXml, *context));

    m_fromProperty = m_schema->GetClassP ("TestClass")->GetPropertyP ("FromProperty");
    m_toProperty = m_schema->GetClassP ("TestClass")->GetPropertyP ("ToProperty");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitsTest::TestUnitConversion (double fromVal, Utf8CP fromUnitName, double expectedVal, Utf8CP targetUnitName, double tolerance)
    {
    SetUnits (fromUnitName, targetUnitName);
    Unit srcUnit, dstUnit;
    GetUnits (srcUnit, dstUnit);

    EXPECT_TRUE (srcUnit.IsCompatible (dstUnit) && dstUnit.IsCompatible (srcUnit)) << fromUnitName << " is incompatible with " << targetUnitName;
    double convertedVal = fromVal;
    EXPECT_TRUE (srcUnit.ConvertTo (convertedVal, dstUnit));
    EXPECT_TRUE (fabs (convertedVal - expectedVal) < tolerance) << "Input " << fromVal << fromUnitName << " Expected " << expectedVal << targetUnitName << " Actual " << convertedVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitsTest::TestUnitSpecification (Utf8CP propName, double expectedValueOfOneMeter)
    {
    ECClassCP ecClass = m_schema->GetClassP ("UnitSpecClass");

    Unit meter ("METRE", "m", UnitConverter (false), "METRE");
    Unit propUnit;
    ECPropertyCP ecprop = ecClass->GetPropertyP (propName);
    EXPECT_TRUE (Unit::GetUnitForECProperty (propUnit, *ecprop));

    EXPECT_TRUE (propUnit.IsCompatible (meter));

    EXPECT_TRUE (propUnit.ConvertTo (expectedValueOfOneMeter, meter));
    EXPECT_EQ (1.0, expectedValueOfOneMeter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitsTest::TestUnitFormatting (Utf8CP propName, double storedValue, Utf8CP formattedValue)
    {
    ECPropertyCP ecprop = m_schema->GetClassP ("UnitSpecClass")->GetPropertyP (propName);

    Utf8String formatted;
    EXPECT_TRUE (Unit::FormatValue (formatted, ECValue (storedValue), *ecprop, NULL));
    EXPECT_TRUE (formatted.Equals (formattedValue)) << "Expected: " << formattedValue << " Actual: " << formatted.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* The following tests were generated from managed tests using regex replace.
* Generated from ecf\ecobjects\atp\units\UnitTestCase.cs using hg revision e1e9c347ce20
* Given: Assert.AreEqual (12.0, StandardUnits.INCH.ConvertFrom (1.0, StandardUnits.FOOT), 1.0e-8);
* Using VIM:
*   :%s/Assert.AreEqual (\(.*\), StandardUnits\.\(.*\)\.ConvertFrom (\(.*\), StandardUnits\.\(.*\)), \(.*\));/TestUnitConversion (\3, "\4", \1, "\2", \5);/g
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitsTest, Conversions)
    {
    // Dimension L
    TestUnitConversion (1.0, "FOOT", 12.0, "INCH", 1.0e-8);
    TestUnitConversion (12.0, "INCH", 1.0, "FOOT", 1.0e-8);
    TestUnitConversion (1.0, "KILOMETRE", 1000.0, "METRE", 1.0e-8);
    TestUnitConversion (1.0, "DECIMETRE", 0.1, "METRE", 1.0e-8);
    TestUnitConversion (1.0, "CENTIMETRE", 0.01, "METRE", 1.0e-8);
    TestUnitConversion (1.0, "MILLIMETRE", 0.001, "METRE", 1.0e-8);
    TestUnitConversion (1.0, "METRE", 1.0, "METRE", 1.0e-8);
    TestUnitConversion (1.0, "FOOT", 0.3048, "METRE", 1.0e-8);
    TestUnitConversion (1.0, "INCH", 0.0254, "METRE", 1.0e-8);
    TestUnitConversion (1.0, "YARD", 0.9144, "METRE", 1.0e-8);
    TestUnitConversion (1.0, "MILE", 1609.344, "METRE", 1.0e-8);
    TestUnitConversion (100000.0, "MICROINCH", 2.54e-3, "METRE", 1.0e-8);
    TestUnitConversion (100000.0, "MICROMETRE", 0.1, "METRE", 1.0e-8);
    TestUnitConversion (10000.0, "MILLIINCH", 0.254, "METRE", 1.0e-8);
    TestUnitConversion (1000.0, "MILLIFOOT", 0.3048, "METRE", 1.0e-8);
    TestUnitConversion (1.0, "US_SURVEY_INCH", 2.54000508001016e-02, "METRE", 1.0e-8);
    TestUnitConversion (1.0, "US_SURVEY_FOOT", 0.30480060960122, "METRE", 1.0e-8);
    TestUnitConversion (1.0, "US_SURVEY_MILE", 1609.34721869444, "METRE", 1.0e-8);

    TestUnitConversion (1000000.0, "FOOT", 12000000.0, "INCH", 1.0e-8);
    TestUnitConversion (12000000.0, "INCH", 1000000.0, "FOOT", 1.0e-8);
    TestUnitConversion (1000000.0, "KILOMETRE", 1000000000.0, "METRE", 1.0e-8);
    TestUnitConversion (1000000.0, "DECIMETRE", 100000.0, "METRE", 1.0e-8);
    TestUnitConversion (1000000.0, "CENTIMETRE", 10000.0, "METRE", 1.0e-8);
    TestUnitConversion (1000000000.0, "MILLIMETRE", 1000000, "METRE", 1.0e-8);
    TestUnitConversion (1000000.0, "METRE", 1000000.0, "METRE", 1.0e-8);
    TestUnitConversion (10000000.0, "FOOT", 3048000.0, "METRE", 1.0e-8);
    TestUnitConversion (10000000.0, "INCH", 254000.0, "METRE", 1.0e-8);
    TestUnitConversion (10000000.0, "YARD", 9144000.0, "METRE", 1.0e-8);
    TestUnitConversion (1000000.0, "MILE", 1609344000.0, "METRE", 1.0e-6); // Does not pass w/ 1.0e-8
    TestUnitConversion (1000000.0, "MILE", 1609344.0, "KILOMETRE", 1.0e-8);
    TestUnitConversion (10000000.0, "MICROINCH", 0.254, "METRE", 1.0e-8);
    TestUnitConversion (10000000.0, "MICROMETRE", 10.0, "METRE", 1.0e-8);
    TestUnitConversion (10000000.0, "MILLIINCH", 254.0, "METRE", 1.0e-8);
    TestUnitConversion (10000000.0, "MILLIFOOT", 3048.0, "METRE", 1.0e-8);
    TestUnitConversion (1000000.0, "US_SURVEY_INCH", 25400.0508001016, "METRE", 1.0e-8);
    TestUnitConversion (1000000.0, "US_SURVEY_FOOT", 304800.60960122, "METRE", 1.0e-8);
    TestUnitConversion (1000000.0, "US_SURVEY_MILE", 1609347218.69444, "METRE", 1.0e-5); // Does not pass w/ 1.0e-8

    // Dimension L2
    // All results calculated in excel and double checked at http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "CENTIMETRE_SQUARED", 100.0, "MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, "METRE_SQUARED", 1000000.0, "MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, "HECTARE", 10000000000.0, "MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, "KILOMETRE_SQUARED", 1000000000000.0, "MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, "INCH_SQUARED", 645.16, "MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, "FOOT_SQUARED", 92903.04, "MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (100, "THOUSAND_FOOT_SQUARED", 100000, "FOOT_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, "YARD_SQUARED", 836127.36, "MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, "ACRE", 4.04685642240001000E+03, "METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, "MILE_SQUARED", 2.589988110336E+06, "METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, "MILLIMETRE_SQUARED", 1e-6, "METRE_SQUARED", 1.0e-8);

    TestUnitConversion (1000000.0, "CENTIMETRE_SQUARED", 1.0e8, "MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (1000000.0, "METRE_SQUARED", 1.0e12, "MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (1000000.0, "HECTARE", 1.0e16, "MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (1000000.0, "KILOMETRE_SQUARED", 1.0e18, "MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (1000000.0, "INCH_SQUARED", 645160000.0, "MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (1000000.0, "FOOT_SQUARED", 92903040000.0, "MILLIMETRE_SQUARED", 10e-5); // Does not pass w/ 1.0e-8
    TestUnitConversion (100000000, "THOUSAND_FOOT_SQUARED", 1.0e11, "FOOT_SQUARED", 1.0e-8);
    TestUnitConversion (1000000.0, "YARD_SQUARED", 836127360000.0, "MILLIMETRE_SQUARED", 1.0e-3); // Does not pass w/ 1.0e-8
    TestUnitConversion (1000000.0, "ACRE", 4.0468564224e9, "METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1000000.0, "MILE_SQUARED", 2.589988110336e12, "METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1000000.0, "MILLIMETRE_SQUARED", 1.0, "METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, "ACRE", 1.0, "ACRE", 1.0e-8);

    // Diameter Length
    // All lines tagged with 'Value from excel' were calculated using the conversion factors in excel
    // All the lines tagged with 'Value from Windows Calc' were calculated using the conversion factors in Windows Calculator
    // All other values where alreay correct.  Any values that changed were close to the previous value.
    TestUnitConversion (-1162.5, "INCH_MILE", -1162.5, "INCH_MILE", 1.0e-8);
    TestUnitConversion (-1, "INCH_MILE", -1, "INCH_MILE", 1.0e-8);
    TestUnitConversion (0, "INCH_MILE", 0, "INCH_MILE", 1.0e-8);
    TestUnitConversion (1, "INCH_MILE", 1, "INCH_MILE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_MILE", 112.85, "INCH_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, "INCH_MILE", -6138000, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (-1, "INCH_MILE", -5280, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (0, "INCH_MILE", 0, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (1, "INCH_MILE", 5280, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (112.85, "INCH_MILE", 595848, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (-1162.5, "INCH_MILE", -96.875, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1, "INCH_MILE", -0.083333333, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (0, "INCH_MILE", 0, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (1, "INCH_MILE", 0.083333333, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_MILE", 9.40416666666666, "FOOT_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "INCH_MILE", -511500, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (-1, "INCH_MILE", -440, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (0, "INCH_MILE", 0, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (1, "INCH_MILE", 440, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (112.85, "INCH_MILE", 49654, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (-1162.5, "INCH_MILE", -47519904.96, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1, "INCH_MILE", -40877.3376, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (0, "INCH_MILE", 0, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (1, "INCH_MILE", 40877.3376, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_MILE", 4.61300754816E+06, "MILLIMETRE_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "INCH_MILE", -4.751990496E+04, "MILLIMETRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "INCH_MILE", -40.8773376, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, "INCH_MILE", 0, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, "INCH_MILE", 40.8773376, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_MILE", 4.61300754816E+03, "MILLIMETRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "INCH_MILE", -4.751990496E+04, "METRE_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "INCH_MILE", -40.8773376, "METRE_METRE", 1.0e-8);
    TestUnitConversion (0, "INCH_MILE", 0, "METRE_METRE", 1.0e-8);
    TestUnitConversion (1, "INCH_MILE", 40.8773376, "METRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_MILE", 4.61300754816E+03, "METRE_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "INCH_MILE", -4.75199049599999E+01, "METRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "INCH_MILE", -0.040877338, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, "INCH_MILE", 0, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, "INCH_MILE", 0.040877338, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_MILE", 4.61300754815999, "METRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "INCH_MILE", -1870862.4, "INCH_METRE", 1.0e-8);
    TestUnitConversion (-1, "INCH_MILE", -1609.344, "INCH_METRE", 1.0e-8);
    TestUnitConversion (0, "INCH_MILE", 0, "INCH_METRE", 1.0e-8);
    TestUnitConversion (1, "INCH_MILE", 1609.344, "INCH_METRE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_MILE", 1.816144704E+05, "INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "INCH_MILE", -29527.5, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (-1, "INCH_MILE", -25.4, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (0, "INCH_MILE", 0, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (1, "INCH_MILE", 25.4, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_MILE", 2866.39, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, "INCH_FOOT", -0.22017045, "INCH_MILE", 1.0e-8);
    TestUnitConversion (-1, "INCH_FOOT", -0.00018939394, "INCH_MILE", 1.0e-8);
    TestUnitConversion (0, "INCH_FOOT", 0, "INCH_MILE", 1.0e-8);
    TestUnitConversion (1, "INCH_FOOT", 0.00018939394, "INCH_MILE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_FOOT", 0.0213731, "INCH_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, "INCH_FOOT", -1162.5, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (-1, "INCH_FOOT", -1, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (0, "INCH_FOOT", 0, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (1, "INCH_FOOT", 1, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (112.85, "INCH_FOOT", 112.85, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (-1162.5, "INCH_FOOT", -0.018347538, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1, "INCH_FOOT", -1.5782828E-05, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (0, "INCH_FOOT", 0, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (1, "INCH_FOOT", 1.5782828E-05, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_FOOT", 0.0017810922, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, "INCH_FOOT", -96.875, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (-1, "INCH_FOOT", -0.083333333, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (0, "INCH_FOOT", 0, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (1, "INCH_FOOT", 0.083333333, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (112.85, "INCH_FOOT", 9.40416667, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (-1162.5, "INCH_FOOT", -8999.982, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1, "INCH_FOOT", -7.74192, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (0, "INCH_FOOT", 0, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (1, "INCH_FOOT", 7.74192, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_FOOT", 873.675672, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, "INCH_FOOT", -8.999982, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, "INCH_FOOT", -0.00774192, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, "INCH_FOOT", 0, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, "INCH_FOOT", 0.00774192, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_FOOT", 0.87367567, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, "INCH_FOOT", -8.999982, "METRE_METRE", 1.0e-8);
    TestUnitConversion (-1, "INCH_FOOT", -0.00774192, "METRE_METRE", 1.0e-8);
    TestUnitConversion (0, "INCH_FOOT", 0, "METRE_METRE", 1.0e-8);
    TestUnitConversion (1, "INCH_FOOT", 0.00774192, "METRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_FOOT", 0.87367567, "METRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, "INCH_FOOT", -0.0089999821, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, "INCH_FOOT", -7.7419201E-06, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, "INCH_FOOT", 0, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, "INCH_FOOT", 7.7419201E-06, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_FOOT", 0.00087367568, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, "INCH_FOOT", -354.33, "INCH_METRE", 1.0e-8);
    TestUnitConversion (-1, "INCH_FOOT", -0.3048, "INCH_METRE", 1.0e-8);
    TestUnitConversion (0, "INCH_FOOT", 0, "INCH_METRE", 1.0e-8);
    TestUnitConversion (1, "INCH_FOOT", 0.3048, "INCH_METRE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_FOOT", 34.39668, "INCH_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, "INCH_FOOT", -5.59232955, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (-1, "INCH_FOOT", -0.0048106061, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (0, "INCH_FOOT", 0, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (1, "INCH_FOOT", 0.0048106061, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_FOOT", 0.54287689, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, "FOOT_MILE", -1.395E+04, "INCH_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "FOOT_MILE", -12, "INCH_MILE", 1.0e-8);
    TestUnitConversion (0, "FOOT_MILE", 0, "INCH_MILE", 1.0e-8);
    TestUnitConversion (1, "FOOT_MILE", 12, "INCH_MILE", 1.0e-8);
    TestUnitConversion (112.85, "FOOT_MILE", 1.3542E+03, "INCH_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "FOOT_MILE", -73656000, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (-1, "FOOT_MILE", -6.336E+04, "INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (0, "FOOT_MILE", 0, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (1, "FOOT_MILE", 6.336E+04, "INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, "FOOT_MILE", 7150176, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (-1162.5, "FOOT_MILE", -1162.5, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1, "FOOT_MILE", -1, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (0, "FOOT_MILE", 0, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (1, "FOOT_MILE", 1, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (112.85, "FOOT_MILE", 112.85, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, "FOOT_MILE", -6138000, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (-1, "FOOT_MILE", -5.28E+03, "FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (0, "FOOT_MILE", 0, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (1, "FOOT_MILE", 5.28E+03, "FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, "FOOT_MILE", 595848, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (-1162.5, "FOOT_MILE", -570238859.52, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1, "FOOT_MILE", -490528.0512, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (0, "FOOT_MILE", 0, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (1, "FOOT_MILE", 490528.0512, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, "FOOT_MILE", 55356090.57792, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, "FOOT_MILE", -570238.85952, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, "FOOT_MILE", -4.905280512E+02, "MILLIMETRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (0, "FOOT_MILE", 0, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, "FOOT_MILE", 4.905280512E+02, "MILLIMETRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, "FOOT_MILE", 5.535609057792E+04, "MILLIMETRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "FOOT_MILE", -570238.85952, "METRE_METRE", 1.0e-8);
    TestUnitConversion (-1, "FOOT_MILE", -4.905280512E+02, "METRE_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (0, "FOOT_MILE", 0, "METRE_METRE", 1.0e-8);
    TestUnitConversion (1, "FOOT_MILE", 4.905280512E+02, "METRE_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, "FOOT_MILE", 5.535609057792E+04, "METRE_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "FOOT_MILE", -5.70238859519999E+02, "METRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "FOOT_MILE", -0.49052806, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, "FOOT_MILE", 0, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, "FOOT_MILE", 0.49052806, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, "FOOT_MILE", 5.53560905779199E+01, "METRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "FOOT_MILE", -22450348.8, "INCH_METRE", 1.0e-8);
    TestUnitConversion (-1, "FOOT_MILE", -1.9312128E+04, "INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (0, "FOOT_MILE", 0, "INCH_METRE", 1.0e-8);
    TestUnitConversion (1, "FOOT_MILE", 1.9312128E+04, "INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, "FOOT_MILE", 2179373.6448, "INCH_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, "FOOT_MILE", -3.5433E+05, "MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "FOOT_MILE", -3.048E+02, "MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (0, "FOOT_MILE", 0, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (1, "FOOT_MILE", 3.048E+02, "MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, "FOOT_MILE", 3.439668E+04, "MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "FOOT_FOOT", -2.64204545, "INCH_MILE", 1.0e-8);
    TestUnitConversion (-1, "FOOT_FOOT", -0.0022727273, "INCH_MILE", 1.0e-8);
    TestUnitConversion (0, "FOOT_FOOT", 0, "INCH_MILE", 1.0e-8);
    TestUnitConversion (1, "FOOT_FOOT", 0.0022727273, "INCH_MILE", 1.0e-8);
    TestUnitConversion (112.85, "FOOT_FOOT", 0.25647727, "INCH_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, "FOOT_FOOT", -13950, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (-1, "FOOT_FOOT", -12, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (0, "FOOT_FOOT", 0, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (1, "FOOT_FOOT", 12, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (112.85, "FOOT_FOOT", 1354.2, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (-1162.5, "FOOT_FOOT", -0.22017045, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1, "FOOT_FOOT", -0.00018939394, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (0, "FOOT_FOOT", 0, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (1, "FOOT_FOOT", 0.00018939394, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (112.85, "FOOT_FOOT", 0.0213731, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, "FOOT_FOOT", -1162.5, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (-1, "FOOT_FOOT", -1, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (0, "FOOT_FOOT", 0, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (1, "FOOT_FOOT", 1, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (112.85, "FOOT_FOOT", 112.85, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (-1162.5, "FOOT_FOOT", -107999.784, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1, "FOOT_FOOT", -92.90304, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (0, "FOOT_FOOT", 0, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (1, "FOOT_FOOT", 92.90304, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, "FOOT_FOOT", 1.0484108064E+04, "MILLIMETRE_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "FOOT_FOOT", -107.999784, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, "FOOT_FOOT", -0.09290304, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, "FOOT_FOOT", 0, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, "FOOT_FOOT", 0.09290304, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, "FOOT_FOOT", 1.0484108064E+01, "MILLIMETRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "FOOT_FOOT", -107.999784, "METRE_METRE", 1.0e-8);
    TestUnitConversion (-1, "FOOT_FOOT", -0.09290304, "METRE_METRE", 1.0e-8);
    TestUnitConversion (0, "FOOT_FOOT", 0, "METRE_METRE", 1.0e-8);
    TestUnitConversion (1, "FOOT_FOOT", 0.09290304, "METRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, "FOOT_FOOT", 1.0484108064E+01, "METRE_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "FOOT_FOOT", -0.10799979, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, "FOOT_FOOT", -9.2903041E-05, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, "FOOT_FOOT", 0, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, "FOOT_FOOT", 9.2903041E-05, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, "FOOT_FOOT", 0.0104841, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, "FOOT_FOOT", -4251.96, "INCH_METRE", 1.0e-8);
    TestUnitConversion (-1, "FOOT_FOOT", -3.6576, "INCH_METRE", 1.0e-8);
    TestUnitConversion (0, "FOOT_FOOT", 0, "INCH_METRE", 1.0e-8);
    TestUnitConversion (1, "FOOT_FOOT", 3.6576, "INCH_METRE", 1.0e-8);
    TestUnitConversion (112.85, "FOOT_FOOT", 412.76016, "INCH_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, "FOOT_FOOT", -6.71079545454545E+01, "MILLIMETRE_MILE", 1.0e-8); // Value from Excel
    TestUnitConversion (-1, "FOOT_FOOT", -5.77272727272727E-02, "MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (0, "FOOT_FOOT", 0, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (1, "FOOT_FOOT", 5.77272727272727E-02, "MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, "FOOT_FOOT", 6.51452273, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, "MILLIMETRE_METRE", -0.028438741, "INCH_MILE", 1.0e-8);
    TestUnitConversion (-1, "MILLIMETRE_METRE", -2.4463433E-05, "INCH_MILE", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_METRE", 0, "INCH_MILE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_METRE", 2.4463433E-05, "INCH_MILE", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_METRE", 0.0027606984, "INCH_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, "MILLIMETRE_METRE", -1.50156550313101E+02, "INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "MILLIMETRE_METRE", -0.12916693, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_METRE", 0, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_METRE", 0.12916693, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_METRE", 1.45764874863083E+01, "INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "MILLIMETRE_METRE", -0.002369895, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1, "MILLIMETRE_METRE", -2.0386194E-06, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_METRE", 0, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_METRE", 2.0386194E-06, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_METRE", 0.0002300582, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, "MILLIMETRE_METRE", -1.25130458594251E+01, "FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "MILLIMETRE_METRE", -0.01076391, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_METRE", 0, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_METRE", 0.01076391, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_METRE", 1.21470729, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (-1162.5, "MILLIMETRE_METRE", -1162.5, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1, "MILLIMETRE_METRE", -1, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_METRE", 0, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_METRE", 1, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_METRE", 112.85, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, "MILLIMETRE_METRE", -1.1625, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, "MILLIMETRE_METRE", -0.001, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_METRE", 0, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_METRE", 0.001, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_METRE", 0.11285, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, "MILLIMETRE_METRE", -1.1625, "METRE_METRE", 1.0e-8);
    TestUnitConversion (-1, "MILLIMETRE_METRE", -0.001, "METRE_METRE", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_METRE", 0, "METRE_METRE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_METRE", 0.001, "METRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_METRE", 0.11285, "METRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, "MILLIMETRE_METRE", -0.0011625, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, "MILLIMETRE_METRE", -1E-06, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_METRE", 0, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_METRE", 1E-06, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_METRE", 0.00011285, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, "MILLIMETRE_METRE", -4.57677165354331E+01, "INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "MILLIMETRE_METRE", -0.039370079, "INCH_METRE", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_METRE", 0, "INCH_METRE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_METRE", 0.039370079, "INCH_METRE", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_METRE", 4.44291339, "INCH_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, "MILLIMETRE_METRE", -0.72234401, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (-1, "MILLIMETRE_METRE", -0.00062137119, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_METRE", 0, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_METRE", 0.00062137119, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_METRE", 0.070121739, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, "MILLIMETRE_KILOMETRE", -2.84387405896024E+01, "INCH_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "MILLIMETRE_KILOMETRE", -0.024463433, "INCH_MILE", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_KILOMETRE", 0, "INCH_MILE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_KILOMETRE", 0.024463433, "INCH_MILE", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_KILOMETRE", 2.76069839, "INCH_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, "MILLIMETRE_KILOMETRE", -1.50156550313101E+05, "INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "MILLIMETRE_KILOMETRE", -129.166925, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_KILOMETRE", 0, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_KILOMETRE", 129.166925, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_KILOMETRE", 1.45764874863083E+04, "INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "MILLIMETRE_KILOMETRE", -2.36989504, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1, "MILLIMETRE_KILOMETRE", -0.0020386194, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_KILOMETRE", 0, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_KILOMETRE", 0.0020386194, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_KILOMETRE", 0.2300582, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, "MILLIMETRE_KILOMETRE", -1.25130458594251E+04, "FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "MILLIMETRE_KILOMETRE", -1.07639104167097E+01, "FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (0, "MILLIMETRE_KILOMETRE", 0, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_KILOMETRE", 1.07639104167097E+01, "FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, "MILLIMETRE_KILOMETRE", 1.21470729052569E+03, "FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "MILLIMETRE_KILOMETRE", -1162500, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1, "MILLIMETRE_KILOMETRE", -1000, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_KILOMETRE", 0, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_KILOMETRE", 1000, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_KILOMETRE", 112850, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, "MILLIMETRE_KILOMETRE", -1162.5, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, "MILLIMETRE_KILOMETRE", -1, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_KILOMETRE", 0, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_KILOMETRE", 1, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_KILOMETRE", 112.85, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, "MILLIMETRE_KILOMETRE", -1162.5, "METRE_METRE", 1.0e-8);
    TestUnitConversion (-1, "MILLIMETRE_KILOMETRE", -1, "METRE_METRE", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_KILOMETRE", 0, "METRE_METRE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_KILOMETRE", 1, "METRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_KILOMETRE", 112.85, "METRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, "MILLIMETRE_KILOMETRE", -1.1625, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, "MILLIMETRE_KILOMETRE", -0.001, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_KILOMETRE", 0, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_KILOMETRE", 0.001, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_KILOMETRE", 0.11285, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, "MILLIMETRE_KILOMETRE", -4.57677165354331E+04, "INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "MILLIMETRE_KILOMETRE", -3.93700787401575E+01, "INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (0, "MILLIMETRE_KILOMETRE", 0, "INCH_METRE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_KILOMETRE", 3.93700787401575E+01, "INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, "MILLIMETRE_KILOMETRE", 4.44291338582677E+03, "INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "MILLIMETRE_KILOMETRE", -7.22344010975901E+02, "MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "MILLIMETRE_KILOMETRE", -6.21371192237334E-01, "MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (0, "MILLIMETRE_KILOMETRE", 0, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_KILOMETRE", 6.21371192237334E-01, "MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, "MILLIMETRE_KILOMETRE", 7.01217390439831E+01, "MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "METRE_METRE", -2.84387405896024E+01, "INCH_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "METRE_METRE", -0.024463433, "INCH_MILE", 1.0e-8);
    TestUnitConversion (0, "METRE_METRE", 0, "INCH_MILE", 1.0e-8);
    TestUnitConversion (1, "METRE_METRE", 0.024463433, "INCH_MILE", 1.0e-8);
    TestUnitConversion (112.85, "METRE_METRE", 2.76069839, "INCH_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, "METRE_METRE", -1.50156550313101E+05, "INCH_FOOT", 1.0e-8); // value from excel
    TestUnitConversion (-1, "METRE_METRE", -129.166925, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (0, "METRE_METRE", 0, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (1, "METRE_METRE", 129.166925, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (112.85, "METRE_METRE", 1.45764874863083E+04, "INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "METRE_METRE", -2.36989504913353E+00, "FOOT_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "METRE_METRE", -0.0020386194, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (0, "METRE_METRE", 0, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (1, "METRE_METRE", 0.0020386194, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (112.85, "METRE_METRE", 0.2300582, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, "METRE_METRE", -1.25130458594251E+04, "FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "METRE_METRE", -1.07639104167097E+01, "FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (0, "METRE_METRE", 0, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (1, "METRE_METRE", 1.07639104167097E+01, "FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, "METRE_METRE", 1.21470729052569E+03, "FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "METRE_METRE", -1162500, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1, "METRE_METRE", -1000, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (0, "METRE_METRE", 0, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (1, "METRE_METRE", 1000, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, "METRE_METRE", 112850, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, "METRE_METRE", -1162.5, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, "METRE_METRE", -1, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, "METRE_METRE", 0, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, "METRE_METRE", 1, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, "METRE_METRE", 112.85, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, "METRE_METRE", -1162.5, "METRE_METRE", 1.0e-8);
    TestUnitConversion (-1, "METRE_METRE", -1, "METRE_METRE", 1.0e-8);
    TestUnitConversion (0, "METRE_METRE", 0, "METRE_METRE", 1.0e-8);
    TestUnitConversion (1, "METRE_METRE", 1, "METRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, "METRE_METRE", 112.85, "METRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, "METRE_METRE", -1.1625, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, "METRE_METRE", -0.001, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, "METRE_METRE", 0, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, "METRE_METRE", 0.001, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, "METRE_METRE", 0.11285, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, "METRE_METRE", -4.57677165354331E+04, "INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "METRE_METRE", -3.93700787401575E+01, "INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (0, "METRE_METRE", 0, "INCH_METRE", 1.0e-8);
    TestUnitConversion (1, "METRE_METRE", 3.93700787401575E+01, "INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, "METRE_METRE", 4.44291338582677E+03, "INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "METRE_METRE", -7.22344010975901E+02, "MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "METRE_METRE", -6.21371192237334E-01, "MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (0, "METRE_METRE", 0, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (1, "METRE_METRE", 6.21371192237334E-01, "MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, "METRE_METRE", 7.01217390439831E+01, "MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "METRE_KILOMETRE", -2.84387405896025E+04, "INCH_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "METRE_KILOMETRE", -2.44634327652494E+01, "INCH_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (0, "METRE_KILOMETRE", 0, "INCH_MILE", 1.0e-8);
    TestUnitConversion (1, "METRE_KILOMETRE", 2.44634327652494E+01, "INCH_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, "METRE_KILOMETRE", 2.7606983875584E+03, "INCH_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "METRE_KILOMETRE", -150156550.31310064824132043958274, "INCH_FOOT", 1.0e-9); // Value from Windows Calc
    TestUnitConversion (-1, "METRE_KILOMETRE", -1.29166925000517E+05, "INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (0, "METRE_KILOMETRE", 0, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (1, "METRE_KILOMETRE", 1.29166925000517E+05, "INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, "METRE_KILOMETRE", 14576487.486308308089490762672612, "INCH_FOOT", 1.0e-8); // Value from Windows Calc
    TestUnitConversion (-1162.5, "METRE_KILOMETRE", -2.36989504913354E+03, "FOOT_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "METRE_KILOMETRE", -2.03861939710412, "FOOT_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (0, "METRE_KILOMETRE", 0, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (1, "METRE_KILOMETRE", 2.03861939710412, "FOOT_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, "METRE_KILOMETRE", 2.300581989632E+02, "FOOT_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "METRE_KILOMETRE", -12513045.859425054020110036631895, "FOOT_FOOT", 1.0e-8); // Value from Windows Calc
    TestUnitConversion (-1, "METRE_KILOMETRE", -1.07639104167098E+04, "FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (0, "METRE_KILOMETRE", 0, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (1, "METRE_KILOMETRE", 1.07639104167098E+04, "FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, "METRE_KILOMETRE", 1214707.2905256923407908968893844, "FOOT_FOOT", 1.0e-8); // Value from Windows Calc
    TestUnitConversion (-1162.5, "METRE_KILOMETRE", -1162500000, "MILLIMETRE_METRE", 1.0e-6);
    TestUnitConversion (-1, "METRE_KILOMETRE", -1000000, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (0, "METRE_KILOMETRE", 0, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (1, "METRE_KILOMETRE", 1000000, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, "METRE_KILOMETRE", 112850000, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, "METRE_KILOMETRE", -1162500, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, "METRE_KILOMETRE", -1000, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, "METRE_KILOMETRE", 0, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, "METRE_KILOMETRE", 1000, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, "METRE_KILOMETRE", 112850, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, "METRE_KILOMETRE", -1162500, "METRE_METRE", 1.0e-8);
    TestUnitConversion (-1, "METRE_KILOMETRE", -1000, "METRE_METRE", 1.0e-8);
    TestUnitConversion (0, "METRE_KILOMETRE", 0, "METRE_METRE", 1.0e-8);
    TestUnitConversion (1, "METRE_KILOMETRE", 1000, "METRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, "METRE_KILOMETRE", 112850, "METRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, "METRE_KILOMETRE", -1162.5, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, "METRE_KILOMETRE", -1, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, "METRE_KILOMETRE", 0, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, "METRE_KILOMETRE", 1, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, "METRE_KILOMETRE", 112.85, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, "METRE_KILOMETRE", -4.57677165354332000E+07, "INCH_METRE", 1.0e-6); // Value from excel
    TestUnitConversion (-1, "METRE_KILOMETRE", -3.93700787401576E+04, "INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (0, "METRE_KILOMETRE", 0, "INCH_METRE", 1.0e-8);
    TestUnitConversion (1, "METRE_KILOMETRE", 3.93700787401576E+04, "INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, "METRE_KILOMETRE", 4.44291338582678000E+06, "INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "METRE_KILOMETRE", -7.22344010975903000E+05, "MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "METRE_KILOMETRE", -6.21371192237336000E+02, "MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (0, "METRE_KILOMETRE", 0, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (1, "METRE_KILOMETRE", 6.21371192237336000E+02, "MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, "METRE_KILOMETRE", 7.01217390439833000E+04, "MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "INCH_METRE", -7.22344010975901000E-01, "INCH_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "INCH_METRE", -0.00062137119, "INCH_MILE", 1.0e-8);
    TestUnitConversion (0, "INCH_METRE", 0, "INCH_MILE", 1.0e-8);
    TestUnitConversion (1, "INCH_METRE", 0.00062137119, "INCH_MILE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_METRE", 0.070121739, "INCH_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, "INCH_METRE", -3.81397637795276000E+03, "INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "INCH_METRE", -3.2808399, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (0, "INCH_METRE", 0, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (1, "INCH_METRE", 3.2808399, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (112.85, "INCH_METRE", 3.70242782152231000E+02, "INCH_FOOT", 1.0e-8); // Valu from excel
    TestUnitConversion (-1162.5, "INCH_METRE", -0.060195334, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1, "INCH_METRE", -5.1780932E-05, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (0, "INCH_METRE", 0, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (1, "INCH_METRE", 5.1780932E-05, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_METRE", 0.0058434782, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, "INCH_METRE", -3.17831364829396000E+02, "FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "INCH_METRE", -0.27340332, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (0, "INCH_METRE", 0, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (1, "INCH_METRE", 0.27340332, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (112.85, "INCH_METRE", 3.08535651793526000E+01, "FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "INCH_METRE", -29527.5, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1, "INCH_METRE", -25.4, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (0, "INCH_METRE", 0, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (1, "INCH_METRE", 25.4, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_METRE", 2866.39, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, "INCH_METRE", -29.5275, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, "INCH_METRE", -0.0254, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, "INCH_METRE", 0, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, "INCH_METRE", 0.0254, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_METRE", 2.86639, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, "INCH_METRE", -29.5275, "METRE_METRE", 1.0e-8);
    TestUnitConversion (-1, "INCH_METRE", -0.0254, "METRE_METRE", 1.0e-8);
    TestUnitConversion (0, "INCH_METRE", 0, "METRE_METRE", 1.0e-8);
    TestUnitConversion (1, "INCH_METRE", 0.0254, "METRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_METRE", 2.86639, "METRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, "INCH_METRE", -0.0295275, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, "INCH_METRE", -2.54E-05, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, "INCH_METRE", 0, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, "INCH_METRE", 2.54E-05, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_METRE", 0.00286639, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, "INCH_METRE", -1162.5, "INCH_METRE", 1.0e-8);
    TestUnitConversion (-1, "INCH_METRE", -1, "INCH_METRE", 1.0e-8);
    TestUnitConversion (0, "INCH_METRE", 0, "INCH_METRE", 1.0e-8);
    TestUnitConversion (1, "INCH_METRE", 1, "INCH_METRE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_METRE", 112.85, "INCH_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, "INCH_METRE", -1.83475378787879000E+01, "MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "INCH_METRE", -0.015782828, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (0, "INCH_METRE", 0, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (1, "INCH_METRE", 0.015782828, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (112.85, "INCH_METRE", 1.78109217, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, "MILLIMETRE_MILE", -4.57677165354331000E+01, "INCH_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "MILLIMETRE_MILE", -0.039370079, "INCH_MILE", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_MILE", 0, "INCH_MILE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_MILE", 0.039370079, "INCH_MILE", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_MILE", 4.44291339, "INCH_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, "MILLIMETRE_MILE", -2.41653543307087000E+05, "INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "MILLIMETRE_MILE", -2.07874015748031000E+02, "INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (0, "MILLIMETRE_MILE", 0, "INCH_FOOT", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_MILE", 2.07874015748031000E+02, "INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, "MILLIMETRE_MILE", 2.34585826771654000E+04, "INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "MILLIMETRE_MILE", -3.81397637795275, "FOOT_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "MILLIMETRE_MILE", -3.28083989501312000E-03, "FOOT_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (0, "MILLIMETRE_MILE", 0, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_MILE", 3.28083989501312000E-03, "FOOT_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, "MILLIMETRE_MILE", 0.37024278, "FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, "MILLIMETRE_MILE", -2.01377952755906000E+04, "FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "MILLIMETRE_MILE", -1.73228346456693000E+01, "FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (0, "MILLIMETRE_MILE", 0, "FOOT_FOOT", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_MILE", 1.73228346456693000E+01, "FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, "MILLIMETRE_MILE", 1.95488188976378000E+03, "FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "MILLIMETRE_MILE", -1870862.4, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1, "MILLIMETRE_MILE", -1609.344, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_MILE", 0, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_MILE", 1609.344, "MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_MILE", 1.816144704E+05, "MILLIMETRE_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "MILLIMETRE_MILE", -1870.8624, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, "MILLIMETRE_MILE", -1.609344, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_MILE", 0, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_MILE", 1.609344, "MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_MILE", 1.81614470400000000E+02, "MILLIMETRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "MILLIMETRE_MILE", -1870.8624, "METRE_METRE", 1.0e-8);
    TestUnitConversion (-1, "MILLIMETRE_MILE", -1.609344, "METRE_METRE", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_MILE", 0, "METRE_METRE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_MILE", 1.609344, "METRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_MILE", 1.81614470400000000E+02, "METRE_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "MILLIMETRE_MILE", -1.8708624, "METRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, "MILLIMETRE_MILE", -0.001609344, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_MILE", 0, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_MILE", 0.001609344, "METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_MILE", 1.81614470400000000E-01, "METRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, "MILLIMETRE_MILE", -73656, "INCH_METRE", 1.0e-8);
    TestUnitConversion (-1, "MILLIMETRE_MILE", -63.36, "INCH_METRE", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_MILE", 0, "INCH_METRE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_MILE", 63.36, "INCH_METRE", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_MILE", 7150.176, "INCH_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, "MILLIMETRE_MILE", -1162.5, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (-1, "MILLIMETRE_MILE", -1, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (0, "MILLIMETRE_MILE", 0, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (1, "MILLIMETRE_MILE", 1, "MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (112.85, "MILLIMETRE_MILE", 112.85, "MILLIMETRE_MILE", 1.0e-8);

    // L3
    // All results calculated in excel and double checked at http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "ACRE_FOOT", 1.0, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1.0, "ACRE_INCH", 8.33333333333333e-02, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1000.0, "CENTIMETRE_CUBED", 8.10713193789912e-07, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100.0, "FOOT_CUBED", 2.29568411386593e-03, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100.0, "GALLON", 3.06888327721661e-04, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100.0, "GALLON_IMPERIAL", 3.68557514315638e-04, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1000.0, "INCH_CUBED", 1.32852089922797e-05, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100.0, "LITRE", 8.10713193789912e-05, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100.0, "METRE_CUBED", 8.10713193789912e-02, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1.0, "MILLION_GALLON", 3.06888327721661, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1.0, "MILLION_LITRE", 8.10713193789912e-01, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1.0, "THOUSAND_GALLON", 3.06888327721661e-03, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100.0, "THOUSAND_LITRE", 8.10713193789912e-02, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100.0, "YARD_CUBED", 6.19834710743802e-02, "ACRE_FOOT", 1.0e-8);

    TestUnitConversion (1000000.0, "ACRE_FOOT", 1.0e6, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1000000.0, "ACRE_INCH", 8.33333333333333e4, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1000000000.0, "CENTIMETRE_CUBED", 8.10713193789912e-01, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100000000.0, "FOOT_CUBED", 2.29568411386593e3, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100000000.0, "GALLON", 3.06888327721661e2, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100000000.0, "GALLON_IMPERIAL", 3.68557514315638e2, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1000000000.0, "INCH_CUBED", 1.32852089922797e1, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100000000.0, "LITRE", 8.10713193789912e1, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100000000.0, "METRE_CUBED", 8.10713193789912e4, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1000000.0, "MILLION_GALLON", 3.06888327721661e6, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1000000.0, "MILLION_LITRE", 8.10713193789912e5, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1000000.0, "THOUSAND_GALLON", 3.06888327721661e3, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100000000.0, "THOUSAND_LITRE", 8.10713193789912e4, "ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100000000.0, "YARD_CUBED", 6.19834710743802e4, "ACRE_FOOT", 1.0e-8);

    // Mass
    // All results calculated in excel and double checked at http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "GRAIN", 1.42857142857143e-4, "POUND", 1.0e-8);
    TestUnitConversion (1.0, "GRAM", 2.20462262184878e-3, "POUND", 1.0e-8);
    TestUnitConversion (1.0, "KILOGRAM", 2.20462262184878, "POUND", 1.0e-8);
    TestUnitConversion (1.0, "LONG_TON", 2240, "POUND", 1.0e-8);
    TestUnitConversion (1.0, "MEGAGRAM", 2204.62262184878, "POUND", 1.0e-8);
    TestUnitConversion (1000.0, "MICROGRAM", 2.20462262184878e-6, "POUND", 1.0e-8);
    TestUnitConversion (1000.0, "MILLIGRAM", 2.20462262184878e-3, "POUND", 1.0e-8);
    TestUnitConversion (1.0, "POUND", 1.0, "POUND", 1.0e-8);
    TestUnitConversion (1.0, "SHORT_TON", 2000.0, "POUND", 1.0e-8);

    TestUnitConversion (1000000.0, "GRAIN", 1.42857142857143e2, "POUND", 1.0e-8);
    TestUnitConversion (1000000.0, "GRAM", 2.20462262184878e3, "POUND", 1.0e-8);
    TestUnitConversion (1000000.0, "KILOGRAM", 2204622.62184878, "POUND", 1.0e-8);
    TestUnitConversion (1000000.0, "LONG_TON", 2240000000, "POUND", 1.0e-8);
    TestUnitConversion (1000000.0, "MEGAGRAM", 2204622621.84878, "POUND", 1.0e-5); // Does not pass w/ 1.0e-8
    TestUnitConversion (1000000000.0, "MICROGRAM", 2.20462262184878, "POUND", 1.0e-8);
    TestUnitConversion (1000000000.0, "MILLIGRAM", 2.20462262184878e3, "POUND", 1.0e-8);
    TestUnitConversion (1000000.0, "POUND", 1000000.0, "POUND", 1.0e-8);
    TestUnitConversion (1000000.0, "SHORT_TON", 2000000000.0, "POUND", 1.0e-6); // Does not pass w/ 1.0e-8

    // Time
    // All results calculated in excel and double checked at http://online.unitconverterpro.com/conversion-tables/convert-alpha/factors.php?cat=time&unit=29&val=
    TestUnitConversion (10000.0, "MILLISECOND", 3.17097919837647e-7, "YEAR", 1.0e-8);
    TestUnitConversion (10000.0, "SECOND", 3.17097919837647e-4, "YEAR", 1.0e-8);
    TestUnitConversion (1.0, "MINUTE", 1.90258751902588e-06, "YEAR", 1.0e-8);
    TestUnitConversion (1.0, "HOUR", 1.14155251141553e-04, "YEAR", 1.0e-8);
    TestUnitConversion (1.0, "DAY", 2.73972602739727e-03, "YEAR", 1.0e-8);
    TestUnitConversion (1.0, "YEAR", 1.0, "YEAR", 1.0e-8);

    TestUnitConversion (1000000000.0, "MILLISECOND", 3.17097919837647e-2, "YEAR", 1.0e-8);
    TestUnitConversion (100000000.0, "SECOND", 3.17097919837647, "YEAR", 1.0e-8);
    TestUnitConversion (1000000.0, "MINUTE", 1.90258751902588, "YEAR", 1.0e-8);
    TestUnitConversion (1000000.0, "HOUR", 114.155251141553, "YEAR", 1.0e-8);
    TestUnitConversion (1000000.0, "DAY", 2739.72602739727, "YEAR", 1.0e-8);
    TestUnitConversion (1000000.0, "YEAR", 1000000.0, "YEAR", 1.0e-8);
    

    // Plane Angle
    // All results calculated in excel and double checked at http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "DEGREE", 1.0, "DEGREE", 1.0e-8);
    TestUnitConversion (1.0, "GRADIAN", 0.9, "DEGREE", 1.0e-8);
    TestUnitConversion (1.0, "ANGLE_MINUTE", 1.66666666666667e-02, "DEGREE", 1.0e-8);
    TestUnitConversion (1.0, "ANGLE_QUADRANT", 90, "DEGREE", 1.0e-8);
    TestUnitConversion (1.0, "RADIAN", 5.72957795130823e1, "DEGREE", 1.0e-8);
    TestUnitConversion (1.0, "REVOLUTION", 360.0, "DEGREE", 1.0e-8);
    TestUnitConversion (1000.0, "ANGLE_SECOND", 2.77777777777778e-1, "DEGREE", 1.0e-8);

    TestUnitConversion (1000000.0, "DEGREE", 1000000.0, "DEGREE", 1.0e-8);
    TestUnitConversion (1000000.0, "GRADIAN", 900000, "DEGREE", 1.0e-8);
    TestUnitConversion (1000000.0, "ANGLE_MINUTE", 1.66666666666667e4, "DEGREE", 1.0e-8);
    TestUnitConversion (1000000.0, "ANGLE_QUADRANT", 90000000, "DEGREE", 1.0e-8);
    TestUnitConversion (1000000.0, "RADIAN", 5.72957795130823e7, "DEGREE", 1.0e-7);
    TestUnitConversion (1000000.0, "REVOLUTION", 360000000.0, "DEGREE", 1.0e-8);
    TestUnitConversion (1000000000.0, "ANGLE_SECOND", 2.77777777777778e5, "DEGREE", 1.0e-8);

    // K
    // Test Absolute temperatures
    // All results calculated at http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    // and double checked at http://www.chemie.fu-berlin.de/chemistry/general/units_en.html#temp
    TestUnitConversion (1.0, "DEGREE_CELSIUS", 493.47, "DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (1.0, "DEGREE_KELVIN", 1.8, "DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (1.0, "DEGREE_FAHRENHEIT", 460.67, "DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (1.0, "DEGREE_RANKINE", 0.55555555556, "DEGREE_KELVIN", 1.0e-8);

    TestUnitConversion (100.0, "DEGREE_CELSIUS", 671.67, "DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (100.0, "DEGREE_KELVIN", 180.0, "DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (100.0, "DEGREE_FAHRENHEIT", 559.67, "DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (100.0, "DEGREE_RANKINE", 55.55555555556, "DEGREE_KELVIN", 1.0e-8);

    TestUnitConversion (-50.0, "DEGREE_CELSIUS", 401.67, "DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (-50.0, "DEGREE_KELVIN", -90.0, "DEGREE_RANKINE", 1.0e-8); // Now that's what I call COLD
    TestUnitConversion (-50.0, "DEGREE_FAHRENHEIT", 409.67, "DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (-50.0, "DEGREE_RANKINE", -27.777777778, "DEGREE_KELVIN", 1.0e-8); // This is only a little nippy

    // Test Delta temperatures
    // Not Verified in unit converter.
    TestUnitConversion (1.0, "DELTA_DEGREE_CELSIUS", 1.8, "DELTA_DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (1.0, "DELTA_DEGREE_KELVIN", 1.8, "DELTA_DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (1.0, "DELTA_DEGREE_FAHRENHEIT", 1.0, "DELTA_DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (1.0, "DELTA_DEGREE_RANKINE", 0.55555555556, "DELTA_DEGREE_KELVIN", 1.0e-8);

    TestUnitConversion (100.0, "DELTA_DEGREE_CELSIUS", 180.0, "DELTA_DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (100.0, "DELTA_DEGREE_KELVIN", 180.0, "DELTA_DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (100.0, "DELTA_DEGREE_FAHRENHEIT", 100.0, "DELTA_DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (100.0, "DELTA_DEGREE_RANKINE", 55.55555555556, "DELTA_DEGREE_KELVIN", 1.0e-8);

    TestUnitConversion (-50.0, "DELTA_DEGREE_CELSIUS", -90.0, "DELTA_DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (-50.0, "DELTA_DEGREE_KELVIN", -90.0, "DELTA_DEGREE_RANKINE", 1.0e-8); // Now that's what I call COLD
    TestUnitConversion (-50.0, "DELTA_DEGREE_FAHRENHEIT", -50.0, "DELTA_DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (-50.0, "DELTA_DEGREE_RANKINE", -27.777777778, "DELTA_DEGREE_KELVIN", 1.0e-8); // This is only a little nippy

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "CENTIMETRE_PER_HOUR", 1.0, "CENTIMETRE_PER_HOUR", 1.0e-8);

    TestUnitConversion (1.0, "CENTIMETRE_PER_MINUTE", 60, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, "CENTIMETRE_PER_SECOND", 3600, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, "FOOT_PER_HOUR", 30.48, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, "FOOT_PER_MINUTE", 30.48 * 60, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, "FOOT_PER_SECOND", 30.48 * 3600, "CENTIMETRE_PER_HOUR", 1.0e-8);

    TestUnitConversion (1.0, "INCH_PER_HOUR", 2.54, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, "INCH_PER_MINUTE", 2.54 * 60, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, "INCH_PER_SECOND", 2.54 * 3600, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, "KILOMETRE_PER_HOUR", 100000, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, "KNOT_INTERNATIONAL", 1.852e5, "CENTIMETRE_PER_HOUR", 1.0e-8); // Value from excel

    TestUnitConversion (1.0, "KNOT", 1853.184 * 100, "CENTIMETRE_PER_HOUR", 1.0e-8); // value checked at http://www.unitconversion.org/velocity/knots-uk-to-centimeters-per-hour-conversion.html
    TestUnitConversion (1.0, "METRE_PER_HOUR", 100, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, "METRE_PER_MINUTE", 6000, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, "METRE_PER_SECOND", 360000, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, "MILE_PER_HOUR", 30.48 * 5280, "CENTIMETRE_PER_HOUR", 1.0e-8);
    
    TestUnitConversion (1.0, "CENTIMETRE_PER_DAY", 1.0 / 24.0, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, "FOOT_PER_DAY", 30.48 / 24.0, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, "INCH_PER_DAY", 2.54 / 24.0, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, "METRE_PER_DAY", 100.0 / 24.0, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, "MILLIMETRE_PER_DAY", 0.1 / 24.0, "CENTIMETRE_PER_HOUR", 1.0e-8);
    
    TestUnitConversion (1.0, "MILLIMETRE_PER_HOUR", 0.1, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, "MILLIMETRE_PER_MINUTE", 6.0, "CENTIMETRE_PER_HOUR", 1.0e-8);

    TestUnitConversion (1.0e6, "CENTIMETRE_PER_MINUTE", 60e6, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, "CENTIMETRE_PER_SECOND", 3600e6, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, "FOOT_PER_HOUR", 30.48e6, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, "FOOT_PER_MINUTE", 30.48 * 60e6, "CENTIMETRE_PER_HOUR", 1.0e-6);
    TestUnitConversion (1.0e6, "FOOT_PER_SECOND", 30.48 * 3600e6, "CENTIMETRE_PER_HOUR", 1.0e-4);

    TestUnitConversion (1.0e6, "INCH_PER_HOUR", 2.54e6, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, "INCH_PER_MINUTE", 2.54 * 60e6, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, "INCH_PER_SECOND", 2.54 * 3600e6, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, "KILOMETRE_PER_HOUR", 1e11, "CENTIMETRE_PER_HOUR", 1.0e-4);
    TestUnitConversion (1.0e6, "KNOT_INTERNATIONAL", 1.852e11, "CENTIMETRE_PER_HOUR", 1.0e-4); // Value from excel

    TestUnitConversion (1.0e6, "KNOT", 1853.184 * 100 * 1e6, "CENTIMETRE_PER_HOUR", 1.0e-5); // value checked at http://www.unitconversion.org/velocity/knots-uk-to-centimeters-per-hour-conversion.html
    TestUnitConversion (1.0e6, "METRE_PER_HOUR", 1e8, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, "METRE_PER_MINUTE", 6.0e9, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, "METRE_PER_SECOND", 36e10, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, "MILE_PER_HOUR", 1.0e6 * 30.48 * 5280, "CENTIMETRE_PER_HOUR", 1.0e-4);

    TestUnitConversion (1.0e6, "CENTIMETRE_PER_DAY", 1.0e6 / 24.0, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, "FOOT_PER_DAY", 30.48e6 / 24.0, "CENTIMETRE_PER_HOUR", 1.0e-2);
    TestUnitConversion (1.0e6, "INCH_PER_DAY", 2.54e6 / 24.0, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, "METRE_PER_DAY", 1.0e8 / 24.0, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, "MILLIMETRE_PER_DAY", 1.0e5 / 24.0, "CENTIMETRE_PER_HOUR", 1.0e-8);

    TestUnitConversion (1.0e6, "MILLIMETRE_PER_HOUR", 1.0e5, "CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, "MILLIMETRE_PER_MINUTE", 6.0e6, "CENTIMETRE_PER_HOUR", 1.0e-8);

    TestUnitConversion (1.0, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8);

    TestUnitConversion (1.0, "FOOT_CUBED_PER_FOOT_SQUARED_PER_SECOND", 4.356e4, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, "FOOT_CUBED_PER_MILE_SQUARED_PER_SECOND", 1.5625e-3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e3, "GALLON_PER_ACRE_PER_DAY", 1.54722865226337e-3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, "GALLON_PER_ACRE_PER_MINUTE", 2.22800925925925e-3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, "LITRE_PER_METRE_SQUARED_PER_SECOND", 1.42913385826772e2, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel

    TestUnitConversion (1.0, "GALLON_PER_FOOT_SQUARED_PER_DAY", 6.73972800925924e-2, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, "GALLON_PER_FOOT_SQUARED_PER_MINUTE", 9.70520833333332e1, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e6, "GALLON_PER_MILE_SQUARED_PER_DAY", 2.41754476916152e-3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e3, "GALLON_PER_MILE_SQUARED_PER_MINUTE", 3.48126446759259e-3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel

    TestUnitConversion (1.0e4, "LITRE_PER_HECTARE_PER_DAY", 1.65409011373578e-3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e6, "LITRE_PER_KILOMETRE_SQUARED_PER_DAY", 1.65409011373578e-3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, "LITRE_PER_METRE_SQUARED_PER_DAY", 1.65409011373578e-3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e1, "METRE_CUBED_PER_HECTARE_PER_DAY", 1.65409011373578e-3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel

    TestUnitConversion (1.0e3, "METRE_CUBED_PER_KILOMETRE_SQUARED_PER_DAY", 1.65409011373578e-3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, "METRE_CUBED_PER_METRE_SQUARED_PER_DAY", 1.65409011373578, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, "FOOT_CUBED_PER_FOOT_SQUARED_PER_MINUTE", 7.25999999999998e2, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel

    TestUnitConversion (1.0e6, "FOOT_CUBED_PER_FOOT_SQUARED_PER_SECOND", 4.356e10, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-5); // Value from excel
    TestUnitConversion (1.0e6, "FOOT_CUBED_PER_MILE_SQUARED_PER_SECOND", 1.5625e3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e9, "GALLON_PER_ACRE_PER_DAY", 1.54722865226337e3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e6, "GALLON_PER_ACRE_PER_MINUTE", 2.22800925925925e3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e6, "LITRE_PER_METRE_SQUARED_PER_SECOND", 1.42913385826772e8, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-6); // Value from excel

    TestUnitConversion (1.0e6, "GALLON_PER_FOOT_SQUARED_PER_DAY", 6.73972800925924e4, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e6, "GALLON_PER_FOOT_SQUARED_PER_MINUTE", 9.70520833333332e7, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-6); // Value from excel
    TestUnitConversion (1.0e12, "GALLON_PER_MILE_SQUARED_PER_DAY", 2.41754476916152e3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e9, "GALLON_PER_MILE_SQUARED_PER_MINUTE", 3.48126446759259e3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e10, "LITRE_PER_HECTARE_PER_DAY", 1.65409011373578e3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel

    TestUnitConversion (1.0e10, "LITRE_PER_HECTARE_PER_DAY", 1.65409011373578e3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e12, "LITRE_PER_KILOMETRE_SQUARED_PER_DAY", 1.65409011373578e3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e6, "LITRE_PER_METRE_SQUARED_PER_DAY", 1.65409011373578e3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e7, "METRE_CUBED_PER_HECTARE_PER_DAY", 1.65409011373578e3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel

    TestUnitConversion (1.0e9, "METRE_CUBED_PER_KILOMETRE_SQUARED_PER_DAY", 1.65409011373578e3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e6, "METRE_CUBED_PER_METRE_SQUARED_PER_DAY", 1.65409011373578e6, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e6, "FOOT_CUBED_PER_FOOT_SQUARED_PER_MINUTE", 7.25999999999998e8, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-5); // Value from excel

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "GIGANEWTON_SECOND_PER_KILOGRAM", 1.0, "GIGANEWTON_SECOND_PER_KILOGRAM", 1.0e-8);
    TestUnitConversion (1.0, "INCH_SQUARED_INCH_HG_CONVENTIONAL_HOUR_PER_GRAIN_MASS", 1.21377748762502e-1, "GIGANEWTON_SECOND_PER_KILOGRAM", 1.0e-8);

    TestUnitConversion (1.0e6, "GIGANEWTON_SECOND_PER_KILOGRAM", 1.0e6, "GIGANEWTON_SECOND_PER_KILOGRAM", 1.0e-8);
    TestUnitConversion (1.0e6, "INCH_SQUARED_INCH_HG_CONVENTIONAL_HOUR_PER_GRAIN_MASS", 1.21377748762502e5, "GIGANEWTON_SECOND_PER_KILOGRAM", 1.0e-8);

    TestUnitConversion (1.0, "CENTISTOKE", 1.0, "CENTISTOKE", 1.0e-8);
    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    // because stoke was not included in the online calculator the following was used in its place:
    // (m^2/s)*1.0e-6
    // 1 Stoke = 1.0e-4 m^2/s (NIST doc), 1 CentiStoke = .01 Stokes = 1.0e-2 Stokes
    // 1 CentiStoke = 1.0e-6 m^2/s
    TestUnitConversion (1.0, "FOOT_SQUARED_PER_SECOND", 9.29030400000002e4, "CENTISTOKE", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, "METRE_SQUARED_PER_SECOND", 1.0e6, "CENTISTOKE", 1.0e-8);
    TestUnitConversion (1.0, "STOKE", 100.0, "CENTISTOKE", 1.0e-8);


    TestUnitConversion (1.0e6, "FOOT_SQUARED_PER_SECOND", 9.29030400000002e10, "CENTISTOKE", 1.0e-3); // Value from excel
    TestUnitConversion (1.0e6, "METRE_SQUARED_PER_SECOND", 1.0e12, "CENTISTOKE", 1.0e-8);
    TestUnitConversion (1.0e6, "STOKE", 1.0e8, "CENTISTOKE", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (100, "GALLON_PER_MINUTE", 4.41919191919192000E-01, "ACRE_FOOT_PER_DAY", 1.0e-8); // Value from excel
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 2.27220117845118000E-01, "ACRE_FOOT_PER_HOUR", 1.0e-8); // Value from excel
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 3.78700196408530000E-03, "ACRE_FOOT_PER_MINUTE", 1.0e-8); // Value from excel
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 2.72664141414141, "ACRE_INCH_PER_HOUR", 1.0e-8); // Value from excel
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 4.54440235690236000E-02, "ACRE_INCH_PER_MINUTE", 1.0e-8); // Value from excel
    
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 1.64961805555556000E+02, "FOOT_CUBED_PER_MINUTE", 1.0e-8); // Value from excel
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 2.74936342592593, "FOOT_CUBED_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 2.37545E+05, "FOOT_CUBED_PER_DAY", 1.0e-8); // Value from excel
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 2.80271888487360000E+02, "METRE_CUBED_PER_HOUR", 1.0e-8); // Value from excel
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 1.47962871911833000E+06, "GALLON_IMPERIAL_PER_DAY", 1.0e-8); // Value from excel
    
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 1.02751994383218000E+03, "GALLON_IMPERIAL_PER_MINUTE", 1.0e-8); // Value from excel
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 1.71253323972029000E+01, "GALLON_IMPERIAL_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 1.77696E+06, "GALLON_PER_DAY", 1.0e-8); // Value from excel
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 1234, "GALLON_PER_MINUTE", 1.0e-8); // Value from excel
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 2.05666666666666000E+01, "GALLON_PER_SECOND", 1.0e-8); // Value from excel
    
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 6.72652532369664000E+06, "LITRE_PER_DAY", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, "GALLON_PER_MINUTE", 3.785411784, "LITRE_PER_MINUTE", 1.0e-8); // Value from excel
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 4.67119814145600000E+03, "LITRE_PER_MINUTE", 1.0e-8); // Value from excel
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 7.78533023576000000E+01, "LITRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 6.72652532369664, "MEGA_LITRE_PER_DAY", 1.0e-8); // Value from excel
    
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 6.72652532369664000E+03, "METRE_CUBED_PER_DAY", 1.0e-8); // Value from excel
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 4.671198141456, "METRE_CUBED_PER_MINUTE", 1.0e-8); // Value from excel
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 7.78533023576000000E-02, "METRE_CUBED_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 1.77696, "MILLION_GALLON_PER_DAY", 1.0e-8); // Value from excel
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 1.47962871911833, "MILLION_GALLON_IMPERIAL_PER_DAY", 1.0e-8); // Value from excel
    
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 6.72652532369664, "MILLION_LITRE_PER_DAY", 1.0e-8); // Value from excel
    TestUnitConversion (1234, "GALLON_PER_MINUTE", 2.8027188848736e5, "LITRE_PER_HOUR", 1.0e-8); // Value from excel

    TestUnitConversion (1234, "LITRE_PER_SECOND_PER_PERSON", 2.816538995808e7, "GALLON_PER_DAY_PER_PERSON", 1.0e-8); // Value from excel
    TestUnitConversion (1234e6, "LITRE_PER_SECOND_PER_PERSON", 2.816538995808e13, "GALLON_PER_DAY_PER_PERSON", 1.0e-8); // Value from excel
    
    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    // Some expected values are from NIST, others are calculated, calculated values are noted
    TestUnitConversion (1.0, "DYNE", 1.0, "DYNE", 1.0e-8);
    TestUnitConversion (1.0, "KILOGRAM_FORCE", 9.80665 / 1.0e-5, "DYNE", 1.0e-8);
    TestUnitConversion (1.0, "KILONEWTON", 1000 / 1.0e-5, "DYNE", 1.0e-5);
    TestUnitConversion (1.0, "KILOPOUND_FORCE", 4.4482216152605e8, "DYNE", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, "LONG_TON_FORCE", 9.96401641818353e8, "DYNE", 1.0e-5); // Value calculated in excel

    TestUnitConversion (1.0, "MILLINEWTON", 0.001 / 1.0e-5, "DYNE", 1.0e-8);
    TestUnitConversion (1.0, "NEWTON", 1.0 / 1.0e-5, "DYNE", 1.0e-8);
    TestUnitConversion (1.0, "POUND_FORCE", 4.4482216152605e5, "DYNE", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, "SHORT_TON_FORCE", 8.896443230521e8, "DYNE", 1.0e-8); // Value calculated in excel

    TestUnitConversion (1.0e6, "DYNE", 1.0e6, "DYNE", 1.0e-8);
    TestUnitConversion (1.0e6, "KILOGRAM_FORCE", 9.80665e6 / 1.0e-5, "DYNE", 1.0e-3);
    TestUnitConversion (1.0e6, "KILONEWTON", 1000e11, "DYNE", 1.0e-6);
    TestUnitConversion (1.0e6, "KILOPOUND_FORCE", 4.4482216152605e14, "DYNE", 0.1); // Value calculated in excel
    TestUnitConversion (1.0e6, "LONG_TON_FORCE", 9.96401641818353e14, "DYNE", 10); // Value calculated in excel, large difference in expected value is due to the limits of excels precision

    TestUnitConversion (1.0e6, "MILLINEWTON", 1.0e8, "DYNE", 1.0e-7);
    TestUnitConversion (1.0e6, "NEWTON", 1.0e11, "DYNE", 1.0e-8);
    TestUnitConversion (1.0e6, "POUND_FORCE", 4.4482216152605e11, "DYNE", 1.0e-4); // Value calculated in excel
    TestUnitConversion (1.0e6, "SHORT_TON_FORCE", 8.896443230521e14, "DYNE", 1); // Value calculated in excel

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "WATT_PER_METRE", 1.0, "WATT_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0, "BTU_PER_HOUR_PER_FOOT", 9.61519259095214e-1, "WATT_PER_METRE", 1.0e-8);

    TestUnitConversion (1.0e6, "WATT_PER_METRE", 1.0e6, "WATT_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0e6, "BTU_PER_HOUR_PER_FOOT", 9.61519259095214e5, "WATT_PER_METRE", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "WATT_PER_METRE_PER_DEGREE_KELVIN", 1.0, "WATT_PER_METRE_PER_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0, "WATT_PER_METRE_PER_DEGREE_CELSIUS", 1.0, "WATT_PER_METRE_PER_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0, "BTU_INCH_PER_FOOT_SQUARED_PER_HOUR_PER_DEGREE_FAHRENHEIT", 1.44227888864283e-1, "WATT_PER_METRE_PER_DEGREE_KELVIN", 1.0e-8);

    TestUnitConversion (1.0e6, "WATT_PER_METRE_PER_DEGREE_KELVIN", 1.0e6, "WATT_PER_METRE_PER_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0e6, "WATT_PER_METRE_PER_DEGREE_CELSIUS", 1.0e6, "WATT_PER_METRE_PER_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0e6, "BTU_INCH_PER_FOOT_SQUARED_PER_HOUR_PER_DEGREE_FAHRENHEIT", 1.44227888864283e5, "WATT_PER_METRE_PER_DEGREE_KELVIN", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "FOOT_POUNDAL", 1.0, "FOOT_POUNDAL", 1.0e-8);

    TestUnitConversion (1.0, "JOULE", 2.37303604042319e1, "FOOT_POUNDAL", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, "KILOJOULE", 2.37303604042319e4, "FOOT_POUNDAL", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, "KILOWATT_HOUR", 8.54292974552351e7, "FOOT_POUNDAL", 1.0e-6); // Value calculated in excel
    TestUnitConversion (1.0, "BTU", 2.50368556292668e4, "FOOT_POUNDAL", 1e-8);
    TestUnitConversion (1.0, "GIGAJOULE", 2.37303604042319e10, "FOOT_POUNDAL", 1e-4);
    
    TestUnitConversion (1.0, "KILOBTU", 2.50368556292668e7, "FOOT_POUNDAL", 1e-6);
    TestUnitConversion (1.0, "GIGAWATT_HOUR", 8.54292974552351e13, "FOOT_POUNDAL", 1.0e1);
    TestUnitConversion (1.0, "MEGAJOULE", 2.37303604042319e7, "FOOT_POUNDAL", 1e-7);
    TestUnitConversion (1.0, "MEGAWATT_HOUR", 8.54292974552351e10, "FOOT_POUNDAL", 1.0e-3);
    TestUnitConversion (1.0, "WATT_SECOND", 2.37303604042319e1, "FOOT_POUNDAL", 1.0e-8);

    TestUnitConversion (1.0e6, "JOULE", 2.37303604042319e7, "FOOT_POUNDAL", 1.0e-7); // Value calculated in excel
    TestUnitConversion (1.0e6, "KILOJOULE", 2.37303604042319e10, "FOOT_POUNDAL", 1.0e-4); // Value calculated in excel
    TestUnitConversion (1.0e6, "KILOWATT_HOUR", 8.54292974552351e13, "FOOT_POUNDAL", 1.0); // Value calculated in excel
    TestUnitConversion (1.0e6, "BTU", 2.50368556292668e10, "FOOT_POUNDAL", 1e-4);
    TestUnitConversion (1.0e6, "GIGAJOULE", 2.37303604042319e16, "FOOT_POUNDAL", 1e2);
    
    TestUnitConversion (1.0e6, "KILOBTU", 2.50368556292668e13, "FOOT_POUNDAL", 1e-1);
    TestUnitConversion (1.0e6, "GIGAWATT_HOUR", 8.54292974552351e19, "FOOT_POUNDAL", 1.0e6);
    TestUnitConversion (1.0e6, "MEGAJOULE", 2.37303604042319e13, "FOOT_POUNDAL", 0.1);
    TestUnitConversion (1.0e6, "MEGAWATT_HOUR", 8.54292974552351e16, "FOOT_POUNDAL", 1000);
    TestUnitConversion (1.0e6, "WATT_SECOND", 2.37303604042319e7, "FOOT_POUNDAL", 1.0e-7);

    TestUnitConversion (1.0, "NEWTON_METRE", 1.0, "NEWTON_METRE", 1.0e-8);
    TestUnitConversion (1.0, "POUND_FOOT", 1.3558179483314, "NEWTON_METRE", 1.0e-8); // Value calculated in excel

    TestUnitConversion (1.0e6, "POUND_FOOT", 1.3558179483314e6, "NEWTON_METRE", 1.0e-8); // Value calculated in excel
    
    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "WATT", 1.0, "WATT", 1.0e-8);

    TestUnitConversion (1.0, "HORSEPOWER", 7.45699871582275e2, "WATT", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, "BTU_PER_HOUR", 2.93071070172222e-1, "WATT", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, "KILOBTU_PER_HOUR", 2.93071070172222e2, "WATT", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, "KILOWATT", 1000.0, "WATT", 1.0e-8);
    TestUnitConversion (1.0, "GIGAWATT", 1.0e9, "WATT", 1.0e-8);
    TestUnitConversion (1.0, "MEGAWATT", 1.0e6, "WATT", 1.0e-8);

    TestUnitConversion (1.0e6, "HORSEPOWER", 7.45699871582275e8, "WATT", 1.0e-5); // Value calculated in excel
    TestUnitConversion (1.0e6, "BTU_PER_HOUR", 2.93071070172222e5, "WATT", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0e6, "KILOBTU_PER_HOUR", 2.93071070172222e8, "WATT", 1.0e-6); // Value calculated in excel
    TestUnitConversion (1.0e6, "KILOWATT", 1.0e9, "WATT", 1.0e-8);
    TestUnitConversion (1.0e6, "GIGAWATT", 1.0e15, "WATT", 1.0e-8);
    TestUnitConversion (1.0e6, "MEGAWATT", 1.0e12, "WATT", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "GIGAJOULE_PER_MONTH", 1.0, "GIGAJOULE_PER_MONTH", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0e6, "BTU_PER_MONTH", 1.05505585262, "GIGAJOULE_PER_MONTH", 1.0e-8); // Value calculated in excel

    TestUnitConversion (1.0e6, "GIGAJOULE_PER_MONTH", 1.0e6, "GIGAJOULE_PER_MONTH", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0e12, "BTU_PER_MONTH", 1.05505585262e6, "GIGAJOULE_PER_MONTH", 1.0e-8); // Value calculated in excel
   
    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "POUND_FORCE_FOOT_SQUARED", 4.1325331065141e-1, "NEWTON_METRE_SQUARED", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, "NEWTON_METRE_SQUARED", 2.41982332440048, "POUND_FORCE_FOOT_SQUARED", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, "KILOGRAM_FORCE_METRE_SQUARED", 23.7303604570562, "POUND_FORCE_FOOT_SQUARED", 1.0e-8); // Value calculated in excel

    TestUnitConversion (1.0e6, "POUND_FORCE_FOOT_SQUARED", 4.1325331065141e5, "NEWTON_METRE_SQUARED", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0e6, "NEWTON_METRE_SQUARED", 2.41982332440048e6, "POUND_FORCE_FOOT_SQUARED", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0e6, "KILOGRAM_FORCE_METRE_SQUARED", 2.37303604570562e7, "POUND_FORCE_FOOT_SQUARED", 1.0e-7); // Value calculated in excel

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    // Gauge offsets are 101.325 kPa converted into working unit, since the above website does not support Gauge units
    // the converted gauge offset was applied before the conversion was done.
    TestUnitConversion (1.0, "ATMOSPHERE", 1.0, "ATMOSPHERE", 1.0e-8);
    TestUnitConversion (1.0, "BAR", 1.0e5 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0, "BAR_GAUGE", ((1 - 1.01325) * 1.0e5) / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0, "BARYE", 0.1 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Bayre == 0.1 pascal from wikipedia, other values from NIST
    TestUnitConversion (1.0, "FOOT_OF_H2O_CONVENTIONAL", 2.989067e3 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, could not be fully double checked on web because site does not support ftH2O_Conv

    TestUnitConversion (1.0, "INCH_OF_H2O_AT_32_FAHRENHEIT", 249.1083 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // PASCAL <-> INCH_OF_H2O_AT_32_FAHRENHEIT is from PDS, I have no way of verifying this number, but it is within the range you would expect.
    TestUnitConversion (1.0, "INCH_OF_H2O_AT_39_2_FAHRENHEIT", 2.49082e2 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, could not be fully double checked on web because site does not support inH2O_39.2
    TestUnitConversion (1.0, "INCH_OF_H2O_AT_60_FAHRENHEIT", 2.4884e2 / 1.01325e5, "ATMOSPHERE", 1.0e-8);  // Value from NIST conversion factors, could not be fully double checked on web because site does not support inH2O_60
    TestUnitConversion (1.0, "INCH_OF_HG_AT_32_FAHRENHEIT", 3.38638e3 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, could not be fully double checked on web because site does not support inHg_32
    TestUnitConversion (1.0, "INCH_OF_HG_CONVENTIONAL", 3.386389e3 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, double checked at http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx?from=inch-of-mercury-conventional

    TestUnitConversion (1.0, "INCH_OF_HG_AT_60_FAHRENHEIT", 3.37685e3 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, double checked at http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx?from=inch-of-mercury-60
    TestUnitConversion (1.0, "KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED", 9.80665e4 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0, "KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED_GAUGE", ((1 - 101.325 / 98.0665) * 9.80665e4) / 1.01325e5, "ATMOSPHERE", 1.0e-8);
    TestUnitConversion (1.0, "KILOGRAM_FORCE_PER_METRE_SQUARED", 9.80665 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0, "KILOPASCAL", 1000 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors

    TestUnitConversion (1.0, "KILOPASCAL_GAUGE", ((1 - 101.325) * 1000) / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0, "MEGAPASCAL", 1000000 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0, "MEGAPASCAL_GAUGE", ((1 - 101.325 / 1000) * 1000000) / 1.01325e5, "ATMOSPHERE", 1.0e-8);
    TestUnitConversion (1.0, "METRE_OF_H2O_CONVENTIONAL", 9806.65 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // value is 1000 * mmH2O Conv from NIST, could not be fully double checked on web because site does not support mHg Conv
    TestUnitConversion (1.0, "MILLIMETRE_OF_H2O_CONVENTIONAL", 9.80665 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, could not be fully double checked on web because site does not support mmH2O Conv

    TestUnitConversion (1.0, "MILLIMETRE_OF_HG_AT_32_FAHRENHEIT", (3.38638e3 / 25.4) / 1.01325e5, "ATMOSPHERE", 1.0e-8); // value is inHg@32/25.4 inHg@32 from NIST, could not be fully double checked on web because site does not support mmHg 32
    TestUnitConversion (1.0, "NEWTON_PER_METRE_SQUARED", 1 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // is pascal, so used ATM <-> PASCAL NIST conversion factor
    TestUnitConversion (1.0, "PASCAL", 1 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0, "NEWTON_PER_METRE_SQUARED", 1.0, "PASCAL", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0, "POUND_FORCE_PER_FOOT_SQUARED", 47.88026 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors

    TestUnitConversion (1.0, "POUND_FORCE_PER_INCH_SQUARED", 6.894757e3 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0, "POUND_FORCE_PER_INCH_SQUARED_GAUGE", ((1 - 101.325 / 6.894757) * 6.894757e3) / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0, "TORR", 1.333224e2 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0, "MILLIBAR", 1.0e2 / 1.01325e5, "ATMOSPHERE", 1e-8);
    TestUnitConversion (1.0, "HECTOPASCAL", 100.0 / 1.01325e5, "ATMOSPHERE", 1e-8);


    TestUnitConversion (1000000.0, "ATMOSPHERE", 1000000.0, "ATMOSPHERE", 1.0e-8);
    TestUnitConversion (1000000.0, "BAR", 1000000.0 * 1.0e5 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1000000.0, "BAR_GAUGE", ((1000000.0 - 1.01325) * 1.0e5) / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1000000.0, "BARYE", 100000 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Bayre == 0.1 pascal from wikipedia, other values from NIST
    TestUnitConversion (1000000.0, "FOOT_OF_H2O_CONVENTIONAL", 1000000.0 * 2.989067e3 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, could not be fully double checked on web because site does not support ftH2O_Conv

    TestUnitConversion (1000000.0, "INCH_OF_H2O_AT_32_FAHRENHEIT", 1000000.0 * 249.1083 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // PASCAL <-> INCH_OF_H2O_AT_32_FAHRENHEIT is from PDS, I have no way of verifying this number, but it is within the range you would expect.
    TestUnitConversion (1000000.0, "INCH_OF_H2O_AT_39_2_FAHRENHEIT", 1000000.0 * 2.49082e2 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, could not be fully double checked on web because site does not support inH2O_39.2
    TestUnitConversion (1000000.0, "INCH_OF_H2O_AT_60_FAHRENHEIT", 1000000.0 * 2.4884e2 / 1.01325e5, "ATMOSPHERE", 1.0e-8);  // Value from NIST conversion factors, could not be fully double checked on web because site does not support inH2O_60
    TestUnitConversion (1000000.0, "INCH_OF_HG_AT_32_FAHRENHEIT", 1000000.0 * 3.38638e3 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, could not be fully double checked on web because site does not support inHg_32
    TestUnitConversion (1000000.0, "INCH_OF_HG_CONVENTIONAL", 1000000.0 * 3.386389e3 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, could not be fully double checked on web because site does not support inHg_Conv

    TestUnitConversion (1000000.0, "INCH_OF_HG_AT_60_FAHRENHEIT", 1000000.0 * 3.37685e3 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, could not be fully double checked on web because site does not support inHg_60
    TestUnitConversion (1000000.0, "KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED", 1000000.0 * 9.80665e4 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1000000.0, "KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED_GAUGE", ((1000000 - 101.325 / 98.0665) * 9.80665e4) / 1.01325e5, "ATMOSPHERE", 1.0e-8);
    TestUnitConversion (1000000.0, "KILOGRAM_FORCE_PER_METRE_SQUARED", 1000000.0 * 9.80665 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1000000.0, "KILOPASCAL", 1000000000 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors

    TestUnitConversion (1000000.0, "KILOPASCAL_GAUGE", ((1000000 - 101.325) * 1000) / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1000000.0, "MEGAPASCAL", 1000000000000 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1000000.0, "MEGAPASCAL_GAUGE", ((1000000 - 101.325 / 1000) * 1000000) / 1.01325e5, "ATMOSPHERE", 1.0e-8);
    TestUnitConversion (1000000.0, "METRE_OF_H2O_CONVENTIONAL", 1000000.0 * 9806.65 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // value is 1000 * mmHg Conv from NIST, could not be fully double checked on web because site does not support mHg Conv
    TestUnitConversion (1000000.0, "MILLIMETRE_OF_H2O_CONVENTIONAL", 1000000.0 * 9.80665 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, could not be fully double checked on web because site does not support mmH2O Conv

    TestUnitConversion (1000000.0, "MILLIMETRE_OF_HG_AT_32_FAHRENHEIT", (1000000.0 * 133.322) / 1.01325e5, "ATMOSPHERE", 1.0e-8); // value is cmHg@0c/10 cmHg@0c from NIST, could not be fully double checked on web because site does not support mmHg 32
    TestUnitConversion (1000000.0, "NEWTON_PER_METRE_SQUARED", 1000000 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // is pascal, so used ATM <-> PASCAL NIST conversion factor
    TestUnitConversion (1000000.0, "PASCAL", 1000000 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1000000.0, "NEWTON_PER_METRE_SQUARED", 1000000.0, "PASCAL", 1.0e-8); // Value from NIST conversion factors
    double psf_per_Pascal = (4.4482216152605 / (0.3048 * 0.3048)); // 0.3048 is meters per foot, 4.4482216152605 is newtons per pound force
    TestUnitConversion (1000000.0, "POUND_FORCE_PER_FOOT_SQUARED", 1000000.0 * psf_per_Pascal / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, can't use the stated value for lbf/ft^2 because it uses 4.448222 Newtons per lbf, we use the more precise value of 4.4482216152605

    double psi_per_Pascal = (4.4482216152605 / (0.0254 * 0.0254));// 0.0254 is meters per inch, 4.4482216152605 is newtons per pound force
    TestUnitConversion (1000000.0, "POUND_FORCE_PER_INCH_SQUARED", 1000000.0 * psi_per_Pascal / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1000000.0, "POUND_FORCE_PER_INCH_SQUARED_GAUGE", ((1000000 - 101.325 / (psi_per_Pascal / 1000)) * psi_per_Pascal) / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, can't use the stated value for lbf/in^2 because it uses 4.448222 newtons per lbf, we sue the more precise value of 4.4482216152605
    TestUnitConversion (1000000.0, "TORR", 1000000.0 * 1.333224e2 / 1.01325e5, "ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0e6, "MILLIBAR", 1.0e8 / 1.01325e5, "ATMOSPHERE", 1e-8);
    TestUnitConversion (1.0e6, "HECTOPASCAL", 100.0e6 / 1.01325e5, "ATMOSPHERE", 1e-8);

    TestUnitConversion (1.0, "KILOWATT_HOUR_PER_METRE_CUBED", 1.0, "KILOWATT_HOUR_PER_METRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, "KILOWATT_HOUR_PER_MILLION_GALLON", 2.64172052358148e-4, "KILOWATT_HOUR_PER_METRE_CUBED", 1.0e-8); // Not found in NIST doc, only confermed via web
    TestUnitConversion (1.0, "KILOWATT_HOUR_PER_MILLION_LITRE", 0.001, "KILOWATT_HOUR_PER_METRE_CUBED", 1.0e-8); // Not found in NIST doc, only confermed via web
    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "JOULE_PER_METRE_CUBED", 1.0 / (1000.0 * 3600.0), "KILOWATT_HOUR_PER_METRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, "KILOJOULE_PER_METRE_CUBED", 1.0 / 3600, "KILOWATT_HOUR_PER_METRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, "KILOWATT_HOUR_PER_FOOT_CUBED", 1.0 / pow (0.3048, 3.0), "KILOWATT_HOUR_PER_METRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, "MEGAJOULE_PER_METRE_CUBED", 1.0 / 3.6, "KILOWATT_HOUR_PER_METRE_CUBED", 1.0e-8);

    TestUnitConversion (1.0e6, "JOULE_PER_METRE_CUBED", 1.0e6 / (1000 * 3600), "KILOWATT_HOUR_PER_METRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, "KILOJOULE_PER_METRE_CUBED", 1.0e6 / 3600, "KILOWATT_HOUR_PER_METRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, "KILOWATT_HOUR_PER_FOOT_CUBED", 1.0e6 / pow (0.3048, 3.0), "KILOWATT_HOUR_PER_METRE_CUBED", 1.0e-7);
    TestUnitConversion (1.0e6, "MEGAJOULE_PER_METRE_CUBED", 1.0e6 / 3.6, "KILOWATT_HOUR_PER_METRE_CUBED", 1.0e-8);

    TestUnitConversion (1.0, "WATT_PER_METRE_CUBED", 1.0, "WATT_PER_METRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, "BTU_PER_HOUR_PER_FOOT_CUBED", 1.0349707168842e1, "WATT_PER_METRE_CUBED", 1.0e-8);

    TestUnitConversion (1.0e6, "WATT_PER_METRE_CUBED", 1.0e6, "WATT_PER_METRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, "BTU_PER_HOUR_PER_FOOT_CUBED", 1.0349707168842e7, "WATT_PER_METRE_CUBED", 1.0e-7);

    TestUnitConversion (1.0, "POUND_PER_ACRE", 1.12085115619445, "KILOGRAM_PER_HECTARE", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, "KILOGRAM_PER_HECTARE", 8.92179121619709e-1, "POUND_PER_ACRE", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, "KILOGRAM_PER_METRE_SQUARED", 8921.79121619709, "POUND_PER_ACRE", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, "GRAM_PER_METRE_SQUARED", 8.92179121619701, "POUND_PER_ACRE", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, "POUND_MASS_PER_FOOT_SQUARED", 4.356e4, "POUND_PER_ACRE", 1.0e-8); // Value calculated in excel

    TestUnitConversion (1.0e6, "POUND_PER_ACRE", 1.12085115619445e6, "KILOGRAM_PER_HECTARE", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0e6, "KILOGRAM_PER_HECTARE", 8.92179121619709e5, "POUND_PER_ACRE", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0e6, "KILOGRAM_PER_METRE_SQUARED", 8.92179121619709e9, "POUND_PER_ACRE", 1.0e-4); // Value calculated in excel
    TestUnitConversion (1.0e6, "GRAM_PER_METRE_SQUARED", 8.92179121619701e6, "POUND_PER_ACRE", 1.0e-7); // Value calculated in excel
    TestUnitConversion (1.0e6, "POUND_MASS_PER_FOOT_SQUARED", 4.356e10, "POUND_PER_ACRE", 1.0e-8); // Value calculated in excel

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "KILONEWTON_PER_FOOT_CUBED", 3.53146667214886e1, "KILONEWTON_PER_METRE_CUBED", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, "KILONEWTON_PER_METRE_CUBED", 2.8316846592e-2, "KILONEWTON_PER_FOOT_CUBED", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0e2, "NEWTON_PER_METRE_CUBED", 2.8316846592e-3, "KILONEWTON_PER_FOOT_CUBED", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, "POUND_FORCE_PER_FOOT_CUBED", 4.4482216152605e-3, "KILONEWTON_PER_FOOT_CUBED", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, "POUND_FORCE_PER_INCH_SQUARED_PER_FOOT", 6.40543912597512e-1, "KILONEWTON_PER_FOOT_CUBED", 1.0e-8); // Value calculated in excel

    TestUnitConversion (1.0e6, "KILONEWTON_PER_FOOT_CUBED", 3.53146667214886e7, "KILONEWTON_PER_METRE_CUBED", 1.0e-7); // Value calculated in excel
    TestUnitConversion (1.0e6, "KILONEWTON_PER_METRE_CUBED", 2.8316846592e4, "KILONEWTON_PER_FOOT_CUBED", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0e8, "NEWTON_PER_METRE_CUBED", 2.8316846592e3, "KILONEWTON_PER_FOOT_CUBED", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0e6, "POUND_FORCE_PER_FOOT_CUBED", 4.4482216152605e3, "KILONEWTON_PER_FOOT_CUBED", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0e6, "POUND_FORCE_PER_INCH_SQUARED_PER_FOOT", 6.40543912597512e5, "KILONEWTON_PER_FOOT_CUBED", 1.0e-8); // Value calculated in excel

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "PASCAL_PER_METRE", 1.0, "PASCAL_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0, "BAR_PER_KILOMETRE", 100, "PASCAL_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0e6, "BAR_PER_KILOMETRE", 1.0e8, "PASCAL_PER_METRE", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "GRAM_PER_CENTIMETRE_CUBED", 1.0, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, "KILOGRAM_PER_DECIMETRE_CUBED", 1.0, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, "KILOGRAM_PER_LITRE", 1.0, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, "KILOGRAM_PER_METRE_CUBED", 0.001, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, "KIP_PER_FOOT_CUBED", 1.60184633739601e1, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);

    TestUnitConversion (1.0, "MICROGRAM_PER_LITRE", 1e-9, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, "MILLIGRAM_PER_LITRE", 1e-6, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, "POUND_PER_FOOT_CUBED", 1.60184633739601e-2, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, "POUND_PER_GALLON", 1.19826427316897e-1, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, "POUND_PER_IMPERIAL_GALLON", 9.97763726631017e-2, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);

    TestUnitConversion (1.0, "POUND_PER_INCH_CUBED", 2.76799047102031e1, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, "POUND_PER_MILLION_GALLON", 1.19826427316897e-7, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, "SHORT_TON_PER_FOOT_CUBED", 3.20369267479203e1, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, "SLUG_PER_FOOT_CUBED", 5.15378818393195e-1, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);


    TestUnitConversion (1.0e6, "GRAM_PER_CENTIMETRE_CUBED", 1.0e6, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, "KILOGRAM_PER_DECIMETRE_CUBED", 1.0e6, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, "KILOGRAM_PER_LITRE", 1.0e6, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, "KILOGRAM_PER_METRE_CUBED", 1000, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, "KIP_PER_FOOT_CUBED", 1.60184633739601e7, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-7);

    TestUnitConversion (1.0e6, "MICROGRAM_PER_LITRE", 1e-3, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, "MILLIGRAM_PER_LITRE", 1.0, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, "POUND_PER_FOOT_CUBED", 1.60184633739601e4, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, "POUND_PER_GALLON", 1.19826427316897e5, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, "POUND_PER_IMPERIAL_GALLON", 9.97763726631017e4, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);

    TestUnitConversion (1.0e6, "POUND_PER_INCH_CUBED", 2.76799047102031e7, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-7);
    TestUnitConversion (1.0e6, "POUND_PER_MILLION_GALLON", 1.19826427316897e-1, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, "SHORT_TON_PER_FOOT_CUBED", 3.20369267479203e7, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-7);
    TestUnitConversion (1.0e6, "SLUG_PER_FOOT_CUBED", 5.15378818393195e5, "GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "GRAM_PER_SECOND", 1.0, "GRAM_PER_SECOND", 1.0e-8);

    TestUnitConversion (1.0, "GRAM_PER_SECOND", 3600 * 24, "GRAM_PER_DAY", 1.0e-8);
    TestUnitConversion (1.0, "GRAM_PER_SECOND", 3600, "GRAM_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, "GRAM_PER_SECOND", 60, "GRAM_PER_MINUTE", 1.0e-8);
    TestUnitConversion (1.0, "GRAM_PER_SECOND", 3.6 * 24, "KILOGRAM_PER_DAY", 1.0e-8);
    TestUnitConversion (1.0, "GRAM_PER_SECOND", 3.6, "KILOGRAM_PER_HOUR", 1.0e-8);

    TestUnitConversion (1.0, "GRAM_PER_SECOND", 6.0e-2, "KILOGRAM_PER_MINUTE", 1.0e-8);
    TestUnitConversion (1.0, "GRAM_PER_SECOND", 1.0e-3, "KILOGRAM_PER_SECOND", 1.0e-8);
    TestUnitConversion (1.0, "GRAM_PER_SECOND", 1.0e6 * 3600 * 24, "MICROGRAM_PER_DAY", 1.0e-8);
    TestUnitConversion (1.0, "GRAM_PER_SECOND", 1.0e6 * 3600, "MICROGRAM_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, "GRAM_PER_SECOND", 1.0e6 * 60, "MICROGRAM_PER_MINUTE", 1.0e-8);

    TestUnitConversion (1.0, "GRAM_PER_SECOND", 1.0e6, "MICROGRAM_PER_SECOND", 1.0e-8);
    TestUnitConversion (1.0, "GRAM_PER_SECOND", 1.0e3 * 3600 * 24, "MILLIGRAM_PER_DAY", 1.0e-8);
    TestUnitConversion (1.0, "GRAM_PER_SECOND", 1.0e3 * 3600, "MILLIGRAM_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, "GRAM_PER_SECOND", 1.0e3 * 60, "MILLIGRAM_PER_MINUTE", 1.0e-8);
    TestUnitConversion (1.0, "GRAM_PER_SECOND", 1.0e3, "MILLIGRAM_PER_SECOND", 1.0e-8);

    double gramPerPound = 453.59237; // From NIST doc kg per lb = 4.5359237e-1
    TestUnitConversion (1.0, "GRAM_PER_SECOND", (3600 * 24) / gramPerPound, "POUND_PER_DAY", 1.0e-8);
    TestUnitConversion (1.0, "GRAM_PER_SECOND", 3600 / gramPerPound, "POUND_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, "GRAM_PER_SECOND", 60 / gramPerPound, "POUND_PER_MINUTE", 1.0e-8);
    TestUnitConversion (1.0, "GRAM_PER_SECOND", 1 / gramPerPound, "POUND_PER_SECOND", 1.0e-8);


    TestUnitConversion (1.0e6, "GRAM_PER_SECOND", 3600 * 24 * 1.0e6, "GRAM_PER_DAY", 1.0e-8);
    TestUnitConversion (1.0e6, "GRAM_PER_SECOND", 3600 * 1.0e6, "GRAM_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, "GRAM_PER_SECOND", 60 * 1.0e6, "GRAM_PER_MINUTE", 1.0e-8);
    TestUnitConversion (1.0e6, "GRAM_PER_SECOND", 3.6e6 * 24, "KILOGRAM_PER_DAY", 1.0e-8);
    TestUnitConversion (1.0e6, "GRAM_PER_SECOND", 3.6e6, "KILOGRAM_PER_HOUR", 1.0e-8);

    TestUnitConversion (1.0e6, "GRAM_PER_SECOND", 6.0e4, "KILOGRAM_PER_MINUTE", 1.0e-8);
    TestUnitConversion (1.0e6, "GRAM_PER_SECOND", 1.0e3, "KILOGRAM_PER_SECOND", 1.0e-8);
    TestUnitConversion (1.0e6, "GRAM_PER_SECOND", 1.0e12 * 3600 * 24, "MICROGRAM_PER_DAY", 1.0e-8);
    TestUnitConversion (1.0e6, "GRAM_PER_SECOND", 1.0e12 * 3600, "MICROGRAM_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, "GRAM_PER_SECOND", 1.0e12 * 60, "MICROGRAM_PER_MINUTE", 1.0e-8);

    TestUnitConversion (1.0e6, "GRAM_PER_SECOND", 1.0e12, "MICROGRAM_PER_SECOND", 1.0e-8);
    TestUnitConversion (1.0e6, "GRAM_PER_SECOND", 1.0e9 * 3600 * 24, "MILLIGRAM_PER_DAY", 1.0e-8);
    TestUnitConversion (1.0e6, "GRAM_PER_SECOND", 1.0e9 * 3600, "MILLIGRAM_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, "GRAM_PER_SECOND", 1.0e9 * 60, "MILLIGRAM_PER_MINUTE", 1.0e-8);
    TestUnitConversion (1.0e6, "GRAM_PER_SECOND", 1.0e9, "MILLIGRAM_PER_SECOND", 1.0e-8);

    TestUnitConversion (1.0e6, "GRAM_PER_SECOND", (3600 * 24 * 1.0e6) / gramPerPound, "POUND_PER_DAY", 1.0e-7);
    TestUnitConversion (1.0e6, "GRAM_PER_SECOND", (3600 * 1.0e6) / gramPerPound, "POUND_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, "GRAM_PER_SECOND", (60 * 1.0e6) / gramPerPound, "POUND_PER_MINUTE", 1.0e-8);
    TestUnitConversion (1.0e6, "GRAM_PER_SECOND", 1e6 / gramPerPound, "POUND_PER_SECOND", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    // Some expected values are from NIST, others are calculated, calculated values are noted
    TestUnitConversion (1.0, "NEWTON_PER_METRE", 1.0, "NEWTON_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0, "NEWTON_PER_MILLIMETRE", 1000, "NEWTON_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0, "POUND_FORCE_PER_INCH", 175.126835246476, "NEWTON_PER_METRE", 1.0e-5);

    TestUnitConversion (1.0e6, "NEWTON_PER_METRE", 1.0e6, "NEWTON_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0e6, "NEWTON_PER_MILLIMETRE", 1.0e9, "NEWTON_PER_METRE", 1.0e-6);
    TestUnitConversion (1.0e6, "POUND_FORCE_PER_INCH", 1.75126835246476e8, "NEWTON_PER_METRE", 1.0e-5);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "WATT_PER_METRE_SQUARED", 1.0, "WATT_PER_METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, "BTU_PER_HOUR_PER_FOOT_SQUARED", 3.15459074506304, "WATT_PER_METRE_SQUARED", 1.0e-8);

    TestUnitConversion (1.0e6, "WATT_PER_METRE_SQUARED", 1.0e6, "WATT_PER_METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0e6, "BTU_PER_HOUR_PER_FOOT_SQUARED", 3.15459074506304e6, "WATT_PER_METRE_SQUARED", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_KELVIN", 1.0, "WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0, "WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_CELSIUS", 1.0, "WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0, "BTU_PER_FOOT_SQUARED_PER_HOUR_PER_DELTA_DEGREE_FAHRENHEIT", 5.67826334111348, "WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_KELVIN", 1.0e-8);

    TestUnitConversion (1.0e6, "WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_KELVIN", 1.0e6, "WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0e6, "WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_CELSIUS", 1.0e6, "WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0e6, "BTU_PER_FOOT_SQUARED_PER_HOUR_PER_DELTA_DEGREE_FAHRENHEIT", 5.67826334111348e6, "WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_KELVIN", 1.0e-8);

    TestUnitConversion (1.0e6, "CAPITA", 1.0e6, "PERSON", 1.0e-8);
    TestUnitConversion (1.0e6, "CUSTOMER", 1.0e6, "PERSON", 1.0e-8);
    TestUnitConversion (1.0e6, "EMPLOYEE", 1.0e6, "PERSON", 1.0e-8);
    TestUnitConversion (1.0e6, "GUEST", 1.0e6, "PERSON", 1.0e-8);
    TestUnitConversion (1.0e6, "HUNDRED_CAPITA", 1.0e8, "PERSON", 1.0e-8);
    TestUnitConversion (1.0e6, "THOUSAND_CAPITA", 1.0e9, "PERSON", 1.0e-8);
    TestUnitConversion (1.0e6, "PASSENGER", 1.0e6, "PERSON", 1.0e-8);
    TestUnitConversion (1.0e6, "RESIDENT", 1.0e6, "PERSON", 1.0e-8);
    TestUnitConversion (1.0e6, "STUDENT", 1.0e6, "PERSON", 1.0e-8);

    TestUnitConversion (100.0, "PERCENT_PERCENT", 1.0, "UNITLESS_PERCENT", 1.0e-8);
    TestUnitConversion (0.5, "UNITLESS_PERCENT", 50.0, "PERCENT_PERCENT", 1.0e-8);

    TestUnitConversion (50.0, "PERCENT_SLOPE", 5280.0 / 2, "FOOT_PER_MILE", 1.0e-8);
    TestUnitConversion (1320, "FOOT_PER_MILE", 25.0, "PERCENT_SLOPE", 1.0e-8);
    TestUnitConversion (.45, "VERTICAL_PER_HORIZONTAL", 45.0, "PERCENT_SLOPE", 1.0e-8);

    TestUnitConversion (5, "METRE_VERTICAL_PER_METRE_HORIZONTAL", 5, "FOOT_VERTICAL_PER_FOOT_HORIZONTAL", 1.0e-8);
    TestUnitConversion (0.1, "FOOT_VERTICAL_PER_FOOT_HORIZONTAL", 0.1, "FOOT_PER_FOOT", 1.0e-8);
    TestUnitConversion (0.1, "FOOT_HORIZONTAL_PER_FOOT_VERTICAL", 10.0, "FOOT_PER_FOOT", 1.0e-8);
    TestUnitConversion (0.1, "METRE_PER_METRE", 0.1, "FOOT_PER_FOOT", 1.0e-8);
    TestUnitConversion (0.1, "METRE_VERTICAL_PER_METRE_HORIZONTAL", 0.1, "FOOT_PER_FOOT", 1.0e-8);
    TestUnitConversion (0.1, "METRE_HORIZONTAL_PER_METRE_VERTICAL", 10.0, "FOOT_PER_FOOT", 1.0e-8);
    TestUnitConversion (0.1, "VERTICAL_PER_HORIZONTAL", 0.1, "FOOT_PER_FOOT", 1.0e-8);
    TestUnitConversion (0.1, "HORIZONTAL_PER_VERTICAL", 10.0, "FOOT_PER_FOOT", 1.0e-8);
    TestUnitConversion (0.1, "FOOT_PER_FOOT", 100.0, "MILLIMETRE_VERTICAL_PER_METRE_HORIZONTAL", 1.0e-8);
    TestUnitConversion (0.1, "FOOT_PER_FOOT", 10.0, "CENTIMETRE_PER_METRE", 1.0e-8);
    TestUnitConversion (2.0, "FOOT_PER_FOOT", 0.5, "HORIZONTAL_PER_VERTICAL", 1.0e-8);
    TestUnitConversion (2.0, "FOOT_PER_FOOT", 2.0, "VERTICAL_PER_HORIZONTAL", 1.0e-8);
    TestUnitConversion (1000.0, "FOOT_PER_1000_FOOT", 1.0, "VERTICAL_PER_HORIZONTAL", 1.0e-8);
    TestUnitConversion (12.0, "INCH_PER_FOOT", 0.083333333333333333333333333333333, "FOOT_PER_INCH", 1.0e-8);
    TestUnitConversion (1.0, "VERTICAL_PER_HORIZONTAL", 0.083333333333333333333333333333333, "FOOT_PER_INCH", 1.0e-8);
    TestUnitConversion (1.0, "FOOT_PER_INCH", 12.0, "VERTICAL_PER_HORIZONTAL", 1.0e-8);
    TestUnitConversion (5280, "FOOT_PER_MILE", 1.0, "VERTICAL_PER_HORIZONTAL", 1.0e-8);
    TestUnitConversion (12.0, "INCH_PER_FOOT", 1.0, "VERTICAL_PER_HORIZONTAL", 1.0e-8);
    TestUnitConversion (1.0, "METRE_PER_CENTIMETRE", 100.0, "VERTICAL_PER_HORIZONTAL", 1.0e-8);
    TestUnitConversion (1000.0, "METRE_PER_KILOMETRE", 1.0, "VERTICAL_PER_HORIZONTAL", 1.0e-8);
    TestUnitConversion (1000.0, "MILLIMETRE_PER_METRE", 1.0, "VERTICAL_PER_HORIZONTAL", 1.0e-8);
    TestUnitConversion (10.0, "ONE_OVER_SLOPE", 10.0, "HORIZONTAL_PER_VERTICAL", 1.0e-8);
    TestUnitConversion (33.0, "ONE_OVER_SLOPE", 1.0 / 33.0, "VERTICAL_PER_HORIZONTAL", 1.0e-8);
    
    TestUnitConversion (5.0, "PARTS_PER_MILLION", 5000.0, "PARTS_PER_BILLION", 1.0e-8);
    TestUnitConversion (2600.0, "PARTS_PER_BILLION", 2.6, "PARTS_PER_MILLION", 1.0e-8);

    TestUnitConversion (1.0, "MILLIMETRE_SQUARED_PER_METRE_SQUARED", 1.0, "MILLIMETRE_SQUARED_PER_METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, "INCH_SQUARED_PER_FOOT_SQUARED", 6944.4444444444443, "MILLIMETRE_SQUARED_PER_METRE_SQUARED", 1.0e-8);
    TestUnitConversion (7e9, "MILLIMETRE_SQUARED_PER_METRE_SQUARED", 1.008e+6, "INCH_SQUARED_PER_FOOT_SQUARED", 1.0e-8);

    TestUnitConversion (1.0, "KILOGRAM_PER_KILOGRAM", 1.0, "KILOGRAM_PER_KILOGRAM", 1.0e-8);
    TestUnitConversion (1.0, "GRAIN_MASS_PER_POUND_MASS", 1.0 / 7000.0, "KILOGRAM_PER_KILOGRAM", 1.0e-8);
    TestUnitConversion (7e9, "GRAIN_MASS_PER_POUND_MASS", 1.0e6, "KILOGRAM_PER_KILOGRAM", 1.0e-8);
    
    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    // Some expected values are from NIST, others are calculated, calculated values are noted
    TestUnitConversion (1.0, "RECIPROCAL_DELTA_DEGREE_CELSIUS", 5.0 / 9.0, "RECIPROCAL_DELTA_DEGREE_FAHRENHEIT", 1.0e-8);
    TestUnitConversion (1.0, "RECIPROCAL_DELTA_DEGREE_KELVIN", 5.0 / 9.0, "RECIPROCAL_DELTA_DEGREE_FAHRENHEIT", 1.0e-8);
    TestUnitConversion (1.0, "RECIPROCAL_DELTA_DEGREE_FAHRENHEIT", 9.0 / 5.0, "RECIPROCAL_DELTA_DEGREE_CELSIUS", 1.0e-8);
    TestUnitConversion (1.0, "RECIPROCAL_DELTA_DEGREE_RANKINE", 9.0 / 5.0, "RECIPROCAL_DELTA_DEGREE_CELSIUS", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    // Some expected values are from NIST, others are calculated, calculated values are noted
    TestUnitConversion (1.0, "ONE_PER_FOOT", 1.0 / 0.3048, "ONE_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0, "ONE_PER_METRE", 0.3048, "ONE_PER_FOOT", 1.0e-8);
    TestUnitConversion (1.0, "ONE_PER_THOUSAND_FOOT", 1.0 / 1000.0, "ONE_PER_FOOT", 1.0e-8);
    TestUnitConversion (1.0, "ONE_PER_KILOMETRE", 3.048e-4, "ONE_PER_FOOT", 1.0e-8);
    TestUnitConversion (1.0, "ONE_PER_MILE", 1.0 / 5280.0, "ONE_PER_FOOT", 1.0e-8);
    TestUnitConversion (1.0, "ONE_PER_MILLIMETRE", 304.8, "ONE_PER_FOOT", 1.0e-8);

    TestUnitConversion (1.0e6, "ONE_PER_FOOT", 1.0e6 / 0.3048, "ONE_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0e6, "ONE_PER_METRE", 3.048e5, "ONE_PER_FOOT", 1.0e-8);
    TestUnitConversion (1.0e6, "ONE_PER_THOUSAND_FOOT", 1.0e6 / 1000.0, "ONE_PER_FOOT", 1.0e-8);
    TestUnitConversion (1.0e6, "ONE_PER_KILOMETRE", 3.048e2, "ONE_PER_FOOT", 1.0e-8);
    TestUnitConversion (1.0e6, "ONE_PER_MILE", 1.0e6 / 5280.0, "ONE_PER_FOOT", 1.0e-8);
    TestUnitConversion (1.0e6, "ONE_PER_MILLIMETRE", 3.048e8, "ONE_PER_FOOT", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    // All values calculated in excel
    TestUnitConversion (1.0, "PERSON_PER_FOOT_SQUARED", 4.356e4, "PERSON_PER_ACRE", 1.0e-8);
    TestUnitConversion (1.0e6, "PERSON_PER_ACRE", 2.47105381467165e6, "PERSON_PER_HECTARE", 1.0e-8);
    TestUnitConversion (1.0e6, "PERSON_PER_KILOMETRE_SQUARED", 4.0468564224e3, "PERSON_PER_ACRE", 1.0e-8);
    TestUnitConversion (1.0, "PERSON_PER_METRE_SQUARED", 4.0468564224e3, "PERSON_PER_ACRE", 1.0e-8);
    TestUnitConversion (1.0e6, "PERSON_PER_MILE_SQUARED", 1.5625e3, "PERSON_PER_ACRE", 1.0e-8);

    TestUnitConversion (1.0, "ONE_PER_METRE_SQUARED", 1.0, "ONE_PER_METRE_SQUARED", 1e-8);

    TestUnitConversion (1.0, "ONE_PER_FOOT_SQUARED", 1.07639104167097e1, "ONE_PER_METRE_SQUARED", 1e-8);
    TestUnitConversion (1.0e6, "ONE_PER_FOOT_SQUARED", 1.07639104167097e7, "ONE_PER_METRE_SQUARED", 1e-7);
    
    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    // All values calculated in excel
    TestUnitConversion (1.0e6, "ONE_PER_ACRE_FOOT", 2.29568411386593e1, "ONE_PER_FOOT_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, "ONE_PER_ACRE_INCH", 2.75482093663912e2, "ONE_PER_FOOT_CUBED", 1.0e-8);
    TestUnitConversion (1.0, "ONE_PER_CENTIMETRE_CUBED", 2.8316846592e4, "ONE_PER_FOOT_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, "ONE_PER_FOOT_CUBED", 5.78703703703704e2, "ONE_PER_INCH_CUBED", 1.0e-8);
    TestUnitConversion (1.0, "ONE_PER_GALLON", 7.48051948051948, "ONE_PER_FOOT_CUBED", 1.0e-8);

    TestUnitConversion (1.0, "ONE_PER_IMPERIAL_GALLON", 6.22883545904283, "ONE_PER_FOOT_CUBED", 1.0e-8);
    TestUnitConversion (1.0, "ONE_PER_INCH_CUBED", 1.728e3, "ONE_PER_FOOT_CUBED", 1.0e-8);
    TestUnitConversion (1.0, "ONE_PER_LITRE", 2.8316846592e1, "ONE_PER_FOOT_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, "ONE_PER_METRE_CUBED", 2.8316846592e4, "ONE_PER_FOOT_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, "ONE_PER_MILLION_GALLON", 7.48051948051948, "ONE_PER_FOOT_CUBED", 1.0e-8);

    TestUnitConversion (1.0e6, "ONE_PER_MILLION_LITRE", 2.8316846592e1, "ONE_PER_FOOT_CUBED", 1.0e-8);
    TestUnitConversion (1.0e3, "ONE_PER_THOUSAND_GALLON", 7.48051948051948, "ONE_PER_FOOT_CUBED", 1.0e-8);
    TestUnitConversion (1.0e3, "ONE_PER_THOUSAND_LITRE", 2.8316846592e1, "ONE_PER_FOOT_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, "ONE_PER_YARD_CUBED", 3.7037037037037e4, "ONE_PER_FOOT_CUBED", 1.0e-8);

    // Values are not double checked.  This ATP soley assures that the conversion factors don't change
    // All values calculated in excel
    TestUnitConversion (1.0, "FOOT_CUBED_PER_SECOND_PER_SQUARE_ROOT_FOOT_H20", 6.81670857167927e2, "GALLON_PER_MINUTE_PER_SQUARE_ROOT_PSI", 1.0e-8);
    TestUnitConversion (1.0, "LITRE_PER_SEC_PER_SQUARE_ROOT_KPA", 4.16194993948617e1, "GALLON_PER_MINUTE_PER_SQUARE_ROOT_PSI", 1.0e-8);
    TestUnitConversion (1.0, "METRE_CUBED_PER_SECOND_PER_SQUARE_ROOT_METRE_H20", 1.329037762e4, "GALLON_PER_MINUTE_PER_SQUARE_ROOT_PSI", 1.0e-8);
    TestUnitConversion (1.0e6, "GALLON_PER_MINUTE_PER_SQUARE_ROOT_PSI", 7.52424068444189e1, "METRE_CUBED_PER_SECOND_PER_SQUARE_ROOT_METRE_H20", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    // All values calculated in excel
    TestUnitConversion (1.0e6, "ONE_PER_HORSEPOWER", 1.34102208959503e6, "ONE_PER_KILOWATT", 1.0e-8);
    TestUnitConversion (1.0e6, "ONE_PER_KILOWATT", 7.4569987158227e5, "ONE_PER_HORSEPOWER", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "METRE_SQUARED_DELTA_DEGREE_KELVIN_PER_WATT", 1.0, "METRE_SQUARED_DELTA_DEGREE_KELVIN_PER_WATT", 1.0e-8);
    TestUnitConversion (1.0, "METRE_SQUARED_DELTA_DEGREE_CELSIUS_PER_WATT", 1.0, "METRE_SQUARED_DELTA_DEGREE_KELVIN_PER_WATT", 1.0e-8);
    TestUnitConversion (1.0, "FOOT_SQUARED_HOUR_DELTA_DEGREE_FAHRENHEIT_PER_BTU", 1.76110183682306e-1, "METRE_SQUARED_DELTA_DEGREE_KELVIN_PER_WATT", 1.0e-8);

    TestUnitConversion (1.0e6, "METRE_SQUARED_DELTA_DEGREE_KELVIN_PER_WATT", 1.0e6, "METRE_SQUARED_DELTA_DEGREE_KELVIN_PER_WATT", 1.0e-8);
    TestUnitConversion (1.0e6, "METRE_SQUARED_DELTA_DEGREE_CELSIUS_PER_WATT", 1.0e6, "METRE_SQUARED_DELTA_DEGREE_KELVIN_PER_WATT", 1.0e-8);
    TestUnitConversion (1.0e6, "FOOT_SQUARED_HOUR_DELTA_DEGREE_FAHRENHEIT_PER_BTU", 1.76110183682306e5, "METRE_SQUARED_DELTA_DEGREE_KELVIN_PER_WATT", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "MEGANEWTON_SECOND_PER_METRE_SQUARED", 1.0, "MEGANEWTON_SECOND_PER_METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, "MILLINEWTON_SECOND_PER_METRE_SQUARED", 1.0e-9, "MEGANEWTON_SECOND_PER_METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, "PASCAL_SECOND", 1.0e-6, "MEGANEWTON_SECOND_PER_METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, "PASCAL_SECOND", 1.0e3, "CENTIPOISE", 1.0e-8);

    TestUnitConversion (1.0e6, "MEGANEWTON_SECOND_PER_METRE_SQUARED", 1.0e6, "MEGANEWTON_SECOND_PER_METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0e6, "MILLINEWTON_SECOND_PER_METRE_SQUARED", 1.0e-3, "MEGANEWTON_SECOND_PER_METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0e6, "PASCAL_SECOND", 1.0, "MEGANEWTON_SECOND_PER_METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0e6, "PASCAL_SECOND", 1.0e9, "CENTIPOISE", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "KILOJOULE_PER_KILOGRAM", 1.0, "KILOJOULE_PER_KILOGRAM", 1.0e-8);
    TestUnitConversion (1.0, "MEGAJOULE_PER_KILOGRAM", 1.0e3, "KILOJOULE_PER_KILOGRAM", 1.0e-8);
    TestUnitConversion (1.0, "BTU_PER_POUND_MASS", 2.326, "KILOJOULE_PER_KILOGRAM", 1.0e-8);

    TestUnitConversion (1.0e6, "KILOJOULE_PER_KILOGRAM", 1.0e6, "KILOJOULE_PER_KILOGRAM", 1.0e-8);
    TestUnitConversion (1.0e6, "MEGAJOULE_PER_KILOGRAM", 1.0e9, "KILOJOULE_PER_KILOGRAM", 1.0e-8);
    TestUnitConversion (1.0e6, "BTU_PER_POUND_MASS", 2.326e6, "KILOJOULE_PER_KILOGRAM", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "METRE_CUBED_PER_KILOGRAM", 1.0, "METRE_CUBED_PER_KILOGRAM", 1.0e-8);
    TestUnitConversion (1.0, "FOOT_CUBED_PER_POUND_MASS", 6.24279605761448e-2, "METRE_CUBED_PER_KILOGRAM", 1.0e-8);

    TestUnitConversion (1.0e6, "METRE_CUBED_PER_KILOGRAM", 1.0e6, "METRE_CUBED_PER_KILOGRAM", 1.0e-8);
    TestUnitConversion (1.0e6, "FOOT_CUBED_PER_POUND_MASS", 6.24279605761448e4, "METRE_CUBED_PER_KILOGRAM", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "ONE_PER_BTU", 1.0, "ONE_PER_BTU", 1.0e-8);
    TestUnitConversion (1.0, "ONE_PER_MEGAJOULE", 1.05505585262e-3, "ONE_PER_BTU", 1.0e-8);
    TestUnitConversion (1.0, "ONE_PER_KILOWATT_HOUR", 2.93071070172222e-4, "ONE_PER_BTU", 1.0e-8);

    TestUnitConversion (1.0e6, "ONE_PER_BTU", 1.0e6, "ONE_PER_BTU", 1.0e-8);
    TestUnitConversion (1.0e6, "ONE_PER_MEGAJOULE", 1.05505585262e3, "ONE_PER_BTU", 1.0e-8);
    TestUnitConversion (1.0e6, "ONE_PER_KILOWATT_HOUR", 2.93071070172222e2, "ONE_PER_BTU", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "ONE_PER_TONNE", 1.0, "ONE_PER_TONNE", 1.0e-8);
    TestUnitConversion (1.0, "ONE_PER_SHORT_TON", 1.10231131092439, "ONE_PER_TONNE", 1.0e-8);

    TestUnitConversion (1.0e6, "ONE_PER_TONNE", 1.0e6, "ONE_PER_TONNE", 1.0e-8);
    TestUnitConversion (1.0e6, "ONE_PER_SHORT_TON", 1.10231131092439e6, "ONE_PER_TONNE", 1.0e-8);

   // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "LUMEN_PER_FOOT_SQUARED", 1.07639104167097e1, "LUX", 1.0e-8);
    TestUnitConversion (1.0, "LUX", 9.290304e-2, "LUMEN_PER_FOOT_SQUARED", 1.0e-8);

    TestUnitConversion (1.0e6, "LUMEN_PER_FOOT_SQUARED", 1.07639104167097e7, "LUX", 1.0e-7);
    TestUnitConversion (1.0e6, "LUX", 9.290304e4, "LUMEN_PER_FOOT_SQUARED", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
#ifdef WIP_UNITS_TEST_OBSOLETE_UNITS
    // Managed tests disable asserts for these, because the units are not compatible (dimension M2_PER_S2_K vs L2_PER_T2_K)
    TestUnitConversion (1.0, "JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0, "JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0, "JOULE_PER_KILOGRAM_PER_DELTA_DEGREE_CELSIUS", 1.0, "JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0, "BTU_PER_POUND_MASS_PER_DELTA_DEGREE_FAHRENHEIT", 4.1868e3, "JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0e-8);

    TestUnitConversion (1.0e6, "JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0e6, "JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0e6, "JOULE_PER_KILOGRAM_PER_DELTA_DEGREE_CELSIUS", 1.0e6, "JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0e6, "BTU_PER_POUND_MASS_PER_DELTA_DEGREE_FAHRENHEIT", 4.1868e9, "JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0e-6);
    
    TestUnitConversion (1.0e6, "BTU_PER_POUND_MASS_PER_DELTA_DEGREE_RANKINE", 1.0e6, "BTU_PER_POUND_MASS_PER_DELTA_DEGREE_FAHRENHEIT", 1.0e-8);
    TestUnitConversion (1.0e6, "JOULE_PER_KILOGRAM_PER_DELTA_DEGREE_KELVIN", 1.0e6, "JOULE_PER_KILOGRAM_PER_DELTA_DEGREE_CELSIUS", 1.0e-8);
    TestUnitConversion (1.0e6, "JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0e6, "JOULE_PER_KILOGRAM_PER_DELTA_DEGREE_CELSIUS", 1.0e-8);
#endif
    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "ONE_PER_HOUR", 1.0 / 3600.0, "HERTZ", 1.0e-8);
    TestUnitConversion (1.0, "ONE_PER_YEAR", 1.0 / 31536000.0, "HERTZ", 1.0e-8); // Not directly tested on knowledgedoor because they define a year as 365.25 days validated.  Did the one_per_day -> Hertz then multiplied by 365
    TestUnitConversion (1.0, "ONE_PER_DAY", 1.0 / 86400.0, "HERTZ", 1.0e-8);
    TestUnitConversion (1.0, "ONE_PER_MINUTE", 1.0 / 60.0, "HERTZ", 1.0e-8);

    TestUnitConversion (1.0e6, "ONE_PER_HOUR", 1.0e6 / 3600.0, "HERTZ", 1.0e-8);
    TestUnitConversion (1.0e6, "ONE_PER_YEAR", 1.0e6 / 31536000.0, "HERTZ", 1.0e-8); // Not directly tested on knowledgedoor because they define a year as 365.25 days validated.  Did the one_per_day -> Hertz then multiplied by 365
    TestUnitConversion (1.0e6, "ONE_PER_DAY", 1.0e6 / 86400.0, "HERTZ", 1.0e-8);
    TestUnitConversion (1.0e6, "ONE_PER_MINUTE", 1.0e6 / 60.0, "HERTZ", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "GIGANEWTON_SECOND_PER_KILOGRAM_PER_METRE", 1.0, "GIGANEWTON_SECOND_PER_KILOGRAM_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0e19, "GRAIN_FORCE_PER_HOUR_PER_FOOT_SQUARED_PER_INCH_HG_CONVENTIONAL", 5.61072657924843, "GIGANEWTON_SECOND_PER_KILOGRAM_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0e25, "GRAIN_FORCE_PER_HOUR_PER_FOOT_SQUARED_PER_INCH_HG_CONVENTIONAL", 5.61072657924843e6, "GIGANEWTON_SECOND_PER_KILOGRAM_PER_METRE", 1.0e-8);

    TestUnitConversion (1.0, "MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0, "MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, "GRAIN_MASS_PER_HOUR_PER_FOOT_SQUARED", 6.97489662340437e2, "MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, "MICROGRAM_PER_FOOT_SQUARED_PER_DAY", 4.48496267362905e-4, "MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, "MICROGRAM_PER_METRE_SQUARED_PER_DAY", 4.16666666666667e-5, "MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel

    TestUnitConversion (1.0, "MICROGRAM_PER_METRE_SQUARED_PER_SECOND", 3.6, "MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, "MILLIGRAM_PER_FOOT_SQUARED_PER_DAY", 4.48496267362905e-1, "MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, "MILLIGRAM_PER_METRE_SQUARED_PER_DAY", 4.16666666666667e-2, "MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, "MILLIGRAM_PER_METRE_SQUARED_PER_SECOND", 3.6e3, "MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel

    TestUnitConversion (1.0e6, "GRAIN_MASS_PER_HOUR_PER_FOOT_SQUARED", 6.97489662340437e8, "MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-5);
    TestUnitConversion (1.0e6, "MICROGRAM_PER_FOOT_SQUARED_PER_DAY", 4.48496267362905e2, "MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e6, "MICROGRAM_PER_METRE_SQUARED_PER_DAY", 4.16666666666667e1, "MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel

    TestUnitConversion (1.0e6, "MICROGRAM_PER_METRE_SQUARED_PER_SECOND", 3.6e6, "MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-2); // Value from excel
    TestUnitConversion (1.0e6, "MILLIGRAM_PER_FOOT_SQUARED_PER_DAY", 4.48496267362905e5, "MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e6, "MILLIGRAM_PER_METRE_SQUARED_PER_DAY", 4.16666666666667e4, "MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e6, "MILLIGRAM_PER_METRE_SQUARED_PER_SECOND", 3.6e9, "MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel
  
    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "KILOGRAM_PER_METRE", 1.0, "KILOGRAM_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0, "POUND_MASS_PER_FOOT", 1.48816394356955, "KILOGRAM_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0e6, "POUND_MASS_PER_FOOT", 1.48816394356955e6, "KILOGRAM_PER_METRE", 1.0e-8);

    TestUnitConversion (1.0, "WEIR_COEFFICIENT_SI", 1.0, "WEIR_COEFFICIENT_SI", 1.0e-8);
    TestUnitConversion (1.0, "WEIR_COEFFICIENT_SI", 1.8113530173065724, "WEIR_COEFFICIENT_US", 1.0e-8);
    TestUnitConversion (1.0e6, "WEIR_COEFFICIENT_SI", 1.8113530173065724e6, "WEIR_COEFFICIENT_US", 1.0e-8);

    TestUnitConversion (1.0, "SIDE_WEIR_COEFFICIENT_SI", 1.0, "SIDE_WEIR_COEFFICIENT_SI", 1.0e-8);
    TestUnitConversion (1.0, "SIDE_WEIR_COEFFICIENT_SI", 1.4859185774421794, "SIDE_WEIR_COEFFICIENT_US", 1.0e-8);
    TestUnitConversion (1.0e6, "SIDE_WEIR_COEFFICIENT_SI", 1.4859185774421794e6, "SIDE_WEIR_COEFFICIENT_US", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "INCH_PER_HOUR_PER_DEGREE_FAHRENHEIT", 1.0, "INCH_PER_HOUR_PER_DEGREE_FAHRENHEIT", 1.0e-8);
    TestUnitConversion (1.0, "MILLIMETRE_PER_HOUR_PER_DEGREE_CELSIUS", 2.18722659667541e-2, "INCH_PER_HOUR_PER_DEGREE_FAHRENHEIT", 1.0e-8);
    TestUnitConversion (1.0e6, "MILLIMETRE_PER_HOUR_PER_DEGREE_CELSIUS", 2.18722659667541e4, "INCH_PER_HOUR_PER_DEGREE_FAHRENHEIT", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "ONE_PER_PASCAL", 1.0, "ONE_PER_PASCAL", 1.0e-8);
    TestUnitConversion (1.0, "ONE_PER_BAR", 1.0e-5, "ONE_PER_PASCAL", 1.0e-8);
    TestUnitConversion (1.0e6, "ONE_PER_BAR", 10.0, "ONE_PER_PASCAL", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "KILOMOLE_PER_SECOND", 1000.0, "MOLE_PER_SECOND", 1.0e-8);
    TestUnitConversion (1.0, "MOLE_PER_SECOND", 1.0e-3, "KILOMOLE_PER_SECOND", 1.0e-8);
    TestUnitConversion (1.0e6, "KILOMOLE_PER_SECOND", 1.0e9, "MOLE_PER_SECOND", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "KILOMOLE_PER_METRE_CUBED", 1000.0, "MOLE_PER_METRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, "MOLE_PER_METRE_CUBED", 1.0e-3, "KILOMOLE_PER_METRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, "MOLE_PER_METRE_CUBED", 6.24279605761446e-5, "POUND_MOLE_PER_FOOT_CUBED", 1.0e-8);
    TestUnitConversion (1.0e9, "MOLE_PER_METRE_CUBED", 1.0e6, "KILOMOLE_PER_METRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, "MOLE_PER_METRE_CUBED", 62.4279605761446, "POUND_MOLE_PER_FOOT_CUBED", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "METRE_CUBED_PER_KILOMOLE", 1.0e-3, "METRE_CUBED_PER_MOLE", 1.0e-8);
    TestUnitConversion (1.0, "METRE_CUBED_PER_MOLE", 1000.0, "METRE_CUBED_PER_KILOMOLE", 1.0e-8);
    TestUnitConversion (1.0, "METRE_CUBED_PER_MOLE", 16018.4633739601, "FOOT_CUBED_PER_POUND_MOLE", 1.0e-8);
    
    TestUnitConversion (1.0e9, "METRE_CUBED_PER_KILOMOLE", 1.0e6, "METRE_CUBED_PER_MOLE", 1.0e-8);
    TestUnitConversion (1.0e6, "METRE_CUBED_PER_MOLE", 16018.4633739601e6, "FOOT_CUBED_PER_POUND_MOLE", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "BTU_PER_POUND_MASS_PER_DELTA_DEGREE_RANKINE", 1.0 / 0.00023884589662749597, "JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0, "JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 0.00023884589662749597, "BTU_PER_POUND_MASS_PER_DELTA_DEGREE_RANKINE", 1.0e-8);

    TestUnitConversion (1.0e6, "BTU_PER_POUND_MASS_PER_DELTA_DEGREE_RANKINE", 1.0e6 / 0.00023884589662749597, "JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0e6, "JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 2.3884589662749597e2, "BTU_PER_POUND_MASS_PER_DELTA_DEGREE_RANKINE", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "BTU_PER_POUND_MOLE", 1.0 / 0.42992261392949271, "KILOJOULE_PER_KILOMOLE", 1.0e-8);
    TestUnitConversion (1.0, "KILOJOULE_PER_KILOMOLE", 0.42992261392949271, "BTU_PER_POUND_MOLE", 1.0e-8);

    TestUnitConversion (1.0e6, "BTU_PER_POUND_MOLE", 1.0e6 / 0.42992261392949271, "KILOJOULE_PER_KILOMOLE", 1.0e-8);
    TestUnitConversion (1.0e6, "KILOJOULE_PER_KILOMOLE", 4.2992261392949271e5, "BTU_PER_POUND_MOLE", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, "JOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN", 0.001, "KILOJOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0, "KILOJOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN", 1000.0, "JOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0, "KILOJOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN", 0.23884589662749595, "BTU_PER_POUND_MOLE_PER_DELTA_DEGREE_RANKINE", 1.0e-8);

    TestUnitConversion (1.0e6, "JOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN", 1.0e3, "KILOJOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0e6, "KILOJOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN", 1.0e9, "JOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0e6, "KILOJOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN", 2.3884589662749595e5, "BTU_PER_POUND_MOLE_PER_DELTA_DEGREE_RANKINE", 1.0e-8);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitsTest, TestUnitSpecifications)
    {
    TestUnitSpecification ("FromKOQ", 100.0);
    TestUnitSpecification ("FromParentKOQ", 100.0);
    TestUnitSpecification ("FromDimension", 0.001);
    TestUnitSpecification ("FromKOQDimension", 0.001);

    TestUnitSpecification ("FromNonExistentKOQWithDimension", 10.0);

    // Non-standard Unit produces a Unit which is not convertible to any other Unit
    Unit nonStandardUnit;
    EXPECT_TRUE (Unit::GetUnitForECProperty (nonStandardUnit, *m_schema->GetClassP ("UnitSpecClass")->GetPropertyP ("FromNonExistentKOQWithUnit")));
    EXPECT_EQ (0, strcmp (nonStandardUnit.GetShortLabel(), "NONEXISTENT_UNIT"));
    EXPECT_EQ (0, strcmp (nonStandardUnit.GetBaseUnitName(), "NONEXISTENT_UNIT"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitsTest, Formatting)
    {
    // Storage: cm Display: none
    TestUnitFormatting ("FromKOQ", 123.456, "123.456 cm");

    // Storage: cm Display: decimetre
    TestUnitFormatting ("FromParentKOQ", 123.456, "12.345600000000001 dm");

    // Storage: km Display: none
    TestUnitFormatting ("FromKOQDimension", 123.456, "123.456 km");

    // Storage: km Display: miles Format: 0000.###### \"ignored\"
    TestUnitFormatting ("FromDimension", 123.456, "0076.712002 ignored mi");
    }
END_BENTLEY_ECN_TEST_NAMESPACE
