/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/DateTimeInfoCustomAttributeTests.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include "TestFixture.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
//********************** DateTimeInfoTestFixture base classes *************************************
//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      02/2013
//+===============+===============+===============+===============+===============+======
struct DateTimeInfoTestFixture : ECTestFixture 
    {
protected:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Krischan.Eberle                  02/13                               
    //+---------------+---------------+---------------+---------------+---------------+------
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

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Krischan.Eberle                  02/13                               
    //+---------------+---------------+---------------+---------------+---------------+------
    static ECSchemaPtr CreateTestSchema (ECSchemaReadContextPtr& context)
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

        return DeserializeSchema (context, testSchemaXml);
        }
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      02/2013
//+===============+===============+===============+===============+===============+======
struct StandardCustomAttributeHelperTestFixture : public DateTimeInfoTestFixture
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

    protected:
        //---------------------------------------------------------------------------------------
        // @bsimethod                                   Krischan.Eberle                  02/13                               
        //+---------------+---------------+---------------+---------------+---------------+------
        static void Assert (ECPropertyCR dateTimeProperty, ExpectedResult const& expected)
            {
            DateTimeInfo actual;
            bool found = StandardCustomAttributeHelper::TryGetDateTimeInfo (actual, dateTimeProperty);

            EXPECT_EQ (expected.m_HasDateTimeInfo, found);
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                   Krischan.Eberle                  02/13                               
        //+---------------+---------------+---------------+---------------+---------------+------
        static ECSchemaPtr CreateTestSchema (ECSchemaReadContextPtr& context, ExpectedResults& expectedResults)
            {
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

            return DateTimeInfoTestFixture::CreateTestSchema (context);
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                   Krischan.Eberle                  02/13                               
        //+---------------+---------------+---------------+---------------+---------------+------
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

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      02/2013
//+===============+===============+===============+===============+===============+======
struct ECInstanceGetSetDateTimeTestFixture : DateTimeInfoTestFixture
    {
protected:
    static void AssertSetValue (IECInstancePtr instance, WCharCP propertyName, ECN::DateTimeInfo const& expected = DateTimeInfo (true, DateTime::DATETIMEKIND_Unspecified, true, DateTime::DATETIMECOMPONENT_DateTime))
        {
        bvector<DateTime> testDateTimes;
        testDateTimes.push_back (DateTime::GetCurrentTimeUtc ());
        testDateTimes.push_back (DateTime (DateTime::DATETIMEKIND_Unspecified, 2013, 2, 18, 14, 22));
        testDateTimes.push_back (DateTime (2013, 2, 18));

        FOR_EACH (DateTimeCR testDateTime, testDateTimes)
            {
            ECValue value;
            EXPECT_EQ (SUCCESS, value.SetDateTime (testDateTime)) << "Return value of ECValue::SetDateTime";

            const bool isExpectedToSucceed = (expected.IsKindNull () || testDateTime.GetInfo ().GetKind () == expected.GetInfo ().GetKind ()) &&
                                             (expected.IsComponentNull () || testDateTime.GetInfo ().GetComponent () == expected.GetInfo ().GetComponent ()); 

            if (!isExpectedToSucceed)
                {
                DISABLE_ASSERTS
                ECObjectsStatus stat = instance->SetValue (propertyName, value);
                EXPECT_NE (ECOBJECTS_STATUS_Success, stat) << "IECInstance::SetValue with a mismatching DateTimeKind> Property: " << propertyName << " DateTime: " << testDateTime.ToString ().c_str ();
                }
            else
                {
                ECObjectsStatus stat = instance->SetValue (propertyName, value);
                EXPECT_EQ (ECOBJECTS_STATUS_Success, stat) << "IECInstance::SetValue with a matching DateTimeKind> Property: " << propertyName << " DateTime: " << testDateTime.ToString ().c_str ();
                }
            }
        }

    static void AssertSetValueWithInvalidDateTimeInfo (IECInstancePtr instance, WCharCP propertyName)
        {
        bvector<DateTime> testDateTimes;
        testDateTimes.push_back (DateTime (DateTime::DATETIMEKIND_Utc, 2013, 2, 18, 14, 22));
        testDateTimes.push_back (DateTime (DateTime::DATETIMEKIND_Unspecified, 2013, 2, 18, 14, 22));
        testDateTimes.push_back (DateTime (DateTime::DATETIMEKIND_Local, 2013, 2, 18, 14, 22));
        testDateTimes.push_back (DateTime (2013, 2, 18));

        DISABLE_ASSERTS

        FOR_EACH (DateTimeCR testDateTime, testDateTimes)
            {
            ECValue value;
            const BentleyStatus expectedStat = testDateTime.GetInfo ().GetKind () != DateTime::DATETIMEKIND_Local ? SUCCESS : ERROR;
            EXPECT_EQ (expectedStat, value.SetDateTime (testDateTime)) << "Return value of ECValue::SetDateTime ('" << testDateTime.ToString ().c_str () << "')";

            EXPECT_NE (ECOBJECTS_STATUS_Success, instance->SetValue (propertyName, value)) << "IECInstance::SetValue> Property name: " << propertyName << " DateTime: " << testDateTime.ToString ().c_str ();
            }
        }

    static void AssertGetValue (IECInstancePtr instance, WCharCP propertyName, DateTimeCR expectedDateTime, bool expectedMatch)
        {
            {
            ECValue value (expectedDateTime);
            ASSERT_EQ (ECOBJECTS_STATUS_Success, instance->SetValue (propertyName, value)) << "AssertGetValue:Setup> Setting datetime value in ECInstance";
            }

        ECValue value;
        ECObjectsStatus stat = instance->GetValue (value, propertyName);
        ASSERT_EQ (ECOBJECTS_STATUS_Success, stat) << "IECInstance::GetValue";

        DateTime actualDateTime = value.GetDateTime ();

        if (expectedMatch)
            {
            EXPECT_TRUE (expectedDateTime == actualDateTime) << "Instance::GetValue (" << propertyName << ") returned mismatching dateTime. Expected: " << expectedDateTime.ToString ().c_str () << ". Actual: " << actualDateTime.ToString ().c_str ();
            }
        else
            {
            EXPECT_FALSE (expectedDateTime == actualDateTime) << "Instance::GetValue (" << propertyName << ") returned matching dateTime. Expected: " << expectedDateTime.ToString ().c_str () << ". Actual: " << actualDateTime.ToString ().c_str ();
            }
        }
    };

//********************** StandardCustomAttributeHelper Tests *************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
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
// @bsimethod                                   Krischan.Eberle                  02/13                               
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

//********************** ECInstance SetValue / GetValue Tests *************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceGetSetDateTimeTestFixture, SetDateTime)
    {
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema (context);

    ECClassP testClass = testSchema->GetClassP (L"TestClass");
    ASSERT_TRUE (testClass != NULL);

    IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ASSERT_TRUE (instance.IsValid ());

    AssertSetValue (instance, L"nodatetimeinfo");
    AssertSetValue (instance, L"emptydatetimeinfo");
    AssertSetValue (instance, L"utc", DateTimeInfo (false, DateTime::DATETIMEKIND_Utc, true, DateTime::DATETIMECOMPONENT_DateTime));
    AssertSetValue (instance, L"unspecified", DateTimeInfo (false, DateTime::DATETIMEKIND_Unspecified, true, DateTime::DATETIMECOMPONENT_DateTime));
    AssertSetValue (instance, L"dateonly", DateTimeInfo (true, DateTime::DATETIMEKIND_Unspecified, false, DateTime::DATETIMECOMPONENT_Date));

    //wrong values are treated as if the meta data wasn't specified
    AssertSetValue (instance, L"garbagekind", DateTimeInfo (true, DateTime::DATETIMEKIND_Unspecified, true, DateTime::DATETIMECOMPONENT_DateTime));
    AssertSetValue (instance, L"garbagecomponent", DateTimeInfo (true, DateTime::DATETIMEKIND_Unspecified, true, DateTime::DATETIMECOMPONENT_DateTime));
    AssertSetValue (instance, L"garbagekindgarbagecomponent", DateTimeInfo (true, DateTime::DATETIMEKIND_Unspecified, true, DateTime::DATETIMECOMPONENT_DateTime));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceGetSetDateTimeTestFixture, SetDateTimeWithInvalidDateTimeInfo)
    {
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema (context);

    ECClassP testClass = testSchema->GetClassP (L"TestClass");
    ASSERT_TRUE (testClass != NULL);

    IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ASSERT_TRUE (instance.IsValid ());

    AssertSetValueWithInvalidDateTimeInfo (instance, L"local");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceGetSetDateTimeTestFixture, GetDateTime)
    {
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema (context);

    ECClassP testClass = testSchema->GetClassP (L"TestClass");
    ASSERT_TRUE (testClass != NULL);

    IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ASSERT_TRUE (instance.IsValid ());

    DateTime expectedDateTime (DateTime::DATETIMEKIND_Utc, 2013, 2, 18, 14, 28, 34, 1234567);

    AssertGetValue (instance, L"utc", expectedDateTime, true);
    AssertGetValue (instance, L"nodatetimeinfo", expectedDateTime, false);
    AssertGetValue (instance, L"emptydatetimeinfo", expectedDateTime, false);
    AssertGetValue (instance, L"garbagekind", expectedDateTime, false);
    AssertGetValue (instance, L"garbagekindgarbagecomponent", expectedDateTime, false);
    AssertGetValue (instance, L"garbagecomponent", expectedDateTime, false);

    //Unspecified is the default kind, therefore any property that doesn't have a kind CA value (or an invalid value) return
    //a matching date time
    instance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ASSERT_TRUE (instance.IsValid ());

    expectedDateTime = DateTime (DateTime::DATETIMEKIND_Unspecified, 2013, 2, 18, 14, 28, 34, 1234567);

    AssertGetValue (instance, L"unspecified", expectedDateTime, true);
    AssertGetValue (instance, L"nodatetimeinfo", expectedDateTime, true);
    AssertGetValue (instance, L"emptydatetimeinfo", expectedDateTime, true);
    AssertGetValue (instance, L"garbagekind", expectedDateTime, true);
    AssertGetValue (instance, L"garbagekindgarbagecomponent", expectedDateTime, true);
    AssertGetValue (instance, L"garbagecomponent", expectedDateTime, true);

    //DateTime is the default component. Therefore any property that 
    //doesn't have a component CA value (or an invalid value) returns a mismatching date time
    instance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ASSERT_TRUE (instance.IsValid ());

    expectedDateTime = DateTime (2013, 2, 18);

    AssertGetValue (instance, L"dateonly", expectedDateTime, true);
    AssertGetValue (instance, L"nodatetimeinfo", expectedDateTime, false);
    AssertGetValue (instance, L"emptydatetimeinfo", expectedDateTime, false);
    AssertGetValue (instance, L"garbagekind", expectedDateTime, false);
    AssertGetValue (instance, L"garbagekindgarbagecomponent", expectedDateTime, false);
    AssertGetValue (instance, L"garbagecomponent", expectedDateTime, false);
    }

END_BENTLEY_ECOBJECT_NAMESPACE
