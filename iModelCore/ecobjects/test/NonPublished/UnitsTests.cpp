/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/UnitsTests.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include "TestFixture.h"
#include <ECUnits/Units.h>

using namespace Bentley::ECN;

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

static WCharCP s_refSchemaXml =
    L"<ECSchema schemaName=\"RefSchema\" nameSpacePrefix=\"ref\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
    L"  <ECSchemaReference name=\"Unit_Attributes\" version=\"01.00\" prefix=\"units_attribs\" />"
    L"  <ECCustomAttributes>"
    L"      <IsUnitSystemSchema xmlns=\"Unit_Attributes.01.00\" />"
    L"      <UnitSpecifications xmlns=\"Unit_Attributes.01.00\">"
    L"          <UnitSpecificationList>"
    L"              <UnitSpecification>"
    L"                  <KindOfQuantityName>LENGTH</KindOfQuantityName>"
    L"                  <!-- Note no UnitName specified -->"
    L"              </UnitSpecification>"
    L"              <UnitSpecification>"
    L"                  <KindOfQuantityName>DIAMETER</KindOfQuantityName>"
    L"                  <UnitName>CENTIMETRE</UnitName>"
    L"              </UnitSpecification>"
    L"              <UnitSpecification>"
    L"                  <DimensionName>L</DimensionName>"
    L"                  <UnitName>KILOMETRE</UnitName>"
    L"              </UnitSpecification>"
    L"              <UnitSpecification>"
    L"                  <KindOfQuantityName>NONEXISTENT_KOQ_WITH_DIMENSION</KindOfQuantityName>"
    L"                  <DimensionName>L3</DimensionName>"
    L"              </UnitSpecification>"
    L"              <UnitSpecification>"
    L"                  <KindOfQuantityName>NONEXISTENT_KOQ_WITH_UNIT</KindOfQuantityName>"
    L"                  <UnitName>NONEXISTENT_UNIT</UnitName>"
    L"              </UnitSpecification>"
    L"          </UnitSpecificationList>"
    L"      </UnitSpecifications>"
    L"  </ECCustomAttributes>"
    L"</ECSchema>";

