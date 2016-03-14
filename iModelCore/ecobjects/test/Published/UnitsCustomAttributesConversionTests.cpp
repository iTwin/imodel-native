/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/UnitsCustomAttributesConversionTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

BEGIN_BENTLEY_ECN_TEST_NAMESPACE
using namespace BentleyApi::ECN;

struct UnitsCustomAttributesConversionTests : ECTestFixture
    {
    ECSchemaPtr     m_schema;

    //---------------------------------------------------------------------------------------//
    // Stores the format of the reference schema xml as a string
    // @bsimethod                             Prasanna.Prakash                       03/2016
    //+---------------+---------------+---------------+---------------+---------------+------//
    static Utf8CP   TestSchemaXmlString()
        {
        Utf8CP format = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.00.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
            "  <ECSchemaReference name=\"Unit_Attributes\" version=\"01.00.00\" prefix=\"units_attribs\" />"
            "  <ECCustomAttributes>"
            "      <IsUnitSystemSchema xmlns=\"Unit_Attributes.01.00\" />"
            "      <UnitSpecifications xmlns=\"Unit_Attributes.01.00\">"
            "          <UnitSpecificationList>"
            "              <UnitSpecification>"
            "                  <KindOfQuantityName>LENGTH</KindOfQuantityName>"
            "                  <UnitName>KILOMETRE</UnitName>"
            "              </UnitSpecification>"
            "              <UnitSpecification>"
            "                  <DimensionName>L</DimensionName>"
            "                  <UnitName>KILOMETRE</UnitName>"
            "              </UnitSpecification>"
            "          </UnitSpecificationList>"
            "      </UnitSpecifications>"
            "  </ECCustomAttributes>"
            "  <ECEntityClass typeName =\"TestClass\">"
            "        <ECProperty propertyName=\"PropertyA\" typeName=\"double\">"
            "          <ECCustomAttributes>"
            "              <UnitSpecification xmlns=\"Unit_Attributes.01.00\">"
            "                  <UnitName>KILOMETRE</UnitName>"
            "              </UnitSpecification>"
            "          </ECCustomAttributes>"
            "        </ECProperty>"
            "        <ECProperty propertyName=\"PropertyB\" typeName=\"double\">"
            "          <ECCustomAttributes>"
            "              <DisplayUnitSpecification xmlns = \"Unit_Attributes.01.00\">"
            "                  <DisplayUnitName>MILE</DisplayUnitName>"
            "                  <DisplayFormatString>0000.###### \"ignored\"</DisplayFormatString>"
            "              </DisplayUnitSpecification>"
            "          </ECCustomAttributes>"
            "        </ECProperty>"
            "    </ECEntityClass>"
            "</ECSchema>";

        return format;
        }

    //---------------------------------------------------------------------------------------//
    // Creates the test schema using the TestSchemaXml string
    // @bsimethod                             Prasanna.Prakash                       03/2016
    //+---------------+---------------+---------------+---------------+---------------+------//
    void CreateTestSchema()
        {
        ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();

        ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(m_schema, TestSchemaXmlString(), *schemaContext))
               << "Failed to read the test reference schema from xml string";
        ASSERT_TRUE(m_schema.IsValid()) << "Test Schema is not valid";
        }
    };

void SerializeAndCheck(ECSchemaPtr &outputSchema, ECSchemaPtr inputSchema, Utf8CP customAttributeName, Utf8CP customAttributeInMemory = "")
    {
    Utf8String schemaXmlString;
    ASSERT_EQ(SchemaWriteStatus::Success, inputSchema->WriteToXmlString(schemaXmlString, 3, 0))
           << "Cannot serialize the schema";

    ASSERT_NE(Utf8String::npos, schemaXmlString.find(customAttributeName));
    if (!Utf8String::IsNullOrEmpty(customAttributeInMemory))
        ASSERT_EQ(Utf8String::npos, schemaXmlString.find(customAttributeInMemory));

    ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(outputSchema, schemaXmlString.c_str(), *schemaContext))
           << "Failed to read the test reference schema from xml string";
    }

void SerializeAndCheck(ECSchemaPtr inputSchema, bvector<Utf8CP> customAttributeNames)
    {
    Utf8String schemaXmlString;
    ASSERT_EQ(SchemaWriteStatus::Success, inputSchema->WriteToXmlString(schemaXmlString, 3, 0))
           << "Cannot serialize the schema";
    
    for (auto customAttributeName: customAttributeNames)
        ASSERT_NE(Utf8String::npos, schemaXmlString.find(customAttributeName));
    }

