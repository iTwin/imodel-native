/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/DateTimeInfoCustomAttributeTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

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
    static void AssertDateTime (DateTimeCR expected, DateTimeCR actual, bool ignoreDateTimeInfo)
        {
        Utf8String expectedActualStr;
        expectedActualStr.Sprintf ("Expected: %s - Actual: %s", expected.ToString ().c_str(), actual.ToString ().c_str());

        if (ignoreDateTimeInfo)
            {
            EXPECT_TRUE (expected.Equals (actual, true)) << "DateTimes are expected to be equal except for date time info. " << expectedActualStr.c_str ();
            EXPECT_FALSE (expected == actual) << "DateTime metadata is expected to differ. " << expectedActualStr.c_str ();
            }
        else
            {
            EXPECT_TRUE (expected == actual) << "DateTimes are expected to be equal. " << expectedActualStr.c_str ();
            }
        }


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

        typedef bpair<Utf8String, ExpectedResult> ExpectedResultPerProperty;
        typedef bvector<ExpectedResultPerProperty> ExpectedResults;

    protected:
        //---------------------------------------------------------------------------------------
        // @bsimethod                                   Krischan.Eberle                  02/13                               
        //+---------------+---------------+---------------+---------------+---------------+------
        static void Assert (ECPropertyCR dateTimeProperty, ExpectedResult const& expected)
            {
            DateTimeInfo actual;
            const ECObjectsStatus stat = StandardCustomAttributeHelper::GetDateTimeInfo (actual, dateTimeProperty);
            //if retrieval failed, m_HasDateTimeInfo should be false, too.
            if (stat != ECOBJECTS_STATUS_Success)
                ASSERT_FALSE (expected.m_HasDateTimeInfo);
                
            EXPECT_EQ (expected.m_HasDateTimeInfo, !actual.IsNull ());
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                   Krischan.Eberle                  02/13                               
        //+---------------+---------------+---------------+---------------+---------------+------
        static ECSchemaPtr CreateTestSchema (ECSchemaReadContextPtr& context, ExpectedResults& expectedResults)
            {
            expectedResults.clear ();
            expectedResults.push_back (ExpectedResultPerProperty ("nodatetimeinfo", ExpectedResult ()));
            expectedResults.push_back (ExpectedResultPerProperty ("emptydatetimeinfo", ExpectedResult ()));
            expectedResults.push_back (ExpectedResultPerProperty ("utc", ExpectedResult (DateTime::Kind::Utc)));
            expectedResults.push_back (ExpectedResultPerProperty ("unspecified", ExpectedResult (DateTime::Kind::Unspecified)));
            expectedResults.push_back (ExpectedResultPerProperty ("local", ExpectedResult (DateTime::Kind::Local)));
            expectedResults.push_back (ExpectedResultPerProperty ("garbagekind", ExpectedResult ()));
            expectedResults.push_back (ExpectedResultPerProperty ("dateonly", ExpectedResult (DateTime::Component::Date)));
            expectedResults.push_back (ExpectedResultPerProperty ("garbagecomponent", ExpectedResult ()));
            expectedResults.push_back (ExpectedResultPerProperty ("garbagekindgarbagecomponent", ExpectedResult ()));
            expectedResults.push_back (ExpectedResultPerProperty ("dateTimeArrayProp", ExpectedResult (DateTime::Kind::Utc)));

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
    static void AssertSetValue (IECInstancePtr instance, Utf8CP propertyName, bool retrievalExpectedToFail, bool expectedKindIsNull, bool expectedComponentIsNull, DateTime::Info const& expectedInfo)
        {
        bvector<DateTime> testDateTimes;
        testDateTimes.push_back (DateTime::GetCurrentTimeUtc ());
        testDateTimes.push_back (DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 14, 22));
        testDateTimes.push_back (DateTime (2013, 2, 18));

        for (DateTimeCR testDateTime: testDateTimes)
            {
            ECValue value;
            EXPECT_EQ (SUCCESS, value.SetDateTime (testDateTime)) << "Return value of ECValue::SetDateTime";

            DateTime::Info const& actualInfo = testDateTime.GetInfo ();
            const bool isExpectedToSucceed = !retrievalExpectedToFail && (expectedKindIsNull || expectedInfo.GetKind () == actualInfo.GetKind ()) &&
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

    static void AssertGetValue (IECInstancePtr instance, Utf8CP propertyName, DateTimeCR expectedDateTime, bool expectedMatch)
        {
            {
            ECValue value (expectedDateTime);
            ASSERT_EQ (ECOBJECTS_STATUS_Success, instance->SetValue (propertyName, value)) << "AssertGetValue:Setup> Setting datetime value in ECInstance";
            }

        int64_t expectedTicks = 0LL;
        expectedDateTime.ToCommonEraTicks (expectedTicks);

        ECValue value;
        ECObjectsStatus stat = instance->GetValue (value, propertyName);
        ASSERT_EQ (ECOBJECTS_STATUS_Success, stat) << "IECInstance::GetValue";

        DateTime actualDateTime = value.GetDateTime ();

        bool actualHasMetadata = false;
        DateTime::Info actualMetadata;
        int64_t actualTicks = value.GetDateTimeTicks (actualHasMetadata, actualMetadata);

        if (expectedMatch)
            {
            EXPECT_TRUE (expectedDateTime == actualDateTime) << "Instance::GetValue (" << propertyName << ") returned mismatching dateTime. Expected: " << expectedDateTime.ToString ().c_str () << ". Actual: " << actualDateTime.ToString ().c_str ();
            if (actualHasMetadata)
                {
                EXPECT_TRUE (expectedDateTime.GetInfo () == actualMetadata);
                }

            EXPECT_EQ (expectedTicks, actualTicks);
            }
        else
            {
            EXPECT_FALSE (expectedDateTime == actualDateTime) << "Instance::GetValue (" << propertyName << ") returned matching dateTime. Expected: " << expectedDateTime.ToString ().c_str () << ". Actual: " << actualDateTime.ToString ().c_str ();
            if (actualHasMetadata)
                {
                EXPECT_FALSE (expectedDateTime.GetInfo () == actualMetadata);
                }

            EXPECT_EQ (expectedTicks, actualTicks);
            }
        }
    };

//********************** StandardCustomAttributeHelper Tests *************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeHelperTestFixture, GetDateTimeInfo)
    {
    ExpectedResults expectedResults;
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema (context, expectedResults);
    
    ECClassP testClass = testSchema->GetClassP ("TestClass");
    ASSERT_TRUE (testClass != NULL);

    for (ExpectedResultPerProperty const& result: expectedResults)
        {
        ECPropertyP prop = testClass->GetPropertyP (result.first.c_str ());
        ASSERT_TRUE (prop != NULL);
        Assert (*prop, result.second);
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeHelperTestFixture, GetDateTimeInfoInSchemaNotReferencingBSCA)
    {
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchemaNotReferencingBSCA (context);

    ECClassP testClass = testSchema->GetClassP ("TestClass");
    ASSERT_TRUE (testClass != NULL);

    ECPropertyP prop = testClass->GetPropertyP ("prop1");
    DateTimeInfo dti;
    const ECObjectsStatus stat = StandardCustomAttributeHelper::GetDateTimeInfo (dti, *prop);
    EXPECT_TRUE (stat == ECOBJECTS_STATUS_Success && dti.IsNull ()) << "No DateTimeInfo CA expected on property that doesn't have the DateTimeInfo CA";
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeHelperTestFixture, GetDateTimeForNonDateTimeProperties)
    {
    ExpectedResults expectedResults;
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema (context, expectedResults);

    ECClassP testClass = testSchema->GetClassP ("TestClass");
    ASSERT_TRUE (testClass != NULL);

    ECPropertyP prop = testClass->GetPropertyP ("intProp");
    ASSERT_TRUE (prop != NULL);
    DISABLE_ASSERTS

    DateTimeInfo dti;
    ECObjectsStatus stat = StandardCustomAttributeHelper::GetDateTimeInfo (dti, *prop);
    ASSERT_NE (ECOBJECTS_STATUS_Success, stat);

    prop = testClass->GetPropertyP ("intArrayProp");
    ASSERT_TRUE (prop != NULL);
    stat = StandardCustomAttributeHelper::GetDateTimeInfo (dti, *prop);
    ASSERT_NE (ECOBJECTS_STATUS_Success, stat);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeHelperTestFixture, GetDateTimeInfoWithCorruptCADefinition)
    {
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchemaWithCorruptDateTimeInfoCA (context);

    ECClassP testClass = testSchema->GetClassP ("TestClass");
    ASSERT_TRUE (testClass != NULL);

    DISABLE_ASSERTS
    ECPropertyP prop = testClass->GetPropertyP ("prop");
    DateTimeInfo dti;
    const ECObjectsStatus stat = StandardCustomAttributeHelper::GetDateTimeInfo (dti, *prop);
    ASSERT_NE (ECOBJECTS_STATUS_Success, stat);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StandardCustomAttributeHelperTestFixture, DateTimeInfoToString)
    {
    ExpectedResults expectedResults;
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema (context, expectedResults);

    ECClassP testClass = testSchema->GetClassP ("TestClass");
    ASSERT_TRUE (testClass != NULL);

    DateTimeInfo dti;
    Utf8String str = dti.ToString ();
    EXPECT_TRUE (str.empty ()) << "DateTimeInfo::ToString () is expected to return an empty string on an empty DateTimeInfo.";

    ECPropertyP ecproperty = testClass->GetPropertyP ("nodatetimeinfo");
    dti = DateTimeInfo ();
    StandardCustomAttributeHelper::GetDateTimeInfo (dti, *ecproperty);
    str = dti.ToString ();
    EXPECT_TRUE (str.empty ()) << "DateTimeInfo::ToString () is expected to return an empty string for an ECProperty not having the DateTimeInfo custom attribute.";

    ecproperty = testClass->GetPropertyP ("emptydatetimeinfo");
    dti = DateTimeInfo ();
    StandardCustomAttributeHelper::GetDateTimeInfo (dti, *ecproperty);
    str = dti.ToString ();
    EXPECT_TRUE (str.empty ()) << "DateTimeInfo::ToString () is expected to return an empty string for an ECProperty having an empty DateTimeInfo custom attribute.";

    ecproperty = testClass->GetPropertyP ("utc");
    dti = DateTimeInfo ();
    StandardCustomAttributeHelper::GetDateTimeInfo (dti, *ecproperty);
    str = dti.ToString ();
    EXPECT_STREQ ("Kind: Utc", str.c_str ()) << "DateTimeInfo::ToString ()";

    ecproperty = testClass->GetPropertyP ("unspecified");
    dti = DateTimeInfo ();
    StandardCustomAttributeHelper::GetDateTimeInfo (dti, *ecproperty);
    str = dti.ToString ();
    EXPECT_STREQ ("Kind: Unspecified", str.c_str ()) << "DateTimeInfo::ToString ()";

    ecproperty = testClass->GetPropertyP ("local");
    dti = DateTimeInfo ();
    StandardCustomAttributeHelper::GetDateTimeInfo (dti, *ecproperty);
    str = dti.ToString ();
    EXPECT_STREQ ("Kind: Local", str.c_str ()) << "DateTimeInfo::ToString ()";

    ecproperty = testClass->GetPropertyP ("dateonly");
    dti = DateTimeInfo ();
    StandardCustomAttributeHelper::GetDateTimeInfo (dti, *ecproperty);
    str = dti.ToString ();
    EXPECT_STREQ ("Component: Date", str.c_str ()) << "DateTimeInfo::ToString ()";

    ecproperty = testClass->GetPropertyP ("garbagekind");
    dti = DateTimeInfo ();
    StandardCustomAttributeHelper::GetDateTimeInfo (dti, *ecproperty);
    str = dti.ToString ();
    EXPECT_TRUE (str.empty ()) << "DateTimeInfo::ToString () is expected to return an empty string for an ECProperty having an DateTimeInfo custom attribute with garbage content.";

    ecproperty = testClass->GetPropertyP ("garbagecomponent");
    dti = DateTimeInfo ();
    StandardCustomAttributeHelper::GetDateTimeInfo (dti, *ecproperty);
    str = dti.ToString ();
    EXPECT_TRUE (str.empty ()) << "DateTimeInfo::ToString () is expected to return an empty string for an ECProperty having an DateTimeInfo custom attribute with garbage content.";

    ecproperty = testClass->GetPropertyP ("garbagekindgarbagecomponent");
    dti = DateTimeInfo ();
    StandardCustomAttributeHelper::GetDateTimeInfo (dti, *ecproperty);
    str = dti.ToString ();
    EXPECT_TRUE (str.empty ()) << "DateTimeInfo::ToString () is expected to return an empty string for an ECProperty having an DateTimeInfo custom attribute with garbage content.";
    };

//********************** ECInstance SetValue / GetValue Tests *************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceGetSetDateTimeTestFixture, SetDateTime)
    {
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema (context);

    ECClassP testClass = testSchema->GetClassP ("TestClass");
    ASSERT_TRUE (testClass != NULL);

    IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ASSERT_TRUE (instance.IsValid ());

    AssertSetValue (instance, "nodatetimeinfo", false, true, true, DateTime::Info ());
    AssertSetValue (instance, "emptydatetimeinfo", false, true, true, DateTime::Info ());
    AssertSetValue (instance, "utc", false, false, true, DateTime::Info (DateTime::Kind::Utc, DateTime::Component::DateAndTime));
    AssertSetValue (instance, "unspecified", false, false, true, DateTime::Info (DateTime::Kind::Unspecified, DateTime::Component::DateAndTime));
    AssertSetValue (instance, "dateonly", false, true, false, DateTime::Info (DateTime::Kind::Unspecified, DateTime::Component::Date));

    //wrong values are treated as if the meta data wasn't specified
    AssertSetValue (instance, "garbagekind", true, true, true, DateTime::Info ());
    AssertSetValue (instance, "garbagecomponent", true, true, true, DateTime::Info ());
    AssertSetValue (instance, "garbagekindgarbagecomponent", true, true, true, DateTime::Info ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceGetSetDateTimeTestFixture, SetDateTimeTicks)
    {
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema (context);

    ECClassP testClass = testSchema->GetClassP ("TestClass");
    ASSERT_TRUE (testClass != NULL);

    DateTime expectedDate = DateTime::GetCurrentTimeUtc ();
    int64_t ceTicks = 0LL;
    ASSERT_EQ (SUCCESS, expectedDate.ToCommonEraTicks (ceTicks));

    ECValue ticksOnlyValue;
    ticksOnlyValue.SetDateTimeTicks (ceTicks);
    ECValue ticksWithUtc;
    ticksWithUtc.SetDateTimeTicks (ceTicks, DateTime::Info (DateTime::Kind::Utc, DateTime::Component::DateAndTime));

    ECValue ticksWithDateOnly;
    ticksWithDateOnly.SetDateTimeTicks (ceTicks, DateTime::Info (DateTime::Kind::Unspecified, DateTime::Component::Date));

    IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ASSERT_TRUE (instance.IsValid ());

    ECObjectsStatus stat = instance->SetValue ("nodatetimeinfo", ticksOnlyValue);
    EXPECT_EQ (ECOBJECTS_STATUS_Success, stat) << "IECInstance::SetValue> ECProperty without DateTimeInfo - ECValue::SetDateTimeTicks (ticks): Expected to never mismatch with the DateTimeInfo custom attribute.";
    
    stat = instance->SetValue ("emptydatetimeinfo", ticksOnlyValue);
    EXPECT_EQ (ECOBJECTS_STATUS_Success, stat) << "IECInstance::SetValue> Empty DateTimeInfo - ECValue::SetDateTimeTicks (ticks): Expected to never mismatch with the DateTimeInfo custom attribute.";

    stat = instance->SetValue ("unspecified", ticksOnlyValue);
    EXPECT_EQ (ECOBJECTS_STATUS_Success, stat) << "IECInstance::SetValue> ECValue::SetDateTimeTicks (ticks): Expected to never mismatch with the DateTimeInfo custom attribute.";

    stat = instance->SetValue ("dateonly", ticksOnlyValue);
    EXPECT_EQ (ECOBJECTS_STATUS_Success, stat) << "IECInstance::SetValue> ECValue::SetDateTimeTicks (ticks): Expected to never mismatch with the DateTimeInfo custom attribute.";

    stat = instance->SetValue ("garbagekind", ticksOnlyValue);
    EXPECT_EQ (ECOBJECTS_STATUS_ParseError, stat) << "IECInstance::SetValue> ECValue::SetDateTimeTicks (ticks) is expected to fail because of invalid DateTimeInfo CA";

    stat = instance->SetValue ("garbagekindgarbagecomponent", ticksOnlyValue);
    EXPECT_EQ (ECOBJECTS_STATUS_ParseError, stat) << "IECInstance::SetValue> ECValue::SetDateTimeTicks (ticks) is expected to fail because of invalid DateTimeInfo CA";


    stat = instance->SetValue ("nodatetimeinfo", ticksWithUtc);
    EXPECT_EQ (ECOBJECTS_STATUS_Success, stat) << "IECInstance::SetValue> ECProperty without DateTimeInfo - ECValue::SetDateTimeTicks (ticks, Info): Expected to never mismatch with the DateTimeInfo custom attribute.";

    stat = instance->SetValue ("nodatetimeinfo", ticksWithDateOnly);
    EXPECT_EQ (ECOBJECTS_STATUS_Success, stat) << "IECInstance::SetValue> ECProperty without DateTimeInfo - ECValue::SetDateTimeTicks (ticks, Info): Expected to never mismatch with the DateTimeInfo custom attribute.";

    stat = instance->SetValue ("utc", ticksWithUtc);
    EXPECT_EQ (ECOBJECTS_STATUS_Success, stat) << "IECInstance::SetValue> DateTimeInfo (Utc)  - ECValue::SetDateTimeTicks (ticks, Utc): Expected to match with the DateTimeInfo custom attribute.";

    stat = instance->SetValue ("utc", ticksWithDateOnly);
    EXPECT_EQ (ECOBJECTS_STATUS_DataTypeMismatch, stat) << "IECInstance::SetValue> DateTimeInfo (Utc)  - ECValue::SetDateTimeTicks (ticks, Date): Expected to mismatch with the DateTimeInfo custom attribute.";

    stat = instance->SetValue ("dateonly", ticksWithUtc);
    EXPECT_EQ (ECOBJECTS_STATUS_DataTypeMismatch, stat) << "IECInstance::SetValue> DateTimeInfo (Date)  - ECValue::SetDateTimeTicks (ticks, Utc): Expected to mismatch with the DateTimeInfo custom attribute.";

    stat = instance->SetValue ("dateonly", ticksWithDateOnly);
    EXPECT_EQ (ECOBJECTS_STATUS_Success, stat) << "IECInstance::SetValue> DateTimeInfo (Date)  - ECValue::SetDateTimeTicks (ticks, Date): Expected to match with the DateTimeInfo custom attribute.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceGetSetDateTimeTestFixture, SetDateTimeTicksGetAsDateTime)
    {
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema (context);

    ECClassP testClass = testSchema->GetClassP ("TestClass");
    ASSERT_TRUE (testClass != NULL);
    Utf8CP propertyName = "nodatetimeinfo";

    //test 1: Original date is UTC
    DateTime expectedDate = DateTime::GetCurrentTimeUtc ();
    int64_t expectedTicks = 0LL;
    ASSERT_EQ (SUCCESS, expectedDate.ToCommonEraTicks (expectedTicks));

    ECValue v;
    BentleyStatus stat = v.SetDateTimeTicks (expectedTicks);
    EXPECT_EQ (SUCCESS, stat);
    
    DateTime actualDate = v.GetDateTime ();
    EXPECT_EQ ((int) DateTime::Kind::Unspecified, (int) actualDate.GetInfo ().GetKind ());
    AssertDateTime (expectedDate, actualDate, true);

    IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ECObjectsStatus ecstat = instance->SetValue (propertyName, v);
    EXPECT_EQ (ECOBJECTS_STATUS_Success, ecstat);

    ECValue actualValue;
    ecstat = instance->GetValue (actualValue, propertyName);
    EXPECT_EQ (ECOBJECTS_STATUS_Success, ecstat);

    actualDate = actualValue.GetDateTime ();
    EXPECT_EQ ((int)DateTime::Kind::Unspecified, (int) actualDate.GetInfo ().GetKind ());
    AssertDateTime (expectedDate, actualDate, true);

    //test 2: Original date is Unspecified
    expectedDate = DateTime (DateTime::Kind::Unspecified, 2013, 2, 2, 13, 14);
    expectedTicks = 0LL;
    ASSERT_EQ (SUCCESS, expectedDate.ToCommonEraTicks (expectedTicks));

    v.Clear ();
    stat = v.SetDateTimeTicks (expectedTicks);
    EXPECT_EQ (SUCCESS, stat);

    actualDate = v.GetDateTime ();
    EXPECT_EQ ((int) DateTime::Kind::Unspecified, (int) actualDate.GetInfo ().GetKind ());
    AssertDateTime (expectedDate, actualDate, false);

    instance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ecstat = instance->SetValue (propertyName, v);
    EXPECT_EQ (ECOBJECTS_STATUS_Success, ecstat);

    actualValue.Clear ();
    ecstat = instance->GetValue (actualValue, propertyName);
    EXPECT_EQ (ECOBJECTS_STATUS_Success, ecstat);

    actualDate = actualValue.GetDateTime ();
    EXPECT_EQ ((int) DateTime::Kind::Unspecified, (int) actualDate.GetInfo ().GetKind ());
    AssertDateTime (expectedDate, actualDate, false);


    //test 3: Original date has component Date
    expectedDate = DateTime (2013, 2, 2);
    expectedTicks = 0LL;
    ASSERT_EQ (SUCCESS, expectedDate.ToCommonEraTicks (expectedTicks));

    v.Clear ();
    stat = v.SetDateTimeTicks (expectedTicks);
    EXPECT_EQ (SUCCESS, stat);

    actualDate = v.GetDateTime ();
    EXPECT_EQ ((int) DateTime::Kind::Unspecified, (int) actualDate.GetInfo ().GetKind ());
    AssertDateTime (expectedDate, actualDate, true);

    instance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ecstat = instance->SetValue (propertyName, v);
    EXPECT_EQ (ECOBJECTS_STATUS_Success, ecstat);

    actualValue.Clear ();
    ecstat = instance->GetValue (actualValue, propertyName);
    EXPECT_EQ (ECOBJECTS_STATUS_Success, ecstat);

    actualDate = actualValue.GetDateTime ();
    EXPECT_EQ ((int) DateTime::Kind::Unspecified, (int) actualDate.GetInfo ().GetKind ());
    AssertDateTime (expectedDate, actualDate, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceGetSetDateTimeTestFixture, SetDateTimeWithLocalDateTimeKind)
    {
    bvector<DateTime> testDateTimes;
    testDateTimes.push_back (DateTime (DateTime::Kind::Utc, 2013, 2, 18, 14, 22));
    testDateTimes.push_back (DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 14, 22));
    testDateTimes.push_back (DateTime (DateTime::Kind::Local, 2013, 2, 18, 14, 22));
    testDateTimes.push_back (DateTime (2013, 2, 18));

    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema (context);

    ECClassP testClass = testSchema->GetClassP ("TestClass");
    ASSERT_TRUE (testClass != NULL);
    Utf8CP localDateTimePropName = "local";

    DISABLE_ASSERTS

    for (DateTimeCR testDateTime: testDateTimes)
        {
        const bool isLocal = testDateTime.GetInfo ().GetKind () == DateTime::Kind::Local;
        ECValue value;
        const BentleyStatus expectedStat = isLocal ? ERROR : SUCCESS;
        EXPECT_EQ (expectedStat, value.SetDateTime (testDateTime)) << "Return value of ECValue::SetDateTime ('" << testDateTime.ToString ().c_str () << "')";
        int64_t ceTicks = 0LL;
        testDateTime.ToCommonEraTicks (ceTicks);
        EXPECT_EQ (expectedStat, value.SetDateTimeTicks (ceTicks, testDateTime.GetInfo ())) << "Return value of ECValue::SetDateTimeTicks () with '" << value.ToString ().c_str ();

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

    ECClassP testClass = testSchema->GetClassP ("TestClass");
    ASSERT_TRUE (testClass != NULL);

    IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ASSERT_TRUE (instance.IsValid ());

    DateTime expectedDateTime (DateTime::Kind::Utc, 2013, 2, 18, 14, 28, 34, 1234567);

    AssertGetValue (instance, "utc", expectedDateTime, true);
    AssertGetValue (instance, "nodatetimeinfo", expectedDateTime, false);
    AssertGetValue (instance, "emptydatetimeinfo", expectedDateTime, false);

    //Unspecified is the default kind, therefore any property that doesn't have a kind CA value (or an invalid value) return
    //a matching date time
    instance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ASSERT_TRUE (instance.IsValid ());

    expectedDateTime = DateTime (DateTime::Kind::Unspecified, 2013, 2, 18, 14, 28, 34, 1234567);

    AssertGetValue (instance, "unspecified", expectedDateTime, true);
    AssertGetValue (instance, "nodatetimeinfo", expectedDateTime, true);
    AssertGetValue (instance, "emptydatetimeinfo", expectedDateTime, true);

    //DateTime is the default component. Therefore any property that 
    //doesn't have a component CA value (or an invalid value) returns a mismatching date time
    instance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ASSERT_TRUE (instance.IsValid ());

    expectedDateTime = DateTime (2013, 2, 18);

    AssertGetValue (instance, "dateonly", expectedDateTime, true);
    AssertGetValue (instance, "nodatetimeinfo", expectedDateTime, false);
    AssertGetValue (instance, "emptydatetimeinfo", expectedDateTime, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceGetSetDateTimeTestFixture, DateTimeArrayRoundtrip)
    {
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema (context);

    ECClassP testClass = testSchema->GetClassP ("TestClass");
    ASSERT_TRUE (testClass != NULL);
    Utf8CP propertyName = "dateTimeArrayProp";
    IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ASSERT_TRUE (instance.IsValid ());

    const uint32_t expectedArraySize = 3;
    DateTime expected [expectedArraySize];
    expected[0] = DateTime::GetCurrentTimeUtc ();
    expected[1] = DateTime (DateTime::Kind::Utc, 2012, 1, 1, 13, 14);
    expected[2] = DateTime (DateTime::Kind::Utc, 2011, 1, 1, 13, 14);

    instance->AddArrayElements (propertyName, expectedArraySize);
    for (uint32_t i = 0; i < expectedArraySize; i++)
        {
        ECValue v (expected[i]);
        ECObjectsStatus stat = instance->SetValue (propertyName, v, i);
        EXPECT_EQ (ECOBJECTS_STATUS_Success, stat);
        }
    

    ECValue actualArrayInfoValue;
    ECObjectsStatus stat = instance->GetValue (actualArrayInfoValue, propertyName);
    EXPECT_EQ (ECOBJECTS_STATUS_Success, stat);
    EXPECT_TRUE (actualArrayInfoValue.IsArray ());
    ArrayInfo actualArrayInfo = actualArrayInfoValue.GetArrayInfo ();
    EXPECT_TRUE (PRIMITIVETYPE_DateTime == actualArrayInfo.GetElementPrimitiveType ());
    EXPECT_EQ (expectedArraySize, actualArrayInfo.GetCount ());

    for (uint32_t i = 0; i < expectedArraySize; i++)
        {
        ECValue v;
        ECObjectsStatus stat = instance->GetValue (v, propertyName, i);
        EXPECT_EQ (ECOBJECTS_STATUS_Success, stat);

        DateTime actual = v.GetDateTime ();
        AssertDateTime (expected[i], actual, false);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/13                               
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceGetSetDateTimeTestFixture, SetDateTimeArrayWithMismatchingArrayElements)
    {
    ECSchemaReadContextPtr context = NULL;
    ECSchemaPtr testSchema = CreateTestSchema (context);

    ECClassP testClass = testSchema->GetClassP ("TestClass");
    ASSERT_TRUE (testClass != NULL);
    Utf8CP propertyName = "dateTimeArrayProp";
    IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ASSERT_TRUE (instance.IsValid ());

    const uint32_t expectedArraySize = 3;
    DateTime expected [expectedArraySize];
    ECObjectsStatus expectedSuccess [expectedArraySize];
    expected[0] = DateTime::GetCurrentTimeUtc ();
    expectedSuccess[0] = ECOBJECTS_STATUS_Success;
    expected[1] = DateTime (DateTime::Kind::Unspecified, 2012, 1, 1, 13, 14);
    expectedSuccess[1] = ECOBJECTS_STATUS_DataTypeMismatch;
    expected[2] = DateTime (2011, 1, 1);
    expectedSuccess[2] = ECOBJECTS_STATUS_DataTypeMismatch;
    
    instance->AddArrayElements (propertyName, expectedArraySize);
    for (uint32_t i = 0; i < expectedArraySize; i++)
        {
        ECValue v (expected[i]);
        ECObjectsStatus stat = instance->SetValue (propertyName, v, i);
        EXPECT_EQ (expectedSuccess[i], stat);
        }

    //retrieving the date time from the one array element that was inserted successfully
    ECValue actualArrayInfoValue;
    ECObjectsStatus stat = instance->GetValue (actualArrayInfoValue, propertyName);
    EXPECT_EQ (ECOBJECTS_STATUS_Success, stat);
    EXPECT_TRUE (actualArrayInfoValue.IsArray ());
    ArrayInfo actualArrayInfo = actualArrayInfoValue.GetArrayInfo ();
    EXPECT_TRUE (PRIMITIVETYPE_DateTime == actualArrayInfo.GetElementPrimitiveType ());
    //array was initialized with the full size, although only one element was actually successfully inserted.
    EXPECT_EQ (expectedArraySize, actualArrayInfo.GetCount ());

    for (uint32_t i = 0; i < expectedArraySize; i++)
        {
        ECValue v;
        ECObjectsStatus stat = instance->GetValue (v, propertyName, i);
        EXPECT_EQ (ECOBJECTS_STATUS_Success, stat);

        if (expectedSuccess[i] == ECOBJECTS_STATUS_Success)
            {
            DateTime actual = v.GetDateTime ();
            AssertDateTime (expected[i], actual, false);
            }
        else
            {
            EXPECT_TRUE (v.IsDateTime () && v.IsNull ());
            }
        }
    }

END_BENTLEY_ECN_TEST_NAMESPACE
