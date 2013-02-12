/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/StandardCustomAttributeHelperTests.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include "TestFixture.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
struct StandardCustomAttributeHelperTestFixture : ECTestFixture 
    {
public:
    struct ExpectedResult
        {
        bool m_HasDateTimeInfo;
        bool m_hasKind;
        DateTime::Kind m_kind;
        bool m_hasComponent;
        DateTime::Component m_component;

        ExpectedResult () : m_HasDateTimeInfo (false) {};
        ExpectedResult (DateTime::Kind kind, DateTime::Component component) : m_HasDateTimeInfo (true), m_hasKind (true), m_kind (kind), m_hasComponent (true), m_component (component) {}
        ExpectedResult (DateTime::Kind kind) : m_HasDateTimeInfo (true), m_hasKind (true), m_kind (kind), m_hasComponent (false) {}
        ExpectedResult (DateTime::Component component) : m_HasDateTimeInfo (true), m_hasKind (false), m_hasComponent (true), m_component (component) {}
        };

    typedef bpair<WString, ExpectedResult> ExpectedResultPerProperty;
    typedef bvector<ExpectedResultPerProperty> ExpectedResults;

private:
    static ECSchemaPtr DeserializeSchema (ECSchemaReadContextPtr& context, Utf8CP schemaXml)
        {
        EXPECT_FALSE (Utf8String::IsNullOrEmpty (schemaXml));

        context = ECSchemaReadContext::CreateContext ();

        ECSchemaPtr schema;
        SchemaReadStatus stat = ECSchema::ReadFromXmlString (schema, schemaXml, *context);
        EXPECT_EQ (SCHEMA_READ_STATUS_Success, stat);
        EXPECT_TRUE (schema.IsValid ());

        return schema;
        }

protected:
    static void Assert (ECPropertyCR dateTimeProperty, ExpectedResult const& expected)
        {
        DateTimeInfo actual;
        bool found = StandardCustomAttributeHelper::TryGetDateTimeInfo (actual, dateTimeProperty);

        EXPECT_EQ (expected.m_HasDateTimeInfo, found);
        }

    static ECSchemaPtr CreateTestSchema (ECSchemaReadContextPtr& context, ExpectedResults& expectedResults)
        {
        Utf8CP testSchemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<ECSchema schemaName=\"StandardClassesHelperTest\" nameSpacePrefix=\"t\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "   <ECSchemaReference name=\"Bentley_Standard_CustomAttributes\" version=\"01.05\" prefix=\"bsca\" />"
            "   <ECClass typeName=\"TestClass\" isDomainClass=\"True\">"
            "        <ECProperty propertyName=\"nodatetimeinfo\" typeName=\"dateTime\" />"
            "        <ECProperty propertyName=\"emptydatetimeinfo\" typeName=\"dateTime\" >"
            "           <ECCustomAttributes>"
            "               <DateTimeInfo xmlns=\"Bentley_Standard_CustomAttributes.01.05\">"
            "               </DateTimeInfo>"
            "           </ECCustomAttributes>"
            "        </ECProperty>"                    
            "        <ECProperty propertyName=\"utc\" typeName=\"dateTime\" >"
            "           <ECCustomAttributes>"
            "               <DateTimeInfo xmlns=\"Bentley_Standard_CustomAttributes.01.05\">"
            "                   <DateTimeKind>Utc</DateTimeKind>"
            "               </DateTimeInfo>"
            "           </ECCustomAttributes>"
            "        </ECProperty>"                    
            "        <ECProperty propertyName=\"unspecified\" typeName=\"dateTime\" >"
            "           <ECCustomAttributes>"
            "               <DateTimeInfo xmlns=\"Bentley_Standard_CustomAttributes.01.05\">"
            "                   <DateTimeKind>Unspecified</DateTimeKind>"
            "               </DateTimeInfo>"
            "           </ECCustomAttributes>"
            "        </ECProperty>"                    
            "        <ECProperty propertyName=\"local\" typeName=\"dateTime\" >"
            "           <ECCustomAttributes>"
            "               <DateTimeInfo xmlns=\"Bentley_Standard_CustomAttributes.01.05\">"
            "                   <DateTimeKind>Local</DateTimeKind>"
            "               </DateTimeInfo>"
            "           </ECCustomAttributes>"
            "        </ECProperty>"                    
            "        <ECProperty propertyName=\"garbagekind\" typeName=\"dateTime\" >"
            "           <ECCustomAttributes>"
            "               <DateTimeInfo xmlns=\"Bentley_Standard_CustomAttributes.01.05\">"
            "                   <DateTimeKind>Garbage</DateTimeKind>"
            "               </DateTimeInfo>"
            "           </ECCustomAttributes>"
            "        </ECProperty>"                    
            "        <ECProperty propertyName=\"dateonly\" typeName=\"dateTime\" >"
            "           <ECCustomAttributes>"
            "               <DateTimeInfo xmlns=\"Bentley_Standard_CustomAttributes.01.05\">"
            "                   <DateTimeComponent>Date</DateTimeComponent>"
            "               </DateTimeInfo>"
            "           </ECCustomAttributes>"
            "        </ECProperty>"                    
            "        <ECProperty propertyName=\"garbagecomponent\" typeName=\"dateTime\" >"
            "           <ECCustomAttributes>"
            "               <DateTimeInfo xmlns=\"Bentley_Standard_CustomAttributes.01.05\">"
            "                   <DateTimeComponent>Garbage</DateTimeComponent>"
            "               </DateTimeInfo>"
            "           </ECCustomAttributes>"
            "        </ECProperty>"                    
            "        <ECProperty propertyName=\"garbagekindgarbagecomponent\" typeName=\"dateTime\" >"
            "           <ECCustomAttributes>"
            "               <DateTimeInfo xmlns=\"Bentley_Standard_CustomAttributes.01.05\">"
            "                   <DateTimeKind>Garbage</DateTimeKind>"
            "                   <DateTimeComponent>Garbage</DateTimeComponent>"
            "               </DateTimeInfo>"
            "           </ECCustomAttributes>"
            "        </ECProperty>"                    
            "    </ECClass>"
            "</ECSchema>";

        expectedResults.clear ();
        expectedResults.push_back (ExpectedResultPerProperty (L"nodatetimeinfo", ExpectedResult ()));
        expectedResults.push_back (ExpectedResultPerProperty (L"emptydatetimeinfo", ExpectedResult ()));
        expectedResults.push_back (ExpectedResultPerProperty (L"utc", ExpectedResult (DateTime::DATETIMEKIND_Utc)));
        expectedResults.push_back (ExpectedResultPerProperty (L"unspecified", ExpectedResult (DateTime::DATETIMEKIND_Unspecified)));
        expectedResults.push_back (ExpectedResultPerProperty (L"local", ExpectedResult (DateTime::DATETIMEKIND_Local)));
        expectedResults.push_back (ExpectedResultPerProperty (L"garbagekind", ExpectedResult ()));
        expectedResults.push_back (ExpectedResultPerProperty (L"dateonly", ExpectedResult (DateTime::DATETIMECOMPONENT_Date)));
        expectedResults.push_back (ExpectedResultPerProperty (L"garbagecomponent", ExpectedResult ()));
        expectedResults.push_back (ExpectedResultPerProperty (L"garbagekindgarbagecomponent", ExpectedResult ()));

        return DeserializeSchema (context, testSchemaXml);
        }

    static ECSchemaPtr CreateTestSchemaWithCorruptDateTimeInfoCA (ECSchemaReadContextPtr& context)
        {
        Utf8CP testSchemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<ECSchema schemaName=\"StandardClassesHelperTest\" nameSpacePrefix=\"t\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "   <ECClass typeName=\"DateTimeInfo\" isDomainClass=\"False\" isCustomAttributeClass=\"True\" >"
            "        <ECProperty propertyName=\"SomethingUnexpected\" typeName=\"string\" />"
            "   </ECClass>"
            "   <ECClass typeName=\"TestClass\" isDomainClass=\"True\">"
            "        <ECProperty propertyName=\"prop\" typeName=\"dateTime\" >"
            "           <ECCustomAttributes>"
            "               <DateTimeInfo>"
            "                   <SomethingUnexpected>Utc</SomethingUnexpected>"
            "               </DateTimeInfo>"
            "           </ECCustomAttributes>"
            "        </ECProperty>"                    
            "    </ECClass>"
            "</ECSchema>";

        return DeserializeSchema (context, testSchemaXml);
        }
    };


//---------------------------------------------------------------------------------------
// @bsimethod                                                    
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeHelperTestFixture, TryGetDateTimeInfo)
    {
    ExpectedResults expectedResults;
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema (context, expectedResults);
    
    ECClassP testClass = testSchema->GetClassP (L"TestClass");
    ASSERT_TRUE (testClass != NULL);

    FOR_EACH (ExpectedResultPerProperty const& result, expectedResults)
        {
        ECPropertyP prop = testClass->GetPropertyP (result.first.c_str ());
        Assert (*prop, result.second);
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                    
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeHelperTestFixture, TryGetDateTimeInfoWithCorruptCADefinition)
    {
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchemaWithCorruptDateTimeInfoCA (context);

    ECClassP testClass = testSchema->GetClassP (L"TestClass");
    ASSERT_TRUE (testClass != NULL);

    DISABLE_ASSERTS
    ECPropertyP prop = testClass->GetPropertyP (L"prop");
    DateTimeInfo dti;
    bool found = StandardCustomAttributeHelper::TryGetDateTimeInfo (dti, *prop);
    EXPECT_FALSE (found);
    };

END_BENTLEY_ECOBJECT_NAMESPACE