//---------------------------------------------------------------------------------------//
// Tests the IsUnitSystemSchema conversion at the schema level
// @bsimethod                             Prasanna.Prakash                       03/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(UnitsCustomAttributesConversionTests, TestIsUnitSystemSchemaCustomAttributeConversion)
    {
    CreateTestSchema();

    ASSERT_TRUE(m_schema->IsDefined("Unit_Attributes", "IsUnitSystemSchema"))
             << "IsUnitSystemSchema custom attribute was not read at schema level";

    ECSchemaPtr schema;
    SerializeAndCheck(schema, m_schema, "IsUnitSystemSchema");

    ASSERT_TRUE(schema->IsDefined("Unit_Attributes", "IsUnitSystemSchema"))
             << "IsUnitSystemSchema custom attribute was not read at schema level";
    }

//---------------------------------------------------------------------------------------//
// Tests the UnitSpecifications conversion at the schema level
// @bsimethod                             Prasanna.Prakash                       03/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(UnitsCustomAttributesConversionTests, TestUnitSpecificationsCustomAttributeConversion)
    {
    CreateTestSchema();

    ASSERT_TRUE(m_schema->IsDefined("Unit_Attributes", "UnitSpecifications"))
             << "UnitSpecifications custom attribute was not read at schema level";

    ECSchemaPtr schema;
    SerializeAndCheck(schema, m_schema, "UnitSpecifications");

    ASSERT_TRUE(schema->IsDefined("Unit_Attributes", "UnitSpecifications"))
             << "UnitSpecifications custom attribute was not read at schema level";
    }

//---------------------------------------------------------------------------------------//
// Tests the UnitSpecification conversion at the property level
// @bsimethod                             Prasanna.Prakash                       03/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(UnitsCustomAttributesConversionTests, TestUnitSpecificationCustomAttributeConversion)
    {
    CreateTestSchema();

    ASSERT_TRUE(m_schema->GetClassP("TestClass")->GetPropertyP("PropertyA")->IsDefined("Unit_Attributes", "UnitSpecificationAttr"))
             << "UnitSpecifications custom attribute was not read at property level";

    ECSchemaPtr schema;
    SerializeAndCheck(schema, m_schema, "UnitSpecification", "UnitSpecificationAttr");

    ASSERT_TRUE(schema->GetClassP("TestClass")->GetPropertyP("PropertyA")->IsDefined("Unit_Attributes", "UnitSpecificationAttr"))
             << "UnitSpecifications custom attribute was not read at property level";
    }


Utf8String tempoutString;

//---------------------------------------------------------------------------------------//
// Tests the DisplayUnitSpecification conversion at the property level
// @bsimethod                             Prasanna.Prakash                       03/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(UnitsCustomAttributesConversionTests, TestDisplayUnitSpecificationCustomAttributeConversion)
    {
    CreateTestSchema();

    ASSERT_TRUE(m_schema->GetClassP("TestClass")->GetPropertyP("PropertyB")->IsDefined("Unit_Attributes", "DisplayUnitSpecificationAttr"))
             << "UnitSpecifications custom attribute was not read at schema level";

    ECSchemaPtr schema;
    SerializeAndCheck(schema, m_schema, "DisplayUnitSpecification", "DisplayUnitSpecificationAttr");

    ASSERT_TRUE(schema->GetClassP("TestClass")->GetPropertyP("PropertyB")->IsDefined("Unit_Attributes", "DisplayUnitSpecificationAttr"))
             << "UnitSpecifications custom attribute was not read at schema level";
    }

//---------------------------------------------------------------------------------------//
// Tests whether the ElementNames of the internal attributes defined under the BaseUnit 
// Custom Attribute remains the same.
// @bsimethod                             Prasanna.Prakash                       03/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(UnitsCustomAttributesConversionTests, TestInternalCustomAttributes)
    {
    CreateTestSchema();

    ECSchemaPtr tempSchema;
    bvector<Utf8CP> testCustomAttributes;
    
    testCustomAttributes.push_back("UnitSpecificationList");
    testCustomAttributes.push_back("UnitName");
    testCustomAttributes.push_back("KindOfQuantityName");
    testCustomAttributes.push_back("DimensionName");
    testCustomAttributes.push_back("DisplayUnitName");
    testCustomAttributes.push_back("DisplayFormatString");

    SerializeAndCheck(m_schema, testCustomAttributes);
    }

END_BENTLEY_ECN_TEST_NAMESPACE