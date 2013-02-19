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
            "        <ECArrayProperty propertyName=\"dateTimeArrayProp\" typeName=\"dateTime\" minOccurs=\"0\" maxOccurs=\"unbounded\">"
            "           <ECCustomAttributes>"
            "               <DateTimeInfo xmlns=\"Bentley_Standard_CustomAttributes.01.05\">"
            "                   <DateTimeKind>Utc</DateTimeKind>"
            "               </DateTimeInfo>"
            "           </ECCustomAttributes>"
            "        </ECArrayProperty>"                    
            "        <ECProperty propertyName=\"intProp\" typeName=\"int\" />"
            "        <ECArrayProperty propertyName=\"intArrayProp\" typeName=\"int\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
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
            expectedResults.push_back (ExpectedResultPerProperty (L"dateTimeArrayProp", ExpectedResult (DateTime::DATETIMEKIND_Utc)));

            return DateTimeInfoTestFixture::CreateTestSchema (context);
            }


        //---------------------------------------------------------------------------------------
        // @bsimethod                                   Krischan.Eberle                  02/13                               
        //+---------------+---------------+---------------+---------------+---------------+------
        static ECSchemaPtr CreateTestSchemaNotReferencingBSCA (ECSchemaReadContextPtr& context)
            {
            Utf8CP testSchemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                "<ECSchema schemaName=\"StandardClassesHelperTest\" nameSpacePrefix=\"t\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                "   <ECClass typeName=\"TestClass\" isDomainClass=\"True\">"
                "        <ECProperty propertyName=\"prop1\" typeName=\"dateTime\" />"
                "        <ECProperty propertyName=\"prop2\" typeName=\"int\" />"
                "    </ECClass>"
                "</ECSchema>";

            return DeserializeSchema (context, testSchemaXml);
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
    static void AssertSetValue (IECInstancePtr instance, WCharCP propertyName, bool expectedKindIsNull, bool expectedComponentIsNull, DateTime::Info const& expectedInfo)
        {
        bvector<DateTime> testDateTimes;
        testDateTimes.push_back (DateTime::GetCurrentTimeUtc ());
        testDateTimes.push_back (DateTime (DateTime::DATETIMEKIND_Unspecified, 2013, 2, 18, 14, 22));
        testDateTimes.push_back (DateTime (2013, 2, 18));

        FOR_EACH (DateTimeCR testDateTime, testDateTimes)
            {
            ECValue value;
            EXPECT_EQ (SUCCESS, value.SetDateTime (testDateTime)) << "Return value of ECValue::SetDateTime";

            DateTime::Info const& actualInfo = testDateTime.GetInfo ();
            const bool isExpectedToSucceed = (expectedKindIsNull || expectedInfo.GetKind () == actualInfo.GetKind ()) &&
                                            (expectedComponentIsNull || expectedInfo.GetComponent () == actualInfo.GetComponent ());
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
        ASSERT_TRUE (prop != NULL);
        Assert (*prop, result.second);
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeHelperTestFixture, TryGetDateTimeInfoInSchemaNotReferencingBSCA)
    {
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchemaNotReferencingBSCA (context);

    ECClassP testClass = testSchema->GetClassP (L"TestClass");
    ASSERT_TRUE (testClass != NULL);

    ECPropertyP prop = testClass->GetPropertyP (L"prop1");
    DateTimeInfo dti;
    bool found = StandardCustomAttributeHelper::TryGetDateTimeInfo (dti, *prop);
    EXPECT_FALSE (found) << "No DateTimeInfo CA expected on property that doesn't have the DateTimeInfo CA";
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeHelperTestFixture, TryGetDateTimeForNonDateTimeProperties)
    {
    ExpectedResults expectedResults;
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema (context, expectedResults);

    ECClassP testClass = testSchema->GetClassP (L"TestClass");
    ASSERT_TRUE (testClass != NULL);

    ECPropertyP prop = testClass->GetPropertyP (L"intProp");
    ASSERT_TRUE (prop != NULL);
    DISABLE_ASSERTS

    DateTimeInfo dti;
    bool found = StandardCustomAttributeHelper::TryGetDateTimeInfo (dti, *prop);
    ASSERT_FALSE (found);

    prop = testClass->GetPropertyP (L"intArrayProp");
    ASSERT_TRUE (prop != NULL);
    found = StandardCustomAttributeHelper::TryGetDateTimeInfo (dti, *prop);
    ASSERT_FALSE (found);
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

    AssertSetValue (instance, L"nodatetimeinfo", true, true, DateTime::Info ());
    AssertSetValue (instance, L"emptydatetimeinfo", true, true, DateTime::Info ());
    AssertSetValue (instance, L"utc", false, true, DateTime::Info (DateTime::DATETIMEKIND_Utc, DateTime::DATETIMECOMPONENT_DateTime));
    AssertSetValue (instance, L"unspecified", false, true, DateTime::Info (DateTime::DATETIMEKIND_Unspecified, DateTime::DATETIMECOMPONENT_DateTime));
    AssertSetValue (instance, L"dateonly", true, false, DateTime::Info (DateTime::DATETIMEKIND_Unspecified, DateTime::DATETIMECOMPONENT_Date));

    //wrong values are treated as if the meta data wasn't specified
    AssertSetValue (instance, L"garbagekind", true, true, DateTime::Info ());
    AssertSetValue (instance, L"garbagecomponent", true, true, DateTime::Info ());
    AssertSetValue (instance, L"garbagekindgarbagecomponent", true, true, DateTime::Info ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceGetSetDateTimeTestFixture, SetDateTimeTicks)
    {
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema (context);

    ECClassP testClass = testSchema->GetClassP (L"TestClass");
    ASSERT_TRUE (testClass != NULL);

    DateTime expectedDate = DateTime::GetCurrentTimeUtc ();
    Int64 ceTicks = 0LL;
    ASSERT_EQ (SUCCESS, expectedDate.ToCommonEraTicks (ceTicks));

    ECValue value;
    value.SetDateTimeTicks (ceTicks);

    WCharCP propName = L"utc";
    IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ASSERT_TRUE (instance.IsValid ());
    ECObjectsStatus stat = instance->SetValue (propName, value);
    ASSERT_EQ (ECOBJECTS_STATUS_DataTypeMismatch, stat) << "Setting an ECValue which has only been populated from Common Era ticks in a date time ECProperty which has a DateTimeInfo CA is not supported.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceGetSetDateTimeTestFixture, SetDateTimeWithLocalDateTimeKind)
    {
    bvector<DateTime> testDateTimes;
    testDateTimes.push_back (DateTime (DateTime::DATETIMEKIND_Utc, 2013, 2, 18, 14, 22));
    testDateTimes.push_back (DateTime (DateTime::DATETIMEKIND_Unspecified, 2013, 2, 18, 14, 22));
    testDateTimes.push_back (DateTime (DateTime::DATETIMEKIND_Local, 2013, 2, 18, 14, 22));
    testDateTimes.push_back (DateTime (2013, 2, 18));

    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema (context);

    ECClassP testClass = testSchema->GetClassP (L"TestClass");
    ASSERT_TRUE (testClass != NULL);
    WCharCP localDateTimePropName = L"local";

    DISABLE_ASSERTS

    FOR_EACH (DateTimeCR testDateTime, testDateTimes)
        {
        const bool isLocal = testDateTime.GetInfo ().GetKind () == DateTime::DATETIMEKIND_Local;
        ECValue value;
        const BentleyStatus expectedStat = isLocal ? ERROR : SUCCESS;
        EXPECT_EQ (expectedStat, value.SetDateTime (testDateTime)) << "Return value of ECValue::SetDateTime ('" << testDateTime.ToString ().c_str () << "')";

        value = ECValue (testDateTime);
        if (!isLocal)
            {
            EXPECT_FALSE (value.IsNull ()) << "ECValue (DateTime) is expected to return an ECValue which is not IsNull if the passed DateTime was not local.";
            EXPECT_TRUE (testDateTime == value.GetDateTime ()) << "ECValue::GetDateTime () is expected to return a non-empty DateTime if the passed DateTime was not local.";
            }
        else
            {
            EXPECT_TRUE (value.IsNull ()) << "ECValue (local DateTime) is expected to return an ECValue which is IsNull.";
            EXPECT_TRUE (DateTime () == value.GetDateTime ()) << "ECValue (local DateTime)::GetDateTime () is expected to return an empty DateTime.";
            }

        IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
        ASSERT_TRUE (instance.IsValid ());

        if (!isLocal)
            {
            EXPECT_NE (ECOBJECTS_STATUS_Success, instance->SetValue (localDateTimePropName, value)) << "IECInstance::SetValue> Property name: " << localDateTimePropName << " DateTime: " << testDateTime.ToString ().c_str ();
            }
        else
            {
            EXPECT_EQ (ECOBJECTS_STATUS_Success, instance->SetValue (localDateTimePropName, value)) << "IECInstance::SetValue> Property name: " << localDateTimePropName << " with a local date time is expected to succeed, but the inserted value is null.";
            }
        }
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