static WCharCP s_schemaXml =
    L"<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
    L"  <ECSchemaReference name=\"Unit_Attributes\" version=\"01.00\" prefix=\"units_attribs\" />"
    L"  <ECSchemaReference name=\"RefSchema\" version=\"01.00\" prefix=\"refSchema\" />"
    L"  <ECCustomAttributes>"
    L"      <UnitSpecifications xmlns=\"Unit_Attributes.01.00\">"
    L"          <UnitSpecificationList>"
    L"              <!-- Dimension L3 referenced in UnitSpecification in referenced schema, with Unit defined in domain schema -->"
    L"              <UnitSpecification>"
    L"                  <DimensionName>L3</DimensionName>"
    L"                  <UnitName>DECIMETRE</UnitName>"
    L"              </UnitSpecification>"
    L"          </UnitSpecificationList>"
    L"      </UnitSpecifications>"
    L"  </ECCustomAttributes>"
    L"    <!-- Class for testing units defined on properties, for unit conversions -->"
    L"    <ECClass typeName=\"TestClass\" isDomainClass=\"True\">"
    L"        <ECProperty propertyName=\"FromProperty\" typeName=\"double\">"
    L"          <ECCustomAttributes>"
    L"              <UnitSpecification xmlns=\"Unit_Attributes.01.00\">"
    L"                  <UnitName>KILOMETRE</UnitName>"
    L"              </UnitSpecification>"
    L"          </ECCustomAttributes>"
    L"        </ECProperty>"
    L"        <ECProperty propertyName=\"ToProperty\" typeName=\"double\">"
    L"          <ECCustomAttributes>"
    L"              <UnitSpecification xmlns=\"Unit_Attributes.01.00\">"
    L"                  <UnitName>KILOMETRE</UnitName>"
    L"              </UnitSpecification>"
    L"          </ECCustomAttributes>"
    L"        </ECProperty>"
    L"    </ECClass>"
    L"  <!-- Class for testing UnitSpecifications defined at schema level (including referenced schema) -->"
    L"  <ECClass typeName=\"UnitSpecClass\" isDomainClass=\"True\">"
    L"        <ECProperty propertyName=\"FromKOQ\" typeName=\"double\">"
    L"          <ECCustomAttributes>"
    L"              <UnitSpecification xmlns=\"Unit_Attributes.01.00\">"
    L"                  <KindOfQuantityName>DIAMETER</KindOfQuantityName>"
    L"              </UnitSpecification>"
    L"          </ECCustomAttributes>"
    L"        </ECProperty>"
    L"        <ECProperty propertyName=\"FromParentKOQ\" typeName=\"double\">"
    L"          <ECCustomAttributes>"
    L"              <UnitSpecification xmlns=\"Unit_Attributes.01.00\">"
    L"                  <KindOfQuantityName>DIAMETER_LARGE</KindOfQuantityName>"
    L"              </UnitSpecification>"
    L"              <DisplayUnitSpecification xmlns=\"Unit_Attributes.01.00\">"
    L"                  <DisplayUnitName>DECIMETRE</DisplayUnitName>"
    L"              </DisplayUnitSpecification>"
    L"          </ECCustomAttributes>"
    L"        </ECProperty>"
    L"        <ECProperty propertyName=\"FromKOQDimension\" typeName=\"double\">"
    L"          <ECCustomAttributes>"
    L"              <UnitSpecification xmlns=\"Unit_Attributes.01.00\">"
    L"                  <KindOfQuantityName>LENGTH</KindOfQuantityName>"
    L"              </UnitSpecification>"
    L"          </ECCustomAttributes>"
    L"        </ECProperty>"
    L"        <ECProperty propertyName=\"FromDimension\" typeName=\"double\">"
    L"          <ECCustomAttributes>"
    L"              <UnitSpecification xmlns=\"Unit_Attributes.01.00\">"
    L"                  <DimensionName>L</DimensionName>"
    L"              </UnitSpecification>"
    L"              <DisplayUnitSpecification xmlns=\"Unit_Attributes.01.00\">"
    L"                  <DisplayUnitName>MILE</DisplayUnitName>"
    L"                  <DisplayFormatString>0000.###### \"ignored\"</DisplayFormatString>"
    L"              </DisplayUnitSpecification>"
    L"          </ECCustomAttributes>"
    L"        </ECProperty>"
    L"        <ECProperty propertyName=\"FromNonExistentKOQWithDimension\" typeName=\"double\">"
    L"          <ECCustomAttributes>"
    L"              <UnitSpecification xmlns=\"Unit_Attributes.01.00\">"
    L"                  <KindOfQuantityName>NONEXISTENT_KOQ_WITH_DIMENSION</KindOfQuantityName>"
    L"              </UnitSpecification>"
    L"          </ECCustomAttributes>"
    L"        </ECProperty>"
    L"        <ECProperty propertyName=\"FromNonExistentKOQWithUnit\" typeName=\"double\">"
    L"          <ECCustomAttributes>"
    L"              <UnitSpecification xmlns=\"Unit_Attributes.01.00\">"
    L"                  <KindOfQuantityName>NONEXISTENT_KOQ_WITH_UNIT</KindOfQuantityName>"
    L"              </UnitSpecification>"
    L"          </ECCustomAttributes>"
    L"        </ECProperty>"
    L"  </ECClass>"
    L"</ECSchema>";

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct UnitsTest : ECTestFixture
    {
    ECSchemaPtr     m_refSchema;
    ECSchemaPtr     m_schema;
    ECPropertyCP    m_fromProperty, m_toProperty;

    virtual void    SetUp() override;

    void            SetUnits (WCharCP fromUnitName, WCharCP toUnitName)
        {
        m_fromProperty->GetCustomAttribute (L"Unit_Attributes", L"UnitSpecification")->SetValue (L"UnitName", ECValue (fromUnitName));
        m_toProperty->GetCustomAttribute (L"Unit_Attributes", L"UnitSpecification")->SetValue (L"UnitName", ECValue (toUnitName));
        }

    void            GetUnits (UnitR from, UnitR to)
        {
        EXPECT_TRUE (Unit::GetUnitForECProperty (from, *m_fromProperty));
        EXPECT_TRUE (Unit::GetUnitForECProperty (to, *m_toProperty));
        }

    void            TestUnitConversion (double fromVal, WCharCP fromUnitName, double expectedVal, WCharCP targetUnitName, double tolerance);
    void            TestUnitSpecification (WCharCP propName, double expectedValueOfOneMeter);
    void            TestUnitFormatting (WCharCP propName, double storedValue, WCharCP formattedValue);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitsTest::SetUp()
    {
    ECTestFixture::SetUp();

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    EXPECT_EQ (SUCCESS, ECSchema::ReadFromXmlString (m_refSchema, s_refSchemaXml, *context));
    EXPECT_EQ (SUCCESS, ECSchema::ReadFromXmlString (m_schema, s_schemaXml, *context));

    m_fromProperty = m_schema->GetClassP (L"TestClass")->GetPropertyP (L"FromProperty");
    m_toProperty = m_schema->GetClassP (L"TestClass")->GetPropertyP (L"ToProperty");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitsTest::TestUnitConversion (double fromVal, WCharCP fromUnitName, double expectedVal, WCharCP targetUnitName, double tolerance)
    {
    SetUnits (fromUnitName, targetUnitName);
    Unit srcUnit, dstUnit;
    GetUnits (srcUnit, dstUnit);
    
    EXPECT_TRUE (srcUnit.IsCompatible (dstUnit) && dstUnit.IsCompatible (srcUnit));
    double convertedVal = fromVal;
    EXPECT_TRUE (srcUnit.ConvertTo (convertedVal, dstUnit));
    EXPECT_TRUE (fabs (convertedVal - expectedVal) < tolerance) << L"Input " << fromVal << fromUnitName << L" Expected " << expectedVal << targetUnitName << L" Actual " << convertedVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitsTest::TestUnitSpecification (WCharCP propName, double expectedValueOfOneMeter)
    {
    ECClassCP ecClass = m_schema->GetClassP (L"UnitSpecClass");

    Unit meter (L"METRE", L"m", UnitConverter (false), L"METRE");
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
void UnitsTest::TestUnitFormatting (WCharCP propName, double storedValue, WCharCP formattedValue)
    {
    ECPropertyCP ecprop = m_schema->GetClassP (L"UnitSpecClass")->GetPropertyP (propName);

    WString formatted;
    EXPECT_TRUE (Unit::FormatValue (formatted, ECValue (storedValue), *ecprop, NULL, propName));
    EXPECT_TRUE (formatted.Equals (formattedValue)) << L"Expected: " << formattedValue << L" Actual: " << formatted.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* The following tests were generated from managed tests using regex replace.
* Generated from ecf\ecobjects\atp\units\UnitTestCase.cs using hg revision e1e9c347ce20
* Given: Assert.AreEqual (12.0, StandardUnits.INCH.ConvertFrom (1.0, StandardUnits.FOOT), 1.0e-8);
* Using VIM:
*   :%s/Assert.AreEqual (\(.*\), StandardUnits\.\(.*\)\.ConvertFrom (\(.*\), StandardUnits\.\(.*\)), \(.*\));/TestUnitConversion (\3, L"\4", \1, L"\2", \5);/g
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitsTest, Conversions)
    {
    // Dimension L
    TestUnitConversion (1.0, L"FOOT", 12.0, L"INCH", 1.0e-8);
    TestUnitConversion (12.0, L"INCH", 1.0, L"FOOT", 1.0e-8);
    TestUnitConversion (1.0, L"KILOMETRE", 1000.0, L"METRE", 1.0e-8);
    TestUnitConversion (1.0, L"DECIMETRE", 0.1, L"METRE", 1.0e-8);
    TestUnitConversion (1.0, L"CENTIMETRE", 0.01, L"METRE", 1.0e-8);
    TestUnitConversion (1.0, L"MILLIMETRE", 0.001, L"METRE", 1.0e-8);
    TestUnitConversion (1.0, L"METRE", 1.0, L"METRE", 1.0e-8);
    TestUnitConversion (1.0, L"FOOT", 0.3048, L"METRE", 1.0e-8);
    TestUnitConversion (1.0, L"INCH", 0.0254, L"METRE", 1.0e-8);
    TestUnitConversion (1.0, L"YARD", 0.9144, L"METRE", 1.0e-8);
    TestUnitConversion (1.0, L"MILE", 1609.344, L"METRE", 1.0e-8);
    TestUnitConversion (100000.0, L"MICROINCH", 2.54e-3, L"METRE", 1.0e-8);
    TestUnitConversion (100000.0, L"MICROMETRE", 0.1, L"METRE", 1.0e-8);
    TestUnitConversion (10000.0, L"MILLIINCH", 0.254, L"METRE", 1.0e-8);
    TestUnitConversion (1000.0, L"MILLIFOOT", 0.3048, L"METRE", 1.0e-8);
    TestUnitConversion (1.0, L"US_SURVEY_INCH", 2.54000508001016e-02, L"METRE", 1.0e-8);
    TestUnitConversion (1.0, L"US_SURVEY_FOOT", 0.30480060960122, L"METRE", 1.0e-8);
    TestUnitConversion (1.0, L"US_SURVEY_MILE", 1609.34721869444, L"METRE", 1.0e-8);

    TestUnitConversion (1000000.0, L"FOOT", 12000000.0, L"INCH", 1.0e-8);
    TestUnitConversion (12000000.0, L"INCH", 1000000.0, L"FOOT", 1.0e-8);
    TestUnitConversion (1000000.0, L"KILOMETRE", 1000000000.0, L"METRE", 1.0e-8);
    TestUnitConversion (1000000.0, L"DECIMETRE", 100000.0, L"METRE", 1.0e-8);
    TestUnitConversion (1000000.0, L"CENTIMETRE", 10000.0, L"METRE", 1.0e-8);
    TestUnitConversion (1000000000.0, L"MILLIMETRE", 1000000, L"METRE", 1.0e-8);
    TestUnitConversion (1000000.0, L"METRE", 1000000.0, L"METRE", 1.0e-8);
    TestUnitConversion (10000000.0, L"FOOT", 3048000.0, L"METRE", 1.0e-8);
    TestUnitConversion (10000000.0, L"INCH", 254000.0, L"METRE", 1.0e-8);
    TestUnitConversion (10000000.0, L"YARD", 9144000.0, L"METRE", 1.0e-8);
    TestUnitConversion (1000000.0, L"MILE", 1609344000.0, L"METRE", 1.0e-6); // Does not pass w/ 1.0e-8
    TestUnitConversion (1000000.0, L"MILE", 1609344.0, L"KILOMETRE", 1.0e-8);
    TestUnitConversion (10000000.0, L"MICROINCH", 0.254, L"METRE", 1.0e-8);
    TestUnitConversion (10000000.0, L"MICROMETRE", 10.0, L"METRE", 1.0e-8);
    TestUnitConversion (10000000.0, L"MILLIINCH", 254.0, L"METRE", 1.0e-8);
    TestUnitConversion (10000000.0, L"MILLIFOOT", 3048.0, L"METRE", 1.0e-8);
    TestUnitConversion (1000000.0, L"US_SURVEY_INCH", 25400.0508001016, L"METRE", 1.0e-8);
    TestUnitConversion (1000000.0, L"US_SURVEY_FOOT", 304800.60960122, L"METRE", 1.0e-8);
    TestUnitConversion (1000000.0, L"US_SURVEY_MILE", 1609347218.69444, L"METRE", 1.0e-5); // Does not pass w/ 1.0e-8

    // Dimension L2
    // All results calculated in excel and double checked at http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"CENTIMETRE_SQUARED", 100.0, L"MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, L"METRE_SQUARED", 1000000.0, L"MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, L"HECTARE", 10000000000.0, L"MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, L"KILOMETRE_SQUARED", 1000000000000.0, L"MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, L"INCH_SQUARED", 645.16, L"MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, L"FOOT_SQUARED", 92903.04, L"MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (100, L"THOUSAND_FOOT_SQUARED", 100000, L"FOOT_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, L"YARD_SQUARED", 836127.36, L"MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, L"ACRE", 4.04685642240001000E+03, L"METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, L"MILE_SQUARED", 2.589988110336E+06, L"METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, L"MILLIMETRE_SQUARED", 1e-6, L"METRE_SQUARED", 1.0e-8);

    TestUnitConversion (1000000.0, L"CENTIMETRE_SQUARED", 1.0e8, L"MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (1000000.0, L"METRE_SQUARED", 1.0e12, L"MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (1000000.0, L"HECTARE", 1.0e16, L"MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (1000000.0, L"KILOMETRE_SQUARED", 1.0e18, L"MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (1000000.0, L"INCH_SQUARED", 645160000.0, L"MILLIMETRE_SQUARED", 1.0e-8);
    TestUnitConversion (1000000.0, L"FOOT_SQUARED", 92903040000.0, L"MILLIMETRE_SQUARED", 10e-5); // Does not pass w/ 1.0e-8
    TestUnitConversion (100000000, L"THOUSAND_FOOT_SQUARED", 1.0e11, L"FOOT_SQUARED", 1.0e-8);
    TestUnitConversion (1000000.0, L"YARD_SQUARED", 836127360000.0, L"MILLIMETRE_SQUARED", 1.0e-3); // Does not pass w/ 1.0e-8
    TestUnitConversion (1000000.0, L"ACRE", 4.0468564224e9, L"METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1000000.0, L"MILE_SQUARED", 2.589988110336e12, L"METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1000000.0, L"MILLIMETRE_SQUARED", 1.0, L"METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, L"ACRE", 1.0, L"ACRE", 1.0e-8);

    // Diameter Length
    // All lines tagged with 'Value from excel' were calculated using the conversion factors in excel
    // All the lines tagged with 'Value from Windows Calc' were calculated using the conversion factors in Windows Calculator
    // All other values where alreay correct.  Any values that changed were close to the previous value.
    TestUnitConversion (-1162.5, L"INCH_MILE", -1162.5, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (-1, L"INCH_MILE", -1, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (0, L"INCH_MILE", 0, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (1, L"INCH_MILE", 1, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_MILE", 112.85, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, L"INCH_MILE", -6138000, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (-1, L"INCH_MILE", -5280, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (0, L"INCH_MILE", 0, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (1, L"INCH_MILE", 5280, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_MILE", 595848, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (-1162.5, L"INCH_MILE", -96.875, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1, L"INCH_MILE", -0.083333333, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (0, L"INCH_MILE", 0, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (1, L"INCH_MILE", 0.083333333, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_MILE", 9.40416666666666, L"FOOT_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"INCH_MILE", -511500, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (-1, L"INCH_MILE", -440, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (0, L"INCH_MILE", 0, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (1, L"INCH_MILE", 440, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_MILE", 49654, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (-1162.5, L"INCH_MILE", -47519904.96, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1, L"INCH_MILE", -40877.3376, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (0, L"INCH_MILE", 0, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (1, L"INCH_MILE", 40877.3376, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_MILE", 4.61300754816E+06, L"MILLIMETRE_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"INCH_MILE", -4.751990496E+04, L"MILLIMETRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"INCH_MILE", -40.8773376, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, L"INCH_MILE", 0, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, L"INCH_MILE", 40.8773376, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_MILE", 4.61300754816E+03, L"MILLIMETRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"INCH_MILE", -4.751990496E+04, L"METRE_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"INCH_MILE", -40.8773376, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (0, L"INCH_MILE", 0, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (1, L"INCH_MILE", 40.8773376, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_MILE", 4.61300754816E+03, L"METRE_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"INCH_MILE", -4.75199049599999E+01, L"METRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"INCH_MILE", -0.040877338, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, L"INCH_MILE", 0, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, L"INCH_MILE", 0.040877338, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_MILE", 4.61300754815999, L"METRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"INCH_MILE", -1870862.4, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (-1, L"INCH_MILE", -1609.344, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (0, L"INCH_MILE", 0, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (1, L"INCH_MILE", 1609.344, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_MILE", 1.816144704E+05, L"INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"INCH_MILE", -29527.5, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (-1, L"INCH_MILE", -25.4, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (0, L"INCH_MILE", 0, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (1, L"INCH_MILE", 25.4, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_MILE", 2866.39, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, L"INCH_FOOT", -0.22017045, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (-1, L"INCH_FOOT", -0.00018939394, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (0, L"INCH_FOOT", 0, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (1, L"INCH_FOOT", 0.00018939394, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_FOOT", 0.0213731, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, L"INCH_FOOT", -1162.5, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (-1, L"INCH_FOOT", -1, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (0, L"INCH_FOOT", 0, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (1, L"INCH_FOOT", 1, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_FOOT", 112.85, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (-1162.5, L"INCH_FOOT", -0.018347538, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1, L"INCH_FOOT", -1.5782828E-05, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (0, L"INCH_FOOT", 0, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (1, L"INCH_FOOT", 1.5782828E-05, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_FOOT", 0.0017810922, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, L"INCH_FOOT", -96.875, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (-1, L"INCH_FOOT", -0.083333333, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (0, L"INCH_FOOT", 0, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (1, L"INCH_FOOT", 0.083333333, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_FOOT", 9.40416667, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (-1162.5, L"INCH_FOOT", -8999.982, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1, L"INCH_FOOT", -7.74192, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (0, L"INCH_FOOT", 0, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (1, L"INCH_FOOT", 7.74192, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_FOOT", 873.675672, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"INCH_FOOT", -8.999982, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, L"INCH_FOOT", -0.00774192, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, L"INCH_FOOT", 0, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, L"INCH_FOOT", 0.00774192, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_FOOT", 0.87367567, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"INCH_FOOT", -8.999982, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (-1, L"INCH_FOOT", -0.00774192, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (0, L"INCH_FOOT", 0, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (1, L"INCH_FOOT", 0.00774192, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_FOOT", 0.87367567, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"INCH_FOOT", -0.0089999821, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, L"INCH_FOOT", -7.7419201E-06, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, L"INCH_FOOT", 0, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, L"INCH_FOOT", 7.7419201E-06, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_FOOT", 0.00087367568, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"INCH_FOOT", -354.33, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (-1, L"INCH_FOOT", -0.3048, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (0, L"INCH_FOOT", 0, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (1, L"INCH_FOOT", 0.3048, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_FOOT", 34.39668, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"INCH_FOOT", -5.59232955, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (-1, L"INCH_FOOT", -0.0048106061, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (0, L"INCH_FOOT", 0, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (1, L"INCH_FOOT", 0.0048106061, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_FOOT", 0.54287689, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, L"FOOT_MILE", -1.395E+04, L"INCH_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"FOOT_MILE", -12, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (0, L"FOOT_MILE", 0, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (1, L"FOOT_MILE", 12, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (112.85, L"FOOT_MILE", 1.3542E+03, L"INCH_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"FOOT_MILE", -73656000, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (-1, L"FOOT_MILE", -6.336E+04, L"INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (0, L"FOOT_MILE", 0, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (1, L"FOOT_MILE", 6.336E+04, L"INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, L"FOOT_MILE", 7150176, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (-1162.5, L"FOOT_MILE", -1162.5, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1, L"FOOT_MILE", -1, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (0, L"FOOT_MILE", 0, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (1, L"FOOT_MILE", 1, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (112.85, L"FOOT_MILE", 112.85, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, L"FOOT_MILE", -6138000, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (-1, L"FOOT_MILE", -5.28E+03, L"FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (0, L"FOOT_MILE", 0, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (1, L"FOOT_MILE", 5.28E+03, L"FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, L"FOOT_MILE", 595848, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (-1162.5, L"FOOT_MILE", -570238859.52, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1, L"FOOT_MILE", -490528.0512, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (0, L"FOOT_MILE", 0, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (1, L"FOOT_MILE", 490528.0512, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"FOOT_MILE", 55356090.57792, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"FOOT_MILE", -570238.85952, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, L"FOOT_MILE", -4.905280512E+02, L"MILLIMETRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (0, L"FOOT_MILE", 0, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, L"FOOT_MILE", 4.905280512E+02, L"MILLIMETRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, L"FOOT_MILE", 5.535609057792E+04, L"MILLIMETRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"FOOT_MILE", -570238.85952, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (-1, L"FOOT_MILE", -4.905280512E+02, L"METRE_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (0, L"FOOT_MILE", 0, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (1, L"FOOT_MILE", 4.905280512E+02, L"METRE_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, L"FOOT_MILE", 5.535609057792E+04, L"METRE_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"FOOT_MILE", -5.70238859519999E+02, L"METRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"FOOT_MILE", -0.49052806, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, L"FOOT_MILE", 0, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, L"FOOT_MILE", 0.49052806, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, L"FOOT_MILE", 5.53560905779199E+01, L"METRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"FOOT_MILE", -22450348.8, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (-1, L"FOOT_MILE", -1.9312128E+04, L"INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (0, L"FOOT_MILE", 0, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (1, L"FOOT_MILE", 1.9312128E+04, L"INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, L"FOOT_MILE", 2179373.6448, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"FOOT_MILE", -3.5433E+05, L"MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"FOOT_MILE", -3.048E+02, L"MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (0, L"FOOT_MILE", 0, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (1, L"FOOT_MILE", 3.048E+02, L"MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, L"FOOT_MILE", 3.439668E+04, L"MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"FOOT_FOOT", -2.64204545, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (-1, L"FOOT_FOOT", -0.0022727273, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (0, L"FOOT_FOOT", 0, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (1, L"FOOT_FOOT", 0.0022727273, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (112.85, L"FOOT_FOOT", 0.25647727, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, L"FOOT_FOOT", -13950, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (-1, L"FOOT_FOOT", -12, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (0, L"FOOT_FOOT", 0, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (1, L"FOOT_FOOT", 12, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (112.85, L"FOOT_FOOT", 1354.2, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (-1162.5, L"FOOT_FOOT", -0.22017045, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1, L"FOOT_FOOT", -0.00018939394, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (0, L"FOOT_FOOT", 0, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (1, L"FOOT_FOOT", 0.00018939394, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (112.85, L"FOOT_FOOT", 0.0213731, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, L"FOOT_FOOT", -1162.5, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (-1, L"FOOT_FOOT", -1, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (0, L"FOOT_FOOT", 0, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (1, L"FOOT_FOOT", 1, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (112.85, L"FOOT_FOOT", 112.85, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (-1162.5, L"FOOT_FOOT", -107999.784, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1, L"FOOT_FOOT", -92.90304, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (0, L"FOOT_FOOT", 0, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (1, L"FOOT_FOOT", 92.90304, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"FOOT_FOOT", 1.0484108064E+04, L"MILLIMETRE_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"FOOT_FOOT", -107.999784, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, L"FOOT_FOOT", -0.09290304, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, L"FOOT_FOOT", 0, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, L"FOOT_FOOT", 0.09290304, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, L"FOOT_FOOT", 1.0484108064E+01, L"MILLIMETRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"FOOT_FOOT", -107.999784, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (-1, L"FOOT_FOOT", -0.09290304, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (0, L"FOOT_FOOT", 0, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (1, L"FOOT_FOOT", 0.09290304, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"FOOT_FOOT", 1.0484108064E+01, L"METRE_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"FOOT_FOOT", -0.10799979, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, L"FOOT_FOOT", -9.2903041E-05, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, L"FOOT_FOOT", 0, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, L"FOOT_FOOT", 9.2903041E-05, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, L"FOOT_FOOT", 0.0104841, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"FOOT_FOOT", -4251.96, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (-1, L"FOOT_FOOT", -3.6576, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (0, L"FOOT_FOOT", 0, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (1, L"FOOT_FOOT", 3.6576, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"FOOT_FOOT", 412.76016, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"FOOT_FOOT", -6.71079545454545E+01, L"MILLIMETRE_MILE", 1.0e-8); // Value from Excel
    TestUnitConversion (-1, L"FOOT_FOOT", -5.77272727272727E-02, L"MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (0, L"FOOT_FOOT", 0, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (1, L"FOOT_FOOT", 5.77272727272727E-02, L"MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, L"FOOT_FOOT", 6.51452273, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, L"MILLIMETRE_METRE", -0.028438741, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (-1, L"MILLIMETRE_METRE", -2.4463433E-05, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_METRE", 0, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_METRE", 2.4463433E-05, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_METRE", 0.0027606984, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, L"MILLIMETRE_METRE", -1.50156550313101E+02, L"INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"MILLIMETRE_METRE", -0.12916693, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_METRE", 0, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_METRE", 0.12916693, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_METRE", 1.45764874863083E+01, L"INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"MILLIMETRE_METRE", -0.002369895, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1, L"MILLIMETRE_METRE", -2.0386194E-06, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_METRE", 0, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_METRE", 2.0386194E-06, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_METRE", 0.0002300582, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, L"MILLIMETRE_METRE", -1.25130458594251E+01, L"FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"MILLIMETRE_METRE", -0.01076391, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_METRE", 0, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_METRE", 0.01076391, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_METRE", 1.21470729, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (-1162.5, L"MILLIMETRE_METRE", -1162.5, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1, L"MILLIMETRE_METRE", -1, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_METRE", 0, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_METRE", 1, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_METRE", 112.85, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"MILLIMETRE_METRE", -1.1625, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, L"MILLIMETRE_METRE", -0.001, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_METRE", 0, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_METRE", 0.001, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_METRE", 0.11285, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"MILLIMETRE_METRE", -1.1625, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (-1, L"MILLIMETRE_METRE", -0.001, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_METRE", 0, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_METRE", 0.001, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_METRE", 0.11285, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"MILLIMETRE_METRE", -0.0011625, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, L"MILLIMETRE_METRE", -1E-06, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_METRE", 0, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_METRE", 1E-06, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_METRE", 0.00011285, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"MILLIMETRE_METRE", -4.57677165354331E+01, L"INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"MILLIMETRE_METRE", -0.039370079, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_METRE", 0, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_METRE", 0.039370079, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_METRE", 4.44291339, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"MILLIMETRE_METRE", -0.72234401, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (-1, L"MILLIMETRE_METRE", -0.00062137119, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_METRE", 0, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_METRE", 0.00062137119, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_METRE", 0.070121739, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, L"MILLIMETRE_KILOMETRE", -2.84387405896024E+01, L"INCH_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"MILLIMETRE_KILOMETRE", -0.024463433, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_KILOMETRE", 0, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_KILOMETRE", 0.024463433, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_KILOMETRE", 2.76069839, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, L"MILLIMETRE_KILOMETRE", -1.50156550313101E+05, L"INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"MILLIMETRE_KILOMETRE", -129.166925, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_KILOMETRE", 0, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_KILOMETRE", 129.166925, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_KILOMETRE", 1.45764874863083E+04, L"INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"MILLIMETRE_KILOMETRE", -2.36989504, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1, L"MILLIMETRE_KILOMETRE", -0.0020386194, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_KILOMETRE", 0, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_KILOMETRE", 0.0020386194, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_KILOMETRE", 0.2300582, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, L"MILLIMETRE_KILOMETRE", -1.25130458594251E+04, L"FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"MILLIMETRE_KILOMETRE", -1.07639104167097E+01, L"FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (0, L"MILLIMETRE_KILOMETRE", 0, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_KILOMETRE", 1.07639104167097E+01, L"FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, L"MILLIMETRE_KILOMETRE", 1.21470729052569E+03, L"FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"MILLIMETRE_KILOMETRE", -1162500, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1, L"MILLIMETRE_KILOMETRE", -1000, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_KILOMETRE", 0, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_KILOMETRE", 1000, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_KILOMETRE", 112850, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"MILLIMETRE_KILOMETRE", -1162.5, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, L"MILLIMETRE_KILOMETRE", -1, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_KILOMETRE", 0, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_KILOMETRE", 1, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_KILOMETRE", 112.85, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"MILLIMETRE_KILOMETRE", -1162.5, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (-1, L"MILLIMETRE_KILOMETRE", -1, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_KILOMETRE", 0, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_KILOMETRE", 1, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_KILOMETRE", 112.85, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"MILLIMETRE_KILOMETRE", -1.1625, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, L"MILLIMETRE_KILOMETRE", -0.001, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_KILOMETRE", 0, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_KILOMETRE", 0.001, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_KILOMETRE", 0.11285, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"MILLIMETRE_KILOMETRE", -4.57677165354331E+04, L"INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"MILLIMETRE_KILOMETRE", -3.93700787401575E+01, L"INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (0, L"MILLIMETRE_KILOMETRE", 0, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_KILOMETRE", 3.93700787401575E+01, L"INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, L"MILLIMETRE_KILOMETRE", 4.44291338582677E+03, L"INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"MILLIMETRE_KILOMETRE", -7.22344010975901E+02, L"MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"MILLIMETRE_KILOMETRE", -6.21371192237334E-01, L"MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (0, L"MILLIMETRE_KILOMETRE", 0, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_KILOMETRE", 6.21371192237334E-01, L"MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, L"MILLIMETRE_KILOMETRE", 7.01217390439831E+01, L"MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"METRE_METRE", -2.84387405896024E+01, L"INCH_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"METRE_METRE", -0.024463433, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (0, L"METRE_METRE", 0, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (1, L"METRE_METRE", 0.024463433, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (112.85, L"METRE_METRE", 2.76069839, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, L"METRE_METRE", -1.50156550313101E+05, L"INCH_FOOT", 1.0e-8); // value from excel
    TestUnitConversion (-1, L"METRE_METRE", -129.166925, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (0, L"METRE_METRE", 0, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (1, L"METRE_METRE", 129.166925, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (112.85, L"METRE_METRE", 1.45764874863083E+04, L"INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"METRE_METRE", -2.36989504913353E+00, L"FOOT_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"METRE_METRE", -0.0020386194, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (0, L"METRE_METRE", 0, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (1, L"METRE_METRE", 0.0020386194, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (112.85, L"METRE_METRE", 0.2300582, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, L"METRE_METRE", -1.25130458594251E+04, L"FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"METRE_METRE", -1.07639104167097E+01, L"FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (0, L"METRE_METRE", 0, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (1, L"METRE_METRE", 1.07639104167097E+01, L"FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, L"METRE_METRE", 1.21470729052569E+03, L"FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"METRE_METRE", -1162500, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1, L"METRE_METRE", -1000, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (0, L"METRE_METRE", 0, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (1, L"METRE_METRE", 1000, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"METRE_METRE", 112850, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"METRE_METRE", -1162.5, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, L"METRE_METRE", -1, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, L"METRE_METRE", 0, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, L"METRE_METRE", 1, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, L"METRE_METRE", 112.85, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"METRE_METRE", -1162.5, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (-1, L"METRE_METRE", -1, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (0, L"METRE_METRE", 0, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (1, L"METRE_METRE", 1, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"METRE_METRE", 112.85, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"METRE_METRE", -1.1625, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, L"METRE_METRE", -0.001, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, L"METRE_METRE", 0, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, L"METRE_METRE", 0.001, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, L"METRE_METRE", 0.11285, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"METRE_METRE", -4.57677165354331E+04, L"INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"METRE_METRE", -3.93700787401575E+01, L"INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (0, L"METRE_METRE", 0, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (1, L"METRE_METRE", 3.93700787401575E+01, L"INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, L"METRE_METRE", 4.44291338582677E+03, L"INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"METRE_METRE", -7.22344010975901E+02, L"MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"METRE_METRE", -6.21371192237334E-01, L"MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (0, L"METRE_METRE", 0, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (1, L"METRE_METRE", 6.21371192237334E-01, L"MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, L"METRE_METRE", 7.01217390439831E+01, L"MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"METRE_KILOMETRE", -2.84387405896025E+04, L"INCH_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"METRE_KILOMETRE", -2.44634327652494E+01, L"INCH_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (0, L"METRE_KILOMETRE", 0, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (1, L"METRE_KILOMETRE", 2.44634327652494E+01, L"INCH_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, L"METRE_KILOMETRE", 2.7606983875584E+03, L"INCH_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"METRE_KILOMETRE", -150156550.31310064824132043958274, L"INCH_FOOT", 1.0e-9); // Value from Windows Calc
    TestUnitConversion (-1, L"METRE_KILOMETRE", -1.29166925000517E+05, L"INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (0, L"METRE_KILOMETRE", 0, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (1, L"METRE_KILOMETRE", 1.29166925000517E+05, L"INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, L"METRE_KILOMETRE", 14576487.486308308089490762672612, L"INCH_FOOT", 1.0e-8); // Value from Windows Calc
    TestUnitConversion (-1162.5, L"METRE_KILOMETRE", -2.36989504913354E+03, L"FOOT_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"METRE_KILOMETRE", -2.03861939710412, L"FOOT_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (0, L"METRE_KILOMETRE", 0, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (1, L"METRE_KILOMETRE", 2.03861939710412, L"FOOT_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, L"METRE_KILOMETRE", 2.300581989632E+02, L"FOOT_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"METRE_KILOMETRE", -12513045.859425054020110036631895, L"FOOT_FOOT", 1.0e-8); // Value from Windows Calc
    TestUnitConversion (-1, L"METRE_KILOMETRE", -1.07639104167098E+04, L"FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (0, L"METRE_KILOMETRE", 0, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (1, L"METRE_KILOMETRE", 1.07639104167098E+04, L"FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, L"METRE_KILOMETRE", 1214707.2905256923407908968893844, L"FOOT_FOOT", 1.0e-8); // Value from Windows Calc
    TestUnitConversion (-1162.5, L"METRE_KILOMETRE", -1162500000, L"MILLIMETRE_METRE", 1.0e-6);
    TestUnitConversion (-1, L"METRE_KILOMETRE", -1000000, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (0, L"METRE_KILOMETRE", 0, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (1, L"METRE_KILOMETRE", 1000000, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"METRE_KILOMETRE", 112850000, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"METRE_KILOMETRE", -1162500, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, L"METRE_KILOMETRE", -1000, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, L"METRE_KILOMETRE", 0, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, L"METRE_KILOMETRE", 1000, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, L"METRE_KILOMETRE", 112850, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"METRE_KILOMETRE", -1162500, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (-1, L"METRE_KILOMETRE", -1000, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (0, L"METRE_KILOMETRE", 0, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (1, L"METRE_KILOMETRE", 1000, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"METRE_KILOMETRE", 112850, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"METRE_KILOMETRE", -1162.5, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, L"METRE_KILOMETRE", -1, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, L"METRE_KILOMETRE", 0, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, L"METRE_KILOMETRE", 1, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, L"METRE_KILOMETRE", 112.85, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"METRE_KILOMETRE", -4.57677165354332000E+07, L"INCH_METRE", 1.0e-6); // Value from excel
    TestUnitConversion (-1, L"METRE_KILOMETRE", -3.93700787401576E+04, L"INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (0, L"METRE_KILOMETRE", 0, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (1, L"METRE_KILOMETRE", 3.93700787401576E+04, L"INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, L"METRE_KILOMETRE", 4.44291338582678000E+06, L"INCH_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"METRE_KILOMETRE", -7.22344010975903000E+05, L"MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"METRE_KILOMETRE", -6.21371192237336000E+02, L"MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (0, L"METRE_KILOMETRE", 0, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (1, L"METRE_KILOMETRE", 6.21371192237336000E+02, L"MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, L"METRE_KILOMETRE", 7.01217390439833000E+04, L"MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"INCH_METRE", -7.22344010975901000E-01, L"INCH_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"INCH_METRE", -0.00062137119, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (0, L"INCH_METRE", 0, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (1, L"INCH_METRE", 0.00062137119, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_METRE", 0.070121739, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, L"INCH_METRE", -3.81397637795276000E+03, L"INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"INCH_METRE", -3.2808399, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (0, L"INCH_METRE", 0, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (1, L"INCH_METRE", 3.2808399, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_METRE", 3.70242782152231000E+02, L"INCH_FOOT", 1.0e-8); // Valu from excel
    TestUnitConversion (-1162.5, L"INCH_METRE", -0.060195334, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1, L"INCH_METRE", -5.1780932E-05, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (0, L"INCH_METRE", 0, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (1, L"INCH_METRE", 5.1780932E-05, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_METRE", 0.0058434782, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, L"INCH_METRE", -3.17831364829396000E+02, L"FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"INCH_METRE", -0.27340332, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (0, L"INCH_METRE", 0, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (1, L"INCH_METRE", 0.27340332, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_METRE", 3.08535651793526000E+01, L"FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"INCH_METRE", -29527.5, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1, L"INCH_METRE", -25.4, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (0, L"INCH_METRE", 0, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (1, L"INCH_METRE", 25.4, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_METRE", 2866.39, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"INCH_METRE", -29.5275, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, L"INCH_METRE", -0.0254, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, L"INCH_METRE", 0, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, L"INCH_METRE", 0.0254, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_METRE", 2.86639, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"INCH_METRE", -29.5275, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (-1, L"INCH_METRE", -0.0254, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (0, L"INCH_METRE", 0, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (1, L"INCH_METRE", 0.0254, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_METRE", 2.86639, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"INCH_METRE", -0.0295275, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, L"INCH_METRE", -2.54E-05, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, L"INCH_METRE", 0, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, L"INCH_METRE", 2.54E-05, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_METRE", 0.00286639, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"INCH_METRE", -1162.5, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (-1, L"INCH_METRE", -1, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (0, L"INCH_METRE", 0, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (1, L"INCH_METRE", 1, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_METRE", 112.85, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"INCH_METRE", -1.83475378787879000E+01, L"MILLIMETRE_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"INCH_METRE", -0.015782828, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (0, L"INCH_METRE", 0, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (1, L"INCH_METRE", 0.015782828, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (112.85, L"INCH_METRE", 1.78109217, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, L"MILLIMETRE_MILE", -4.57677165354331000E+01, L"INCH_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"MILLIMETRE_MILE", -0.039370079, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_MILE", 0, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_MILE", 0.039370079, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_MILE", 4.44291339, L"INCH_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, L"MILLIMETRE_MILE", -2.41653543307087000E+05, L"INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"MILLIMETRE_MILE", -2.07874015748031000E+02, L"INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (0, L"MILLIMETRE_MILE", 0, L"INCH_FOOT", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_MILE", 2.07874015748031000E+02, L"INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, L"MILLIMETRE_MILE", 2.34585826771654000E+04, L"INCH_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"MILLIMETRE_MILE", -3.81397637795275, L"FOOT_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"MILLIMETRE_MILE", -3.28083989501312000E-03, L"FOOT_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (0, L"MILLIMETRE_MILE", 0, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_MILE", 3.28083989501312000E-03, L"FOOT_MILE", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, L"MILLIMETRE_MILE", 0.37024278, L"FOOT_MILE", 1.0e-8);
    TestUnitConversion (-1162.5, L"MILLIMETRE_MILE", -2.01377952755906000E+04, L"FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"MILLIMETRE_MILE", -1.73228346456693000E+01, L"FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (0, L"MILLIMETRE_MILE", 0, L"FOOT_FOOT", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_MILE", 1.73228346456693000E+01, L"FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (112.85, L"MILLIMETRE_MILE", 1.95488188976378000E+03, L"FOOT_FOOT", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"MILLIMETRE_MILE", -1870862.4, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (-1, L"MILLIMETRE_MILE", -1609.344, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_MILE", 0, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_MILE", 1609.344, L"MILLIMETRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_MILE", 1.816144704E+05, L"MILLIMETRE_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"MILLIMETRE_MILE", -1870.8624, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (-1, L"MILLIMETRE_MILE", -1.609344, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_MILE", 0, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_MILE", 1.609344, L"MILLIMETRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_MILE", 1.81614470400000000E+02, L"MILLIMETRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"MILLIMETRE_MILE", -1870.8624, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (-1, L"MILLIMETRE_MILE", -1.609344, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_MILE", 0, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_MILE", 1.609344, L"METRE_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_MILE", 1.81614470400000000E+02, L"METRE_METRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"MILLIMETRE_MILE", -1.8708624, L"METRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1, L"MILLIMETRE_MILE", -0.001609344, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_MILE", 0, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_MILE", 0.001609344, L"METRE_KILOMETRE", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_MILE", 1.81614470400000000E-01, L"METRE_KILOMETRE", 1.0e-8); // Value from excel
    TestUnitConversion (-1162.5, L"MILLIMETRE_MILE", -73656, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (-1, L"MILLIMETRE_MILE", -63.36, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_MILE", 0, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_MILE", 63.36, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_MILE", 7150.176, L"INCH_METRE", 1.0e-8);
    TestUnitConversion (-1162.5, L"MILLIMETRE_MILE", -1162.5, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (-1, L"MILLIMETRE_MILE", -1, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (0, L"MILLIMETRE_MILE", 0, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (1, L"MILLIMETRE_MILE", 1, L"MILLIMETRE_MILE", 1.0e-8);
    TestUnitConversion (112.85, L"MILLIMETRE_MILE", 112.85, L"MILLIMETRE_MILE", 1.0e-8);

    // L3
    // All results calculated in excel and double checked at http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"ACRE_FOOT", 1.0, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1.0, L"ACRE_INCH", 8.33333333333333e-02, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1000.0, L"CENTIMETRE_CUBED", 8.10713193789912e-07, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100.0, L"FOOT_CUBED", 2.29568411386593e-03, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100.0, L"GALLON", 3.06888327721661e-04, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100.0, L"GALLON_IMPERIAL", 3.68557514315638e-04, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1000.0, L"INCH_CUBED", 1.32852089922797e-05, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100.0, L"LITRE", 8.10713193789912e-05, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100.0, L"METRE_CUBED", 8.10713193789912e-02, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1.0, L"MILLION_GALLON", 3.06888327721661, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1.0, L"MILLION_LITRE", 8.10713193789912e-01, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1.0, L"THOUSAND_GALLON", 3.06888327721661e-03, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100.0, L"THOUSAND_LITRE", 8.10713193789912e-02, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100.0, L"YARD_CUBED", 6.19834710743802e-02, L"ACRE_FOOT", 1.0e-8);

    TestUnitConversion (1000000.0, L"ACRE_FOOT", 1.0e6, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1000000.0, L"ACRE_INCH", 8.33333333333333e4, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1000000000.0, L"CENTIMETRE_CUBED", 8.10713193789912e-01, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100000000.0, L"FOOT_CUBED", 2.29568411386593e3, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100000000.0, L"GALLON", 3.06888327721661e2, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100000000.0, L"GALLON_IMPERIAL", 3.68557514315638e2, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1000000000.0, L"INCH_CUBED", 1.32852089922797e1, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100000000.0, L"LITRE", 8.10713193789912e1, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100000000.0, L"METRE_CUBED", 8.10713193789912e4, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1000000.0, L"MILLION_GALLON", 3.06888327721661e6, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1000000.0, L"MILLION_LITRE", 8.10713193789912e5, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (1000000.0, L"THOUSAND_GALLON", 3.06888327721661e3, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100000000.0, L"THOUSAND_LITRE", 8.10713193789912e4, L"ACRE_FOOT", 1.0e-8);
    TestUnitConversion (100000000.0, L"YARD_CUBED", 6.19834710743802e4, L"ACRE_FOOT", 1.0e-8);

    // Mass
    // All results calculated in excel and double checked at http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"GRAIN", 1.42857142857143e-4, L"POUND", 1.0e-8);
    TestUnitConversion (1.0, L"GRAM", 2.20462262184878e-3, L"POUND", 1.0e-8);
    TestUnitConversion (1.0, L"KILOGRAM", 2.20462262184878, L"POUND", 1.0e-8);
    TestUnitConversion (1.0, L"LONG_TON", 2240, L"POUND", 1.0e-8);
    TestUnitConversion (1.0, L"MEGAGRAM", 2204.62262184878, L"POUND", 1.0e-8);
    TestUnitConversion (1000.0, L"MICROGRAM", 2.20462262184878e-6, L"POUND", 1.0e-8);
    TestUnitConversion (1000.0, L"MILLIGRAM", 2.20462262184878e-3, L"POUND", 1.0e-8);
    TestUnitConversion (1.0, L"POUND", 1.0, L"POUND", 1.0e-8);
    TestUnitConversion (1.0, L"SHORT_TON", 2000.0, L"POUND", 1.0e-8);

    TestUnitConversion (1000000.0, L"GRAIN", 1.42857142857143e2, L"POUND", 1.0e-8);
    TestUnitConversion (1000000.0, L"GRAM", 2.20462262184878e3, L"POUND", 1.0e-8);
    TestUnitConversion (1000000.0, L"KILOGRAM", 2204622.62184878, L"POUND", 1.0e-8);
    TestUnitConversion (1000000.0, L"LONG_TON", 2240000000, L"POUND", 1.0e-8);
    TestUnitConversion (1000000.0, L"MEGAGRAM", 2204622621.84878, L"POUND", 1.0e-5); // Does not pass w/ 1.0e-8
    TestUnitConversion (1000000000.0, L"MICROGRAM", 2.20462262184878, L"POUND", 1.0e-8);
    TestUnitConversion (1000000000.0, L"MILLIGRAM", 2.20462262184878e3, L"POUND", 1.0e-8);
    TestUnitConversion (1000000.0, L"POUND", 1000000.0, L"POUND", 1.0e-8);
    TestUnitConversion (1000000.0, L"SHORT_TON", 2000000000.0, L"POUND", 1.0e-6); // Does not pass w/ 1.0e-8

    // Time
    // All results calculated in excel and double checked at http://online.unitconverterpro.com/conversion-tables/convert-alpha/factors.php?cat=time&unit=29&val=
    TestUnitConversion (10000.0, L"MILLISECOND", 3.17097919837647e-7, L"YEAR", 1.0e-8);
    TestUnitConversion (10000.0, L"SECOND", 3.17097919837647e-4, L"YEAR", 1.0e-8);
    TestUnitConversion (1.0, L"MINUTE", 1.90258751902588e-06, L"YEAR", 1.0e-8);
    TestUnitConversion (1.0, L"HOUR", 1.14155251141553e-04, L"YEAR", 1.0e-8);
    TestUnitConversion (1.0, L"DAY", 2.73972602739727e-03, L"YEAR", 1.0e-8);
    TestUnitConversion (1.0, L"YEAR", 1.0, L"YEAR", 1.0e-8);

    TestUnitConversion (1000000000.0, L"MILLISECOND", 3.17097919837647e-2, L"YEAR", 1.0e-8);
    TestUnitConversion (100000000.0, L"SECOND", 3.17097919837647, L"YEAR", 1.0e-8);
    TestUnitConversion (1000000.0, L"MINUTE", 1.90258751902588, L"YEAR", 1.0e-8);
    TestUnitConversion (1000000.0, L"HOUR", 114.155251141553, L"YEAR", 1.0e-8);
    TestUnitConversion (1000000.0, L"DAY", 2739.72602739727, L"YEAR", 1.0e-8);
    TestUnitConversion (1000000.0, L"YEAR", 1000000.0, L"YEAR", 1.0e-8);
    

    // Plane Angle
    // All results calculated in excel and double checked at http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"DEGREE", 1.0, L"DEGREE", 1.0e-8);
    TestUnitConversion (1.0, L"GRADIAN", 0.9, L"DEGREE", 1.0e-8);
    TestUnitConversion (1.0, L"ANGLE_MINUTE", 1.66666666666667e-02, L"DEGREE", 1.0e-8);
    TestUnitConversion (1.0, L"ANGLE_QUADRANT", 90, L"DEGREE", 1.0e-8);
    TestUnitConversion (1.0, L"RADIAN", 5.72957795130823e1, L"DEGREE", 1.0e-8);
    TestUnitConversion (1.0, L"REVOLUTION", 360.0, L"DEGREE", 1.0e-8);
    TestUnitConversion (1000.0, L"ANGLE_SECOND", 2.77777777777778e-1, L"DEGREE", 1.0e-8);

    TestUnitConversion (1000000.0, L"DEGREE", 1000000.0, L"DEGREE", 1.0e-8);
    TestUnitConversion (1000000.0, L"GRADIAN", 900000, L"DEGREE", 1.0e-8);
    TestUnitConversion (1000000.0, L"ANGLE_MINUTE", 1.66666666666667e4, L"DEGREE", 1.0e-8);
    TestUnitConversion (1000000.0, L"ANGLE_QUADRANT", 90000000, L"DEGREE", 1.0e-8);
    TestUnitConversion (1000000.0, L"RADIAN", 5.72957795130823e7, L"DEGREE", 1.0e-7);
    TestUnitConversion (1000000.0, L"REVOLUTION", 360000000.0, L"DEGREE", 1.0e-8);
    TestUnitConversion (1000000000.0, L"ANGLE_SECOND", 2.77777777777778e5, L"DEGREE", 1.0e-8);

    // K
    // Test Absolute temperatures
    // All results calculated at http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    // and double checked at http://www.chemie.fu-berlin.de/chemistry/general/units_en.html#temp
    TestUnitConversion (1.0, L"DEGREE_CELSIUS", 493.47, L"DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (1.0, L"DEGREE_KELVIN", 1.8, L"DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (1.0, L"DEGREE_FAHRENHEIT", 460.67, L"DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (1.0, L"DEGREE_RANKINE", 0.55555555556, L"DEGREE_KELVIN", 1.0e-8);

    TestUnitConversion (100.0, L"DEGREE_CELSIUS", 671.67, L"DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (100.0, L"DEGREE_KELVIN", 180.0, L"DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (100.0, L"DEGREE_FAHRENHEIT", 559.67, L"DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (100.0, L"DEGREE_RANKINE", 55.55555555556, L"DEGREE_KELVIN", 1.0e-8);

    TestUnitConversion (-50.0, L"DEGREE_CELSIUS", 401.67, L"DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (-50.0, L"DEGREE_KELVIN", -90.0, L"DEGREE_RANKINE", 1.0e-8); // Now that's what I call COLD
    TestUnitConversion (-50.0, L"DEGREE_FAHRENHEIT", 409.67, L"DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (-50.0, L"DEGREE_RANKINE", -27.777777778, L"DEGREE_KELVIN", 1.0e-8); // This is only a little nippy

    // Test Delta temperatures
    // Not Verified in unit converter.
    TestUnitConversion (1.0, L"DELTA_DEGREE_CELSIUS", 1.8, L"DELTA_DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (1.0, L"DELTA_DEGREE_KELVIN", 1.8, L"DELTA_DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (1.0, L"DELTA_DEGREE_FAHRENHEIT", 1.0, L"DELTA_DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (1.0, L"DELTA_DEGREE_RANKINE", 0.55555555556, L"DELTA_DEGREE_KELVIN", 1.0e-8);

    TestUnitConversion (100.0, L"DELTA_DEGREE_CELSIUS", 180.0, L"DELTA_DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (100.0, L"DELTA_DEGREE_KELVIN", 180.0, L"DELTA_DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (100.0, L"DELTA_DEGREE_FAHRENHEIT", 100.0, L"DELTA_DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (100.0, L"DELTA_DEGREE_RANKINE", 55.55555555556, L"DELTA_DEGREE_KELVIN", 1.0e-8);

    TestUnitConversion (-50.0, L"DELTA_DEGREE_CELSIUS", -90.0, L"DELTA_DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (-50.0, L"DELTA_DEGREE_KELVIN", -90.0, L"DELTA_DEGREE_RANKINE", 1.0e-8); // Now that's what I call COLD
    TestUnitConversion (-50.0, L"DELTA_DEGREE_FAHRENHEIT", -50.0, L"DELTA_DEGREE_RANKINE", 1.0e-8);
    TestUnitConversion (-50.0, L"DELTA_DEGREE_RANKINE", -27.777777778, L"DELTA_DEGREE_KELVIN", 1.0e-8); // This is only a little nippy

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"CENTIMETRE_PER_HOUR", 1.0, L"CENTIMETRE_PER_HOUR", 1.0e-8);

    TestUnitConversion (1.0, L"CENTIMETRE_PER_MINUTE", 60, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, L"CENTIMETRE_PER_SECOND", 3600, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, L"FOOT_PER_HOUR", 30.48, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, L"FOOT_PER_MINUTE", 30.48 * 60, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, L"FOOT_PER_SECOND", 30.48 * 3600, L"CENTIMETRE_PER_HOUR", 1.0e-8);

    TestUnitConversion (1.0, L"INCH_PER_HOUR", 2.54, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, L"INCH_PER_MINUTE", 2.54 * 60, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, L"INCH_PER_SECOND", 2.54 * 3600, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, L"KILOMETRE_PER_HOUR", 100000, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, L"KNOT_INTERNATIONAL", 1.852e5, L"CENTIMETRE_PER_HOUR", 1.0e-8); // Value from excel

    TestUnitConversion (1.0, L"KNOT", 1853.184 * 100, L"CENTIMETRE_PER_HOUR", 1.0e-8); // value checked at http://www.unitconversion.org/velocity/knots-uk-to-centimeters-per-hour-conversion.html
    TestUnitConversion (1.0, L"METRE_PER_HOUR", 100, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, L"METRE_PER_MINUTE", 6000, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, L"METRE_PER_SECOND", 360000, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, L"MILE_PER_HOUR", 30.48 * 5280, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    
    TestUnitConversion (1.0, L"CENTIMETRE_PER_DAY", 1.0 / 24.0, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, L"FOOT_PER_DAY", 30.48 / 24.0, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, L"INCH_PER_DAY", 2.54 / 24.0, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, L"METRE_PER_DAY", 100.0 / 24.0, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, L"MILLIMETRE_PER_DAY", 0.1 / 24.0, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    
    TestUnitConversion (1.0, L"MILLIMETRE_PER_HOUR", 0.1, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, L"MILLIMETRE_PER_MINUTE", 6.0, L"CENTIMETRE_PER_HOUR", 1.0e-8);

    TestUnitConversion (1.0e6, L"CENTIMETRE_PER_MINUTE", 60e6, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, L"CENTIMETRE_PER_SECOND", 3600e6, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, L"FOOT_PER_HOUR", 30.48e6, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, L"FOOT_PER_MINUTE", 30.48 * 60e6, L"CENTIMETRE_PER_HOUR", 1.0e-6);
    TestUnitConversion (1.0e6, L"FOOT_PER_SECOND", 30.48 * 3600e6, L"CENTIMETRE_PER_HOUR", 1.0e-4);

    TestUnitConversion (1.0e6, L"INCH_PER_HOUR", 2.54e6, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, L"INCH_PER_MINUTE", 2.54 * 60e6, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, L"INCH_PER_SECOND", 2.54 * 3600e6, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, L"KILOMETRE_PER_HOUR", 1e11, L"CENTIMETRE_PER_HOUR", 1.0e-4);
    TestUnitConversion (1.0e6, L"KNOT_INTERNATIONAL", 1.852e11, L"CENTIMETRE_PER_HOUR", 1.0e-4); // Value from excel

    TestUnitConversion (1.0e6, L"KNOT", 1853.184 * 100 * 1e6, L"CENTIMETRE_PER_HOUR", 1.0e-5); // value checked at http://www.unitconversion.org/velocity/knots-uk-to-centimeters-per-hour-conversion.html
    TestUnitConversion (1.0e6, L"METRE_PER_HOUR", 1e8, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, L"METRE_PER_MINUTE", 6.0e9, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, L"METRE_PER_SECOND", 36e10, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, L"MILE_PER_HOUR", 1.0e6 * 30.48 * 5280, L"CENTIMETRE_PER_HOUR", 1.0e-4);

    TestUnitConversion (1.0e6, L"CENTIMETRE_PER_DAY", 1.0e6 / 24.0, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, L"FOOT_PER_DAY", 30.48e6 / 24.0, L"CENTIMETRE_PER_HOUR", 1.0e-2);
    TestUnitConversion (1.0e6, L"INCH_PER_DAY", 2.54e6 / 24.0, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, L"METRE_PER_DAY", 1.0e8 / 24.0, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, L"MILLIMETRE_PER_DAY", 1.0e5 / 24.0, L"CENTIMETRE_PER_HOUR", 1.0e-8);

    TestUnitConversion (1.0e6, L"MILLIMETRE_PER_HOUR", 1.0e5, L"CENTIMETRE_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, L"MILLIMETRE_PER_MINUTE", 6.0e6, L"CENTIMETRE_PER_HOUR", 1.0e-8);

    TestUnitConversion (1.0, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8);

    TestUnitConversion (1.0, L"FOOT_CUBED_PER_FOOT_SQUARED_PER_SECOND", 4.356e4, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, L"FOOT_CUBED_PER_MILE_SQUARED_PER_SECOND", 1.5625e-3, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e3, L"GALLON_PER_ACRE_PER_DAY", 1.54722865226337e-3, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, L"GALLON_PER_ACRE_PER_MINUTE", 2.22800925925925e-3, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, L"LITRE_PER_METRE_SQUARED_PER_SECOND", 1.42913385826772e2, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel

    TestUnitConversion (1.0, L"GALLON_PER_FOOT_SQUARED_PER_DAY", 6.73972800925924e-2, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, L"GALLON_PER_FOOT_SQUARED_PER_MINUTE", 9.70520833333332e1, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e6, L"GALLON_PER_MILE_SQUARED_PER_DAY", 2.41754476916152e-3, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e3, L"GALLON_PER_MILE_SQUARED_PER_MINUTE", 3.48126446759259e-3, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel

    TestUnitConversion (1.0e4, L"LITRE_PER_HECTARE_PER_DAY", 1.65409011373578e-3, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e6, L"LITRE_PER_KILOMETRE_SQUARED_PER_DAY", 1.65409011373578e-3, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, L"LITRE_PER_METRE_SQUARED_PER_DAY", 1.65409011373578e-3, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e1, L"METRE_CUBED_PER_HECTARE_PER_DAY", 1.65409011373578e-3, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel

    TestUnitConversion (1.0e3, L"METRE_CUBED_PER_KILOMETRE_SQUARED_PER_DAY", 1.65409011373578e-3, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, L"METRE_CUBED_PER_METRE_SQUARED_PER_DAY", 1.65409011373578, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, L"FOOT_CUBED_PER_FOOT_SQUARED_PER_MINUTE", 7.25999999999998e2, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel

    TestUnitConversion (1.0e6, L"FOOT_CUBED_PER_FOOT_SQUARED_PER_SECOND", 4.356e10, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-5); // Value from excel
    TestUnitConversion (1.0e6, L"FOOT_CUBED_PER_MILE_SQUARED_PER_SECOND", 1.5625e3, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e9, L"GALLON_PER_ACRE_PER_DAY", 1.54722865226337e3, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e6, L"GALLON_PER_ACRE_PER_MINUTE", 2.22800925925925e3, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e6, L"LITRE_PER_METRE_SQUARED_PER_SECOND", 1.42913385826772e8, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-6); // Value from excel

    TestUnitConversion (1.0e6, L"GALLON_PER_FOOT_SQUARED_PER_DAY", 6.73972800925924e4, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e6, L"GALLON_PER_FOOT_SQUARED_PER_MINUTE", 9.70520833333332e7, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-6); // Value from excel
    TestUnitConversion (1.0e12, L"GALLON_PER_MILE_SQUARED_PER_DAY", 2.41754476916152e3, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e9, L"GALLON_PER_MILE_SQUARED_PER_MINUTE", 3.48126446759259e3, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e10, L"LITRE_PER_HECTARE_PER_DAY", 1.65409011373578e3, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel

    TestUnitConversion (1.0e10, L"LITRE_PER_HECTARE_PER_DAY", 1.65409011373578e3, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e12, L"LITRE_PER_KILOMETRE_SQUARED_PER_DAY", 1.65409011373578e3, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e6, L"LITRE_PER_METRE_SQUARED_PER_DAY", 1.65409011373578e3, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e7, L"METRE_CUBED_PER_HECTARE_PER_DAY", 1.65409011373578e3, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel

    TestUnitConversion (1.0e9, L"METRE_CUBED_PER_KILOMETRE_SQUARED_PER_DAY", 1.65409011373578e3, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e6, L"METRE_CUBED_PER_METRE_SQUARED_PER_DAY", 1.65409011373578e6, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e6, L"FOOT_CUBED_PER_FOOT_SQUARED_PER_MINUTE", 7.25999999999998e8, L"FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e-5); // Value from excel

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"GIGANEWTON_SECOND_PER_KILOGRAM", 1.0, L"GIGANEWTON_SECOND_PER_KILOGRAM", 1.0e-8);
    TestUnitConversion (1.0, L"INCH_SQUARED_INCH_HG_CONVENTIONAL_HOUR_PER_GRAIN_MASS", 1.21377748762502e-1, L"GIGANEWTON_SECOND_PER_KILOGRAM", 1.0e-8);

    TestUnitConversion (1.0e6, L"GIGANEWTON_SECOND_PER_KILOGRAM", 1.0e6, L"GIGANEWTON_SECOND_PER_KILOGRAM", 1.0e-8);
    TestUnitConversion (1.0e6, L"INCH_SQUARED_INCH_HG_CONVENTIONAL_HOUR_PER_GRAIN_MASS", 1.21377748762502e5, L"GIGANEWTON_SECOND_PER_KILOGRAM", 1.0e-8);

    TestUnitConversion (1.0, L"CENTISTOKE", 1.0, L"CENTISTOKE", 1.0e-8);
    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    // because stoke was not included in the online calculator the following was used in its place:
    // (m^2/s)*1.0e-6
    // 1 Stoke = 1.0e-4 m^2/s (NIST doc), 1 CentiStoke = .01 Stokes = 1.0e-2 Stokes
    // 1 CentiStoke = 1.0e-6 m^2/s
    TestUnitConversion (1.0, L"FOOT_SQUARED_PER_SECOND", 9.29030400000002e4, L"CENTISTOKE", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, L"METRE_SQUARED_PER_SECOND", 1.0e6, L"CENTISTOKE", 1.0e-8);
    TestUnitConversion (1.0, L"STOKE", 100.0, L"CENTISTOKE", 1.0e-8);


    TestUnitConversion (1.0e6, L"FOOT_SQUARED_PER_SECOND", 9.29030400000002e10, L"CENTISTOKE", 1.0e-3); // Value from excel
    TestUnitConversion (1.0e6, L"METRE_SQUARED_PER_SECOND", 1.0e12, L"CENTISTOKE", 1.0e-8);
    TestUnitConversion (1.0e6, L"STOKE", 1.0e8, L"CENTISTOKE", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (100, L"GALLON_PER_MINUTE", 4.41919191919192000E-01, L"ACRE_FOOT_PER_DAY", 1.0e-8); // Value from excel
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 2.27220117845118000E-01, L"ACRE_FOOT_PER_HOUR", 1.0e-8); // Value from excel
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 3.78700196408530000E-03, L"ACRE_FOOT_PER_MINUTE", 1.0e-8); // Value from excel
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 2.72664141414141, L"ACRE_INCH_PER_HOUR", 1.0e-8); // Value from excel
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 4.54440235690236000E-02, L"ACRE_INCH_PER_MINUTE", 1.0e-8); // Value from excel
    
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 1.64961805555556000E+02, L"FOOT_CUBED_PER_MINUTE", 1.0e-8); // Value from excel
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 2.74936342592593, L"FOOT_CUBED_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 2.37545E+05, L"FOOT_CUBED_PER_DAY", 1.0e-8); // Value from excel
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 2.80271888487360000E+02, L"METRE_CUBED_PER_HOUR", 1.0e-8); // Value from excel
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 1.47962871911833000E+06, L"GALLON_IMPERIAL_PER_DAY", 1.0e-8); // Value from excel
    
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 1.02751994383218000E+03, L"GALLON_IMPERIAL_PER_MINUTE", 1.0e-8); // Value from excel
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 1.71253323972029000E+01, L"GALLON_IMPERIAL_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 1.77696E+06, L"GALLON_PER_DAY", 1.0e-8); // Value from excel
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 1234, L"GALLON_PER_MINUTE", 1.0e-8); // Value from excel
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 2.05666666666666000E+01, L"GALLON_PER_SECOND", 1.0e-8); // Value from excel
    
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 6.72652532369664000E+06, L"LITRE_PER_DAY", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, L"GALLON_PER_MINUTE", 3.785411784, L"LITRE_PER_MINUTE", 1.0e-8); // Value from excel
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 4.67119814145600000E+03, L"LITRE_PER_MINUTE", 1.0e-8); // Value from excel
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 7.78533023576000000E+01, L"LITRE_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 6.72652532369664, L"MEGA_LITRE_PER_DAY", 1.0e-8); // Value from excel
    
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 6.72652532369664000E+03, L"METRE_CUBED_PER_DAY", 1.0e-8); // Value from excel
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 4.671198141456, L"METRE_CUBED_PER_MINUTE", 1.0e-8); // Value from excel
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 7.78533023576000000E-02, L"METRE_CUBED_PER_SECOND", 1.0e-8); // Value from excel
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 1.77696, L"MILLION_GALLON_PER_DAY", 1.0e-8); // Value from excel
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 1.47962871911833, L"MILLION_GALLON_IMPERIAL_PER_DAY", 1.0e-8); // Value from excel
    
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 6.72652532369664, L"MILLION_LITRE_PER_DAY", 1.0e-8); // Value from excel
    TestUnitConversion (1234, L"GALLON_PER_MINUTE", 2.8027188848736e5, L"LITRE_PER_HOUR", 1.0e-8); // Value from excel

    TestUnitConversion (1234, L"LITRE_PER_SECOND_PER_PERSON", 2.816538995808e7, L"GALLON_PER_DAY_PER_PERSON", 1.0e-8); // Value from excel
    TestUnitConversion (1234e6, L"LITRE_PER_SECOND_PER_PERSON", 2.816538995808e13, L"GALLON_PER_DAY_PER_PERSON", 1.0e-8); // Value from excel
    
    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    // Some expected values are from NIST, others are calculated, calculated values are noted
    TestUnitConversion (1.0, L"DYNE", 1.0, L"DYNE", 1.0e-8);
    TestUnitConversion (1.0, L"KILOGRAM_FORCE", 9.80665 / 1.0e-5, L"DYNE", 1.0e-8);
    TestUnitConversion (1.0, L"KILONEWTON", 1000 / 1.0e-5, L"DYNE", 1.0e-5);
    TestUnitConversion (1.0, L"KILOPOUND_FORCE", 4.4482216152605e8, L"DYNE", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, L"LONG_TON_FORCE", 9.96401641818353e8, L"DYNE", 1.0e-5); // Value calculated in excel

    TestUnitConversion (1.0, L"MILLINEWTON", 0.001 / 1.0e-5, L"DYNE", 1.0e-8);
    TestUnitConversion (1.0, L"NEWTON", 1.0 / 1.0e-5, L"DYNE", 1.0e-8);
    TestUnitConversion (1.0, L"POUND_FORCE", 4.4482216152605e5, L"DYNE", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, L"SHORT_TON_FORCE", 8.896443230521e8, L"DYNE", 1.0e-8); // Value calculated in excel

    TestUnitConversion (1.0e6, L"DYNE", 1.0e6, L"DYNE", 1.0e-8);
    TestUnitConversion (1.0e6, L"KILOGRAM_FORCE", 9.80665e6 / 1.0e-5, L"DYNE", 1.0e-3);
    TestUnitConversion (1.0e6, L"KILONEWTON", 1000e11, L"DYNE", 1.0e-6);
    TestUnitConversion (1.0e6, L"KILOPOUND_FORCE", 4.4482216152605e14, L"DYNE", 0.1); // Value calculated in excel
    TestUnitConversion (1.0e6, L"LONG_TON_FORCE", 9.96401641818353e14, L"DYNE", 10); // Value calculated in excel, large difference in expected value is due to the limits of excels precision

    TestUnitConversion (1.0e6, L"MILLINEWTON", 1.0e8, L"DYNE", 1.0e-7);
    TestUnitConversion (1.0e6, L"NEWTON", 1.0e11, L"DYNE", 1.0e-8);
    TestUnitConversion (1.0e6, L"POUND_FORCE", 4.4482216152605e11, L"DYNE", 1.0e-4); // Value calculated in excel
    TestUnitConversion (1.0e6, L"SHORT_TON_FORCE", 8.896443230521e14, L"DYNE", 1); // Value calculated in excel

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"WATT_PER_METRE", 1.0, L"WATT_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0, L"BTU_PER_HOUR_PER_FOOT", 9.61519259095214e-1, L"WATT_PER_METRE", 1.0e-8);

    TestUnitConversion (1.0e6, L"WATT_PER_METRE", 1.0e6, L"WATT_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0e6, L"BTU_PER_HOUR_PER_FOOT", 9.61519259095214e5, L"WATT_PER_METRE", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"WATT_PER_METRE_PER_DEGREE_KELVIN", 1.0, L"WATT_PER_METRE_PER_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0, L"WATT_PER_METRE_PER_DEGREE_CELSIUS", 1.0, L"WATT_PER_METRE_PER_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0, L"BTU_INCH_PER_FOOT_SQUARED_PER_HOUR_PER_DEGREE_FAHRENHEIT", 1.44227888864283e-1, L"WATT_PER_METRE_PER_DEGREE_KELVIN", 1.0e-8);

    TestUnitConversion (1.0e6, L"WATT_PER_METRE_PER_DEGREE_KELVIN", 1.0e6, L"WATT_PER_METRE_PER_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0e6, L"WATT_PER_METRE_PER_DEGREE_CELSIUS", 1.0e6, L"WATT_PER_METRE_PER_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0e6, L"BTU_INCH_PER_FOOT_SQUARED_PER_HOUR_PER_DEGREE_FAHRENHEIT", 1.44227888864283e5, L"WATT_PER_METRE_PER_DEGREE_KELVIN", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"FOOT_POUNDAL", 1.0, L"FOOT_POUNDAL", 1.0e-8);

    TestUnitConversion (1.0, L"JOULE", 2.37303604042319e1, L"FOOT_POUNDAL", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, L"KILOJOULE", 2.37303604042319e4, L"FOOT_POUNDAL", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, L"KILOWATT_HOUR", 8.54292974552351e7, L"FOOT_POUNDAL", 1.0e-6); // Value calculated in excel
    TestUnitConversion (1.0, L"BTU", 2.50368556292668e4, L"FOOT_POUNDAL", 1e-8);
    TestUnitConversion (1.0, L"GIGAJOULE", 2.37303604042319e10, L"FOOT_POUNDAL", 1e-4);
    
    TestUnitConversion (1.0, L"KILOBTU", 2.50368556292668e7, L"FOOT_POUNDAL", 1e-6);
    TestUnitConversion (1.0, L"GIGAWATT_HOUR", 8.54292974552351e13, L"FOOT_POUNDAL", 1.0e1);
    TestUnitConversion (1.0, L"MEGAJOULE", 2.37303604042319e7, L"FOOT_POUNDAL", 1e-7);
    TestUnitConversion (1.0, L"MEGAWATT_HOUR", 8.54292974552351e10, L"FOOT_POUNDAL", 1.0e-3);
    TestUnitConversion (1.0, L"WATT_SECOND", 2.37303604042319e1, L"FOOT_POUNDAL", 1.0e-8);

    TestUnitConversion (1.0e6, L"JOULE", 2.37303604042319e7, L"FOOT_POUNDAL", 1.0e-7); // Value calculated in excel
    TestUnitConversion (1.0e6, L"KILOJOULE", 2.37303604042319e10, L"FOOT_POUNDAL", 1.0e-4); // Value calculated in excel
    TestUnitConversion (1.0e6, L"KILOWATT_HOUR", 8.54292974552351e13, L"FOOT_POUNDAL", 1.0); // Value calculated in excel
    TestUnitConversion (1.0e6, L"BTU", 2.50368556292668e10, L"FOOT_POUNDAL", 1e-4);
    TestUnitConversion (1.0e6, L"GIGAJOULE", 2.37303604042319e16, L"FOOT_POUNDAL", 1e2);
    
    TestUnitConversion (1.0e6, L"KILOBTU", 2.50368556292668e13, L"FOOT_POUNDAL", 1e-1);
    TestUnitConversion (1.0e6, L"GIGAWATT_HOUR", 8.54292974552351e19, L"FOOT_POUNDAL", 1.0e6);
    TestUnitConversion (1.0e6, L"MEGAJOULE", 2.37303604042319e13, L"FOOT_POUNDAL", 0.1);
    TestUnitConversion (1.0e6, L"MEGAWATT_HOUR", 8.54292974552351e16, L"FOOT_POUNDAL", 1000);
    TestUnitConversion (1.0e6, L"WATT_SECOND", 2.37303604042319e7, L"FOOT_POUNDAL", 1.0e-7);

    TestUnitConversion (1.0, L"NEWTON_METRE", 1.0, L"NEWTON_METRE", 1.0e-8);
    TestUnitConversion (1.0, L"POUND_FOOT", 1.3558179483314, L"NEWTON_METRE", 1.0e-8); // Value calculated in excel

    TestUnitConversion (1.0e6, L"POUND_FOOT", 1.3558179483314e6, L"NEWTON_METRE", 1.0e-8); // Value calculated in excel
    
    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"WATT", 1.0, L"WATT", 1.0e-8);

    TestUnitConversion (1.0, L"HORSEPOWER", 7.45699871582275e2, L"WATT", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, L"BTU_PER_HOUR", 2.93071070172222e-1, L"WATT", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, L"KILOBTU_PER_HOUR", 2.93071070172222e2, L"WATT", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, L"KILOWATT", 1000.0, L"WATT", 1.0e-8);
    TestUnitConversion (1.0, L"GIGAWATT", 1.0e9, L"WATT", 1.0e-8);
    TestUnitConversion (1.0, L"MEGAWATT", 1.0e6, L"WATT", 1.0e-8);

    TestUnitConversion (1.0e6, L"HORSEPOWER", 7.45699871582275e8, L"WATT", 1.0e-5); // Value calculated in excel
    TestUnitConversion (1.0e6, L"BTU_PER_HOUR", 2.93071070172222e5, L"WATT", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0e6, L"KILOBTU_PER_HOUR", 2.93071070172222e8, L"WATT", 1.0e-6); // Value calculated in excel
    TestUnitConversion (1.0e6, L"KILOWATT", 1.0e9, L"WATT", 1.0e-8);
    TestUnitConversion (1.0e6, L"GIGAWATT", 1.0e15, L"WATT", 1.0e-8);
    TestUnitConversion (1.0e6, L"MEGAWATT", 1.0e12, L"WATT", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"GIGAJOULE_PER_MONTH", 1.0, L"GIGAJOULE_PER_MONTH", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0e6, L"BTU_PER_MONTH", 1.05505585262, L"GIGAJOULE_PER_MONTH", 1.0e-8); // Value calculated in excel

    TestUnitConversion (1.0e6, L"GIGAJOULE_PER_MONTH", 1.0e6, L"GIGAJOULE_PER_MONTH", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0e12, L"BTU_PER_MONTH", 1.05505585262e6, L"GIGAJOULE_PER_MONTH", 1.0e-8); // Value calculated in excel
   
    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"POUND_FORCE_FOOT_SQUARED", 4.1325331065141e-1, L"NEWTON_METRE_SQUARED", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, L"NEWTON_METRE_SQUARED", 2.41982332440048, L"POUND_FORCE_FOOT_SQUARED", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, L"KILOGRAM_FORCE_METRE_SQUARED", 23.7303604570562, L"POUND_FORCE_FOOT_SQUARED", 1.0e-8); // Value calculated in excel

    TestUnitConversion (1.0e6, L"POUND_FORCE_FOOT_SQUARED", 4.1325331065141e5, L"NEWTON_METRE_SQUARED", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0e6, L"NEWTON_METRE_SQUARED", 2.41982332440048e6, L"POUND_FORCE_FOOT_SQUARED", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0e6, L"KILOGRAM_FORCE_METRE_SQUARED", 2.37303604570562e7, L"POUND_FORCE_FOOT_SQUARED", 1.0e-7); // Value calculated in excel

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    // Gauge offsets are 101.325 kPa converted into working unit, since the above website does not support Gauge units
    // the converted gauge offset was applied before the conversion was done.
    TestUnitConversion (1.0, L"ATMOSPHERE", 1.0, L"ATMOSPHERE", 1.0e-8);
    TestUnitConversion (1.0, L"BAR", 1.0e5 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0, L"BAR_GAUGE", ((1 - 1.01325) * 1.0e5) / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0, L"BARYE", 0.1 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Bayre == 0.1 pascal from wikipedia, other values from NIST
    TestUnitConversion (1.0, L"FOOT_OF_H2O_CONVENTIONAL", 2.989067e3 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, could not be fully double checked on web because site does not support ftH2O_Conv

    TestUnitConversion (1.0, L"INCH_OF_H2O_AT_32_FAHRENHEIT", 249.1083 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // PASCAL <-> INCH_OF_H2O_AT_32_FAHRENHEIT is from PDS, I have no way of verifying this number, but it is within the range you would expect.
    TestUnitConversion (1.0, L"INCH_OF_H2O_AT_39_2_FAHRENHEIT", 2.49082e2 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, could not be fully double checked on web because site does not support inH2O_39.2
    TestUnitConversion (1.0, L"INCH_OF_H2O_AT_60_FAHRENHEIT", 2.4884e2 / 1.01325e5, L"ATMOSPHERE", 1.0e-8);  // Value from NIST conversion factors, could not be fully double checked on web because site does not support inH2O_60
    TestUnitConversion (1.0, L"INCH_OF_HG_AT_32_FAHRENHEIT", 3.38638e3 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, could not be fully double checked on web because site does not support inHg_32
    TestUnitConversion (1.0, L"INCH_OF_HG_CONVENTIONAL", 3.386389e3 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, double checked at http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx?from=inch-of-mercury-conventional

    TestUnitConversion (1.0, L"INCH_OF_HG_AT_60_FAHRENHEIT", 3.37685e3 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, double checked at http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx?from=inch-of-mercury-60
    TestUnitConversion (1.0, L"KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED", 9.80665e4 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0, L"KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED_GAUGE", ((1 - 101.325 / 98.0665) * 9.80665e4) / 1.01325e5, L"ATMOSPHERE", 1.0e-8);
    TestUnitConversion (1.0, L"KILOGRAM_FORCE_PER_METRE_SQUARED", 9.80665 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0, L"KILOPASCAL", 1000 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors

    TestUnitConversion (1.0, L"KILOPASCAL_GAUGE", ((1 - 101.325) * 1000) / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0, L"MEGAPASCAL", 1000000 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0, L"MEGAPASCAL_GAUGE", ((1 - 101.325 / 1000) * 1000000) / 1.01325e5, L"ATMOSPHERE", 1.0e-8);
    TestUnitConversion (1.0, L"METRE_OF_H2O_CONVENTIONAL", 9806.65 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // value is 1000 * mmH2O Conv from NIST, could not be fully double checked on web because site does not support mHg Conv
    TestUnitConversion (1.0, L"MILLIMETRE_OF_H2O_CONVENTIONAL", 9.80665 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, could not be fully double checked on web because site does not support mmH2O Conv

    TestUnitConversion (1.0, L"MILLIMETRE_OF_HG_AT_32_FAHRENHEIT", (3.38638e3 / 25.4) / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // value is inHg@32/25.4 inHg@32 from NIST, could not be fully double checked on web because site does not support mmHg 32
    TestUnitConversion (1.0, L"NEWTON_PER_METRE_SQUARED", 1 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // is pascal, so used ATM <-> PASCAL NIST conversion factor
    TestUnitConversion (1.0, L"PASCAL", 1 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0, L"NEWTON_PER_METRE_SQUARED", 1.0, L"PASCAL", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0, L"POUND_FORCE_PER_FOOT_SQUARED", 47.88026 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors

    TestUnitConversion (1.0, L"POUND_FORCE_PER_INCH_SQUARED", 6.894757e3 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0, L"POUND_FORCE_PER_INCH_SQUARED_GAUGE", ((1 - 101.325 / 6.894757) * 6.894757e3) / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0, L"TORR", 1.333224e2 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0, L"MILLIBAR", 1.0e2 / 1.01325e5, L"ATMOSPHERE", 1e-8);
    TestUnitConversion (1.0, L"HECTOPASCAL", 100.0 / 1.01325e5, L"ATMOSPHERE", 1e-8);


    TestUnitConversion (1000000.0, L"ATMOSPHERE", 1000000.0, L"ATMOSPHERE", 1.0e-8);
    TestUnitConversion (1000000.0, L"BAR", 1000000.0 * 1.0e5 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1000000.0, L"BAR_GAUGE", ((1000000.0 - 1.01325) * 1.0e5) / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1000000.0, L"BARYE", 100000 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Bayre == 0.1 pascal from wikipedia, other values from NIST
    TestUnitConversion (1000000.0, L"FOOT_OF_H2O_CONVENTIONAL", 1000000.0 * 2.989067e3 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, could not be fully double checked on web because site does not support ftH2O_Conv

    TestUnitConversion (1000000.0, L"INCH_OF_H2O_AT_32_FAHRENHEIT", 1000000.0 * 249.1083 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // PASCAL <-> INCH_OF_H2O_AT_32_FAHRENHEIT is from PDS, I have no way of verifying this number, but it is within the range you would expect.
    TestUnitConversion (1000000.0, L"INCH_OF_H2O_AT_39_2_FAHRENHEIT", 1000000.0 * 2.49082e2 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, could not be fully double checked on web because site does not support inH2O_39.2
    TestUnitConversion (1000000.0, L"INCH_OF_H2O_AT_60_FAHRENHEIT", 1000000.0 * 2.4884e2 / 1.01325e5, L"ATMOSPHERE", 1.0e-8);  // Value from NIST conversion factors, could not be fully double checked on web because site does not support inH2O_60
    TestUnitConversion (1000000.0, L"INCH_OF_HG_AT_32_FAHRENHEIT", 1000000.0 * 3.38638e3 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, could not be fully double checked on web because site does not support inHg_32
    TestUnitConversion (1000000.0, L"INCH_OF_HG_CONVENTIONAL", 1000000.0 * 3.386389e3 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, could not be fully double checked on web because site does not support inHg_Conv

    TestUnitConversion (1000000.0, L"INCH_OF_HG_AT_60_FAHRENHEIT", 1000000.0 * 3.37685e3 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, could not be fully double checked on web because site does not support inHg_60
    TestUnitConversion (1000000.0, L"KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED", 1000000.0 * 9.80665e4 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1000000.0, L"KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED_GAUGE", ((1000000 - 101.325 / 98.0665) * 9.80665e4) / 1.01325e5, L"ATMOSPHERE", 1.0e-8);
    TestUnitConversion (1000000.0, L"KILOGRAM_FORCE_PER_METRE_SQUARED", 1000000.0 * 9.80665 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1000000.0, L"KILOPASCAL", 1000000000 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors

    TestUnitConversion (1000000.0, L"KILOPASCAL_GAUGE", ((1000000 - 101.325) * 1000) / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1000000.0, L"MEGAPASCAL", 1000000000000 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1000000.0, L"MEGAPASCAL_GAUGE", ((1000000 - 101.325 / 1000) * 1000000) / 1.01325e5, L"ATMOSPHERE", 1.0e-8);
    TestUnitConversion (1000000.0, L"METRE_OF_H2O_CONVENTIONAL", 1000000.0 * 9806.65 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // value is 1000 * mmHg Conv from NIST, could not be fully double checked on web because site does not support mHg Conv
    TestUnitConversion (1000000.0, L"MILLIMETRE_OF_H2O_CONVENTIONAL", 1000000.0 * 9.80665 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, could not be fully double checked on web because site does not support mmH2O Conv

    TestUnitConversion (1000000.0, L"MILLIMETRE_OF_HG_AT_32_FAHRENHEIT", (1000000.0 * 133.322) / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // value is cmHg@0c/10 cmHg@0c from NIST, could not be fully double checked on web because site does not support mmHg 32
    TestUnitConversion (1000000.0, L"NEWTON_PER_METRE_SQUARED", 1000000 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // is pascal, so used ATM <-> PASCAL NIST conversion factor
    TestUnitConversion (1000000.0, L"PASCAL", 1000000 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1000000.0, L"NEWTON_PER_METRE_SQUARED", 1000000.0, L"PASCAL", 1.0e-8); // Value from NIST conversion factors
    double psf_per_Pascal = (4.4482216152605 / (0.3048 * 0.3048)); // 0.3048 is meters per foot, 4.4482216152605 is newtons per pound force
    TestUnitConversion (1000000.0, L"POUND_FORCE_PER_FOOT_SQUARED", 1000000.0 * psf_per_Pascal / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, can't use the stated value for lbf/ft^2 because it uses 4.448222 Newtons per lbf, we use the more precise value of 4.4482216152605

    double psi_per_Pascal = (4.4482216152605 / (0.0254 * 0.0254));// 0.0254 is meters per inch, 4.4482216152605 is newtons per pound force
    TestUnitConversion (1000000.0, L"POUND_FORCE_PER_INCH_SQUARED", 1000000.0 * psi_per_Pascal / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1000000.0, L"POUND_FORCE_PER_INCH_SQUARED_GAUGE", ((1000000 - 101.325 / (psi_per_Pascal / 1000)) * psi_per_Pascal) / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors, can't use the stated value for lbf/in^2 because it uses 4.448222 newtons per lbf, we sue the more precise value of 4.4482216152605
    TestUnitConversion (1000000.0, L"TORR", 1000000.0 * 1.333224e2 / 1.01325e5, L"ATMOSPHERE", 1.0e-8); // Value from NIST conversion factors
    TestUnitConversion (1.0e6, L"MILLIBAR", 1.0e8 / 1.01325e5, L"ATMOSPHERE", 1e-8);
    TestUnitConversion (1.0e6, L"HECTOPASCAL", 100.0e6 / 1.01325e5, L"ATMOSPHERE", 1e-8);

    TestUnitConversion (1.0, L"KILOWATT_HOUR_PER_METRE_CUBED", 1.0, L"KILOWATT_HOUR_PER_METRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, L"KILOWATT_HOUR_PER_MILLION_GALLON", 2.64172052358148e-4, L"KILOWATT_HOUR_PER_METRE_CUBED", 1.0e-8); // Not found in NIST doc, only confermed via web
    TestUnitConversion (1.0, L"KILOWATT_HOUR_PER_MILLION_LITRE", 0.001, L"KILOWATT_HOUR_PER_METRE_CUBED", 1.0e-8); // Not found in NIST doc, only confermed via web
    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"JOULE_PER_METRE_CUBED", 1.0 / (1000.0 * 3600.0), L"KILOWATT_HOUR_PER_METRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, L"KILOJOULE_PER_METRE_CUBED", 1.0 / 3600, L"KILOWATT_HOUR_PER_METRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, L"KILOWATT_HOUR_PER_FOOT_CUBED", 1.0 / pow (0.3048, 3.0), L"KILOWATT_HOUR_PER_METRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, L"MEGAJOULE_PER_METRE_CUBED", 1.0 / 3.6, L"KILOWATT_HOUR_PER_METRE_CUBED", 1.0e-8);

    TestUnitConversion (1.0e6, L"JOULE_PER_METRE_CUBED", 1.0e6 / (1000 * 3600), L"KILOWATT_HOUR_PER_METRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, L"KILOJOULE_PER_METRE_CUBED", 1.0e6 / 3600, L"KILOWATT_HOUR_PER_METRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, L"KILOWATT_HOUR_PER_FOOT_CUBED", 1.0e6 / pow (0.3048, 3.0), L"KILOWATT_HOUR_PER_METRE_CUBED", 1.0e-7);
    TestUnitConversion (1.0e6, L"MEGAJOULE_PER_METRE_CUBED", 1.0e6 / 3.6, L"KILOWATT_HOUR_PER_METRE_CUBED", 1.0e-8);

    TestUnitConversion (1.0, L"WATT_PER_METRE_CUBED", 1.0, L"WATT_PER_METRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, L"BTU_PER_HOUR_PER_FOOT_CUBED", 1.0349707168842e1, L"WATT_PER_METRE_CUBED", 1.0e-8);

    TestUnitConversion (1.0e6, L"WATT_PER_METRE_CUBED", 1.0e6, L"WATT_PER_METRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, L"BTU_PER_HOUR_PER_FOOT_CUBED", 1.0349707168842e7, L"WATT_PER_METRE_CUBED", 1.0e-7);

    TestUnitConversion (1.0, L"POUND_PER_ACRE", 1.12085115619445, L"KILOGRAM_PER_HECTARE", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, L"KILOGRAM_PER_HECTARE", 8.92179121619709e-1, L"POUND_PER_ACRE", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, L"KILOGRAM_PER_METRE_SQUARED", 8921.79121619709, L"POUND_PER_ACRE", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, L"GRAM_PER_METRE_SQUARED", 8.92179121619701, L"POUND_PER_ACRE", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, L"POUND_MASS_PER_FOOT_SQUARED", 4.356e4, L"POUND_PER_ACRE", 1.0e-8); // Value calculated in excel

    TestUnitConversion (1.0e6, L"POUND_PER_ACRE", 1.12085115619445e6, L"KILOGRAM_PER_HECTARE", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0e6, L"KILOGRAM_PER_HECTARE", 8.92179121619709e5, L"POUND_PER_ACRE", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0e6, L"KILOGRAM_PER_METRE_SQUARED", 8.92179121619709e9, L"POUND_PER_ACRE", 1.0e-4); // Value calculated in excel
    TestUnitConversion (1.0e6, L"GRAM_PER_METRE_SQUARED", 8.92179121619701e6, L"POUND_PER_ACRE", 1.0e-7); // Value calculated in excel
    TestUnitConversion (1.0e6, L"POUND_MASS_PER_FOOT_SQUARED", 4.356e10, L"POUND_PER_ACRE", 1.0e-8); // Value calculated in excel

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"KILONEWTON_PER_FOOT_CUBED", 3.53146667214886e1, L"KILONEWTON_PER_METRE_CUBED", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, L"KILONEWTON_PER_METRE_CUBED", 2.8316846592e-2, L"KILONEWTON_PER_FOOT_CUBED", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0e2, L"NEWTON_PER_METRE_CUBED", 2.8316846592e-3, L"KILONEWTON_PER_FOOT_CUBED", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, L"POUND_FORCE_PER_FOOT_CUBED", 4.4482216152605e-3, L"KILONEWTON_PER_FOOT_CUBED", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0, L"POUND_FORCE_PER_INCH_SQUARED_PER_FOOT", 6.40543912597512e-1, L"KILONEWTON_PER_FOOT_CUBED", 1.0e-8); // Value calculated in excel

    TestUnitConversion (1.0e6, L"KILONEWTON_PER_FOOT_CUBED", 3.53146667214886e7, L"KILONEWTON_PER_METRE_CUBED", 1.0e-7); // Value calculated in excel
    TestUnitConversion (1.0e6, L"KILONEWTON_PER_METRE_CUBED", 2.8316846592e4, L"KILONEWTON_PER_FOOT_CUBED", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0e8, L"NEWTON_PER_METRE_CUBED", 2.8316846592e3, L"KILONEWTON_PER_FOOT_CUBED", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0e6, L"POUND_FORCE_PER_FOOT_CUBED", 4.4482216152605e3, L"KILONEWTON_PER_FOOT_CUBED", 1.0e-8); // Value calculated in excel
    TestUnitConversion (1.0e6, L"POUND_FORCE_PER_INCH_SQUARED_PER_FOOT", 6.40543912597512e5, L"KILONEWTON_PER_FOOT_CUBED", 1.0e-8); // Value calculated in excel

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"PASCAL_PER_METRE", 1.0, L"PASCAL_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0, L"BAR_PER_KILOMETRE", 100, L"PASCAL_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0e6, L"BAR_PER_KILOMETRE", 1.0e8, L"PASCAL_PER_METRE", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"GRAM_PER_CENTIMETRE_CUBED", 1.0, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, L"KILOGRAM_PER_DECIMETRE_CUBED", 1.0, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, L"KILOGRAM_PER_LITRE", 1.0, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, L"KILOGRAM_PER_METRE_CUBED", 0.001, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, L"KIP_PER_FOOT_CUBED", 1.60184633739601e1, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);

    TestUnitConversion (1.0, L"MICROGRAM_PER_LITRE", 1e-9, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, L"MILLIGRAM_PER_LITRE", 1e-6, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, L"POUND_PER_FOOT_CUBED", 1.60184633739601e-2, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, L"POUND_PER_GALLON", 1.19826427316897e-1, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, L"POUND_PER_IMPERIAL_GALLON", 9.97763726631017e-2, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);

    TestUnitConversion (1.0, L"POUND_PER_INCH_CUBED", 2.76799047102031e1, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, L"POUND_PER_MILLION_GALLON", 1.19826427316897e-7, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, L"SHORT_TON_PER_FOOT_CUBED", 3.20369267479203e1, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, L"SLUG_PER_FOOT_CUBED", 5.15378818393195e-1, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);


    TestUnitConversion (1.0e6, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e6, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, L"KILOGRAM_PER_DECIMETRE_CUBED", 1.0e6, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, L"KILOGRAM_PER_LITRE", 1.0e6, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, L"KILOGRAM_PER_METRE_CUBED", 1000, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, L"KIP_PER_FOOT_CUBED", 1.60184633739601e7, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-7);

    TestUnitConversion (1.0e6, L"MICROGRAM_PER_LITRE", 1e-3, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, L"MILLIGRAM_PER_LITRE", 1.0, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, L"POUND_PER_FOOT_CUBED", 1.60184633739601e4, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, L"POUND_PER_GALLON", 1.19826427316897e5, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, L"POUND_PER_IMPERIAL_GALLON", 9.97763726631017e4, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);

    TestUnitConversion (1.0e6, L"POUND_PER_INCH_CUBED", 2.76799047102031e7, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-7);
    TestUnitConversion (1.0e6, L"POUND_PER_MILLION_GALLON", 1.19826427316897e-1, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, L"SHORT_TON_PER_FOOT_CUBED", 3.20369267479203e7, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-7);
    TestUnitConversion (1.0e6, L"SLUG_PER_FOOT_CUBED", 5.15378818393195e5, L"GRAM_PER_CENTIMETRE_CUBED", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"GRAM_PER_SECOND", 1.0, L"GRAM_PER_SECOND", 1.0e-8);

    TestUnitConversion (1.0, L"GRAM_PER_SECOND", 3600 * 24, L"GRAM_PER_DAY", 1.0e-8);
    TestUnitConversion (1.0, L"GRAM_PER_SECOND", 3600, L"GRAM_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, L"GRAM_PER_SECOND", 60, L"GRAM_PER_MINUTE", 1.0e-8);
    TestUnitConversion (1.0, L"GRAM_PER_SECOND", 3.6 * 24, L"KILOGRAM_PER_DAY", 1.0e-8);
    TestUnitConversion (1.0, L"GRAM_PER_SECOND", 3.6, L"KILOGRAM_PER_HOUR", 1.0e-8);

    TestUnitConversion (1.0, L"GRAM_PER_SECOND", 6.0e-2, L"KILOGRAM_PER_MINUTE", 1.0e-8);
    TestUnitConversion (1.0, L"GRAM_PER_SECOND", 1.0e-3, L"KILOGRAM_PER_SECOND", 1.0e-8);
    TestUnitConversion (1.0, L"GRAM_PER_SECOND", 1.0e6 * 3600 * 24, L"MICROGRAM_PER_DAY", 1.0e-8);
    TestUnitConversion (1.0, L"GRAM_PER_SECOND", 1.0e6 * 3600, L"MICROGRAM_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, L"GRAM_PER_SECOND", 1.0e6 * 60, L"MICROGRAM_PER_MINUTE", 1.0e-8);

    TestUnitConversion (1.0, L"GRAM_PER_SECOND", 1.0e6, L"MICROGRAM_PER_SECOND", 1.0e-8);
    TestUnitConversion (1.0, L"GRAM_PER_SECOND", 1.0e3 * 3600 * 24, L"MILLIGRAM_PER_DAY", 1.0e-8);
    TestUnitConversion (1.0, L"GRAM_PER_SECOND", 1.0e3 * 3600, L"MILLIGRAM_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, L"GRAM_PER_SECOND", 1.0e3 * 60, L"MILLIGRAM_PER_MINUTE", 1.0e-8);
    TestUnitConversion (1.0, L"GRAM_PER_SECOND", 1.0e3, L"MILLIGRAM_PER_SECOND", 1.0e-8);

    double gramPerPound = 453.59237; // From NIST doc kg per lb = 4.5359237e-1
    TestUnitConversion (1.0, L"GRAM_PER_SECOND", (3600 * 24) / gramPerPound, L"POUND_PER_DAY", 1.0e-8);
    TestUnitConversion (1.0, L"GRAM_PER_SECOND", 3600 / gramPerPound, L"POUND_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0, L"GRAM_PER_SECOND", 60 / gramPerPound, L"POUND_PER_MINUTE", 1.0e-8);
    TestUnitConversion (1.0, L"GRAM_PER_SECOND", 1 / gramPerPound, L"POUND_PER_SECOND", 1.0e-8);


    TestUnitConversion (1.0e6, L"GRAM_PER_SECOND", 3600 * 24 * 1.0e6, L"GRAM_PER_DAY", 1.0e-8);
    TestUnitConversion (1.0e6, L"GRAM_PER_SECOND", 3600 * 1.0e6, L"GRAM_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, L"GRAM_PER_SECOND", 60 * 1.0e6, L"GRAM_PER_MINUTE", 1.0e-8);
    TestUnitConversion (1.0e6, L"GRAM_PER_SECOND", 3.6e6 * 24, L"KILOGRAM_PER_DAY", 1.0e-8);
    TestUnitConversion (1.0e6, L"GRAM_PER_SECOND", 3.6e6, L"KILOGRAM_PER_HOUR", 1.0e-8);

    TestUnitConversion (1.0e6, L"GRAM_PER_SECOND", 6.0e4, L"KILOGRAM_PER_MINUTE", 1.0e-8);
    TestUnitConversion (1.0e6, L"GRAM_PER_SECOND", 1.0e3, L"KILOGRAM_PER_SECOND", 1.0e-8);
    TestUnitConversion (1.0e6, L"GRAM_PER_SECOND", 1.0e12 * 3600 * 24, L"MICROGRAM_PER_DAY", 1.0e-8);
    TestUnitConversion (1.0e6, L"GRAM_PER_SECOND", 1.0e12 * 3600, L"MICROGRAM_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, L"GRAM_PER_SECOND", 1.0e12 * 60, L"MICROGRAM_PER_MINUTE", 1.0e-8);

    TestUnitConversion (1.0e6, L"GRAM_PER_SECOND", 1.0e12, L"MICROGRAM_PER_SECOND", 1.0e-8);
    TestUnitConversion (1.0e6, L"GRAM_PER_SECOND", 1.0e9 * 3600 * 24, L"MILLIGRAM_PER_DAY", 1.0e-8);
    TestUnitConversion (1.0e6, L"GRAM_PER_SECOND", 1.0e9 * 3600, L"MILLIGRAM_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, L"GRAM_PER_SECOND", 1.0e9 * 60, L"MILLIGRAM_PER_MINUTE", 1.0e-8);
    TestUnitConversion (1.0e6, L"GRAM_PER_SECOND", 1.0e9, L"MILLIGRAM_PER_SECOND", 1.0e-8);

    TestUnitConversion (1.0e6, L"GRAM_PER_SECOND", (3600 * 24 * 1.0e6) / gramPerPound, L"POUND_PER_DAY", 1.0e-7);
    TestUnitConversion (1.0e6, L"GRAM_PER_SECOND", (3600 * 1.0e6) / gramPerPound, L"POUND_PER_HOUR", 1.0e-8);
    TestUnitConversion (1.0e6, L"GRAM_PER_SECOND", (60 * 1.0e6) / gramPerPound, L"POUND_PER_MINUTE", 1.0e-8);
    TestUnitConversion (1.0e6, L"GRAM_PER_SECOND", 1e6 / gramPerPound, L"POUND_PER_SECOND", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    // Some expected values are from NIST, others are calculated, calculated values are noted
    TestUnitConversion (1.0, L"NEWTON_PER_METRE", 1.0, L"NEWTON_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0, L"NEWTON_PER_MILLIMETRE", 1000, L"NEWTON_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0, L"POUND_FORCE_PER_INCH", 175.126835246476, L"NEWTON_PER_METRE", 1.0e-5);

    TestUnitConversion (1.0e6, L"NEWTON_PER_METRE", 1.0e6, L"NEWTON_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0e6, L"NEWTON_PER_MILLIMETRE", 1.0e9, L"NEWTON_PER_METRE", 1.0e-6);
    TestUnitConversion (1.0e6, L"POUND_FORCE_PER_INCH", 1.75126835246476e8, L"NEWTON_PER_METRE", 1.0e-5);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"WATT_PER_METRE_SQUARED", 1.0, L"WATT_PER_METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, L"BTU_PER_HOUR_PER_FOOT_SQUARED", 3.15459074506304, L"WATT_PER_METRE_SQUARED", 1.0e-8);

    TestUnitConversion (1.0e6, L"WATT_PER_METRE_SQUARED", 1.0e6, L"WATT_PER_METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0e6, L"BTU_PER_HOUR_PER_FOOT_SQUARED", 3.15459074506304e6, L"WATT_PER_METRE_SQUARED", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_KELVIN", 1.0, L"WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0, L"WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_CELSIUS", 1.0, L"WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0, L"BTU_PER_FOOT_SQUARED_PER_HOUR_PER_DELTA_DEGREE_FAHRENHEIT", 5.67826334111348, L"WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_KELVIN", 1.0e-8);

    TestUnitConversion (1.0e6, L"WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_KELVIN", 1.0e6, L"WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0e6, L"WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_CELSIUS", 1.0e6, L"WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0e6, L"BTU_PER_FOOT_SQUARED_PER_HOUR_PER_DELTA_DEGREE_FAHRENHEIT", 5.67826334111348e6, L"WATT_PER_METRE_SQUARED_PER_DELTA_DEGREE_KELVIN", 1.0e-8);

    TestUnitConversion (1.0e6, L"CAPITA", 1.0e6, L"PERSON", 1.0e-8);
    TestUnitConversion (1.0e6, L"CUSTOMER", 1.0e6, L"PERSON", 1.0e-8);
    TestUnitConversion (1.0e6, L"EMPLOYEE", 1.0e6, L"PERSON", 1.0e-8);
    TestUnitConversion (1.0e6, L"GUEST", 1.0e6, L"PERSON", 1.0e-8);
    TestUnitConversion (1.0e6, L"HUNDRED_CAPITA", 1.0e8, L"PERSON", 1.0e-8);
    TestUnitConversion (1.0e6, L"THOUSAND_CAPITA", 1.0e9, L"PERSON", 1.0e-8);
    TestUnitConversion (1.0e6, L"PASSENGER", 1.0e6, L"PERSON", 1.0e-8);
    TestUnitConversion (1.0e6, L"RESIDENT", 1.0e6, L"PERSON", 1.0e-8);
    TestUnitConversion (1.0e6, L"STUDENT", 1.0e6, L"PERSON", 1.0e-8);

    TestUnitConversion (100.0, L"PERCENT_PERCENT", 1.0, L"UNITLESS_PERCENT", 1.0e-8);
    TestUnitConversion (0.5, L"UNITLESS_PERCENT", 50.0, L"PERCENT_PERCENT", 1.0e-8);

    TestUnitConversion (50.0, L"PERCENT_SLOPE", 5280.0 / 2, L"FOOT_PER_MILE", 1.0e-8);
    TestUnitConversion (1320, L"FOOT_PER_MILE", 25.0, L"PERCENT_SLOPE", 1.0e-8);
    TestUnitConversion (.45, L"VERTICAL_PER_HORIZONTAL", 45.0, L"PERCENT_SLOPE", 1.0e-8);

    TestUnitConversion (5, L"METRE_VERTICAL_PER_METRE_HORIZONTAL", 5, L"FOOT_VERTICAL_PER_FOOT_HORIZONTAL", 1.0e-8);
    TestUnitConversion (0.1, L"FOOT_VERTICAL_PER_FOOT_HORIZONTAL", 0.1, L"FOOT_PER_FOOT", 1.0e-8);
    TestUnitConversion (0.1, L"FOOT_HORIZONTAL_PER_FOOT_VERTICAL", 10.0, L"FOOT_PER_FOOT", 1.0e-8);
    TestUnitConversion (0.1, L"METRE_PER_METRE", 0.1, L"FOOT_PER_FOOT", 1.0e-8);
    TestUnitConversion (0.1, L"METRE_VERTICAL_PER_METRE_HORIZONTAL", 0.1, L"FOOT_PER_FOOT", 1.0e-8);
    TestUnitConversion (0.1, L"METRE_HORIZONTAL_PER_METRE_VERTICAL", 10.0, L"FOOT_PER_FOOT", 1.0e-8);
    TestUnitConversion (0.1, L"VERTICAL_PER_HORIZONTAL", 0.1, L"FOOT_PER_FOOT", 1.0e-8);
    TestUnitConversion (0.1, L"HORIZONTAL_PER_VERTICAL", 10.0, L"FOOT_PER_FOOT", 1.0e-8);
    TestUnitConversion (0.1, L"FOOT_PER_FOOT", 100.0, L"MILLIMETRE_VERTICAL_PER_METRE_HORIZONTAL", 1.0e-8);
    TestUnitConversion (0.1, L"FOOT_PER_FOOT", 10.0, L"CENTIMETRE_PER_METRE", 1.0e-8);
    TestUnitConversion (2.0, L"FOOT_PER_FOOT", 0.5, L"HORIZONTAL_PER_VERTICAL", 1.0e-8);
    TestUnitConversion (2.0, L"FOOT_PER_FOOT", 2.0, L"VERTICAL_PER_HORIZONTAL", 1.0e-8);
    TestUnitConversion (1000.0, L"FOOT_PER_1000_FOOT", 1.0, L"VERTICAL_PER_HORIZONTAL", 1.0e-8);
    TestUnitConversion (12.0, L"INCH_PER_FOOT", 0.083333333333333333333333333333333, L"FOOT_PER_INCH", 1.0e-8);
    TestUnitConversion (1.0, L"VERTICAL_PER_HORIZONTAL", 0.083333333333333333333333333333333, L"FOOT_PER_INCH", 1.0e-8);
    TestUnitConversion (1.0, L"FOOT_PER_INCH", 12.0, L"VERTICAL_PER_HORIZONTAL", 1.0e-8);
    TestUnitConversion (5280, L"FOOT_PER_MILE", 1.0, L"VERTICAL_PER_HORIZONTAL", 1.0e-8);
    TestUnitConversion (12.0, L"INCH_PER_FOOT", 1.0, L"VERTICAL_PER_HORIZONTAL", 1.0e-8);
    TestUnitConversion (1.0, L"METRE_PER_CENTIMETRE", 100.0, L"VERTICAL_PER_HORIZONTAL", 1.0e-8);
    TestUnitConversion (1000.0, L"METRE_PER_KILOMETRE", 1.0, L"VERTICAL_PER_HORIZONTAL", 1.0e-8);
    TestUnitConversion (1000.0, L"MILLIMETRE_PER_METRE", 1.0, L"VERTICAL_PER_HORIZONTAL", 1.0e-8);
    TestUnitConversion (10.0, L"ONE_OVER_SLOPE", 10.0, L"HORIZONTAL_PER_VERTICAL", 1.0e-8);
    TestUnitConversion (33.0, L"ONE_OVER_SLOPE", 1.0 / 33.0, L"VERTICAL_PER_HORIZONTAL", 1.0e-8);
    
    TestUnitConversion (5.0, L"PARTS_PER_MILLION", 5000.0, L"PARTS_PER_BILLION", 1.0e-8);
    TestUnitConversion (2600.0, L"PARTS_PER_BILLION", 2.6, L"PARTS_PER_MILLION", 1.0e-8);

    TestUnitConversion (1.0, L"MILLIMETRE_SQUARED_PER_METRE_SQUARED", 1.0, L"MILLIMETRE_SQUARED_PER_METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, L"INCH_SQUARED_PER_FOOT_SQUARED", 6944.4444444444443, L"MILLIMETRE_SQUARED_PER_METRE_SQUARED", 1.0e-8);
    TestUnitConversion (7e9, L"MILLIMETRE_SQUARED_PER_METRE_SQUARED", 1.008e+6, L"INCH_SQUARED_PER_FOOT_SQUARED", 1.0e-8);

    TestUnitConversion (1.0, L"KILOGRAM_PER_KILOGRAM", 1.0, L"KILOGRAM_PER_KILOGRAM", 1.0e-8);
    TestUnitConversion (1.0, L"GRAIN_MASS_PER_POUND_MASS", 1.0 / 7000.0, L"KILOGRAM_PER_KILOGRAM", 1.0e-8);
    TestUnitConversion (7e9, L"GRAIN_MASS_PER_POUND_MASS", 1.0e6, L"KILOGRAM_PER_KILOGRAM", 1.0e-8);
    
    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    // Some expected values are from NIST, others are calculated, calculated values are noted
    TestUnitConversion (1.0, L"RECIPROCAL_DELTA_DEGREE_CELSIUS", 5.0 / 9.0, L"RECIPROCAL_DELTA_DEGREE_FAHRENHEIT", 1.0e-8);
    TestUnitConversion (1.0, L"RECIPROCAL_DELTA_DEGREE_KELVIN", 5.0 / 9.0, L"RECIPROCAL_DELTA_DEGREE_FAHRENHEIT", 1.0e-8);
    TestUnitConversion (1.0, L"RECIPROCAL_DELTA_DEGREE_FAHRENHEIT", 9.0 / 5.0, L"RECIPROCAL_DELTA_DEGREE_CELSIUS", 1.0e-8);
    TestUnitConversion (1.0, L"RECIPROCAL_DELTA_DEGREE_RANKINE", 9.0 / 5.0, L"RECIPROCAL_DELTA_DEGREE_CELSIUS", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    // Some expected values are from NIST, others are calculated, calculated values are noted
    TestUnitConversion (1.0, L"ONE_PER_FOOT", 1.0 / 0.3048, L"ONE_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0, L"ONE_PER_METRE", 0.3048, L"ONE_PER_FOOT", 1.0e-8);
    TestUnitConversion (1.0, L"ONE_PER_THOUSAND_FOOT", 1.0 / 1000.0, L"ONE_PER_FOOT", 1.0e-8);
    TestUnitConversion (1.0, L"ONE_PER_KILOMETRE", 3.048e-4, L"ONE_PER_FOOT", 1.0e-8);
    TestUnitConversion (1.0, L"ONE_PER_MILE", 1.0 / 5280.0, L"ONE_PER_FOOT", 1.0e-8);
    TestUnitConversion (1.0, L"ONE_PER_MILLIMETRE", 304.8, L"ONE_PER_FOOT", 1.0e-8);

    TestUnitConversion (1.0e6, L"ONE_PER_FOOT", 1.0e6 / 0.3048, L"ONE_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0e6, L"ONE_PER_METRE", 3.048e5, L"ONE_PER_FOOT", 1.0e-8);
    TestUnitConversion (1.0e6, L"ONE_PER_THOUSAND_FOOT", 1.0e6 / 1000.0, L"ONE_PER_FOOT", 1.0e-8);
    TestUnitConversion (1.0e6, L"ONE_PER_KILOMETRE", 3.048e2, L"ONE_PER_FOOT", 1.0e-8);
    TestUnitConversion (1.0e6, L"ONE_PER_MILE", 1.0e6 / 5280.0, L"ONE_PER_FOOT", 1.0e-8);
    TestUnitConversion (1.0e6, L"ONE_PER_MILLIMETRE", 3.048e8, L"ONE_PER_FOOT", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    // All values calculated in excel
    TestUnitConversion (1.0, L"PERSON_PER_FOOT_SQUARED", 4.356e4, L"PERSON_PER_ACRE", 1.0e-8);
    TestUnitConversion (1.0e6, L"PERSON_PER_ACRE", 2.47105381467165e6, L"PERSON_PER_HECTARE", 1.0e-8);
    TestUnitConversion (1.0e6, L"PERSON_PER_KILOMETRE_SQUARED", 4.0468564224e3, L"PERSON_PER_ACRE", 1.0e-8);
    TestUnitConversion (1.0, L"PERSON_PER_METRE_SQUARED", 4.0468564224e3, L"PERSON_PER_ACRE", 1.0e-8);
    TestUnitConversion (1.0e6, L"PERSON_PER_MILE_SQUARED", 1.5625e3, L"PERSON_PER_ACRE", 1.0e-8);

    TestUnitConversion (1.0, L"ONE_PER_METRE_SQUARED", 1.0, L"ONE_PER_METRE_SQUARED", 1e-8);

    TestUnitConversion (1.0, L"ONE_PER_FOOT_SQUARED", 1.07639104167097e1, L"ONE_PER_METRE_SQUARED", 1e-8);
    TestUnitConversion (1.0e6, L"ONE_PER_FOOT_SQUARED", 1.07639104167097e7, L"ONE_PER_METRE_SQUARED", 1e-7);
    
    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    // All values calculated in excel
    TestUnitConversion (1.0e6, L"ONE_PER_ACRE_FOOT", 2.29568411386593e1, L"ONE_PER_FOOT_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, L"ONE_PER_ACRE_INCH", 2.75482093663912e2, L"ONE_PER_FOOT_CUBED", 1.0e-8);
    TestUnitConversion (1.0, L"ONE_PER_CENTIMETRE_CUBED", 2.8316846592e4, L"ONE_PER_FOOT_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, L"ONE_PER_FOOT_CUBED", 5.78703703703704e2, L"ONE_PER_INCH_CUBED", 1.0e-8);
    TestUnitConversion (1.0, L"ONE_PER_GALLON", 7.48051948051948, L"ONE_PER_FOOT_CUBED", 1.0e-8);

    TestUnitConversion (1.0, L"ONE_PER_IMPERIAL_GALLON", 6.22883545904283, L"ONE_PER_FOOT_CUBED", 1.0e-8);
    TestUnitConversion (1.0, L"ONE_PER_INCH_CUBED", 1.728e3, L"ONE_PER_FOOT_CUBED", 1.0e-8);
    TestUnitConversion (1.0, L"ONE_PER_LITRE", 2.8316846592e1, L"ONE_PER_FOOT_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, L"ONE_PER_METRE_CUBED", 2.8316846592e4, L"ONE_PER_FOOT_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, L"ONE_PER_MILLION_GALLON", 7.48051948051948, L"ONE_PER_FOOT_CUBED", 1.0e-8);

    TestUnitConversion (1.0e6, L"ONE_PER_MILLION_LITRE", 2.8316846592e1, L"ONE_PER_FOOT_CUBED", 1.0e-8);
    TestUnitConversion (1.0e3, L"ONE_PER_THOUSAND_GALLON", 7.48051948051948, L"ONE_PER_FOOT_CUBED", 1.0e-8);
    TestUnitConversion (1.0e3, L"ONE_PER_THOUSAND_LITRE", 2.8316846592e1, L"ONE_PER_FOOT_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, L"ONE_PER_YARD_CUBED", 3.7037037037037e4, L"ONE_PER_FOOT_CUBED", 1.0e-8);

    // Values are not double checked.  This ATP soley assures that the conversion factors don't change
    // All values calculated in excel
    TestUnitConversion (1.0, L"FOOT_CUBED_PER_SECOND_PER_SQUARE_ROOT_FOOT_H20", 6.81670857167927e2, L"GALLON_PER_MINUTE_PER_SQUARE_ROOT_PSI", 1.0e-8);
    TestUnitConversion (1.0, L"LITRE_PER_SEC_PER_SQUARE_ROOT_KPA", 4.16194993948617e1, L"GALLON_PER_MINUTE_PER_SQUARE_ROOT_PSI", 1.0e-8);
    TestUnitConversion (1.0, L"METRE_CUBED_PER_SECOND_PER_SQUARE_ROOT_METRE_H20", 1.329037762e4, L"GALLON_PER_MINUTE_PER_SQUARE_ROOT_PSI", 1.0e-8);
    TestUnitConversion (1.0e6, L"GALLON_PER_MINUTE_PER_SQUARE_ROOT_PSI", 7.52424068444189e1, L"METRE_CUBED_PER_SECOND_PER_SQUARE_ROOT_METRE_H20", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    // All values calculated in excel
    TestUnitConversion (1.0e6, L"ONE_PER_HORSEPOWER", 1.34102208959503e6, L"ONE_PER_KILOWATT", 1.0e-8);
    TestUnitConversion (1.0e6, L"ONE_PER_KILOWATT", 7.4569987158227e5, L"ONE_PER_HORSEPOWER", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"METRE_SQUARED_DELTA_DEGREE_KELVIN_PER_WATT", 1.0, L"METRE_SQUARED_DELTA_DEGREE_KELVIN_PER_WATT", 1.0e-8);
    TestUnitConversion (1.0, L"METRE_SQUARED_DELTA_DEGREE_CELSIUS_PER_WATT", 1.0, L"METRE_SQUARED_DELTA_DEGREE_KELVIN_PER_WATT", 1.0e-8);
    TestUnitConversion (1.0, L"FOOT_SQUARED_HOUR_DELTA_DEGREE_FAHRENHEIT_PER_BTU", 1.76110183682306e-1, L"METRE_SQUARED_DELTA_DEGREE_KELVIN_PER_WATT", 1.0e-8);

    TestUnitConversion (1.0e6, L"METRE_SQUARED_DELTA_DEGREE_KELVIN_PER_WATT", 1.0e6, L"METRE_SQUARED_DELTA_DEGREE_KELVIN_PER_WATT", 1.0e-8);
    TestUnitConversion (1.0e6, L"METRE_SQUARED_DELTA_DEGREE_CELSIUS_PER_WATT", 1.0e6, L"METRE_SQUARED_DELTA_DEGREE_KELVIN_PER_WATT", 1.0e-8);
    TestUnitConversion (1.0e6, L"FOOT_SQUARED_HOUR_DELTA_DEGREE_FAHRENHEIT_PER_BTU", 1.76110183682306e5, L"METRE_SQUARED_DELTA_DEGREE_KELVIN_PER_WATT", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"MEGANEWTON_SECOND_PER_METRE_SQUARED", 1.0, L"MEGANEWTON_SECOND_PER_METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, L"MILLINEWTON_SECOND_PER_METRE_SQUARED", 1.0e-9, L"MEGANEWTON_SECOND_PER_METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, L"PASCAL_SECOND", 1.0e-6, L"MEGANEWTON_SECOND_PER_METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, L"PASCAL_SECOND", 1.0e3, L"CENTIPOISE", 1.0e-8);

    TestUnitConversion (1.0e6, L"MEGANEWTON_SECOND_PER_METRE_SQUARED", 1.0e6, L"MEGANEWTON_SECOND_PER_METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0e6, L"MILLINEWTON_SECOND_PER_METRE_SQUARED", 1.0e-3, L"MEGANEWTON_SECOND_PER_METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0e6, L"PASCAL_SECOND", 1.0, L"MEGANEWTON_SECOND_PER_METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0e6, L"PASCAL_SECOND", 1.0e9, L"CENTIPOISE", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"KILOJOULE_PER_KILOGRAM", 1.0, L"KILOJOULE_PER_KILOGRAM", 1.0e-8);
    TestUnitConversion (1.0, L"MEGAJOULE_PER_KILOGRAM", 1.0e3, L"KILOJOULE_PER_KILOGRAM", 1.0e-8);
    TestUnitConversion (1.0, L"BTU_PER_POUND_MASS", 2.326, L"KILOJOULE_PER_KILOGRAM", 1.0e-8);

    TestUnitConversion (1.0e6, L"KILOJOULE_PER_KILOGRAM", 1.0e6, L"KILOJOULE_PER_KILOGRAM", 1.0e-8);
    TestUnitConversion (1.0e6, L"MEGAJOULE_PER_KILOGRAM", 1.0e9, L"KILOJOULE_PER_KILOGRAM", 1.0e-8);
    TestUnitConversion (1.0e6, L"BTU_PER_POUND_MASS", 2.326e6, L"KILOJOULE_PER_KILOGRAM", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"METRE_CUBED_PER_KILOGRAM", 1.0, L"METRE_CUBED_PER_KILOGRAM", 1.0e-8);
    TestUnitConversion (1.0, L"FOOT_CUBED_PER_POUND_MASS", 6.24279605761448e-2, L"METRE_CUBED_PER_KILOGRAM", 1.0e-8);

    TestUnitConversion (1.0e6, L"METRE_CUBED_PER_KILOGRAM", 1.0e6, L"METRE_CUBED_PER_KILOGRAM", 1.0e-8);
    TestUnitConversion (1.0e6, L"FOOT_CUBED_PER_POUND_MASS", 6.24279605761448e4, L"METRE_CUBED_PER_KILOGRAM", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"ONE_PER_BTU", 1.0, L"ONE_PER_BTU", 1.0e-8);
    TestUnitConversion (1.0, L"ONE_PER_MEGAJOULE", 1.05505585262e-3, L"ONE_PER_BTU", 1.0e-8);
    TestUnitConversion (1.0, L"ONE_PER_KILOWATT_HOUR", 2.93071070172222e-4, L"ONE_PER_BTU", 1.0e-8);

    TestUnitConversion (1.0e6, L"ONE_PER_BTU", 1.0e6, L"ONE_PER_BTU", 1.0e-8);
    TestUnitConversion (1.0e6, L"ONE_PER_MEGAJOULE", 1.05505585262e3, L"ONE_PER_BTU", 1.0e-8);
    TestUnitConversion (1.0e6, L"ONE_PER_KILOWATT_HOUR", 2.93071070172222e2, L"ONE_PER_BTU", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"ONE_PER_TONNE", 1.0, L"ONE_PER_TONNE", 1.0e-8);
    TestUnitConversion (1.0, L"ONE_PER_SHORT_TON", 1.10231131092439, L"ONE_PER_TONNE", 1.0e-8);

    TestUnitConversion (1.0e6, L"ONE_PER_TONNE", 1.0e6, L"ONE_PER_TONNE", 1.0e-8);
    TestUnitConversion (1.0e6, L"ONE_PER_SHORT_TON", 1.10231131092439e6, L"ONE_PER_TONNE", 1.0e-8);

   // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"LUMEN_PER_FOOT_SQUARED", 1.07639104167097e1, L"LUX", 1.0e-8);
    TestUnitConversion (1.0, L"LUX", 9.290304e-2, L"LUMEN_PER_FOOT_SQUARED", 1.0e-8);

    TestUnitConversion (1.0e6, L"LUMEN_PER_FOOT_SQUARED", 1.07639104167097e7, L"LUX", 1.0e-7);
    TestUnitConversion (1.0e6, L"LUX", 9.290304e4, L"LUMEN_PER_FOOT_SQUARED", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
#ifdef WIP_UNITS_TEST_OBSOLETE_UNITS
    // Managed tests disable asserts for these, because the units are not compatible (dimension M2_PER_S2_K vs L2_PER_T2_K)
    TestUnitConversion (1.0, L"JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0, L"JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0, L"JOULE_PER_KILOGRAM_PER_DELTA_DEGREE_CELSIUS", 1.0, L"JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0, L"BTU_PER_POUND_MASS_PER_DELTA_DEGREE_FAHRENHEIT", 4.1868e3, L"JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0e-8);

    TestUnitConversion (1.0e6, L"JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0e6, L"JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0e6, L"JOULE_PER_KILOGRAM_PER_DELTA_DEGREE_CELSIUS", 1.0e6, L"JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0e6, L"BTU_PER_POUND_MASS_PER_DELTA_DEGREE_FAHRENHEIT", 4.1868e9, L"JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0e-6);
    
    TestUnitConversion (1.0e6, L"BTU_PER_POUND_MASS_PER_DELTA_DEGREE_RANKINE", 1.0e6, L"BTU_PER_POUND_MASS_PER_DELTA_DEGREE_FAHRENHEIT", 1.0e-8);
    TestUnitConversion (1.0e6, L"JOULE_PER_KILOGRAM_PER_DELTA_DEGREE_KELVIN", 1.0e6, L"JOULE_PER_KILOGRAM_PER_DELTA_DEGREE_CELSIUS", 1.0e-8);
    TestUnitConversion (1.0e6, L"JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0e6, L"JOULE_PER_KILOGRAM_PER_DELTA_DEGREE_CELSIUS", 1.0e-8);
#endif
    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"ONE_PER_HOUR", 1.0 / 3600.0, L"HERTZ", 1.0e-8);
    TestUnitConversion (1.0, L"ONE_PER_YEAR", 1.0 / 31536000.0, L"HERTZ", 1.0e-8); // Not directly tested on knowledgedoor because they define a year as 365.25 days validated.  Did the one_per_day -> Hertz then multiplied by 365
    TestUnitConversion (1.0, L"ONE_PER_DAY", 1.0 / 86400.0, L"HERTZ", 1.0e-8);
    TestUnitConversion (1.0, L"ONE_PER_MINUTE", 1.0 / 60.0, L"HERTZ", 1.0e-8);

    TestUnitConversion (1.0e6, L"ONE_PER_HOUR", 1.0e6 / 3600.0, L"HERTZ", 1.0e-8);
    TestUnitConversion (1.0e6, L"ONE_PER_YEAR", 1.0e6 / 31536000.0, L"HERTZ", 1.0e-8); // Not directly tested on knowledgedoor because they define a year as 365.25 days validated.  Did the one_per_day -> Hertz then multiplied by 365
    TestUnitConversion (1.0e6, L"ONE_PER_DAY", 1.0e6 / 86400.0, L"HERTZ", 1.0e-8);
    TestUnitConversion (1.0e6, L"ONE_PER_MINUTE", 1.0e6 / 60.0, L"HERTZ", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"GIGANEWTON_SECOND_PER_KILOGRAM_PER_METRE", 1.0, L"GIGANEWTON_SECOND_PER_KILOGRAM_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0e19, L"GRAIN_FORCE_PER_HOUR_PER_FOOT_SQUARED_PER_INCH_HG_CONVENTIONAL", 5.61072657924843, L"GIGANEWTON_SECOND_PER_KILOGRAM_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0e25, L"GRAIN_FORCE_PER_HOUR_PER_FOOT_SQUARED_PER_INCH_HG_CONVENTIONAL", 5.61072657924843e6, L"GIGANEWTON_SECOND_PER_KILOGRAM_PER_METRE", 1.0e-8);

    TestUnitConversion (1.0, L"MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0, L"MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8);
    TestUnitConversion (1.0, L"GRAIN_MASS_PER_HOUR_PER_FOOT_SQUARED", 6.97489662340437e2, L"MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, L"MICROGRAM_PER_FOOT_SQUARED_PER_DAY", 4.48496267362905e-4, L"MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, L"MICROGRAM_PER_METRE_SQUARED_PER_DAY", 4.16666666666667e-5, L"MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel

    TestUnitConversion (1.0, L"MICROGRAM_PER_METRE_SQUARED_PER_SECOND", 3.6, L"MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, L"MILLIGRAM_PER_FOOT_SQUARED_PER_DAY", 4.48496267362905e-1, L"MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, L"MILLIGRAM_PER_METRE_SQUARED_PER_DAY", 4.16666666666667e-2, L"MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel
    TestUnitConversion (1.0, L"MILLIGRAM_PER_METRE_SQUARED_PER_SECOND", 3.6e3, L"MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel

    TestUnitConversion (1.0e6, L"GRAIN_MASS_PER_HOUR_PER_FOOT_SQUARED", 6.97489662340437e8, L"MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-5);
    TestUnitConversion (1.0e6, L"MICROGRAM_PER_FOOT_SQUARED_PER_DAY", 4.48496267362905e2, L"MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e6, L"MICROGRAM_PER_METRE_SQUARED_PER_DAY", 4.16666666666667e1, L"MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel

    TestUnitConversion (1.0e6, L"MICROGRAM_PER_METRE_SQUARED_PER_SECOND", 3.6e6, L"MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-2); // Value from excel
    TestUnitConversion (1.0e6, L"MILLIGRAM_PER_FOOT_SQUARED_PER_DAY", 4.48496267362905e5, L"MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e6, L"MILLIGRAM_PER_METRE_SQUARED_PER_DAY", 4.16666666666667e4, L"MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel
    TestUnitConversion (1.0e6, L"MILLIGRAM_PER_METRE_SQUARED_PER_SECOND", 3.6e9, L"MILLIGRAM_PER_HOUR_PER_METRE_SQUARED", 1.0e-8); // Value from excel
  
    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"KILOGRAM_PER_METRE", 1.0, L"KILOGRAM_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0, L"POUND_MASS_PER_FOOT", 1.48816394356955, L"KILOGRAM_PER_METRE", 1.0e-8);
    TestUnitConversion (1.0e6, L"POUND_MASS_PER_FOOT", 1.48816394356955e6, L"KILOGRAM_PER_METRE", 1.0e-8);

    TestUnitConversion (1.0, L"WEIR_COEFFICIENT_SI", 1.0, L"WEIR_COEFFICIENT_SI", 1.0e-8);
    TestUnitConversion (1.0, L"WEIR_COEFFICIENT_SI", 1.8113530173065724, L"WEIR_COEFFICIENT_US", 1.0e-8);
    TestUnitConversion (1.0e6, L"WEIR_COEFFICIENT_SI", 1.8113530173065724e6, L"WEIR_COEFFICIENT_US", 1.0e-8);

    TestUnitConversion (1.0, L"SIDE_WEIR_COEFFICIENT_SI", 1.0, L"SIDE_WEIR_COEFFICIENT_SI", 1.0e-8);
    TestUnitConversion (1.0, L"SIDE_WEIR_COEFFICIENT_SI", 1.4859185774421794, L"SIDE_WEIR_COEFFICIENT_US", 1.0e-8);
    TestUnitConversion (1.0e6, L"SIDE_WEIR_COEFFICIENT_SI", 1.4859185774421794e6, L"SIDE_WEIR_COEFFICIENT_US", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"INCH_PER_HOUR_PER_DEGREE_FAHRENHEIT", 1.0, L"INCH_PER_HOUR_PER_DEGREE_FAHRENHEIT", 1.0e-8);
    TestUnitConversion (1.0, L"MILLIMETRE_PER_HOUR_PER_DEGREE_CELSIUS", 2.18722659667541e-2, L"INCH_PER_HOUR_PER_DEGREE_FAHRENHEIT", 1.0e-8);
    TestUnitConversion (1.0e6, L"MILLIMETRE_PER_HOUR_PER_DEGREE_CELSIUS", 2.18722659667541e4, L"INCH_PER_HOUR_PER_DEGREE_FAHRENHEIT", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"ONE_PER_PASCAL", 1.0, L"ONE_PER_PASCAL", 1.0e-8);
    TestUnitConversion (1.0, L"ONE_PER_BAR", 1.0e-5, L"ONE_PER_PASCAL", 1.0e-8);
    TestUnitConversion (1.0e6, L"ONE_PER_BAR", 10.0, L"ONE_PER_PASCAL", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"KILOMOLE_PER_SECOND", 1000.0, L"MOLE_PER_SECOND", 1.0e-8);
    TestUnitConversion (1.0, L"MOLE_PER_SECOND", 1.0e-3, L"KILOMOLE_PER_SECOND", 1.0e-8);
    TestUnitConversion (1.0e6, L"KILOMOLE_PER_SECOND", 1.0e9, L"MOLE_PER_SECOND", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"KILOMOLE_PER_METRE_CUBED", 1000.0, L"MOLE_PER_METRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, L"MOLE_PER_METRE_CUBED", 1.0e-3, L"KILOMOLE_PER_METRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0, L"MOLE_PER_METRE_CUBED", 6.24279605761446e-5, L"POUND_MOLE_PER_FOOT_CUBED", 1.0e-8);
    TestUnitConversion (1.0e9, L"MOLE_PER_METRE_CUBED", 1.0e6, L"KILOMOLE_PER_METRE_CUBED", 1.0e-8);
    TestUnitConversion (1.0e6, L"MOLE_PER_METRE_CUBED", 62.4279605761446, L"POUND_MOLE_PER_FOOT_CUBED", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"METRE_CUBED_PER_KILOMOLE", 1.0e-3, L"METRE_CUBED_PER_MOLE", 1.0e-8);
    TestUnitConversion (1.0, L"METRE_CUBED_PER_MOLE", 1000.0, L"METRE_CUBED_PER_KILOMOLE", 1.0e-8);
    TestUnitConversion (1.0, L"METRE_CUBED_PER_MOLE", 16018.4633739601, L"FOOT_CUBED_PER_POUND_MOLE", 1.0e-8);
    
    TestUnitConversion (1.0e9, L"METRE_CUBED_PER_KILOMOLE", 1.0e6, L"METRE_CUBED_PER_MOLE", 1.0e-8);
    TestUnitConversion (1.0e6, L"METRE_CUBED_PER_MOLE", 16018.4633739601e6, L"FOOT_CUBED_PER_POUND_MOLE", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"BTU_PER_POUND_MASS_PER_DELTA_DEGREE_RANKINE", 1.0 / 0.00023884589662749597, L"JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0, L"JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 0.00023884589662749597, L"BTU_PER_POUND_MASS_PER_DELTA_DEGREE_RANKINE", 1.0e-8);

    TestUnitConversion (1.0e6, L"BTU_PER_POUND_MASS_PER_DELTA_DEGREE_RANKINE", 1.0e6 / 0.00023884589662749597, L"JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0e6, L"JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 2.3884589662749597e2, L"BTU_PER_POUND_MASS_PER_DELTA_DEGREE_RANKINE", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"BTU_PER_POUND_MOLE", 1.0 / 0.42992261392949271, L"KILOJOULE_PER_KILOMOLE", 1.0e-8);
    TestUnitConversion (1.0, L"KILOJOULE_PER_KILOMOLE", 0.42992261392949271, L"BTU_PER_POUND_MOLE", 1.0e-8);

    TestUnitConversion (1.0e6, L"BTU_PER_POUND_MOLE", 1.0e6 / 0.42992261392949271, L"KILOJOULE_PER_KILOMOLE", 1.0e-8);
    TestUnitConversion (1.0e6, L"KILOJOULE_PER_KILOMOLE", 4.2992261392949271e5, L"BTU_PER_POUND_MOLE", 1.0e-8);

    // Values double checked at:http://www.knowledgedoor.com/1/Unit_Conversion/Convert_to_New_Units.aspx
    TestUnitConversion (1.0, L"JOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN", 0.001, L"KILOJOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0, L"KILOJOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN", 1000.0, L"JOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0, L"KILOJOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN", 0.23884589662749595, L"BTU_PER_POUND_MOLE_PER_DELTA_DEGREE_RANKINE", 1.0e-8);

    TestUnitConversion (1.0e6, L"JOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN", 1.0e3, L"KILOJOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0e6, L"KILOJOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN", 1.0e9, L"JOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN", 1.0e-8);
    TestUnitConversion (1.0e6, L"KILOJOULE_PER_KILOMOLE_PER_DELTA_DEGREE_KELVIN", 2.3884589662749595e5, L"BTU_PER_POUND_MOLE_PER_DELTA_DEGREE_RANKINE", 1.0e-8);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitsTest, TestUnitSpecifications)
    {
    TestUnitSpecification (L"FromKOQ", 100.0);
    TestUnitSpecification (L"FromParentKOQ", 100.0);
    TestUnitSpecification (L"FromDimension", 0.001);
    TestUnitSpecification (L"FromKOQDimension", 0.001);

    TestUnitSpecification (L"FromNonExistentKOQWithDimension", 10.0);

    // Non-standard Unit produces a Unit which is not convertible to any other Unit
    Unit nonStandardUnit;
    EXPECT_TRUE (Unit::GetUnitForECProperty (nonStandardUnit, *m_schema->GetClassP (L"UnitSpecClass")->GetPropertyP (L"FromNonExistentKOQWithUnit")));
    EXPECT_EQ (0, wcscmp (nonStandardUnit.GetShortLabel(), L"NONEXISTENT_UNIT"));
    EXPECT_EQ (0, wcscmp (nonStandardUnit.GetBaseUnitName(), L"NONEXISTENT_UNIT"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitsTest, Formatting)
    {
    // Storage: cm Display: none
    TestUnitFormatting (L"FromKOQ", 123.456, L"123.46 cm");

    // Storage: cm Display: decimetre
    TestUnitFormatting (L"FromParentKOQ", 123.456, L"12.35 dm");

    // Storage: km Display: none
    TestUnitFormatting (L"FromKOQDimension", 123.456, L"123.46 km");

    // Storage: km Display: miles Format: 0000.###### \"ignored\"
    TestUnitFormatting (L"FromDimension", 123.456, L"0076.712002 ignored mi");
    }
END_BENTLEY_ECOBJECT_NAMESPACE
